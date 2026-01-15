// 我现在扫描到一个 $，接下来到底是 $? 还是 $HOME？我应该往输出里加什么？指针要前进多少？


#include "expander.h"
#include "minishell.h"

/* 本文件用到的库函数来自哪里？
 * - ft_strdup / ft_strjoin / free 来自 libft + libc
 * - ft_itoa / ft_isdigit 来自 libft
 * - var_len / env_value_dup 是你 expan_env.c 的函数
 */

/* 把 b 拼到 a 后面，并 free(a)
 * 这是 expander 里最常用的小工具：每追加一次都返回一段新字符串。
 */
char *str_join_free(char *a, const char *b)
{
	char *res;

	res = NULL;

	/* 两个都没有：没法拼 */
	if (!a && !b)
		return (NULL);

	/* a 没有：直接复制 b */
	if (!a)
		return (ft_strdup(b));

	/* b 没有：保持 a 不变 */
	if (!b)
		return (a);

	/* ft_strjoin：拼接成新字符串（malloc） */
	res = ft_strjoin(a, b);

	/* 旧的 a 不再需要了，释放 */
	free(a);

	return (res);
}

/* 处理特殊变量：
 * 1) $?  -> 上一次退出码
 * 2) $数字 -> 你这里选择“跳过两个字符”，不做展开（相当于不支持 positional params）
 *
 * 返回值：本次应该消耗多少字符
 * - 2：表示处理了 "$?" 或 "$1" 这种形式
 * - 0：表示没处理（交给普通变量处理）
 *
 * ⚠ 注意：这里有个小瑕疵：如果 ft_itoa 失败，你返回 2，但 out 没追加东西。
 */
static int handle_special_exp(t_exp_data *data, const char *s, int j)
{
	char *tmp;
	int  status;

	tmp = NULL;
	status = 0;

	/* $? */
	if (s[j + 1] == '?')
	{
		if (data->minishell)
			status = data->minishell->last_exit_status;

		tmp = ft_itoa(status);          /* int -> 字符串 */
		if (!tmp)
			return (2);                 /* 内存失败时仍然前进 2 */

		*data->out = str_join_free(*data->out, tmp); /* 追加到输出 */
		free(tmp);

		return (2); /* 消耗 "$?" 两个字符 */
	}

	/* $数字：不支持，直接跳过两位 */
	if (ft_isdigit((unsigned char)s[j + 1]))
		return (2);

	return (0);
}

/* 处理普通变量：$VAR
 * 返回：本次消耗字符数
 * - 如果是 $HOME：返回 1 + len（$ + 变量名长度）
 * - 如果不是合法变量名：把 '$' 当普通字符输出，返回 1
 */
static int handle_var_exp(t_exp_data *data, const char *s, int j)
{
	int   len;
	char *tmp;

	len = 0;
	tmp = NULL;

	/* 计算变量名长度：从 s[j+1] 开始 */
	len = var_len(&s[j + 1]);

	if (len > 0)
	{
		/* 复制变量值（没找到变量就得到 ""） */
		tmp = env_value_dup(data->minishell, &s[j + 1], len);

		/* 把变量值拼到输出 */
		*data->out = str_join_free(*data->out, tmp);

		free(tmp);

		return (1 + len);
	}

	/* 不是合法变量名：把 '$' 当普通字符 */
	*data->out = str_join_free(*data->out, "$");
	return (1);
}

/* 这是“处理一次 $”的总入口：
 * - 如果当前在单引号里：不展开，直接输出 '$'
 * - 否则：先试特殊变量 $? / $数字，再试普通变量 $VAR
 */
int scan_expand_one(t_exp_data *data, const char *s, int j, enum qstate q)
{
	int res;

	res = 0;

	/* 单引号里：$ 不展开 */
	if (q == Q_SQ)
	{
		*data->out = str_join_free(*data->out, "$");
		return (1);
	}

	/* 先尝试 $? 或 $数字 */
	res = handle_special_exp(data, s, j);
	if (res > 0)
		return (res);

	/* 再处理普通变量 */
	res = handle_var_exp(data, s, j);
	return (res);
}
