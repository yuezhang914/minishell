// 这个文件是“把重定向 token 变成结构体”的工厂。

#include "minishell.h"
#include "parse.h"

/*
 * create_redir
 * 作用：创建并初始化一个 t_redir 节点
 * 参数：
 * - type：lexer 的 tokentype（TOK_REDIR_IN / TOK_REDIR_OUT / TOK_APPEND / TOK_HEREDOC）
 * - filename：对 < > >> 是文件名；对 << 是 delimiter
 * 返回：新分配的 t_redir*；失败返回 NULL
 */
static t_redir *create_redir(tok_type type, char *filename)
{
    t_redir *new_node;

    /* calloc：分配并清零（避免字段是脏值） */
    new_node = ft_calloc(1, sizeof(t_redir));
    if (!new_node)
        return (NULL);

    /* next 默认 NULL（calloc 已清零） */
    new_node->next = NULL;

    /* filename：这里用 safe_strdup，确保我们自己拥有一份内存 */
    new_node->filename = safe_strdup(filename);
    if (!new_node->filename)
        return (free(new_node), NULL);

    /* heredoc_fd 初始为 -1：表示“还没准备好” */
    new_node->heredoc_fd = -1;

    /* 默认：还没展开 */
    new_node->is_expanded = false;

    /* 把 lexer 的 token 类型映射到 redir_type */
    if (type == TOK_REDIR_IN)
        new_node->type = REDIR_INPUT;
    else if (type == TOK_REDIR_OUT)
        new_node->type = REDIR_OUTPUT;
    else if (type == TOK_APPEND)
        new_node->type = REDIR_APPEND;
    else if (type == TOK_HEREDOC)
        new_node->type = HEREDOC;

    return (new_node);
}

/*
 * redirlst_add_back
 * 作用：把新 redir 节点追加到 redir 链表末尾
 * 参数：
 * - lst：redir 链表头指针的地址（因为可能要改头）
 * - new_node：要加进去的新节点
 */
static void redirlst_add_back(t_redir **lst, t_redir *new_node)
{
    t_redir *tmp;

    if (!lst || !new_node)
        return;

    /* 空链表：直接把新节点作为头 */
    if (*lst == NULL)
    {
        *lst = new_node;
        return;
    }

    /* 找到末尾 */
    tmp = *lst;
    while (tmp->next)
        tmp = tmp->next;

    /* 尾插 */
    tmp->next = new_node;
}

/*
 * consume_redir_pair
 * 作用：从 token 流里吃掉 “重定向操作符 + 文件名/终止符”
 * 例如：吃掉 "<" + "in.txt"
 * 参数：
 * - cur：token 流指针地址（会前进）
 * - op：输出：操作符 token（比如 TOK_REDIR_IN）
 * - filetok：输出：文件名 token（必须是 TOK_WORD）
 * - ms：全局状态（这里用于错误处理/保持一致接口）
 * 返回：成功 1；失败 0（比如缺少文件名）
 */
static int consume_redir_pair(t_lexer **cur, t_lexer **op,
                              t_lexer **filetok, t_minishell *ms)
{
    (void)ms; /* 这份代码里 ms 没用到，但保留参数方便以后扩展 */

    /* op = 当前 token（应该是 < > >> <<） */
    *op = consume_token(cur);
    if (!*op)
        return (0);

    /* filetok = 下一个 token（必须是 TOK_WORD） */
    *filetok = peek_token(cur);
    if (!*filetok || (*filetok)->tokentype != TOK_WORD)
    {
        ft_putstr_fd("minishell: syntax error near unexpected token\n", STDERR_FILENO);
        return (0);
    }

    /* 吃掉文件名 token */
    consume_token(cur);

    return (1);
}

/*
 * build_redir
 * 作用：解析并构建一个 redirection 节点，追加到 redir_list
 * 参数：
 * - cur：token 流指针地址
 * - redir_list：redir 链表头指针地址
 * - ms：全局上下文（用于 heredoc 的 quoted 标记等）
 * 返回：
 * - 成功 1
 * - 失败 0（语法错或 malloc 失败）
 */
int build_redir(t_lexer **cur, t_redir **redir_list, t_minishell *ms)
{
    t_lexer *op;
    t_lexer *filetok;
    t_redir *new_redir;
    char    *text;

    new_redir = NULL;
    text = NULL;

    /* 先吃掉两件套：操作符 + 文件名/终止符 */
    if (!consume_redir_pair(cur, &op, &filetok, ms))
        return (0);

    /* 默认取 raw（含引号原样） */
    text = filetok->raw;

    /* heredoc 的 delimiter：你这里改用 filetok->str（去掉引号的） */
    if (op->tokentype == TOK_HEREDOC)
        text = filetok->str;

    /* 创建 redir 节点（会 safe_strdup 复制一份 text） */
    new_redir = create_redir(op->tokentype, text);
    if (!new_redir)
        return (0);

    /* heredoc：记录 delimiter 是否带引号（带引号就不展开变量） */
    if (op->tokentype == TOK_HEREDOC)
        new_redir->quoted = filetok->had_quotes;

    /* 加到链表末尾 */
    redirlst_add_back(redir_list, new_redir);

    return (1);
}
