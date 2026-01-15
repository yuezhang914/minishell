// unset.c（删除环境变量 + 标识符合法性）
// 文件用途
// is_valid_identifier：检查变量名是否合法
// delete_env_var：从链表里摘掉节点并释放内存
// builtin_unset：循环删多个 key
// 你这里的 bug（前面提过）
// 应该从 argv[1] 开始遍历（跳过 unset 自己）。

#include "minishell.h"
#include "build_in.h"

/* delete_env_var
 * 作用：把 key 对应的节点从链表里删掉，并 free 掉 key/value/节点
 * 典型单链表删除法：
 * - 如果删头节点：更新 *env
 * - 否则用 prev + temp 找到节点，再 prev->next = temp->next
 */
static void delete_env_var(t_env **env, const char *key)
{
	t_env *temp;
	t_env *prev;

	temp = *env;
	prev = NULL;

	if (!temp)
		return;

	/* 头节点就是目标：特殊处理 */
	if (strcmp(temp->key, key) == 0)
	{
		*env = temp->next;
		free(temp->key);
		free(temp->value);
		free(temp);
		return;
	}

	/* 找目标节点，同时保存前驱 prev */
	while (temp != NULL && strcmp(temp->key, key) != 0)
	{
		prev = temp;
		temp = temp->next;
	}

	/* 没找到：静默返回（符合 shell 行为） */
	if (temp == NULL)
		return;

	/* 摘链：跳过 temp */
	prev->next = temp->next;

	/* 释放节点内部资源 */
	free(temp->key);
	free(temp->value);
	free(temp);
}

/* is_valid_identifier
 * 作用：检查变量名是否符合规则：
 * - 第一个字符：字母或 '_'（不能是数字）
 * - 后续字符：字母/数字/'_'
 */
int is_valid_identifier(const char *s)
{
	int i;

	i = 0;
	if (!s || !s[0])
		return 0;

	if (!(ft_isalpha(s[0]) || s[0] == '_'))
		return 0;

	i = 1;
	while (s[i])
	{
		if (!(ft_isalnum(s[i]) || s[i] == '_'))
			return 0;
		i++;
	}
	return 1;
}

/* builtin_unset
 * 作用：unset 总入口：删多个变量
 * 返回：
 * - 0：都合法
 * - 1：至少有一个非法标识符
 *
 * ⚠️ 这里有一个明显问题：
 * 你从 i=0 开始，会把 argv[0]="unset" 也当成要删除的变量名。
 * 正确应该是 i=1。
 */
int builtin_unset(char **argv, t_env **env)
{
	int status;
	int i;

	i = 0;      /* ⚠️ 建议改成 i = 1 */
	status = 0;

	while (argv[i])
	{
		if (!is_valid_identifier(argv[i]))
		{
			ms_put3("unset: ", argv[i], " not a valid identifier\n");
			status = 1;
		}
		else
		{
			delete_env_var(env, argv[i]);
		}
		i++;
	}
	return status;
}
