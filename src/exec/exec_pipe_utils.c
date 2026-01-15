#include "exec.h"
#include "minishell.h"

/*
 * wait_all_and_set_lt_ast
 * 作用：等待 pipeline 里所有子进程结束
 * 规则：
 * - 整条管道的退出码 = 最后一个命令的退出码（bash 规则）
 *
 * 这里还模拟 bash 的 Broken pipe：
 * - 如果（非最后一个）进程被 SIGPIPE 杀死，打印一次 "Broken pipe"
 *
 * 宏解释：
 * - WIFSIGNALED(st)：是否被信号杀死
 * - WTERMSIG(st)：具体哪个信号（SIGPIPE 等）
 */
void	wait_all_and_set_lt_ast(t_minishell *msh, pid_t *pids, int n)
{
	int		i;
	int		st;
	int		saw_pipe;
	pid_t	pid;

	i = 0;
	saw_pipe = 0;
	while (i < n)
	{
		pid = waitpid(pids[i], &st, 0);

		/* 不是最后一个命令，且第一次看到 SIGPIPE：提示 Broken pipe */
		if (pid > 0 && pid != pids[n - 1] && saw_pipe == 0)
		{
			if (WIFSIGNALED(st) && WTERMSIG(st) == SIGPIPE)
			{
				write(2, "Broken pipe\n", 12);
				saw_pipe = 1;
			}
		}

		/* 最后一个命令的状态：决定整条 pipeline 的退出码 */
		if (pid == pids[n - 1])
			set_status_from_wait(msh, st);

		i++;
	}
}

/*
 * pipe_make
 * 作用：决定“当前第 i 段命令”的输出去哪
 * - 如果 i 不是最后一段：
 *   1) 建立新管道 pipe(pfd)
 *   2) 当前段的 stdout 应该写到 pfd[1]（写端）
 * - 如果 i 是最后一段：
 *   stdout 直接写到 out_fd（可能是屏幕，也可能是文件）
 *
 * pfd 初始化为 -1 的好处：
 * - 后面 close 时可以判断 != -1 再关，避免误关
 */
int	pipe_make(int i, int n, int out_fd, int pfd[2])
{
	int	child_out;

	pfd[0] = -1;
	pfd[1] = -1;

	if (i < n - 1)
	{
		if (pipe(pfd) < 0)
			return (-1);

		/* 当前命令写入写端，下一段命令从读端读 */
		child_out = pfd[1];
	}
	else
	{
		/* 最后一段不用新管道，直接写最终 out_fd */
		child_out = out_fd;
	}
	return (child_out);
}

/*
 * handle_parent_pipes   （管道 fd 关闭的“核心口诀实现”）
 * 作用：父进程在 fork 之后必须做的事：
 *
 * 1) close(ctx->in_fd)
 *    - ctx->in_fd 是“刚刚给当前孩子当 stdin 的输入口”
 *    - 父进程不需要再读它了，留着就是泄漏
 *
 * 2) close(pfd[1])  【最关键】
 *    - pfd[1] 是“新管道的写端”
 *    - 父进程绝对不能拿着写端不关
 *    - 否则：下游读端永远等不到 EOF → pipeline 卡死
 *
 * 3) ctx->in_fd = pfd[0]
 *    - 把读端保存下来，给下一段命令当 stdin
 */
void	handle_parent_pipes(t_pipe_ctx *ctx, int pfd[2])
{
	if (ctx->in_fd > 2)
		close(ctx->in_fd);   /* 用完旧输入口就关 */

	if (pfd[1] != -1)
		close(pfd[1]);       /* 父进程关写端：避免 EOF 卡死 */

	ctx->in_fd = pfd[0];     /* 保存读端给下一段 */
}
