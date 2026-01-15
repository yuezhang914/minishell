#include "exec.h"
#include "minishell.h"

/* path_is_dir：用 stat + S_ISDIR 判断路径是不是目录 */
int	path_is_dir(const char *path)
{
	struct stat	st;

	if (!path)
		return (0);
	if (stat(path, &st) != 0)
		return (0);
	if (S_ISDIR(st.st_mode))
		return (1);
	return (0);
}

/* has_bad_heredoc：如果有 heredoc_fd < 0，说明 heredoc 预处理失败/中断 */
int	has_bad_heredoc(t_redir *r)
{
	while (r)
	{
		if (r->type == HEREDOC && r->heredoc_fd < 0)
			return (1);
		r = r->next;
	}
	return (0);
}

/* 关闭一个命令节点下所有 heredoc_fd */
static void	close_heredoc_fds(t_redir *r)
{
	while (r)
	{
		if (r->type == HEREDOC && r->heredoc_fd >= 0)
		{
			close(r->heredoc_fd);
			r->heredoc_fd = -1;
		}
		r = r->next;
	}
}

/* close_all_heredoc_fds：递归整棵 AST 把 heredoc_fd 全关掉（避免 fd 泄漏/继承） */
void	close_all_heredoc_fds(t_ast *node)
{
	if (!node)
		return ;
	if (node->type == NODE_CMD)
		close_heredoc_fds(node->redir);
	if (node->sub)
		close_all_heredoc_fds(node->sub);
	if (node->left)
		close_all_heredoc_fds(node->left);
	if (node->right)
		close_all_heredoc_fds(node->right);
}
