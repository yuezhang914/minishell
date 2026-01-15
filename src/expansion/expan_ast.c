// expan_ast.c：遍历整棵 AST（递归）
// 这个文件很简单：像“走迷宫”，把树的每个 CMD 节点都展开。

#include "minishell.h"
#include "expander.h"

/* 递归走 AST：
 * - node 是 NULL：说明走到尽头，返回成功
 * - node 是 CMD：展开 argv/redir
 * - 继续递归 left/right/sub
 */
static int walk_t_ast(t_minishell *msh, t_ast *node)
{
	if (!node)
		return (1);

	/* 只在 CMD 节点做展开 */
	if (node->type == NODE_CMD)
	{
		if (!expander_expand_cmd_node(msh, node))
			return (0);
	}

	/* PIPE 左边 */
	if (node->left && !walk_t_ast(msh, node->left))
		return (0);

	/* PIPE 右边 */
	if (node->right && !walk_t_ast(msh, node->right))
		return (0);

	/* SUBSHELL 里面那棵树 */
	if (node->sub && !walk_t_ast(msh, node->sub))
		return (0);

	return (1);
}

/* expander 总入口：parse 完成后调用 */
int expander_t_ast(t_minishell *minishell, t_ast *root)
{
	if (!minishell)
		return (0);
	return (walk_t_ast(minishell, root));
}
