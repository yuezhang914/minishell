#include "lexer.h"
#include "minishell.h"

/*
** 作用：从源串 s 的当前读指针起，拷贝“被指定引号包裹”的内容到 res，
** 并维护源/目标索引 idx（含 i/j）。
**
** 参数：
** s：源串
** res：目标缓冲（已分配好足够大）
** idx：idx->i 读位置，idx->j 写位置
** quote：当前处理的引号类型（' 或 "）
**
** 逻辑：
** - 如果当前位置是 quote：跳过开引号
** - 把引号内部字符复制到 res
** - 遇到闭引号后跳过闭引号
*/
static void	copy_quoted_block(const char *s, char *res, t_index *idx, char quote)
{
	if (s[idx->i] == quote)
	{
		(idx->i)++;                               /* 跳过开引号 */
		while (s[idx->i] && s[idx->i] != quote)    /* 复制引号内部 */
			res[(idx->j)++] = s[(idx->i)++];
		if (s[idx->i] == quote)
			(idx->i)++;                           /* 跳过闭引号 */
	}
}

/*
** 作用：扫描 s 并复制到 res，但“不复制引号本身”。
** 遇到引号就调用 copy_quoted_block，把内部内容拷过去。
**
** 返回：最终写入 res 的长度（不含 '\0'）
*/
static size_t	copy_content_no_quotes(const char *s, char *res)
{
	t_index	idx;

	idx.i = 0;   /* 源串读指针 */
	idx.j = 0;   /* 目标串写指针 */
	while (s[idx.i])
	{
		if (s[idx.i] == '\'')
			copy_quoted_block(s, res, &idx, '\'');
		else if (s[idx.i] == '"')
			copy_quoted_block(s, res, &idx, '"');
		else
			res[idx.j++] = s[idx.i++]; /* 普通字符：直接拷贝并前进 */
	}
	return (idx.j);
}

/*
** 作用：预扫描 s，记录是否出现过单/双引号。
**
** 返回：
** - 如果两种引号都没出现：返回 0（表示无需去引号）
** - 否则返回 1
*/
static int	check_and_set_flags(const char *s, int *q_single, int *q_double)
{
	size_t	i;

	i = 0;
	*q_single = 0;
	*q_double = 0;
	while (s[i])
	{
		if (s[i] == '\'')
			*q_single = 1;
		if (s[i] == '"')
			*q_double = 1;
		i++;
	}
	if (*q_single == 0 && *q_double == 0)
		return (0);
	return (1);
}

/*
** 作用：去掉包裹引号，返回新串，并填充 had_q/q_single/q_double。
**
** 参数：
** s：源串（通常是 substr）
** had_q：输出：是否处理过引号
** q_single/q_double：输出：是否出现过单/双引号
**
** 返回约定（非常关键）：
** - 若 s 中没有任何引号：返回 NULL（并把 flags 清零）
** - 若有引号：返回 malloc 新串（调用者/节点要负责 free）
*/
char	*remove_quotes_flag(const char *s, int *had_q, int *q_single, int *q_double)
{
	size_t	len;
	size_t	j;
	char	*res;

	res = NULL;
	len = 0;
	*had_q = 0;

	if (!s)
	{
		*q_single = 0;
		*q_double = 0;
		return (NULL);
	}

	/* 没有任何引号：直接返回 NULL，让上层走“raw==clean”的逻辑 */
	if (check_and_set_flags(s, q_single, q_double) == 0)
		return (NULL);

	len = ft_strlen(s);
	res = malloc(len + 1);   /* 最坏情况：不去掉引号也不会超过原长 */
	if (!res)
		return (NULL);

	j = copy_content_no_quotes(s, res); /* 复制内容但跳过引号本身 */
	res[j] = '\0';

	*had_q = 1;
	return (res);
}
