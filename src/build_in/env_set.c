#include "build_in.h"
#include "minishell.h"

/* create_env_var
 * 作用：创建一个新变量节点（自己 strdup key/value）
 * 为什么要 strdup？
 * - 保证节点里的 key/value 拥有独立堆内存
 * - 外面传进来的字符串就算之后被 free/改动，env 里仍然安全
 */
static t_env *create_env_var(const char *key, const char *value)
{
	t_env *new_var;
	char *new_key;
	char *new_value;

	/* strdup 是 libc：复制字符串到堆里 */
	new_key = strdup(key);
	new_value = strdup(value);

	/* 任意一个失败：释放已成功的，直接退出 */
	if (!new_key || !new_value)
	{
		perror("strdup");
		free(new_key);
		free(new_value);
		exit(EXIT_FAILURE);
	}

	new_var = malloc(sizeof(t_env));
	if (!new_var)
	{
		perror("malloc");
		free(new_key);
		free(new_value);
		exit(EXIT_FAILURE);
	}

	new_var->key = new_key;
	new_var->value = new_value;
	new_var->next = NULL;
	return new_var;
}

/* add_env_var
 * 作用：把 new_var 接到链表末尾
 */
static void add_env_var(t_env **env, t_env *new_var)
{
	t_env *last;

	/* 空链表：直接当头 */
	if (!*env)
	{
		*env = new_var;
		return;
	}

	/* 找最后一个 */
	last = *env;
	while (last->next)
		last = last->next;

	last->next = new_var;
}

/* env_set
 * 作用：设置变量（更新或新增）
 * - 如果 key 已存在：free 旧 value，strdup 新 value
 * - 不存在：创建新节点追加
 */
void env_set(t_env **env, const char *key, const char *value)
{
	t_env *var;

	/* 找有没有同名变量 */
	var = find_env_var(*env, key);

	if (var)
	{
		/* 更新：先释放旧 value，避免内存泄漏 */
		free(var->value);

		/* 再复制新 value */
		var->value = strdup(value);
		if (!var->value)
		{
			perror("strdup");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		/* 新增：创建节点并追加 */
		add_env_var(env, create_env_var(key, value));
	}
}
