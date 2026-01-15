// expan_api.c：对“一个字符串”做完整处理：展开 + 去引号 + free 旧串
// 这个文件就是对外提供的“字符串 API”：
// expander_str(ms, str)：传入一个 malloc 的旧字符串 str
// 返回新字符串，并把旧的 free 掉

#include "minishell.h"
#include "expander.h"

/* 本文件用到：
 * - remove_quotes_flag：来自 lexer 模块（你之前实现的）
 * - ft_strdup：来自 libft
 * - expand_all：来自 expan_scan2.c
 */

/* api_clean_dup：
 * 作用：把 s 去引号；如果没有引号，就复制一份 s。
 *
 * 为什么要这样写？
 * - remove_quotes_flag 的约定是：如果 s 没有引号，它会返回 NULL
 * - 但我们需要“总是返回一个新分配的字符串”
 * 所以：NULL 就用 ft_strdup(s) 补上。
 */
static char	*api_clean_dup(const char *s)
{
	char	*clean;
	int		had_q;
	int		q_s;
	int		q_d;

	clean = NULL;

	/* 这三个变量是 remove_quotes_flag 的输出参数 */
	had_q = 0;
	q_s = 0;
	q_d = 0;

	/* 去掉引号：如果 s 里没有引号，会返回 NULL */
	clean = remove_quotes_flag(s, &had_q, &q_s, &q_d);

	/* 没引号：复制一份 */
	if (!clean)
		clean = ft_strdup(s);

	return (clean);
}

/* expander_str：
 * 作用：对 str 做：
 * 1) expand_all：做 $ 展开（根据引号状态决定是否展开）
 * 2) api_clean_dup：删除引号（把 ' " 去掉）
 * 3) free 旧 str
 *
 * 参数：
 * - minishell：提供 env / last_exit_status
 * - str：必须是 malloc 得到的字符串（函数里会 free 它）
 *
 * 返回：
 * - 新分配的字符串（调用者接管）
 * - 失败返回 NULL
 */
char	*expander_str(t_minishell *minishell, char *str)
{
	char	*tmp;
	char	*clean;

	tmp = NULL;
	clean = NULL;

	if (!str)
		return (NULL);

	/* 1) 先展开：此时 tmp 仍可能包含引号字符 */
	tmp = expand_all(minishell, str);
	if (!tmp)
	{
		/* 展开失败：旧 str 也要 free，避免泄漏 */
		free(str);
		return (NULL);
	}

	/* 2) 再去引号：clean 一定是新分配的字符串 */
	clean = api_clean_dup(tmp);

	/* tmp 不再需要了 */
	free(tmp);

	/* 旧 str 也不再需要 */
	free(str);

	return (clean);
}
