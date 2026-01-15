#include "exec.h"
#include "minishell.h"

/*
 * exec_pipe_child
 * 作用：管道中的子进程逻辑（第 i 段命令）
 * 参数：
 * - ctx：管道执行上下文（含 arr/pids/in_fd/out_fd/root）
 * - i：第几个命令
 * - out_fd：这一段命令的输出口（可能是 pfd[1] 或最终 out_fd）
 * - pfd：这一段新创建的管道（如果不是最后一段）
 *
 * 重点：为什么子进程要 close(pfd[0])？
 * - pfd[0] 是“读端”，是给下一段命令读的
 * - 当前子进程只负责写（往 pfd[1] 写），不需要读端
 * - 不关会导致 fd 泄漏（还可能影响 EOF 判断的复杂性）
 */
static void	exec_pipe_child(t_pipe_ctx *ctx, int i, int out_fd, int pfd[2])
{
	t_ast	*cmd;
	int		in_fd;

	cmd = ctx->arr[i];       /* 第 i 段命令节点 */
	in_fd = ctx->in_fd;      /* 这一段的输入口（上一段读端/初始 in_fd） */

	/* 子进程不需要“新管道的读端” */
	if (pfd[0] != -1)
		close(pfd[0]);

	/* 子进程不需要父进程用来 wait 的数组，释放掉（只是优化，不影响功能） */
	free(ctx->pids);
	free(ctx->arr);

	/* 真正执行：重定向 + dup2 + builtin/execve */
	child_exec_one(ctx->msh, cmd, in_fd, out_fd, ctx->root);
}

/*
 * pipe_step
 * 作用：启动 pipeline 的第 i 段
 * 流程：
 * 1) pipe_make：如果不是最后一段就创建 pipe，得到 child_out（写端）
 * 2) fork
 * 3) 子进程：exec_pipe_child
 * 4) 父进程：记录 pid + handle_parent_pipes（关 fd + 保存读端）
 */
static int	pipe_step(t_pipe_ctx *ctx, int i)
{
	int		pfd[2];
	int		child_out;
	pid_t	pid;

	/* 决定这一段的输出口：可能创建新 pipe */
	child_out = pipe_make(i, ctx->n, ctx->out_fd, pfd);
	if (child_out < 0)
		return (0);

	pid = fork();
	if (pid < 0)
	{
		/* fork 失败：刚创建的管道也要关掉，避免泄漏 */
		if (pfd[0] != -1)
			close(pfd[0]);
		if (pfd[1] != -1)
			close(pfd[1]);
		return (0);
	}

	if (pid == 0)
		exec_pipe_child(ctx, i, child_out, pfd);

	/* 父进程：记录 pid */
	ctx->pids[i] = pid;

	/* 父进程：关旧 in_fd + 关写端 pfd[1] + 保存读端 pfd[0] */
	handle_parent_pipes(ctx, pfd);

	return (1);
}

/*
 * pipe_run_all
 * 作用：循环启动 pipeline 所有命令
 * done：记录成功启动了多少个子进程（用于后面 wait）
 */
static int	pipe_run_all(t_pipe_ctx *ctx, int *done)
{
	int	i;
	int	ok;

	i = 0;
	ok = 1;
	while (i < ctx->n && ok)
	{
		ok = pipe_step(ctx, i);
		i++;
	}
	*done = i;
	return (ok);
}

/*
 * exec_pipe_node
 * 作用：执行 PIPE 节点（整条管道）
 * 参数：
 * - in_fd/out_fd：整条管道的外部输入/输出环境（通常是 0/1）
 *
 * 关键点：
 * - pipe_run_all 结束后，父进程可能还握着“最后一次保存的 ctx.in_fd（某个读端）”
 *   父进程不需要它，必须 close，否则泄漏
 */
int	exec_pipe_node(t_minishell *msh, t_ast *node, int in_fd, int out_fd)
{
	t_pipe_ctx	ctx;
	int			done;
	int			ok;

	ft_bzero(&ctx, sizeof(ctx));
	done = 0;

	ok = pipe_collect(node, &ctx.arr, &ctx.n);
	if (!ok)
		return (0);

	ctx.pids = (pid_t *)malloc(sizeof(*ctx.pids) * ctx.n);
	if (!ctx.pids)
		return (free(ctx.arr), 0);

	ctx.msh = msh;
	ctx.root = node;
	ctx.in_fd = in_fd;     /* 初始输入口（通常 0） */
	ctx.out_fd = out_fd;   /* 最终输出口（通常 1） */

	ok = pipe_run_all(&ctx, &done);

	/* 父进程最后如果还留着读端（>2），关掉 */
	if (ctx.in_fd > STDERR_FILENO)
		close(ctx.in_fd);

	wait_all_and_set_lt_ast(msh, ctx.pids, done);

	if (!ok)
		msh->last_exit_status = 1;

	return (free(ctx.pids), free(ctx.arr), msh->last_exit_status);
}
