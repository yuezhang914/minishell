#include "exec.h"
#include "minishell.h"
#include "parse.h"

/*
 * heredoc_fork_child（static）
 * 作用：子进程负责读 heredoc 内容并写到 pipefd[1]
 * 参数：
 * - redir：当前 heredoc 的 redir 节点（filename=delimiter，quoted 表示是否展开）
 * - shell：全局上下文（expand 变量可能用 env）
 * - pipefd：管道 [0]=读端 [1]=写端
 */
static void heredoc_fork_child(t_redir *redir, t_minishell *shell, int pipefd[2])
{
    int result;

    /* 设置 heredoc 专用信号行为 */
    setup_heredoc_signals();

    /* 子进程只写，所以关闭读端 */
    close(pipefd[0]);

    /* quoted==1：不展开；quoted==0：展开 */
    if (redir->quoted)
        result = heredoc_loop(pipefd[1], redir->filename, shell, 1);
    else
        result = heredoc_loop(pipefd[1], redir->filename, shell, 0);

    /* 如果 heredoc_loop < 0：一般是被 Ctrl-C 中断 */
    if (result < 0)
    {
        close(pipefd[1]);

        /* ms_child_exit：你执行模块里的“子进程安全退出”（会清理资源） */
        ms_child_exit(shell, shell->cur_t_ast, 130);
    }

    /* 正常结束：关闭写端并退出 */
    close(pipefd[1]);
    ms_child_exit(shell, shell->cur_t_ast, 0);
}

/*
 * wait_for_child（static）
 * 作用：父进程等待子进程结束（处理 EINTR）
 * EINTR：waitpid 被信号打断，需要重试
 */
static int wait_for_child(pid_t pid, int *status)
{
    while (waitpid(pid, status, 0) == -1)
    {
        if (errno != EINTR)
            return (-1);
    }
    return (0);
}

/*
 * handle_child_exit（static）
 * 作用：解析子进程退出原因，并决定 heredoc_fd 是否有效
 * - 正常退出且 code==0：把 pipefd[0] 保存到 redir->heredoc_fd
 * - 如果 code!=0 或被信号杀死：关闭 pipefd[0]，并标记 heredoc_fd=-1
 */
static int handle_child_exit(int status, int pipefd[2], t_redir *redir, t_minishell *shell)
{
    if (WIFEXITED(status))
    {
        int code = WEXITSTATUS(status);

        /* 子进程非 0：表示出错或中断 */
        if (code != 0)
        {
            close(pipefd[0]);
            redir->heredoc_fd = -1;
            shell->last_exit_status = code;
            return (-1);
        }
    }
    else if (WIFSIGNALED(status))
    {
        /* 子进程被信号杀死 */
        close(pipefd[0]);
        redir->heredoc_fd = -1;

        /* bash 风格：128 + 信号编号 */
        shell->last_exit_status = 128 + WTERMSIG(status);
        return (-1);
    }

    /* 走到这里表示 heredoc 正常：父进程保留读端 fd */
    redir->heredoc_fd = pipefd[0];
    return (0);
}

/*
 * heredoc_parent（static）
 * 作用：父进程逻辑：
 * - 关闭写端（只读）
 * - 等子进程结束
 * - 根据退出状态决定是否保留读端 fd
 */
static int heredoc_parent(pid_t pid, int pipefd[2], t_redir *redir, t_minishell *shell)
{
    int status;

    /* 父进程只读：关闭写端 */
    close(pipefd[1]);

    /* 等待子进程 */
    if (wait_for_child(pid, &status) < 0)
    {
        close(pipefd[0]);
        return (-1);
    }

    /* 根据退出状态处理 */
    return handle_child_exit(status, pipefd, redir, shell);
}

/*
 * handle_heredoc
 * 作用：对一个 heredoc redir 节点，创建管道+fork 子进程读取内容
 * 返回：
 *  0：成功（redir->heredoc_fd 可用）
 * -1：失败（中断/系统调用失败）
 */
int handle_heredoc(t_redir *redir, t_minishell *shell)
{
    int     pipefd[2];
    pid_t   pid;
    int     ret;
    t_sig   saved;

    /* 保存当前信号处理方式，避免 heredoc 改完影响主 shell */
    save_signals(&saved);

    /* pipe：pipefd[0]=读端 pipefd[1]=写端 */
    if (pipe(pipefd) < 0)
        return (restore_signals(&saved), -1);

    /* fork：子进程负责读 heredoc，父进程负责等并保存 fd */
    pid = fork();
    if (pid < 0)
        return (restore_signals(&saved), -1);

    if (pid == 0)
    {
        /* 子进程路径：写入 heredoc 内容，然后退出 */
        heredoc_fork_child(redir, shell, pipefd);
        return (0);
    }
    else
    {
        /* 父进程路径：等待并设置 redir->heredoc_fd */
        ret = heredoc_parent(pid, pipefd, redir, shell);
        restore_signals(&saved);
        return (ret);
    }
}
