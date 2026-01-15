#ifndef EXPANDER_H
#define EXPANDER_H

/* parse.h 里有：
 * - t_ast / t_redir 结构体（expander 要遍历 AST、修改 argv/redir）
 * - NODE_CMD 枚举（判断节点类型）
 */
#include "parse.h"

/* libft 里你这里会用到的函数非常多，例如：
 * - ft_strdup / ft_strjoin / ft_strlen / ft_strncmp
 * - ft_isalpha / ft_isalnum / ft_isdigit
 * - ft_itoa
 */
#include "../libft/libft.h"

/* build_in.h 里（你项目里）通常会定义：
 * - t_env 环境变量链表结构体（key/value/next）
 * - 一些 builtin 相关声明
 * 你的 expan_env.c / expan_heredoc_val.c 里确实用到了 t_env。
 */
#include "build_in.h"

/* qstate：扫描字符串时记录“现在在什么引号里” */
enum qstate
{
	Q_NONE = 0,  /* 不在引号里 */
	Q_SQ = 1,    /* 在单引号 '...' 里 */
	Q_DQ = 2     /* 在双引号 "..." 里 */
};

/* 扫描时需要带的“上下文小包” */
typedef struct s_exp_data
{
	t_minishell *minishell; /* 用来取 env、取 last_exit_status */
	char **out;             /* 指向输出字符串指针：我们会不断往 *out 追加内容 */
} t_exp_data;

/* 判断字符串里是否含有 ' 或 " */
int word_has_quotes(const char *s);

/* 总入口：遍历整棵 AST，展开所有 CMD 节点 */
int expander_t_ast(t_minishell *minishell, t_ast *root);

/* 展开一个 CMD 节点：argv + redir */
int expander_expand_cmd_node(t_minishell *msh, t_ast *node);

/* 对单个字符串做：$展开 + 去引号，并 free 旧字符串 */
char *expander_str(t_minishell *minishell, char *str);

/* 扫描器：从 s[j] 开始处理一次 $...，返回“消耗了多少字符” */
int scan_expand_one(t_exp_data *data, const char *s, int j, enum qstate q);

/* 对整串做展开：逐字符扫描 + 引号状态机 */
char *expand_all(t_minishell *minishell, const char *str);

/* 变量名规则：首字符、后续字符、变量名长度 */
int is_name_start(int c);
int is_name_char(int c);
int var_len(const char *s);

/* 从 env 链表里取变量值并 strdup 一份（找不到返回 strdup("")） */
char *env_value_dup(t_minishell *minishell, const char *name, int len);

/* 拼接工具：把 b 拼到 a 后面，并 free(a) */
char *str_join_free(char *a, const char *b);

/* 返回字符串里第一个 '=' 的位置（你这里主要是工具函数） */
size_t equal_sign(char *str);

/* 由 lexer 模块提供：去引号并告诉你有没有引号 */
char *remove_quotes_flag(const char *s, int *had_q, int *q_s, int *q_d);

/* heredoc 行变量展开（你项目里可能用于 heredoc） */
char *expand_heredoc_vars(t_minishell *ms, char *line);

#endif
