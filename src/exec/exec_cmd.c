#include "exec.h"
#include "minishell.h"

/*
 * run_redir_only_parent
 * 场景：只有重定向，没有真正命令
 * 例如："> a"、"< infile"、">> out"
 *
 * 要点：
 * 1) 即使没有命令，也要“应用重定向”，因为这会创建文件/检查权限并可能报错
 * 2) 成功：返回 0
 *    失败：返回 1
 * 3) 注意关闭 fd：避免父进程泄漏（尤其是管道里传进来的 in_fd/out_fd）
 */
static int	run_redir_only_parent(t_minishell *msh, t_ast *node, int in_fd,
		int out_fd)
{
	int	new_in;
	int	new_out;
	int	ret;

	new_in = STDIN_FILENO;     /* 默认输入来自终端 */
	new_out = STDOUT_FILENO;   /* 默认输出到终端 */
	ret = 0;

	/* 尝试应用 redir：失败就 ret=1（并且 error 模块已打印原因） */
	if (apply_redir_list(node->redir, &new_in, &new_out) < 0)
		ret = 1;

	/* new_in/new_out 可能是 open 出来的文件 fd，用完就关 */
	if (new_in > STDERR_FILENO)
		close(new_in);
	if (new_out > STDERR_FILENO)
		close(new_out);

	/* 传进来的 in_fd/out_fd 若是额外 fd，也要关掉避免泄漏 */
	if (in_fd > STDERR_FILENO)
		close(in_fd);
	if (out_fd > STDERR_FILENO)
		close(out_fd);

	msh->last_exit_status = ret;
	return (ret);
}

/*
 * handle_parent_wait
 * 作用：父进程收尾：关 fd + waitpid + 更新 last_exit_status
 *
 * 重点：为什么父进程这里要 close(in_fd/out_fd)？
 * - 这些 fd（尤其是 pipe 的读端/写端）父进程不再需要
 * - 不关会造成：
 *   1) fd 泄漏（越跑越多）
 *   2) 更严重：父进程握着 pipe 写端不关 → 下游读不到 EOF → 卡死
 *
 * waitpid(..., WUNTRACED)：
 * - WUNTRACED 表示：孩子如果被暂停（比如 Ctrl-Z），waitpid 也会返回
 */
static int	handle_parent_wait(t_minishell *msh, pid_t pid, int in_fd,
		int out_fd)
{
	int	st;

	/* 父进程用完就关（>2 才关，0/1/2 不动） */
	if (in_fd > STDERR_FILENO)
		close(in_fd);
	if (out_fd > STDERR_FILENO)
		close(out_fd);

	st = 0;
	if (waitpid(pid, &st, WUNTRACED) > 0)
		set_status_from_wait(msh, st);  /* 把 wait 的状态转换成 shell 退出码 */

	setup_prompt_signals();            /* 等完孩子，恢复交互信号 */
	rl_on_new_line();                  /* readline：告诉它新的一行 */
	return (msh->last_exit_status);
}

/*
 * run_external_wait
 * 作用：执行“外部命令”（不是 builtin parent 的那类）
 *
 * 流程：
 * 1) setup_parent_exec_signals：父进程进入执行态信号
 * 2) fork
 *    - 失败：打印 perror，关 fd，返回 1
 *    - 子进程：setup_child_signals + child_exec_one
 *    - 父进程：handle_parent_wait
 */
static int	run_external_wait(t_minishell *msh, t_ast *node, int in_fd,
		int out_fd)
{
	pid_t	pid;

	setup_parent_exec_signals();
	pid = fork();
	if (pid < 0)
	{
		ms_perror("fork");

		/* fork 失败也要关 fd，否则父进程泄漏 */
		if (in_fd > STDERR_FILENO)
			close(in_fd);
		if (out_fd > STDERR_FILENO)
			close(out_fd);

		msh->last_exit_status = 1;
		return (1);
	}

	if (pid == 0)
	{
		setup_child_signals();                 /* 子进程信号策略 */
		child_exec_one(msh, node, in_fd, out_fd, node);
	}

	return (handle_parent_wait(msh, pid, in_fd, out_fd));
}

/*
 * is_builtin_parent
 * 判断：哪些 builtin 必须在父进程跑？
 * - cd：要改变父进程当前目录
 * - export/unset：要改变父进程 env
 * - exit：要结束父进程
 */
static int	is_builtin_parent(char *cmd)
{
	if (!cmd)
		return (0);
	if (ft_strncmp(cmd, "cd", 3) == 0)
		return (1);
	if (ft_strncmp(cmd, "export", 7) == 0)
		return (1);
	if (ft_strncmp(cmd, "unset", 6) == 0)
		return (1);
	if (ft_strncmp(cmd, "exit", 5) == 0)
		return (1);
	return (0);
}

/*
 * exec_cmd_node
 * 作用：执行一个 CMD 节点（单命令）
 * 参数：
 * - in_fd/out_fd：外层传进来的“输入口/输出口”
 *   - 单命令通常是 0/1
 *   - 管道里可能是 pipe 的读端/写端
 *
 * 逻辑顺序（非常像 bash）：
 * 1) heredoc 如果失败/被 Ctrl-C 中断：直接返回（不执行）
 * 2) 没 argv[0]：说明只有重定向，走 run_redir_only_parent
 * 3) 必须父进程跑的 builtin：走 run_builtin_parent_logic
 * 4) 外部命令：fork + 子进程执行 + 父进程 wait
 */
int	exec_cmd_node(t_minishell *msh, t_ast *node, int in_fd, int out_fd)
{
	int	ret;

	ret = 0;
	if (!node)
		return (0);

	/* heredoc 被打断：has_bad_heredoc 会为真，last_exit_status 通常是 130 */
	if (has_bad_heredoc(node->redir))
	{
		ret = msh->last_exit_status;
		return (ret);
	}

	/* 只有重定向没有命令：也要执行“重定向副作用” */
	if (!node->argv || !node->argv[0])
		return (run_redir_only_parent(msh, node, in_fd, out_fd));

	/* 必须在父进程执行的 builtin（cd/export/unset/exit） */
	if (is_builtin_parent(node->argv[0]))
	{
		ret = run_builtin_parent_logic(msh, node, in_fd, out_fd);
		msh->last_exit_status = ret;
		return (ret);
	}

	/* 外部命令依赖 PATH：确保 msh->paths 已准备 */
	ensure_paths_ready(msh);

	return (run_external_wait(msh, node, in_fd, out_fd));
}
