#include "minishell.h"
#include "parse.h"

/*
 * check_consecutive_pipes（static）
 * 作用：检查是否出现连续 pipe，比如： "ls ||" 或 "| |"
 * 你的 minishell 不支持逻辑 OR，所以 "||" 这里会被看成连续管道符错误。
 * 返回：发现错误返回 -1；正常返回 0
 */
static int check_consecutive_pipes(t_lexer **cur)
{
    t_lexer *pt;

    pt = peek_token(cur);

    /* pt->next 是下一个 token，如果也是 TOK_PIPE 就是连续 */
    if (pt && pt->next && pt->next->tokentype == TOK_PIPE)
    {
        ft_putstr_fd("minishell: syntax error near unexpected token `|'\n", STDERR_FILENO);
        return (-1);
    }
    return 0;
}

/*
 * make_pipe_node（static）
 * 作用：创建一个 NODE_PIPE 节点，把 left/right 接上
 * 同时 *n_pipes++ 用于统计管道数量
 */
static t_ast *make_pipe_node(t_ast *left, t_ast *right, int *n_pipes)
{
    t_ast *node;

    node = ft_calloc(1, sizeof(t_ast));
    if (!node)
        return (NULL);

    node->type = NODE_PIPE;
    node->left = left;
    node->right = right;

    /* 管道数量 +1：执行器后面会用这个数量去创建 pipe fd 数组 */
    (*n_pipes)++;

    return node;
}

/*
 * parse_pipeline_1（static）
 * 作用：在已经解析出 left 的情况下，继续处理后续的 | right | right...
 * 参数：
 * - cur：token 流指针地址
 * - left：当前已经构建好的左子树（会不断变成更大的树）
 * - n_pipes：管道数量计数器地址
 * - minishell：全局上下文
 */
static t_ast *parse_pipeline_1(t_lexer **cur, t_ast **left, int *n_pipes,
                               t_minishell *minishell)
{
    t_ast *right;
    t_ast *node;

    /* 只要下一个 token 还是 PIPE，就说明还有管道连接 */
    while (peek_token(cur) && peek_token(cur)->tokentype == TOK_PIPE)
    {
        /* 防连续 pipe */
        if (check_consecutive_pipes(cur) == -1)
            return (free_ast(*left), NULL);

        /* 吃掉 '|' */
        consume_token(cur);

        /* 解析右侧的简单命令 */
        right = parse_simple_cmd_redir_list(cur, minishell);
        if (!right)
            return (free_ast(*left), NULL);

        /* 把 left 和 right 拼成一个新的 pipe 节点 */
        node = make_pipe_node(*left, right, n_pipes);
        if (!node)
            return (free_ast(*left), free_ast(right), NULL);

        /* 新树成为 left，继续看后面是否还有 | */
        *left = node;
    }

    return (*left);
}

/*
 * parse_pipeline
 * 作用：pipeline 的入口：解析第一个 simple_cmd，然后不断吸收后续 | simple_cmd
 * 返回：pipeline 的根节点（可能是 CMD，也可能是 PIPE 树）
 */
t_ast *parse_pipeline(t_lexer **cur, t_minishell *minishell)
{
    t_ast *left;
    int   n_pipes;

    /* 如果一上来就是 '|'，这是语法错 */
    if (peek_token(cur) && peek_token(cur)->tokentype == TOK_PIPE)
    {
        ft_putstr_fd("minishell: syntax error near unexpected token `|'\n",
                     STDERR_FILENO);
        return NULL;
    }

    /* 先解析左边第一个命令 */
    left = parse_simple_cmd_redir_list(cur, minishell);
    if (!left)
        return NULL;

    n_pipes = 0;

    /* 把后续管道都吃掉并建树 */
    t_ast *result = parse_pipeline_1(cur, &left, &n_pipes, minishell);
    if (!result)
        return NULL;

    /* 记录本行管道数量到 minishell（执行器用） */
    minishell->n_pipes = n_pipes;

    return result;
}
