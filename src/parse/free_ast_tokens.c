#include "minishell.h"
#include "parse.h"

/*
 * free_ast_partial
 * 作用：释放 AST 节点（递归释放子树），用于“构建到一半失败”的情况
 * 这里会释放：
 * - left/right/sub 子树
 * - argv 数组（以及里面字符串）
 * - redir 链表
 */
void free_ast_partial(t_ast *node)
{
    int i;

    if (!node)
        return;

    /* 先释放子树 */
    free_ast_partial(node->left);
    free_ast_partial(node->right);
    free_ast_partial(node->sub);

    /* 释放 argv 数组及其字符串 */
    if (node->argv)
    {
        i = 0;
        while (node->argv[i])
        {
            free(node->argv[i]);
            i++;
        }
        free(node->argv);
    }

    /* 释放 redir 链表 */
    if (node->redir)
        free_redir_list(node->redir);

    free(node);
}

/*
 * free_ast
 * 作用：释放完整 AST（和 free_ast_partial 类似）
 * 你这里可能是对外暴露的接口
 */
void free_ast(t_ast *node)
{
    free_ast_partial(node);
}

/*
 * free_tokens
 * 作用：释放 lexer token 链表
 *
 * ⚠️ 风险提醒：
 * 你的 lexer 可能出现 tok->raw == tok->str（同一块内存）。
 * 这里是分别 free(tok->str) 和 free(tok->raw)，会 double free。
 * 如果你项目里真的用它释放 lexer 结果，要改成和 lexer_clear.c 一样的判断。
 */
void free_tokens(t_lexer *tok)
{
    while (tok)
    {
        t_lexer *nx = tok->next;

        if (tok->str)
            free(tok->str);

        if (tok->raw)
            free(tok->raw);

        free(tok);
        tok = nx;
    }
}
