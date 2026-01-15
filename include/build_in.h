#ifndef BUILD_IN_H
# define BUILD_IN_H

/* limits.h：
 * - 里面有 PATH_MAX（一般是 4096）
 * - 你在 pwd.c 里用 char cwd[PATH_MAX]
 */
# include <limits.h>

/* unistd.h：
 * - chdir() / getcwd() / isatty()
 * - STDIN_FILENO / STDOUT_FILENO / STDERR_FILENO
 */
# include <unistd.h>

/* libft：
 * - 你用到大量 ft_ 前缀函数，如 ft_strcmp / ft_strdup / ft_isdigit 等
 */
# include "../libft/libft.h"

/* error.h：
 * - 你项目自己的错误输出函数，如 ms_put3("a","b","c")
 */
# include "error.h"

/* parse.h：
 * - 里面定义了 t_ast（命令节点里有 argv）
 */
# include "parse.h"

/* 环境变量链表节点：
 * key="PATH", value="/bin:/usr/bin"
 * next 指向下一个变量
 */
typedef struct s_env
{
	char			*key;     /* 变量名（堆内存字符串） */
	char			*value;   /* 变量值（堆内存字符串，或 NULL） */
	struct s_env	*next;    /* 下一个节点 */
}	t_env;

/* builtin 总入口：根据 argv[0] 分发到具体 builtin */
int		exec_builtin(t_ast *node, t_env **env, t_minishell *msh);

/* 判断字符串是否为 builtin 命令名 */
int		is_builtin(const char *cmd);

/* 各 builtin 实现 */
int		ft_cd(char **argv, t_env **env);
int		ft_echo(char **argv);
int		builtin_pwd(void);
int		builtin_env(char **argv, t_env *env);
int		builtin_export(char **argv, t_env **env);
int		builtin_unset(char **argv, t_env **env);
int		builtin_exit(char **argv, t_minishell *msh);

/* env 链表相关工具 */
t_env	*init_env(char **envp);
t_env	*env_new(char *key, char *value);
void	env_add_back(t_env **env, t_env *new_env);
t_env	*find_env_var(t_env *env, const char *key);

/* 注意：你这里声明了 get_env_value，但当前 zip 里没提供实现文件
 * （可能在别处模块里）。它的作用应该是：
 * - 在 ms->env 链表里找 key，返回 value
 */
char	*get_env_value(t_minishell *ms, const char *key);

void	env_set(t_env **env, const char *key, const char *value);

/* 把 env 链表转成 char **envp（execve 需要 envp） */
void	change_envp(t_env *env, char ***envp);

void	print_env(t_env *env);
void	free_env(t_env *env);

/* export/unset 里用：检查变量名是否合法 */
int		is_valid_identifier(const char *s);

/* 释放 char **（比如 envp/paths） */
void	free_char_matrix(char **matrix);

#endif
