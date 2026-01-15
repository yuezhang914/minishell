#include "minishell.h"
#include "exec.h"

/*
 * prepare_heredocs
 * 作用：递归遍历 AST，遇到 HEREDOC 重定向就调用 handle_heredoc 读输入
 *
 * 为什么必须“先预处理”？
 * - 例如：cat << EOF | grep a
 * - bash 会先让你把 EOF 内容输入完，再去创建管道、fork 两个子进程
 */
int prepare_heredocs(t_ast *node, t_minishell *ms)
{
	t_redir *r;

	if (!node)
		return (1);

	/* 处理当前节点自己的 redir 链表 */
	r = node->redir;
	while (r)
	{
		if (r->type == HEREDOC)
		{
			/* handle_heredoc 在 parse 模块里：fork 子进程读输入并保存 heredoc_fd */
			if (handle_heredoc(r, ms) == -1)
				return (0);
		}
		r = r->next;
	}

	/* 递归：左子树 / 右子树（管道两侧） */
	if (!prepare_heredocs(node->left, ms))
		return (0);
	if (!prepare_heredocs(node->right, ms))
		return (0);

	return (1);
}
