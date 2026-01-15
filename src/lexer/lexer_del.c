#include "minishell.h"
#include "lexer.h"

/*
** 作用：删除链表头结点（并释放资源）。
**
** 参数：
** lst：链表头指针地址
**
** 逻辑：
** tmp=*lst; *lst=tmp->next; clear_one(&tmp);
** 最后把新头的 prev 置 NULL，保证双向链表正确。
*/
void	del_first(t_lexer **lst)
{
	t_lexer	*tmp;

	if (!lst || !*lst)
		return ;
	tmp = *lst;
	*lst = tmp->next;        /* 头指针后移 */

	clear_one(&tmp);         /* 释放旧头 */

	if (*lst)
		(*lst)->prev = NULL; /* 新头没有前驱 */
}

/*
** 作用：删除索引为 target 的中间节点。
**
** 参数：
** start：从哪个节点开始找（一般是 head）
** target：要删除的 idx
**
** 循环条件：
** while(node && node->idx != target) 一直找，找不到就返回
*/
static void	remove_target(t_lexer *start, int target)
{
	t_lexer	*prev;
	t_lexer	*node;

	prev = start;
	node = start->next;

	while (node && node->idx != target)
	{
		prev = node;
		node = node->next;
	}
	if (!node)
		return ;

	/* 从链表中摘掉 node */
	prev->next = node->next;
	if (node->next)
		node->next->prev = prev;

	clear_one(&node);        /* 释放目标节点 */
}

/*
** 作用：删除链表中指定 idx 的节点。
**
** 参数：
** lst：链表头指针地址
** target：目标 idx
**
** 逻辑：
** 如果头就是目标 → del_first
** 否则 → remove_target
*/
void	del_one(t_lexer **lst, int target)
{
	t_lexer	*start;

	if (!lst || !*lst)
		return ;
	start = *lst;
	if (start->idx == target)
	{
		del_first(lst);
		return ;
	}
	remove_target(start, target);
}
