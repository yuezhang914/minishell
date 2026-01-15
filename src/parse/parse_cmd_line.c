#include "minishell.h"
#include "parse.h"

/*
 * parse_cmdline
 * 作用：parse 的总入口
 * - 先解析 pipeline
 * - 再检查 token 流是否结束（必须是 TOK_END）
 */
t_ast *parse_cmdline(t_lexer **cur, t_minishell *minishell)
{
    t_ast   *root;
    t_lexer *pt;

    /* 先解析 pipeline（可能返回 CMD 节点，也可能返回 PIPE 树） */
    root = parse_pipeline(cur, minishell);
    if (!root)
        return NULL;

    /* 看看是不是已经到结尾 token */
    pt = peek_token(cur);

    /* 如果还有 token 且不是 TOK_END：说明解析后还有多余 token → 语法错 */
    if (pt && pt->tokentype != TOK_END)
    {
        /* ⚠️ 风险点：你这里把 enum 强转成 char*，很可能段错误！
         * ms_put3 估计是拼接 3 个字符串输出，但第二个参数必须是字符串指针。
         * 正确方式应该把 pt->tokentype 转成字符串或数字打印。
         */
        ms_put3("Syntax error: unexpected token at end (type ",
                (char *)(pt->tokentype),  /* ⚠️ 这里几乎肯定不对 */
                "\n");

        /* 已经构建了一部分树了，必须释放避免泄漏 */
        free_ast(root);
        return NULL;
    }

    return root;
}
