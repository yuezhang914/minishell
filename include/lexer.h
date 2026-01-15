/* include/lexer.h */
#ifndef LEXER_H
# define LEXER_H

/* 你项目的 libft + 系统头 */
# include "../libft/libft.h"
# include <ctype.h>
# include <fcntl.h>
# include <limits.h>
# include <readline/history.h>
# include <readline/readline.h>
# include <signal.h>
# include <stdbool.h>
# include <stddef.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <termios.h>
# include <unistd.h>

/* ===== lexer 返回值/状态码约定 ===== */
# define LEX_OK 1
# define LEX_ERR 0

/* 词法发现未闭合引号：告诉上层“需要再读一行” */
# define LEX_NEED_MORE 2

/* 内部使用：match_quotes/word 发现未闭合（handle_lexer 里会转成 LEX_NEED_MORE） */
# define LEX_UNCLOSED_QUOTE -2

/* minishell 总上下文：这里只前置声明（避免头文件互相 include 爆炸） */
typedef struct s_minishell	t_minishell;

/* ===== 单词 token 的“临时信息包” =====
 * clean: 去掉外层引号后的文本（后续 parser/expander 用它）
 * raw:   原始片段（保留引号的原样，用于 debug 或后续逻辑）
 * had_quotes: 是否出现过引号
 * quoted_single/quoted_double: 是否出现过 ' / "
 */
typedef struct s_token_info
{
	char	*clean;
	char	*raw;
	int		had_quotes;
	int		quoted_single;
	int		quoted_double;
}	t_token_info;

/* ===== token 类型 =====
 * 只做 mandatory: | < > << >>
 */
typedef enum
{
	TOK_WORD,
	TOK_PIPE,      // |
	//TOK_AND,     // &&
	//TOK_OR,      // ||
	TOK_LPAREN,    // (
	TOK_RPAREN,    // )
	TOK_REDIR_IN,  // <
	TOK_REDIR_OUT, // >
	TOK_APPEND,    // >>
	TOK_HEREDOC,   // <<
	TOK_END,       // EOF
	TOK_ERROR
}	tok_type;

/* ===== lexer 链表节点 =====
 * str:        去掉外层引号后的文本（可能与 raw 同指针）
 * raw:        原始片段（含引号），或符号 token 的符号字符串
 * idx:        创建顺序编号（new_node 内 static count++）
 * had_quotes: 是否出现过引号
 * quoted_by:  bit 思路：单引号 +1，双引号 +2，所以 0/1/2/3
 * prev/next:  双向链表
 */
typedef struct s_lexer
{
	char			*str;
	tok_type		tokentype;
	int				idx;
	int				had_quotes;
	int				quoted_by;

	char			*raw;
	struct s_lexer	*prev;
	struct s_lexer	*next;
}	t_lexer;

/* ===== remove_quotes 扫描用索引对 =====
 * i: 源串读指针
 * j: 目标串写指针
 */
typedef struct s_index
{
	size_t	i;
	size_t	j;
}	t_index;

/* ===== 工具/链表/lexer 入口函数声明 ===== */
const char	*token_symbol(tok_type t);

t_lexer		*new_node(t_token_info *info, tok_type tokentype);
void		list_add_back(t_lexer **lst, t_lexer *new);
int			add_node(t_token_info *info, tok_type tokentype, t_lexer **list);

t_lexer		*clear_one(t_lexer **lst);
void		del_first(t_lexer **lst);
void		del_one(t_lexer **lst, int target);
void		clear_list(t_lexer **lst);

int			detect_signal(t_lexer **list);   /* 你项目别处实现：一般用于 Ctrl-C 清理 */
tok_type		is_token(int c);

int			finalize_word_node(t_token_info *info, t_lexer **list);

int			handle_token(char *str, int idx, t_lexer **list);
int			match_quotes(int i, char *str, char quote);

char		*remove_quotes_flag(const char *s, int *had_q,
				int *q_single, int *q_double);

/* handle_word：增加 out_unclosed 用于告诉上层未闭合的是 ' 还是 " */
int			handle_word(char *str, int i, t_lexer **list, char *out_unclosed);

int			skip_spaces(char *str, int i);
int			handle_lexer(t_minishell *minishell);

int			check_space(char c);

#endif
