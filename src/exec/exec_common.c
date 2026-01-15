#include "exec.h"
#include "minishell.h"

/*
 * dup_in_out_or_close
 * 作用：
 * - 把 in_fd 接到 STDIN（0）
 * - 把 out_fd 接到 STDOUT（1）
 * - 如果 in_fd/out_fd 不是标准 fd，就在 dup2 后把旧 fd close 掉（防止 fd 泄漏）
 *
 * 为什么要 close？
 * - dup2 之后，STDIN/STDOUT 已经指向同一个“文件/管道口”
 * - 原来的 in_fd/out_fd 编号就没用了，不关会泄漏
 */
int	dup_in_out_or_close(int in_fd, int out_fd)
{
	/* 处理 stdin */
	if (in_fd >= 0 && in_fd != STDIN_FILENO)
	{
		/* dup2：让 STDIN_FILENO(0) 指向 in_fd 同一个来源 */
		if (dup2(in_fd, STDIN_FILENO) < 0)
		{
			/* dup2 失败：这时 in_fd/out_fd 都不要泄漏 */
			close(in_fd);
			if (out_fd >= 0 && out_fd != STDOUT_FILENO)
				close(out_fd); /* 你这里特意修复：避免 out_fd 泄漏 */
			return (-1);
		}
		/* dup2 成功：旧 in_fd 可以关掉 */
		close(in_fd);
	}

	/* 处理 stdout */
	if (out_fd >= 0 && out_fd != STDOUT_FILENO)
	{
		if (dup2(out_fd, STDOUT_FILENO) < 0)
		{
			close(out_fd);
			return (-1);
		}
		close(out_fd);
	}
	return (0);
}

/*
 * save_std_fds
 * 作用：备份当前 STDIN/STDOUT
 * 场景：builtin 在父进程执行，但需要临时重定向输出到文件
 * 做法：
 * - dup(0) 得到一个新的 fd 保存 stdin
 * - dup(1) 得到一个新的 fd 保存 stdout
 */
int	save_std_fds(t_fd_save *save)
{
	if (!save)
		return (1);
	save->in = dup(STDIN_FILENO);
	save->out = dup(STDOUT_FILENO);
	if (save->in < 0 || save->out < 0)
		return (1);
	return (0);
}

/*
 * restore_std_fds
 * 作用：恢复之前保存的 stdin/stdout
 * 为什么必须恢复？
 * - builtin 跑完后，你还要显示 prompt
 * - 如果不恢复，后续输出可能还写到文件里，shell 就“坏掉”了
 */
void	restore_std_fds(t_fd_save *save)
{
	if (!save)
		return ;
	if (save->in >= 0)
	{
		dup2(save->in, STDIN_FILENO);
		close(save->in);
	}
	if (save->out >= 0)
	{
		dup2(save->out, STDOUT_FILENO);
		close(save->out);
	}
}

/*
 * set_status_from_wait
 * 作用：把 waitpid 得到的 status 转成 shell 的 last_exit_status
 *
 * 规则（bash 同款）：
 * - 正常 exit(code)：shell 退出码 = code
 * - 被信号杀死(sig)：shell 退出码 = 128 + sig
 *
 * 你还模拟了 bash 的输出：
 * - SIGQUIT：打印 "Quit (core dumped)"
 * - SIGINT：打印换行
 */
void	set_status_from_wait(t_minishell *msh, int status)
{
	int	sig;

	if (WIFEXITED(status))
	{
		msh->last_exit_status = WEXITSTATUS(status);
	}
	else if (WIFSIGNALED(status))
	{
		sig = WTERMSIG(status);
		if (sig == SIGQUIT)
			write(1, "Quit (core dumped)\n", 19);
		else if (sig == SIGINT)
			write(1, "\n", 1);
		msh->last_exit_status = 128 + sig;
	}
	else if (WIFSTOPPED(status))
	{
		/* 很少见：子进程被暂停（Ctrl-Z 等） */
		write(1, "\n", 1);
		printf("[1]+  Stopped\n");
		msh->last_exit_status = 128 + WSTOPSIG(status);
	}
}

/*
 * wait_pair_set_right
 * 作用：等待一对管道子进程（常见于 a|b 的简化实现）
 * 规则：以“右边命令”的退出码作为整条管道退出码（bash 规则）
 */
int	wait_pair_set_right(t_minishell *msh, pid_t left, pid_t right)
{
	int	st;

	st = 0;
	if (left > 0)
		waitpid(left, NULL, 0);
	if (right > 0)
	{
		if (waitpid(right, &st, 0) > 0)
			set_status_from_wait(msh, st);
		else
			msh->last_exit_status = 1;
	}
	return (msh->last_exit_status);
}
