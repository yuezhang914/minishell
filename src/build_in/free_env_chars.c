#include "minishell.h"
#include "build_in.h"

/* free_env
 * 作用：释放整个 t_env 链表
 * 关键点：
 * - 每个节点都要 free(key) / free(value) / free(node)
 * - 用 tmp 暂存当前节点，防止 free 后丢失 next
 */
void free_env(t_env *env)
{
	t_env *tmp;

	while (env)
	{
		tmp = env;          /* 先记住当前节点地址 */
		env = env->next;    /* 指针先走到下一个节点 */

		free(tmp->key);     /* 释放 key 字符串 */
		free(tmp->value);   /* 释放 value 字符串（NULL 也安全） */
		free(tmp);          /* 释放节点结构体 */
	}
}

/* free_char_matrix
 * 作用：释放 char **（以 NULL 结尾的字符串数组）
 * 例如 envp/paths
 */
void free_char_matrix(char **matrix)
{
	int i;

	i = 0;
	if (!matrix)
		return ;

	while (matrix[i])
	{
		free(matrix[i]);
		i++;
	}
	free(matrix);
}
