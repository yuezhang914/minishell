#include "lexer.h"
#include "minishell.h"

/*
** 函数作用：
** 把 word token 里临时 info 的内容“搬进”新节点里（不 strdup）
**
** 参数含义：
** new：新建的 lexer 节点
** info：word token 的信息包（clean/raw/had_quotes 等）
**
** 重要：这一步决定“字符串所有权转移”
** 成功入链后：node->str/node->raw 由节点负责 free。
*/
static void	init_node_info(t_lexer *new, t_token_info *info)
{
	new->str = NULL;
	new->raw = NULL;
	new->had_quotes = 0;
	new->quoted_by = 0;
	if (!info)
		return ;
	new->str = info->clean;          /* 直接接管指针（不复制） */
	new->raw = info->raw;            /* 直接接管指针（不复制） */
	new->had_quotes = info->had_quotes;

	/* quoted_by 用 bit 思路累加：单引号 +1，双引号 +2 */
	if (info->quoted_single)
		new->quoted_by = new->quoted_by + 1;
	if (info->quoted_double)
		new->quoted_by = new->quoted_by + 2;
}

/*
** 作用：给新节点起个名字。
** 如果没有提供 info（符号 token），就根据类型自动填上符号字符串。
**
** 返回：成功 1，失败 0
*/
static int	assign_node_name(t_lexer *new, tok_type type)
{
	const char	*symbol;

	symbol = token_symbol(type);     /* 例如 TOK_PIPE -> "|" */
	if (!symbol)
		return (1);                  /* 没映射也不算错（可能不会走到这里） */
	new->raw = ft_strdup(symbol);    /* raw/str 都 strdup 一份 */
	if (!new->raw)
		return (0);
	new->str = ft_strdup(symbol);
	if (!new->str)
	{
		free(new->raw);              /* 避免 raw 泄漏 */
		return (0);
	}
	return (1);
}

/*
** 作用：创建一个新的“零件”（节点）。
** 准备好它的位置、编号和类型，方便后面串成链表。
**
** 参数：
** info：WORD 的信息包；若为 NULL 表示创建符号 token
** type：TOK_WORD / TOK_PIPE / ...
**
** 关键点：
** - static count：用于生成 idx（创建顺序编号）
** - WORD：直接接管 info 指针
** - 符号 token：这里 strdup 符号字符串
*/
t_lexer	*new_node(t_token_info *info, tok_type type)
{
	t_lexer		*new;
	static int	count = 0;            /* 全局递增的节点编号 */

	new = malloc(sizeof(*new));
	if (!new)
		return (NULL);
	ft_bzero(new, sizeof(*new));     /* 把结构体清零，避免野值 */

	init_node_info(new, info);       /* 如果 info!=NULL：接管 clean/raw */

	if (info == NULL)                /* info==NULL 表示符号 token */
	{
		if (!assign_node_name(new, type))
			return (free(new), NULL);
	}
	new->tokentype = type;           /* 记录 token 类型 */
	new->idx = count++;              /* 生成并递增 idx */
	new->next = NULL;
	new->prev = NULL;
	return (new);
}

/*
** 函数作用：
** 把一个节点加到双向链表末尾
**
** 参数含义：
** lst：链表头指针地址
** new：要追加的新节点
*/
void	list_add_back(t_lexer **lst, t_lexer *new)
{
	t_lexer	*lt_ast;

	if (!lst || !new)
		return ;
	if (!*lst)                       /* 空链表：new 直接成为头 */
	{
		*lst = new;
		return ;
	}
	lt_ast = *lst;
	while (lt_ast->next)             /* 走到尾部 */
		lt_ast = lt_ast->next;
	lt_ast->next = new;              /* 尾插 */
	new->prev = lt_ast;              /* 维护双向链表 prev */
}

/*
** 函数作用：
** 统一“创建节点并加入链表”的入口
**
** 参数含义：
** info：WORD 信息包；符号 token 时传 NULL
** tokentype：节点类型
** list：链表头指针地址
**
** 返回值：
** 成功返回 1，失败返回 0
*/
int	add_node(t_token_info *info, tok_type tokentype, t_lexer **list)
{
	t_lexer	*node;

	node = NULL;
	node = new_node(info, tokentype); /* 创建节点（可能接管指针或 strdup） */
	if (!node)
		return (0);
	list_add_back(list, node);        /* 插入链表尾部 */
	return (1);
}
