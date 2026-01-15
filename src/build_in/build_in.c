#include "build_in.h"
#include "minishell.h"

/* is_builtin
 * 作用：判断 cmd 是否是你支持的 builtin 名字
 * 返回：
 * - 1：是 builtin
 * - 0：不是（需要走 execve 外部命令）
 */
int is_builtin(const char *cmd)
{
	/* 防御：cmd 为 NULL 直接不是 builtin */
	if (!cmd)
		return (0);

	/* ft_strcmp 来自 libft：比较字符串是否完全相同 */
	if (ft_strcmp(cmd, "cd") == 0)      return (1);
	if (ft_strcmp(cmd, "echo") == 0)    return (1);
	if (ft_strcmp(cmd, "pwd") == 0)     return (1);
	if (ft_strcmp(cmd, "exit") == 0)    return (1);
	if (ft_strcmp(cmd, "export") == 0)  return (1);
	if (ft_strcmp(cmd, "unset") == 0)   return (1);
	if (ft_strcmp(cmd, "env") == 0)     return (1);

	return (0);
}

/* exec_builtin
 * 作用：执行 builtin（根据 argv[0] 分发）
 * 参数：
 * - node：AST 命令节点，node->argv 就是 ["cd","..",NULL] 这种
 * - env：t_env 链表的“地址”（因为 export/unset/cd 会改它）
 * - msh：minishell 总状态（exit 要改 should_exit 等）
 */
int exec_builtin(t_ast *node, t_env **env, t_minishell *msh)
{
	/* 防御：没有命令就返回失败 */
	if (!node || !node->argv || !node->argv[0])
		return (1);

	/* 分发：每个 builtin 返回一个退出码（0 成功） */
	if (ft_strcmp(node->argv[0], "cd") == 0)
		return (ft_cd(node->argv, env));

	if (ft_strcmp(node->argv[0], "echo") == 0)
		return (ft_echo(node->argv));

	if (ft_strcmp(node->argv[0], "pwd") == 0)
		return (builtin_pwd());

	if (ft_strcmp(node->argv[0], "export") == 0)
		return (builtin_export(node->argv, env));

	if (ft_strcmp(node->argv[0], "env") == 0)
		return (builtin_env(node->argv, *env));

	if (ft_strcmp(node->argv[0], "unset") == 0)
		return (builtin_unset(node->argv, env));

	if (ft_strcmp(node->argv[0], "exit") == 0)
		return (builtin_exit(node->argv, msh));

	/* 没匹配上：当作失败（理论上 is_builtin 会拦住） */
	return (1);
}
