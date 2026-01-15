/* ************************************************************************** */
/* parse.h
 *
 * 这是 parse 模块的总头文件：
 * - 定义 AST（语法树）结构 t_ast
 * - 定义重定向链表 t_redir
 * - 声明解析函数 parse_cmdline / parse_pipeline ...
 * - 声明 heredoc 辅助函数
 */
/* ************************************************************************** */

#ifndef PARSE_H
# define PARSE_H

/* libft：你项目自带工具库，parse 里实际用到了：
 * - ft_calloc / ft_strdup / ft_substr（分配与复制）
 * - ft_lstnew / ft_lstadd_back / ft_lstsize（链表）
 * - ft_putstr_fd（输出错误）
 * - 还有你项目里的 get_next_line（在 heredoc 里用）
 */
# include "../libft/libft.h"

/* lexer.h：为了使用 t_lexer、tok_type、TOK_WORD、TOK_PIPE 等 */
# include "lexer.h"

/* signals.h：为了使用 setup_heredoc_signals / save_signals / restore_signals 等（在 heredoc 里） */
# include "signals.h"

/* error.h：为了使用 ms_put3 等你封装的错误输出函数 */
# include "error.h"

/* 这里 forward 声明一下 libft 的链表类型 */
typedef struct s_list t_list;

/* get_next_line 的缓冲大小（你 heredoc 里会用） */
# define BUFFER_SIZE 42

/* AST 节点类型：
 * 你的 parse 目前真正用到的是：
 * - NODE_CMD：普通命令
 * - NODE_PIPE：管道
 * - NODE_SUBSHELL：括号子命令
 * 其他枚举（AND/OR/SEQUENCE）目前没在 parse 逻辑里实现，先占位。
 */
typedef enum node_type
{
    NODE_CMD,
    NODE_PIPE,
    NODE_AND,
    NODE_OR,
    NODE_SUBSHELL,
    NODE_BACKGROUND,
    NODE_SEQUENCE
} t_node_type;

/* 重定向类型：
 * - REDIR_INPUT  : <
 * - REDIR_OUTPUT : >
 * - REDIR_APPEND : >>
 * - HEREDOC      : <<
 */
typedef enum e_redir_type
{
    REDIR_INPUT,
    REDIR_OUTPUT,
    REDIR_APPEND,
    HEREDOC
} t_redir_type;

/* 重定向链表节点：
 * 一条命令可能有多个重定向，比如：cat < a > b
 * 所以用链表串起来。
 */
typedef struct s_redir
{
    struct s_redir *next;   /* 指向下一个重定向节点 */

    char *filename;         /* < > >> 时是文件名；<< 时是 delimiter（终止符） */

    int heredoc_fd;         /* << 时：管道读端 fd（父进程保存），执行时从这里读 */
    bool is_expanded;       /* 是否已经做过变量展开（你后续可能用到） */

    t_redir_type type;      /* 重定向类型 */

    int quoted;             /* heredoc 的 delimiter 是否带引号
                             * quoted=1：不展开变量
                             * quoted=0：可以展开变量
                             */
} t_redir;

/* 你用 libft 的 t_list 来暂存 argv：
 * t_cmd 就是 “装参数字符串的链表节点”
 */
typedef struct s_list t_list;
typedef t_list t_cmd;

/* AST 树节点：
 * - 管道节点：type=NODE_PIPE，left/right 指向左右子树
 * - 命令节点：type=NODE_CMD，argv/redir 填好
 * - subshell：type=NODE_SUBSHELL，sub 指向括号里的那棵树
 */
typedef struct s_ast
{
    t_node_type type;

    char **argv;        /* 参数数组：argv[0]=命令名，argv[最后]=NULL */

    t_redir *redir;     /* 重定向链表头 */

    int n_pipes;        /* 这个字段在你代码里不常用（你把 n_pipes 存到 minishell->n_pipes） */

    struct s_ast *left; /* PIPE 节点左子树 */
    struct s_ast *right;/* PIPE 节点右子树 */

    struct s_ast *sub;  /* SUBSHELL 节点括号里面的子树 */
} t_ast;

/* ===== free 函数：释放 AST / argv / redir ===== */
void free_ast(t_ast *node);
void free_tokens(t_lexer *tok);
void free_ast_partial(t_ast *node);
void free_argv_list(t_cmd *a);
void free_redir_list(t_redir *r);
void free_t_cmd_node(t_cmd *argv_cmd);

/* ===== token 流工具（peek/consume/expect）===== */
t_lexer *peek_token(t_lexer **cur);
t_lexer *consume_token(t_lexer **cur);
t_lexer *expect_token(tok_type type, t_lexer **cur);
int is_redir_token(t_lexer *pt);

/* ===== debug 打印（你项目里可选）===== */
void print_indent(int depth);
void print_ast(t_ast *node, int depth);
void print_ast_by_type(t_ast *node, int depth);
void print_ast_pipe(t_ast *node, int depth);
void print_ast_cmd(t_ast *node);
void print_ast_subshell(t_ast *node, int depth);

/* ===== parsing 主流程 ===== */
t_ast *parse_cmdline(t_lexer **cur, t_minishell *minishell);
t_ast *parse_pipeline(t_lexer **cur, t_minishell *minishell);
t_ast *parse_simple_cmd_redir_list(t_lexer **cur, t_minishell *minishell);
t_ast *parse_subshell(t_lexer **cur, t_ast *node, t_minishell *minishell);
t_ast *parse_normal_cmd_redir_list(t_lexer **cur, t_ast *node,
                                   t_minishell *minishell);

/* argv 构建 */
t_cmd *create_argv(char *str);
char **build_argvs(t_cmd *argv_cmd, t_redir *redir, t_ast *node);

/* env / 安全字符串 */
char *safe_strdup(const char *s);

/* 重定向 / heredoc */
int handle_heredoc(t_redir *new_redir, t_minishell *minishell);
int build_redir(t_lexer **cur, t_redir **redir_list, t_minishell *minishell);

/* heredoc 辅助（提取行、循环） */
int end_line(char *str);
char *extract_line(char *str);
int heredoc_loop(int write_fd, const char *delimiter, t_minishell *msh, int quoted);

int main(int argc, char *argv[], char **envp);

#endif
