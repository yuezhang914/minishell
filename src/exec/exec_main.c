#include "exec.h"
#include "minishell.h"

/*
 * ms_child_exit
 * 作用：子进程退出前的“统一清理出口”
 * 参数：
 * - msh：minishell 全局状态（里面有 env、envp、paths 等）
 * - root：整棵 AST（子进程不再需要，直接释放）
 * - code：子进程要 exit 的退出码
 *
 * 为什么子进程要自己清理？
 * - 子进程 fork 后复制了父进程的内存，如果不清理也能退出，但会影响：
 *   1) 42/valgrind 评测会看子进程泄漏
 *   2) heredoc fd / 其它资源可能“遗留在 execve 前”
 */
void	ms_child_exit(t_minishell *msh, t_ast *root, int code)
{
	if (root)
		free_ast(root);   /* 释放语法树 */
	ms_clear(msh);       /* 清理 msh 的各种资源（envp/paths/临时缓存等） */
	exit(code);          /* 真正结束子进程 */
}

/*
 * exec_cmd_root
 * 作用：执行“单条命令”（不在管道里）
 * 相当于：stdin=终端(0)，stdout=终端(1)
 */
int	exec_cmd_root(t_minishell *msh, t_ast *node)
{
	int	ret;

	ret = exec_cmd_node(msh, node, STDIN_FILENO, STDOUT_FILENO);
	return (ret);
}

/*
 * exec_pipe_root
 * 作用：执行“整条管道”
 * 相当于：整条 pipeline 的起点 stdin=0，终点 stdout=1
 */
int	exec_pipe_root(t_minishell *msh, t_ast *node)
{
	int	ret;

	ret = exec_pipe_node(msh, node, STDIN_FILENO, STDOUT_FILENO);
	return (ret);
}

/*
 * exec_t_ast
 * 作用：执行阶段总入口
 * 关键要求：
 * - msh->last_exit_status 必须等于“最后执行的命令退出码”
 *   这样 $? 才正确，exit 才正确
 *
 * 这里还做了信号切换：
 * - setup_parent_exec_signals：执行时（等待子进程时）父进程的信号策略
 * - setup_prompt_signals：回到交互提示符时的信号策略
 */
int	exec_t_ast(t_minishell *msh, t_ast *root)
{
	int	status;

	status = 0;
	if (!msh || !root)
		return (0);

	setup_parent_exec_signals();       /* 进入“执行态”的信号 */

	if (root->type == NODE_PIPE)       /* 根是管道：走 pipe 执行 */
		status = exec_pipe_root(msh, root);
	else if (root->type == NODE_CMD)   /* 根是命令：走 cmd 执行 */
		status = exec_cmd_root(msh, root);

	msh->last_exit_status = status;    /* 保存最后退出码（用于 $?） */

	setup_prompt_signals();            /* 回到“提示符态”的信号 */
	return (status);
}
