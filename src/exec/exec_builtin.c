#include "exec.h"
#include "minishell.h"

/* close_keep_std：这里只是 int，不是 int*（它不需要改外面的变量） */
static void	close_keep_std(int fd)
{
	if (fd > STDERR_FILENO)
		close(fd);
}

/*
 * run_builtin_parent_logic
 * 作用：在父进程执行 builtin，同时支持重定向
 *
 * 关键步骤：
 * 1) save_std_fds：备份当前 stdin/stdout
 * 2) apply_redir_list：根据 node->redir 算出 new_in/new_out
 * 3) dup_in_out_or_close：把 stdin/stdout 临时接到 new_in/new_out
 * 4) exec_builtin：真正执行 builtin
 * 5) change_envp + refresh_paths：因为 builtin 可能改了 env（PATH 等）
 * 6) restore_std_fds：恢复 stdin/stdout，避免影响后续 prompt
 */
int	run_builtin_parent_logic(t_minishell *msh, t_ast *node, int in_fd, int out_fd)
{
	t_fd_save	save;
	int			new_in;
	int			new_out;
	int			ret;

	ret = 1;

	if (save_std_fds(&save) != 0)
		return (close_keep_std(in_fd), close_keep_std(out_fd), 1);

	new_in = in_fd;
	new_out = out_fd;

	/* 计算最终 in/out（可能把 new_out 改成文件 fd） */
	if (apply_redir_list(node->redir, &new_in, &new_out) < 0)
		return (restore_std_fds(&save), 1);

	/* 临时把父进程 stdin/stdout 接到 new_in/new_out */
	if (dup_in_out_or_close(new_in, new_out) < 0)
		return (restore_std_fds(&save), 1);

	/* 真正跑 builtin（会改 msh->env） */
	ret = exec_builtin(node, &msh->env, msh);

	/* 更新 envp/paths（实现文件这批没给，但逻辑很标准） */
	change_envp(msh->env, &msh->envp);
	exec_refresh_paths(msh);

	/* 恢复标准输入输出（关键！） */
	restore_std_fds(&save);

	return (ret);
}
