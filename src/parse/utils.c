// 这个文件就是“解析器的手”：
// peek_token：偷看下一个 token（不前进）
// consume_token：吃掉一个 token（指针前进）
// expect_token：必须是某种 token（否则语法错）
// is_redir_token：判断是否是 < > >> <<
// safe_strdup：复制字符串但更安全（失败会报错）

#include "minishell.h"
#include "parse.h"
#include "error.h"

/*
 * peek_token
 * 作用：偷看当前 token（不改变 *cur）
 * 参数：cur = “当前 token 指针”的地址（因为别的函数会改它）
 * 返回：当前 token 节点指针；如果 cur 无效或流为空返回 NULL
 */
t_lexer *peek_token(t_lexer **cur)
{
    /* 如果 cur 本身是 NULL，说明调用者传参有问题，直接返回 NULL 防崩 */
    if (!cur)
        return NULL;

    /* 返回当前节点（注意：不移动指针） */
    return *cur;
}

/*
 * consume_token
 * 作用：吃掉（消耗）当前 token，让 *cur 前进到 next
 * 参数：cur = 当前 token 指针地址
 * 返回：被吃掉的旧 token 指针（有些解析器会用它做检查/调试）
 */
t_lexer *consume_token(t_lexer **cur)
{
    t_lexer *pt;

    if (!cur || !*cur)
        return NULL;

    /* pt 先记住当前 token */
    pt = *cur;

    /* *cur 前进到下一个 token */
    *cur = (*cur)->next;

    /* 返回旧 token（可选用） */
    return pt;
}

/*
 * expect_token
 * 作用：要求当前 token 必须是某个类型，否则语法错误
 * 参数：
 * - type：期待的 tok_type，比如 TOK_RPAREN
 * - cur：当前 token 指针地址
 * 返回：
 * - 如果匹配：返回 consume_token 后的旧 token
 * - 不匹配：返回 NULL（并打印错误）
 */
t_lexer *expect_token(tok_type type, t_lexer **cur)
{
    t_lexer *pt;

    pt = peek_token(cur);

    /* pt 为空：说明 token 流结束了，但我们还想要一个 token → 语法错 */
    if (!pt)
    {
        ft_putstr_fd("Syntax error: unexpected end of input\n", STDERR_FILENO);
        return NULL;
    }

    /* 类型不匹配 → 语法错 */
    if (pt->tokentype != type)
    {
        ft_putstr_fd("Syntax error: unexpected token\n", STDERR_FILENO);
        return NULL;
    }

    /* 匹配：吃掉并返回旧 token */
    return consume_token(cur);
}

/*
 * is_redir_token
 * 作用：判断 token 是否是重定向符号：< > >> <<
 * 参数：pt = 当前 token
 * 返回：1 表示是；0 表示不是
 */
int is_redir_token(t_lexer *pt)
{
    /* 注意：这里默认 pt 不为 NULL（调用者一般先 peek_token 再判断） */
    if (pt->tokentype == TOK_REDIR_IN ||
        pt->tokentype == TOK_REDIR_OUT ||
        pt->tokentype == TOK_APPEND ||
        pt->tokentype == TOK_HEREDOC)
        return 1;
    else
        return 0;
}

/*
 * safe_strdup
 * 作用：更安全的字符串复制
 * - 如果 s 为 NULL：直接返回 NULL
 * - 如果分配失败：打印错误信息（方便你调试“为什么突然没内存”）
 */
char *safe_strdup(const char *s)
{
    /* 输入保护：NULL 就不复制 */
    if (!s)
        return NULL;

    /* ft_strdup 会 malloc 一块新内存并复制内容 */
    char *p = ft_strdup(s);

    /* 如果内存分配失败，给出提示（stderr） */
    if (!p)
        fprintf(stderr, "memory error: strdup failed\n");

    return p;
}
