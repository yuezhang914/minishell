#include "lexer.h"
#include "minishell.h"

/*
** 函数作用：
** 给“符号 token”提供它应该显示的文本，比如 TOK_PIPE -> "|"
**
** 参数含义：
** t：token 类型
**
** 返回值：
** 返回对应的字符串常量；没有就返回 NULL
*/
const char	*token_symbol(tok_type t)
{
	/* 逐个映射 mandatory token */
	if (t == TOK_PIPE)
		return ("|");
	if (t == TOK_REDIR_OUT)
		return (">");
	if (t == TOK_REDIR_IN)
		return ("<");
	if (t == TOK_APPEND)
		return (">>");
	if (t == TOK_HEREDOC)
		return ("<<");
	return (NULL); /* 未覆盖的 token 类型返回 NULL */
}

/*
** 作用：当程序出错时，负责把刚才申请的内存全部还给电脑（释放内存）。
** 重点：raw 和 clean 可能是同一个指针，也可能是不同指针。
*/
static void	release_word_memory(t_token_info *info)
{
	/* raw 和 clean 都存在且不相同：说明两块内存都要 free */
	if (info->raw && info->clean && info->raw != info->clean)
	{
		free(info->raw);
		free(info->clean);
	}
	/* 只有 raw：free raw */
	else if (info->raw)
		free(info->raw);
	/* 只有 clean：free clean */
	else if (info->clean)
		free(info->clean);
}

/*
** 作用：完成最后一步，把准备好的单词信息正式塞进零件链表里。
** 参数：
** info：word 的临时信息包（里面指针会被节点“接管”）
** list：lexer 链表头指针地址
**
** 返回：
** 成功 1；失败 -1（并负责释放 info 里申请的内存，防止泄漏）
*/
int	finalize_word_node(t_token_info *info, t_lexer **list)
{
	/* add_node 成功后，节点接管 info->clean/info->raw 的释放责任 */
	if (!add_node(info, TOK_WORD, list))
	{
		/* 如果入链失败：info 里申请的内存没人接管，必须这里释放 */
		release_word_memory(info);
		return (-1);
	}
	return (1);
}
