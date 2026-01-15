#include "lexer.h"
#include "minishell.h"

/*
** 作用：释放一个词法节点内部动态资源（raw/str）。
** 重点：raw 和 str 可能指向同一块内存（raw == str），要避免 double free。
*/
static void	free_lexer_content(t_lexer *node)
{
	if (!node)
		return ;

	/* 情况1：raw 和 str 是同一个指针，只 free 一次 */
	if (node->raw && node->raw == node->str)
	{
		free(node->raw);
		node->raw = NULL;
		node->str = NULL;
	}
	else
	{
		/* 情况2：raw 和 str 是两块独立内存，分别 free */
		if (node->raw)
		{
			free(node->raw);
			node->raw = NULL;
		}
		if (node->str)
		{
			free(node->str);
			node->str = NULL;
		}
	}
}

/*
** 作用：释放并删除当前指向的单个词法节点（含其内部动态内存），将头指针置为 NULL。
**
** 参数：
** lst：指向“要释放的节点指针”的地址（通常传入 &node 或 &head）
**
** 返回：
** 总是返回 NULL（你的接口风格：方便链式写法/统一空返回）
*/
t_lexer	*clear_one(t_lexer **lst)
{
	t_lexer	*node;

	if (!lst || !*lst)
		return (NULL);
	node = *lst;

	free_lexer_content(node); /* 先释放 raw/str */

	/* 断开链表关系，避免外部误用 */
	node->next = NULL;
	node->prev = NULL;

	free(node);               /* 再释放节点本体 */
	*lst = NULL;              /* 把外部指针清空，避免悬垂指针 */
	return (NULL);
}

/*
** 作用：清空并释放整个词法链表。
**
** 参数：
** lst：链表头指针地址
**
** 逻辑：
** while 逐个释放：先缓存 next，再 clear_one 当前，再前进。
*/
void	clear_list(t_lexer **lst)
{
	t_lexer	*tmp;

	if (!lst || !*lst)
		return ;
	while (*lst)
	{
		tmp = (*lst)->next;   /* 先记住下一个，否则 clear_one 后就没法走了 */
		clear_one(lst);       /* 释放当前节点（会把 *lst 置 NULL） */
		*lst = tmp;           /* 前进到下一个节点 */
	}
}
