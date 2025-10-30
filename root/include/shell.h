typedef enum TokenType
{
    TOK_WORD,
    TOK_REDIR_IN,      // <
    TOK_REDIR_OUT,     // >
    TOK_REDIR_APPEND,  // >>
    TOK_REDIR_HEREDOC, // <<
    TOK_PIPE,          // |
    TOK_EOF

};

typedef struct s_redir
{
    TokenType type;       // 重定向类型
    char *target;         // 目标字符串（文件名或 here-doc delimiter）
    bool delim_quoted;    // 对于 here-doc delimiter，是否被引号包裹（true/false）
    struct s_redir *next; // 下一个重定向
} t_redir;

/* 单条命令节点结构体 */
typedef struct s_cmd
{
    /* 原始参数数组（未做 expansion / 通配符 / 引号处理） */
    char **orig_argv; // 参数字符串数组（含命令名自己）
    size_t orig_argc; // 参数数量

    /* 重定向链表 */
    t_redir *redirs;

    /* Here‐doc 标志 / 辅助字段 */
    bool has_heredoc; // 是否有 here-doc 重定向
    int heredoc_fd;   // 读取 here-doc 内容后的文件描述符（供执行时 dup2）
    // （redir 链表里对应节点会有 delim_quoted 信息）

    /* 内建函数指针；非 builtin 则为 NULL */
    t_builtin_fn builtin_fn;

    /* 双向链表指针：prev / next */
    struct s_cmd *prev;
    struct s_cmd *next;

    /* （可选）扩展后使用的 argv 数组 */
    char **exp_argv; // expansion 之后的参数数组
    size_t exp_argc;
} t_cmd;

typedef struct s_token
{
    TokenType type;
    char *text;
} t_token;

typedef struct s_tokenlist
{
    t_token *tokens;
    int n;
    int pos;
} t_tokenlist;

typedef struct s_shell
{
    char *raw_line; // 原始输入行
    t_cmd *cmds;    // 命令链表头

    int n_pipes; // 管道 “|” 的个数（cmd 数 - 1）

    int last_exit_status; // 上一条命令退出状态（用于 $? 扩展）

    char **envp; // 环境变量数组（或指向你的环境结构）

    /* 可以在这里加更多全局状态字段，比如信号处理*/
} t_shell;