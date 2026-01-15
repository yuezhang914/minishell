#include "minishell.h"
#include "parse.h"

/*
 * parse_simple_cmd_redir_list
 * 作用：解析一个“简单命令单元”
 * - 如果看到 '('：解析 subshell
 * - 否则：解析普通命令（word/redir 混合）
 *
 * 注意：它会先 calloc 一个 t_ast 节点（全清零），再交给具体分支填字段
 */
t_ast *parse_simple_cmd_redir_list(t_lexer **cur, t_minishell *minishell)
{
    t_ast   *node;
    t_lexer *pt;

    /* 偷看当前 token */
    pt = peek_token(cur);

    /* 分配一个 AST 节点并清零（避免 left/right/sub 指针是垃圾） */
    node = ft_calloc(1, sizeof(t_ast));
    if (!node)
        return (NULL);

    /* 如果当前 token 是 '('，就是 subshell */
    if (pt && pt->tokentype == TOK_LPAREN)
        return (parse_subshell(cur, node, minishell));

    /* 否则就是普通命令 */
    return (parse_normal_cmd_redir_list(cur, node, minishell));
}
