
#include "minishell.h"
#include "parse.h"

/*
 * parse_subshell
 * 作用：解析形如 "( pipeline )" 的子 shell
 * 参数：
 * - cur：token 流指针地址
 * - node：已经 malloc/calloc 好的 t_ast 节点（parse_simple_cmd_redir_list 传入）
 * - minishell：全局上下文
 * 返回：成功返回 node；失败返回 NULL（并释放 node）
 */
t_ast *parse_subshell(t_lexer **cur, t_ast *node, t_minishell *minishell)
{
    /* 吃掉 '(' */
    consume_token(cur);

    /* 标记这个 AST 节点是 subshell */
    node->type = NODE_SUBSHELL;

    /* 解析括号内部：你直接复用 parse_pipeline（括号里允许管道） */
    node->sub = parse_pipeline(cur, minishell);

    /* 解析完必须遇到 ')'，否则语法错 */
    if (!expect_token(TOK_RPAREN, cur))
    {
        ft_putstr_fd("Syntax error: expected ')'\n", 2);
        free_ast(node);
        return NULL;
    }

    return node;
}
