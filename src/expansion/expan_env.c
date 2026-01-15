// expan_env.c：变量名合法规则 + 从 env 链表拿值
// 你要先懂两个概念（很重要）
// 变量名比如 HOME、USER_42：通常只能是字母数字下划线，并且第一个字符不能是数字
// 你的 t_env 是链表：每个节点有 key 和 value

#include "build_in.h"
#include "expander.h"
#include "minishell.h"

/* 本文件用到的库函数来自哪里？
 * - ft_isalpha/ft_isalnum/ft_strlen/ft_strncmp/ft_strdup 来自 libft
 * - t_env 来自 build_in.h
 */

/* 判断变量名首字符是否合法：字母 or '_' */
int	is_name_start(int c)
{
	/* ft_isalpha：判断是不是英文字母 */
	return (ft_isalpha(c) || c == '_');
}

/* 判断变量名后续字符是否合法：字母/数字/'_' */
int	is_name_char(int c)
{
	/* ft_isalnum：判断是不是字母或数字 */
	return (ft_isalnum(c) || c == '_');
}

/* 计算变量名长度：s 指向 $ 后面的第一个字符 */
int	var_len(const char *s)
{
	int	i;

	i = 0;
	if (!s)
		return (0);

	/* 首字符不合法：长度就是 0 */
	if (!is_name_start((unsigned char)s[i]))
		return (0);

	/* 一直数到不合法字符为止 */
	while (s[i] && is_name_char((unsigned char)s[i]))
		i = i + 1;

	return (i);
}

/* 在 ms->env 链表里找变量名，并复制 value 返回 */
char *env_value_dup(t_minishell *minishell, const char *name, int len)
{
	t_env	*cur;

	/* 任何不合法输入：按“找不到变量”处理，返回空字符串副本 */
	if (!minishell || !name || len <= 0)
		return (ft_strdup(""));

	cur = minishell->env;

	/* 遍历 env 链表：一个个节点比对 key */
	while (cur)
	{
		/* 条件1：key 长度要等于 len（必须完全相等，不能只匹配前缀）
		 * 条件2：前 len 个字符相等
		 */
		if (cur->key && (int)ft_strlen(cur->key) == len
			&& ft_strncmp(cur->key, name, len) == 0)
		{
			/* 找到了：value 可能为 NULL，统一返回 strdup */
			if (cur->value)
				return (ft_strdup(cur->value));
			return (ft_strdup(""));
		}
		cur = cur->next;
	}

	/* 遍历完没找到：返回空字符串副本 */
	return (ft_strdup(""));
}
