#include "minishell.h"
#include "parse.h"

/*
 * handle_redir_token（static）
 * 作用：当当前 token 是重定向符号时，调用 build_redir 解析并追加到 redir 链表
 * 返回：成功 1；失败 0
 */
static int handle_redir_token(t_lexer **cur, t_redir **redir, t_minishell *minishell)
{
    int result;

    /* build_redir 会消耗：操作符 token + 文件名 token */
    result = build_redir(cur, redir, minishell);

    /* result==0 表示语法错或内存失败 */
    if (result == 0)
        return 0;

    return 1;
}

/*
 * handle_word_token（static）
 * 作用：当当前 token 是 WORD 时，把它变成 argv 链表节点并追加到 argv_cmd
 * 说明：argv_cmd 是 libft 的 t_list 链表（这里 typedef 成 t_cmd）
 */
static void handle_word_token(t_lexer **cur, t_cmd **argv_cmd)
{
    t_lexer *pt;
    t_cmd   *new;

    /* pt 是当前 WORD token */
    pt = consume_token(cur);

    /* create_argv 会 ft_strdup 一份字符串，封装成链表节点 */
    new = create_argv(pt->str);

    /* 追加到 argv_cmd 链表末尾 */
    if (new)
        ft_lstadd_back(argv_cmd, new);
}

/*
 * has_no_argv_and_no_redir（static）
 * 作用：如果一个命令节点里既没有 argv（没有任何 WORD），也没有 redir，那它是“空命令”
 * 比如输入： "|" 后面啥也没有 或者 ">" 后面啥也没有
 * 这种情况应该语法错误并返回 NULL
 */
static int has_no_argv_and_no_redir(t_cmd *argv_cmd, t_redir *redir, t_minishell *minishell)
{
    (void)minishell;

    if (!argv_cmd && !redir)
    {
        ft_putstr_fd("minishell: syntax error: empty command\n", STDERR_FILENO);
        return 1;
    }
    return 0;
}

/*
 * parse_normal_cmd_redir_list
 * 作用：解析普通命令（不是 subshell）：
 * - 读取 WORD，放进 argv_cmd 链表
 * - 读取重定向 token，build_redir 挂到 redir 链表
 * 直到遇到不是 WORD/REDIR 的 token（比如 | 或 ) 或 END）
 */
t_ast *parse_normal_cmd_redir_list(t_lexer **cur, t_ast *node, t_minishell *minishell)
{
    t_cmd   *argv_cmd;
    t_redir *redir;
    t_lexer *pt;

    /* 初始化临时链表为空 */
    argv_cmd = NULL;
    redir = NULL;

    /* 命令节点类型 */
    node->type = NODE_CMD;

    /* 只要还有 token，就继续看 */
    while ((pt = peek_token(cur)) != NULL)
    {
        /* 如果当前是重定向符号：< > >> << */
        if (is_redir_token(pt))
        {
            if (!handle_redir_token(cur, &redir, minishell))
                return NULL; /* build_redir 已打印错误 */
        }
        /* 如果当前是普通单词：加入 argv */
        else if (pt->tokentype == TOK_WORD)
        {
            handle_word_token(cur, &argv_cmd);
        }
        else
        {
            /* 遇到 | 或 ) 或 END：说明这个“简单命令”解析结束 */
            break;
        }
    }

    /* 如果既没有 argv 也没有 redir，这是一个空命令，按语法错处理 */
    if (has_no_argv_and_no_redir(argv_cmd, redir, minishell))
        return NULL;

    /* 把临时 redir 链表挂到节点上 */
    node->redir = redir;

    /* 把 argv 链表变成 char**，并释放 argv 链表节点结构体 */
    node->argv = build_argvs(argv_cmd, redir, node);

    return node;
}
