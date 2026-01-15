#include "lexer.h"
#include "minishell.h"

/*
** 作用：判断字符是不是空格、制表符等空白。
** (空格 或 ASCII 9~13：\t \n \v \f \r)
*/
int	check_space(char c)
{
	return (c == ' ' || (c >= 9 && c <= 13));
}

/*
** 作用：专门检查引号是否成对出现。
** 如果发现只有半个双引号或半个单引号，就记下来并报错。
**
** 参数：
** str：整行
** i：当前下标
** out_unclosed：输出：未闭合的是 ' 还是 "
**
** 返回：
** - 如果当前位置是成对引号段：返回该段长度
** - 如果发现未闭合：返回 LEX_UNCLOSED_QUOTE（并写 out_unclosed）
** - 如果当前位置不是引号：返回 0
*/
static int	verify_quote_status(char *str, int i, char *out_unclosed)
{
	int	q_len;

	/* 34 是 '"' */
	q_len = match_quotes(i, str, 34);
	if (q_len == -1)
	{
		if (out_unclosed)
			*out_unclosed = '"';
		return (LEX_UNCLOSED_QUOTE);
	}
	if (q_len > 0)
		return (q_len);

	/* 39 是 '\'' */
	q_len = match_quotes(i, str, 39);
	if (q_len == -1)
	{
		if (out_unclosed)
			*out_unclosed = '\'';
		return (LEX_UNCLOSED_QUOTE);
	}
	return (q_len); /* 0 或 正长度 */
}

/*
** 作用：数一数这个单词到底有多长。
** 会跳过成对的引号，直到遇到空格或特殊符号为止。
**
** 参数：
** str：整行
** start_i：单词起点
** out_unclosed：若中途遇到未闭合引号，写入具体引号类型
**
** 返回：
** - 正数：单词长度
** - LEX_UNCLOSED_QUOTE：未闭合
*/
static int	calc_word_len(char *str, int start_i, char *out_unclosed)
{
	int	j;
	int	q_len;

	j = 0;
	if (out_unclosed)
		*out_unclosed = '\0';

	/* 条件：没结束 且 还没遇到符号 token（| < >） */
	while (str[start_i + j] && !(is_token((unsigned char)str[start_i + j])))
	{
		/* 如果当前位置是引号段，q_len 是整段长度；不是引号则 q_len=0 */
		q_len = verify_quote_status(str, start_i + j, out_unclosed);
		if (q_len == LEX_UNCLOSED_QUOTE)
			return (LEX_UNCLOSED_QUOTE);

		j += q_len; /* 如果是引号段，一次性跳过整段 */

		/* 跳过后如果到行尾/空白，则单词结束 */
		if (!str[start_i + j] || check_space(str[start_i + j]))
			break ;

		/* 如果当前位置刚好是引号字符（理论上 q_len 会处理，但这里做保护） */
		if (str[start_i + j] == 34 || str[start_i + j] == 39)
			continue ;

		j++; /* 普通字符：单步推进 */
	}
	return (j);
}

/*
** 作用：把切好的单词“洗干净”：
** - 去掉外面的引号
** - 记录是否出现过单/双引号
**
** 参数：
** substr：原始截取片段（malloc 得到）
** info：输出信息包
**
** 返回：成功 1；失败 0（你这里实际只用 1/0）
*/
static int	process_word_data(char *substr, t_token_info *info)
{
	char	*clean;

	clean = NULL;
	if (!info)
		return (0);

	/* remove_quotes_flag：有引号 => 返回新串；没引号 => 返回 NULL */
	clean = remove_quotes_flag(substr,
			&info->had_quotes,
			&info->quoted_single,
			&info->quoted_double);

	info->raw = substr; /* raw 永远是 substr（含引号原样） */

	if (!clean)
	{
		/* 没有引号：clean 直接用 substr（raw==clean） */
		info->clean = substr;
		info->had_quotes = 0;
		info->quoted_single = 0;
		info->quoted_double = 0;
		return (1);
	}

	/* 有引号：clean 是新分配的去壳串（raw!=clean） */
	info->clean = clean;
	return (1);
}

/*
** 作用：这是处理普通单词的入口。
** 它负责算长度、截取字符串、处理引号，最后存进列表。
**
** 参数：
** str：整行输入
** i：起点
** list：lexer 链表
** out_unclosed：若未闭合引号，输出是哪一种引号
**
** 返回：
** - 正数：消耗字符数
** - LEX_UNCLOSED_QUOTE：未闭合
** - -1：内存失败等
*/
int	handle_word(char *str, int i, t_lexer **list, char *out_unclosed)
{
	int				j;
	char			*substr;
	t_token_info	info;

	substr = NULL;

	/* 先计算 word 长度（可能检测到未闭合引号） */
	j = calc_word_len(str, i, out_unclosed);
	if (j == LEX_UNCLOSED_QUOTE || j <= 0)
		return (j);

	/* 截取原始片段（含引号） */
	substr = ft_substr(str, i, j);
	if (!substr)
		return (-1);

	/* 生成 info：clean/raw + 引号标记 */
	process_word_data(substr, &info);

	/* 入链：成功后节点接管 info 内指针；失败要释放 */
	if (finalize_word_node(&info, list) < 0)
		return (-1);

	return (j);
}
