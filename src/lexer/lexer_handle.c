#include "lexer.h"
#include "minishell.h"

/*
** 作用：跳过从某个位置开始的所有空格。
**
** 参数：
** str：整行
** i：起点
**
** 返回：
** 连续空白字符数量（用来 i += 返回值）
*/
int	skip_spaces(char *str, int i)
{
	int	j;

	j = 0;
	while (check_space(str[i + j]))  /* 循环条件：当前位置还是空白 */
		j++;
	return (j);
}

/*
** 作用：如果发现引号没写完，就记录状态并告诉系统需要更多输入。
**
** 参数：
** ms：minishell 上下文（记录“需要续行”的状态）
** quote：未闭合引号类型（' 或 "）
**
** 返回：
** LEX_NEED_MORE（2）：上层应进入续行读取逻辑
*/
static int	process_unclosed(t_minishell *ms, char quote)
{
	ms->lexer_unclosed_quote = quote; /* 记录是哪一种引号未闭合 */
	ms->lexer_need_more = 1;          /* 告诉上层：需要再读一行 */
	clear_list(&ms->lexer);           /* 清空本次 lexer 结果，避免脏数据 */
	return (LEX_NEED_MORE);
}

/*
** 作用：根据当前字符是特殊符号还是普通文字，选择不同的处理方式。
**
** 参数：
** ms：上下文（用 ms->raw_line, ms->lexer）
** i：当前位置下标
**
** 返回：
** - 正数：消耗了多少字符
** - 0 或负数：错误/特殊状态
*/
static int	process_segment(t_minishell *ms, int i)
{
	int		step;
	char	quote;

	quote = '\0'; /* 用来接收“未闭合的是哪种引号” */

	if (is_token((unsigned char)ms->raw_line[i]))
		step = handle_token(ms->raw_line, i, &ms->lexer);
	else
		step = handle_word(ms->raw_line, i, &ms->lexer, &quote);

	/* handle_word 检测到未闭合引号 */
	if (step == LEX_UNCLOSED_QUOTE)
		return (process_unclosed(ms, quote));

	/* step < 0 一般是 malloc 失败或 add_node 失败 */
	if (step < 0)
	{
		clear_list(&ms->lexer);
		return (0);
	}
	return (step);
}

/*
** 作用：这是切分命令的总入口，把一整行命令拆成一个一个的“零件”。
**
** 参数：
** ms：minishell 上下文（raw_line 输入；lexer 输出链表）
**
** 返回：
** 1：成功切分
** 0：错误
** LEX_NEED_MORE：需要续行（未闭合引号）
*/
int	handle_lexer(t_minishell *ms)
{
	int	i;
	int	step;

	if (!ms || !ms->raw_line)
		return (0);

	i = 0;
	ms->lexer_unclosed_quote = '\0'; /* 初始化：默认无未闭合引号 */
	ms->lexer_need_more = 0;         /* 初始化：默认不需要续行 */

	while (ms->raw_line[i])          /* 循环条件：还没到字符串结尾 */
	{
		/* detect_signal：你项目别处实现，通常处理 Ctrl-C 时清理 lexer 等 */
		if (detect_signal(&ms->lexer))
			return (0);

		i += skip_spaces(ms->raw_line, i); /* 先跳过空白 */
		if (ms->raw_line[i] == '\0')
			break ;

		step = process_segment(ms, i);     /* 处理一个 token/word */
		if (step <= 0)                     /* 0=错误，2=need_more（也 <=0? 这里是 >0 才继续） */
			return (step);

		i += step;                         /* 前进到下一个片段 */
	}
	return (1);
}
