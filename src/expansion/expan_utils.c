#include "expander.h"
#include "minishell.h"

/* 判断字符串是否包含引号字符 ' 或 " */
int	word_has_quotes(const char *s)
{
	int	i;

	i = 0;
	while (s && s[i])
	{
		if (s[i] == '\'' || s[i] == '"')
			return (1);
		i++;
	}
	return (0);
}

/* equal_sign：
 * 返回 entry 中第一个 '=' 的位置
 * - 找不到 '='：返回 strlen(entry)
 */
size_t	equal_sign(char *entry)
{
	int	i;

	if (!entry)
		return (0);
	i = 0;
	while (entry[i] && entry[i] != '=')
		i++;
	return (i);
}

/* strip_all_quotes_dup：
 * 去掉 s 里的引号；如果没有引号就复制一份 s。
 * 这是一个“只负责返回新字符串”的版本（不会 free s）
 */
char	*strip_all_quotes_dup(const char *s, int *had_q, int *q_s, int *q_d)
{
	char	*clean;

	clean = NULL;
	if (!s)
		return (NULL);

	clean = remove_quotes_flag(s, had_q, q_s, q_d);

	if (!clean)
		clean = ft_strdup(s);

	return (clean);
}
