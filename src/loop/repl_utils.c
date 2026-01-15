// repl_utils.c（辅助：拼接 acc、判断是否含 heredoc、释放 acc）
// 关键点：为什么拼接时要加 \n
// 因为你允许多行输入（比如引号未闭合）。拼接成：
// echo "abc
// def"
// 中间必须保留换行，才能更像真实输入。

#include "minishell.h"
#include "repl.h"

/* 返回 1 表示 token 链表里出现过 TOK_HEREDOC（<<） */
int	lexer_has_heredoc(t_lexer *lx)
{
	while (lx)
	{
		if (lx->tokentype == TOK_HEREDOC)
			return (1);
		lx = lx->next;
	}
	return (0);
}

/* 只复制第一行（遇到 '\n' 就截断）
 * 用途：如果整条命令包含 heredoc，不把 heredoc 大段内容塞进 history
 */
char	*dup_first_line(const char *s)
{
	size_t	i;

	if (!s)
		return (NULL);
	i = 0;
	while (s[i] && s[i] != '\n')
		i++;
	return (ft_substr(s, 0, i));
}

/*
** 作用：把 line 拼到 *acc 后面
** 规则：
** - 如果 acc 为空：acc = strdup(line)
** - 如果 acc 不为空：
**   1) acc += "\n"
**   2) acc += line
**
** 返回：
** - 成功 1
** - 失败 0（通常是 malloc 失败）
*/
int	repl_join(char **acc, char *line)
{
	char	*tmp;

	if (!acc || !line)
		return (0);

	if (!*acc)
	{
		*acc = ft_strdup(line);
		return (*acc != NULL);
	}

	/* 先拼一个换行 */
	tmp = ft_strjoin(*acc, "\n");
	free(*acc);
	*acc = tmp;
	if (!*acc)
		return (0);

	/* 再拼这一行内容 */
	tmp = ft_strjoin(*acc, line);
	free(*acc);
	*acc = tmp;

	return (*acc != NULL);
}

/* 作用：安全释放 acc，并置 NULL */
void	repl_free_acc(char **acc)
{
	if (!acc)
		return ;
	if (*acc)
		free(*acc);
	*acc = NULL;
}

