// expan_scan2.c：整串扫描（引号状态机 + 逐字符拼接）
// 这个文件是 expander 的“发动机”：
// 逐个字符走字符串：
// 看到 ' 或 "：更新引号状态
// 看到 $：交给 scan_expand_one
// 其他字符：原样追加到输出

#include "expander.h"
#include "minishell.h"

/* 本文件用到的库函数：
 * - ft_strdup 来自 libft
 * - str_join_free / scan_expand_one 来自你自己的 expander
 */

/* 切换引号状态：
 * - 遇到单引号：如果当前不在双引号里，就进入/退出单引号
 * - 遇到双引号：如果当前不在单引号里，就进入/退出双引号
 *
 * 这正是 shell 的规则：单引号不会嵌在双引号里改变状态，反过来也一样。
 */
static void	toggle_quote_state(const char c, enum qstate *q)
{
	if (c == '\'' && *q != Q_DQ)
		*q = (*q == Q_SQ) ? Q_NONE : Q_SQ;
	else if (c == '\"' && *q != Q_SQ)
		*q = (*q == Q_DQ) ? Q_NONE : Q_DQ;
}

/* 把一个普通字符追加到 out
 * 这里用一个 buf[2] 把 char 变成字符串 "x"
 */
static int	append_char(const char c, char **out)
{
	char	buf[2];

	buf[0] = c;
	buf[1] = '\0';

	*out = str_join_free(*out, buf);

	return (1); /* 消耗 1 个字符 */
}

/* expand_all：对整串做 $ 展开（注意：此函数不负责删除引号）
 * 参数：
 * - minishell：提供 env / $?
 * - str：输入字符串
 * 返回：展开后的新字符串（malloc），失败返回 NULL
 */
char	*expand_all(t_minishell *minishell, const char *str)
{
	int			i;      /* 扫描位置 */
	char		*out;   /* 输出字符串（不断增长） */
	enum qstate	q;      /* 引号状态 */
	t_exp_data	data;   /* 扫描需要用到的上下文 */

	i = 0;
	out = NULL;
	q = Q_NONE;

	if (!str)
		return (NULL);

	/* out 先从空字符串开始，方便后面不断拼接 */
	out = ft_strdup("");
	if (!out)
		return (NULL);

	/* 填好上下文包 */
	data.minishell = minishell;
	data.out = &out;

	/* 逐字符扫描 */
	while (str[i])
	{
		/* 先根据当前字符更新引号状态（影响 $ 是否展开） */
		toggle_quote_state(str[i], &q);

		if (str[i] == '$')
		{
			/* 扩展一次 $...，并让 i 前进“消耗的字符数” */
			i = i + scan_expand_one(&data, str, i, q);
		}
		else
		{
			/* 普通字符：直接追加 */
			i = i + append_char(str[i], &out);
		}
	}

	return (out);
}
