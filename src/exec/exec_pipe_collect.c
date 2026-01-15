#include "exec.h"
#include "minishell.h"

/*
 * pipe_resize
 * 作用：动态数组扩容（cap=0 时从 4 开始，否则翻倍）
 * arr 是 t_ast**，每个元素就是一个“命令节点”
 */
static int pipe_resize(t_ast ***arr, int len, int *cap)
{
	t_ast **new_arr;
	int new_cap;
	int i;

	new_cap = *cap;
	if (new_cap == 0)
		new_cap = 4;
	else
		new_cap = new_cap * 2;

	new_arr = (t_ast **)malloc(sizeof(**arr) * new_cap);
	if (new_arr == NULL)
		return (0);

	/* 把旧内容搬过去 */
	i = 0;
	while (i < len)
	{
		new_arr[i] = (*arr)[i];
		i = i + 1;
	}

	free(*arr);
	*arr = new_arr;
	*cap = new_cap;
	return (1);
}

/*
 * pipe_push
 * 作用：把一个命令节点塞进数组，不够大就扩容
 */
static int pipe_push(t_ast ***arr, int *len, int *cap, t_ast *node)
{
	if (*len >= *cap)
		if (!pipe_resize(arr, *len, cap))
			return (0);

	(*arr)[*len] = node;
	*len = *len + 1;
	return (1);
}

/*
 * pipe_collect_rec
 * 作用：递归摊平 PIPE 树：a|b|c -> [a,b,c]
 * 规则：
 * - 如果不是 PIPE 节点：它就是一个“命令段”，push 进数组
 * - 如果是 PIPE：递归左边，再递归右边
 */
static int pipe_collect_rec(t_ast *node, t_ast ***arr, int *len, int *cap)
{
	if (!node)
		return (1);
	if (node->type != NODE_PIPE)
		return (pipe_push(arr, len, cap, node));

	if (!pipe_collect_rec(node->left, arr, len, cap))
		return (0);
	if (!pipe_collect_rec(node->right, arr, len, cap))
		return (0);
	return (1);
}

/*
 * pipe_collect（对外接口）
 * 输出：
 * - out_arr：新 malloc 的数组（由调用者 free）
 * - out_n：数组长度
 */
int pipe_collect(t_ast *root, t_ast ***out_arr, int *out_n)
{
	t_ast **arr;
	int len;
	int cap;

	arr = NULL;
	len = 0;
	cap = 0;

	if (!pipe_collect_rec(root, &arr, &len, &cap))
		return (free(arr), 0);

	*out_arr = arr;
	*out_n = len;
	return (1);
}
