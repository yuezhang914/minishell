// 文件用途
// env_new：创建节点（直接接收已分配的 key/value 指针）
// env_add_back：追加到链表尾
// init_env(envp)：把系统 envp（"A=B"）转成你的链表
// builtin_env：实现 env

#include "minishell.h"
#include "build_in.h"

/* env_new
 * 作用：创建一个 env 节点
 * 注意：这里是“浅接管”：
 * - key/value 必须是堆内存字符串（malloc/strdup 出来的）
 * - 节点释放时会 free(key) / free(value)
 */
t_env *env_new(char *key, char *value)
{
	t_env *new_env;

	/* malloc：向堆申请一个节点结构体 */
	new_env = (t_env *)malloc(sizeof(t_env));
	if (!new_env)
		return (NULL);

	/* 把指针直接塞进结构体 */
	new_env->key = key;
	new_env->value = value;
	new_env->next = NULL;

	return (new_env);
}

/* env_add_back
 * 作用：把 new_env 追加到链表末尾
 * 为什么参数是 t_env **env？
 * - 因为链表可能是空的（*env==NULL），这时要修改头指针
 */
void env_add_back(t_env **env, t_env *new_env)
{
	t_env *last;

	if (!env || !new_env)
		return;

	/* 空链表：新节点直接成为头 */
	if (*env == NULL)
	{
		*env = new_env;
		return;
	}

	/* 找到最后一个节点 */
	last = *env;
	while (last->next)
		last = last->next;

	/* 接到末尾 */
	last->next = new_env;
}

/* init_env
 * 作用：把系统 envp（"KEY=VALUE"）转成链表
 * 关键用到的 libc 函数：
 * - strchr：找 '='
 * - strndup：复制 '=' 前面那一段作为 key
 * - strdup：复制 '=' 后面那一段作为 value
 */
t_env *init_env(char **envp)
{
	t_env *env;
	int i;

	env = NULL;
	i = 0;

	while (envp[i])
	{
		/* 找到 "KEY=VALUE" 的 '=' 位置 */
		char *equal = strchr(envp[i], '=');

		if (equal)
		{
			/* key：复制 envp[i] 从开头到 '=' 前 */
			char *key = strndup(envp[i], equal - envp[i]);

			/* value：复制 '=' 后面的字符串 */
			char *value = strdup(equal + 1);

			/* 创建节点并接到链表尾 */
			env_add_back(&env, env_new(key, value));
		}
		i++;
	}
	return env;
}

/* builtin_env
 * 作用：实现 env 命令
 * 规则（你这里选择的行为）：
 * - env 后面带参数：报错并返回 127
 * - 否则：打印 env（只打印有 value 的 KEY=VALUE）
 */
int builtin_env(char **argv, t_env *env)
{
	/* argv[1] 存在说明用户写了：env xxx */
	if (argv[1])
	{
		ms_put3("env: ", argv[1], " No such file or directory\n");
		return (127);
	}
	print_env(env);
	return 0;
}
