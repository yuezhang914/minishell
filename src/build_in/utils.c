// utils.c（env 链表 → envp；查找；打印）
// 文件用途
// change_envp：把链表同步成 char **envp（execve 用）
// print_env：打印 KEY=VALUE（value==NULL 不打印）
// find_env_var：按 key 查节点

#include "minishell.h"
#include "build_in.h"

/* env_len
 * 作用：数一数链表有多少个节点
 * 用途：
 * - change_envp 要分配 len+1 个 char*（最后一个必须是 NULL）
 */
static int env_len(t_env *env)
{
	int len;

	len = 0;
	while (env)
	{
		len++;
		env = env->next;
	}
	return len;
}

/* env_to_str
 * 作用：把一个节点拼成 "KEY=VALUE"
 * 关键点：
 * - 会 malloc 新字符串（ft_strjoin 内部会 malloc）
 * - 返回值需要调用方释放
 */
static char *env_to_str(t_env *env)
{
	char *key_value;
	char *result;

	/* 先拼 "KEY=" */
	key_value = ft_strjoin(env->key, "=");
	if (!key_value)
		return NULL;

	/* 再拼 "KEY=" + "VALUE" */
	result = ft_strjoin(key_value, env->value);

	/* key_value 是中间产物，用完释放 */
	free(key_value);
	return result;
}

/* change_envp
 * 作用：同步 env 链表 → char **envp（execve 需要 envp）
 * 参数：
 * - env：链表头
 * - envp：指向 char** 的地址（因为要改 *envp 指向的新数组）
 *
 * 步骤：
 * 1) 如果原来 *envp 有内容：先整体释放（避免泄漏）
 * 2) 分配新数组：len+1
 * 3) 遍历链表，每个节点转成 "KEY=VALUE"
 * 4) 最后补 NULL 作为结尾
 */
void change_envp(t_env *env, char ***envp)
{
	int i;
	t_env *tmp;
	int len;

	/* 如果原来已有 envp：释放整张“字符串矩阵” */
	if (*envp != NULL)
	{
		free_char_matrix(*envp);
		*envp = NULL;
	}

	/* 计算链表长度，用于分配数组 */
	len = env_len(env);

	/* 分配 len+1 个指针（最后一个必须 NULL） */
	*envp = malloc(sizeof(char *) * (len + 1));
	if (!*envp)
		return ;

	tmp = env;
	i = 0;

	while (tmp)
	{
		/* 每个节点转成 "KEY=VALUE" */
		(*envp)[i] = env_to_str(tmp);
		tmp = tmp->next;
		i++;
	}

	/* 结尾封口：execve 需要以 NULL 结尾 */
	(*envp)[i] = NULL;
}

/* print_env
 * 作用：模拟 env 输出
 * 关键规则：
 * - 只打印 value 不为 NULL 的变量
 *   （这和 export 不同：export 会显示“只声明未赋值”的变量）
 */
void print_env(t_env *env)
{
	while (env)
	{
		if (env->value)
			printf("%s=%s\n", env->key, env->value);
		env = env->next;
	}
}

/* find_env_var
 * 作用：在链表里找 key 相同的节点
 * - 找到：返回节点指针
 * - 找不到：返回 NULL
 */
t_env *find_env_var(t_env *env, const char *key)
{
	while (env)
	{
		if (strcmp(env->key, key) == 0)
			return env;
		env = env->next;
	}
	return NULL;
}
