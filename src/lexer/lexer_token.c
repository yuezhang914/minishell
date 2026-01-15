#include "lexer.h"
#include "minishell.h"

/*
** 函数作用：
** 判断一个字符是不是“需要单独当成符号 token”的字符。
** 这里只保留 minishell 必做的：| < >
**
** 参数：
** c：当前字符
**
** 返回：
** 对应 tok_type；不是符号返回 0
*/
tok_type	is_token(int c)
{
	if (c == '|')
		return (TOK_PIPE);
	if (c == '>')
		return (TOK_REDIR_OUT);
	if (c == '<')
		return (TOK_REDIR_IN);
	return (0);
}

/*
** 函数作用：
** 处理双字符符号：>> 和 <<
**
** 参数：
** tokentype：当前位置单字符 token（> 或 <）
** next_char：下一个字符
** list：token 链表头指针地址
**
** 返回：
** - 成功消费 2 个字符：返回 2
** - 不是双字符：返回 0
** - add_node 失败：返回 -1
*/
static int	handle_double_token(tok_type tokentype, int next_char, t_lexer **list)
{
	/* 当前是 > 且下一个也是 > => >> */
	if (tokentype == TOK_REDIR_OUT && is_token(next_char) == TOK_REDIR_OUT)
	{
		if (!add_node(NULL, TOK_APPEND, list))
			return (-1);
		return (2);
	}
	/* 当前是 < 且下一个也是 < => << */
	if (tokentype == TOK_REDIR_IN && is_token(next_char) == TOK_REDIR_IN)
	{
		if (!add_node(NULL, TOK_HEREDOC, list))
			return (-1);
		return (2);
	}
	return (0);
}

/*
** 函数作用：
** 处理一个符号 token（| < > 或 >> <<）并把它加入 token 链表。
**
** 参数：
** str：整行输入
** i：当前位置下标
** list：链表头指针地址
**
** 返回：
** - 消耗字符数：1 或 2
** - 出错：-1
** - 当前位置不是符号：0
*/
int	handle_token(char *str, int i, t_lexer **list)
{
	tok_type	token;
	int			next_char;
	int			res;

	token = is_token((unsigned char)str[i]); /* 当前字符的单字符 token */
	next_char = (unsigned char)str[i + 1];   /* 预读下一个字符 */

	res = handle_double_token(token, next_char, list);
	if (res != 0)
		return (res);                        /* 2 或 -1 */

	if (token != 0)                          /* 单字符 token */
	{
		if (!add_node(NULL, token, list))    /* 注意：符号 token 传 NULL */
			return (-1);
		return (1);
	}
	return (0);
}

/*
** 函数作用：
** 从 i 开始匹配一段完整引号（'...' 或 "..."），返回这段长度。
**
** 参数：
** i：起点下标
** str：原字符串
** quote：引号字符（' 或 "）
**
** 返回：
** - 找到闭合引号：返回总长度（包含首尾引号）
** - 未闭合：返回 -1
** - 当前位置不是该引号：返回 0
*/
int	match_quotes(int i, char *str, char quote)
{
	int	j;

	j = 0;
	if (str[i + j] == quote)
	{
		j = j + 1;                           /* 跳过开引号 */
		while (str[i + j] != quote && str[i + j])
			j = j + 1;                       /* 找闭引号 */
		if (str[i + j] == quote)
			j = j + 1;                       /* 把闭引号也算进长度 */
		else
			return (-1);                     /* 没找到闭合 => 未闭合 */
	}
	return (j);                              /* 0 表示当前位置不是该 quote */
}
