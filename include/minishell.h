/* ************************************************************************** */
/* minishell.h
 *
 * 这个头文件是“全局总开关”，定义 minishell 的主结构体 t_minishell。
 * parse / exec / expander / env 等模块都会用到这个结构体来共享状态。
 */
/* ************************************************************************** */

#ifndef MINISHELL_H
#define MINISHELL_H

/* signal.h 用来拿到：
 * - sig_atomic_t：一种“信号处理函数里也能安全读写”的整数类型
 * - SIGINT 等信号宏（Ctrl-C）
 */
#include <signal.h>

/* forward declarations（前置声明）
 * 目的：只告诉编译器“有这些结构体类型”，但不展开它们具体长什么样。
 * 好处：避免头文件互相 include 形成“循环依赖爆炸”。
 */
typedef struct s_lexer t_lexer;
typedef struct s_env t_env;
typedef struct s_ast t_ast;

/* 全局信号状态（别的模块会修改它，比如 heredoc 读输入时）
 * volatile：告诉编译器别优化读取（信号可能随时改它）
 */
extern volatile sig_atomic_t g_signal;

/* minishell 的全局状态结构体 */
typedef struct s_minishell
{
    /* lexer 生成的 token 链表头 */
    t_lexer *lexer;

    /* 环境变量链表（env 模块管理） */
    t_env *env;

    /* 当前正在处理/执行的 AST 节点（某些退出逻辑/信号逻辑会用） */
    t_ast *cur_t_ast;

    /* 用户输入的一整行原始字符串（lexer 用得多） */
    char *raw_line;

    /* 当前命令行里管道数量（parse_pipeline 统计） */
    int n_pipes;

    /* 上一个命令的退出码（$?） */
    int last_exit_status;

    /* 让主循环退出的标记 */
    int should_exit;

    /* 最终 exit 用的退出码 */
    int exit_code;

    /* lexer 续行状态：
     * lexer_unclosed_quote：未闭合的是 ' 还是 "
     * lexer_need_more：是否需要继续读下一行补全
     */
    char lexer_unclosed_quote;
    int lexer_need_more;

    /* envp / PATH 搜索路径（执行器常用） */
    char **envp;
    char **paths;

    /* 行号（比如 debug 或 heredoc/错误提示用） */
    int lineno;
} t_minishell;

/* 清理 minishell 状态（释放内存等） */
void ms_clear(t_minishell *ms);

#endif
