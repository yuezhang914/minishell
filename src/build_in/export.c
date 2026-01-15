// export.c（打印/设置环境变量）
// 文件用途
// export 无参数：打印 declare -x ...
// export KEY=VALUE：设置/覆盖
// export KEY：只声明（value=NULL），但如果已存在 value，你选择“不覆盖”（符合你注释里的规则）
// 你实现的一个“bash 差异”
// bash 的 export 无参数时通常会按字典序排序；你这里是链表顺序直接打印。

#include "minishell.h"
#include "build_in.h"

/* print_export
 * 作用：export 无参数时打印所有变量
 * 格式：
 * declare -x KEY="VALUE"
 * 如果 value==NULL：只打印 declare -x KEY
 */
void print_export(t_env *env)
{
	while (env)
	{
		printf("declare -x %s", env->key);
		if (env->value)
			printf("=\"%s\"", env->value);
		printf("\n");
		env = env->next;
	}
}

/* parse_export_arg
 * 作用：把一个参数拆成 key/value
 * 输入可能是：
 * - "A=B"  -> key="A", value="B"
 * - "A"    -> key="A", value=NULL
 */
static int parse_export_arg(char *arg, char **key, char **value)
{
	char *equal;

	/* strchr：找 '=' */
	equal = strchr(arg, '=');

	if (equal)
	{
		/* key：复制 '=' 前 */
		*key = strndup(arg, equal - arg);

		/* value：复制 '=' 后（允许空字符串） */
		*value = strdup(equal + 1);

		if (!*key || !*value)
		{
			perror("strdup");
			free(*key);
			free(*value);
			return -1;
		}
	}
	else
	{
		/* 没有 '='：只声明 */
		*key = strdup(arg);
		*value = NULL;

		if (!*key)
			return (perror("strdup"), -1);
	}
	return 0;
}

/* update_env_var
 * 作用：把 key/value 写入链表（覆盖或新增）
 * 规则：
 * - 已存在：
 *   - 如果 value!=NULL：覆盖旧 value
 *   - 如果 value==NULL：不改旧 value
 * - 不存在：新增节点
 *
 * 注意：key/value 都是堆内存
 * - 如果已存在：key 必须 free（因为链表里已经有一个同名 key）
 */
static void update_env_var(t_env **env, char *key, char *value)
{
	t_env *existing;

	existing = find_env_var(*env, key);
	if (existing)
	{
		if (value)
		{
			free(existing->value);
			existing->value = value;
		}
		free(key);
	}
	else
	{
		env_add_back(env, env_new(key, value));
	}
}

/* export_one
 * 作用：处理 export 的一个参数
 * 步骤：
 * 1) parse_export_arg 拆 key/value
 * 2) is_valid_identifier 检查 key 名是否合法
 * 3) update_env_var 写入链表
 */
static int export_one(char *arg, t_env **env)
{
	char *key;
	char *value;

	if (parse_export_arg(arg, &key, &value) == -1)
		return 1;

	if (!is_valid_identifier(key))
	{
		ms_put3("export: ", arg, " not a valid identifier\n");
		free(key);
		free(value);
		return 1;
	}

	update_env_var(env, key, value);
	return 0;
}

/* builtin_export
 * 作用：export 总入口
 * - 无参数：打印全部
 * - 有参数：逐个处理，遇到非法参数 status=1 但继续处理后续
 */
int builtin_export(char **argv, t_env **env)
{
	int status;
	int i;

	status = 0;
	i = 1;

	if (!argv[1])
	{
		print_export(*env);
		return 0;
	}

	while (argv[i])
	{
		if (export_one(argv[i], env))
			status = 1;
		i++;
	}
	return status;
}
