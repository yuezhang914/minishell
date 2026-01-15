// build_argvs.c（把 argv 临时链表变成 char**）
// 你解析时把 WORD 参数先放进 t_list 链表（t_cmd），
// 最后再变成 char **argv（以 NULL 结尾），执行器用起来更方便。

#include "minishell.h"
#include "parse.h"

/*
 * create_argv
 * 作用：把一个字符串变成 argv 链表节点（t_cmd）
 * 参数：str = token 的字符串
 * 返回：新链表节点；失败返回 NULL
 */
t_cmd *create_argv(char *str)
{
    char *dup;

    if (!str)
        return NULL;

    /* 复制一份字符串：让 argv 自己拥有内存（不会依赖 lexer token 的生命周期） */
    dup = ft_strdup(str);
    if (!dup)
        return NULL;

    /* ft_lstnew：创建链表节点，content 指向 dup */
    return ft_lstnew(dup);
}

/*
 * build_argvs
 * 作用：把 argv 链表转换成 char **argv（以 NULL 结尾）
 *
 * 参数：
 * - argv_cmd：临时 argv 链表（每个节点 content 是 char*）
 * - redir：如果出错要一起释放
 * - node：如果出错要 free node
 *
 * 返回：
 * - 成功：argv 数组（argv[i] 指向字符串）
 * - 失败：返回 NULL（并清理 redir/argv_cmd/node）
 *
 * 注意：你这里“只 free 链表节点结构体”，不 free content 字符串，
 * 因为 content 指针被转移到 argv 数组里继续使用。
 */
char **build_argvs(t_cmd *argv_cmd, t_redir *redir, t_ast *node)
{
    int     size;
    char    **argvs;
    int     i;
    t_cmd   *tmp;

    /* ft_lstsize：数链表节点个数（参数个数） */
    size = ft_lstsize(argv_cmd);

    /* +1 是为了最后的 NULL 结束符 */
    argvs = malloc((size + 1) * sizeof(char *));
    if (!argvs)
        return (free_redir_list(redir), free_argv_list(argv_cmd), free(node), NULL);

    /* 把链表 content 指针搬到数组里 */
    i = 0;
    tmp = argv_cmd;
    while (tmp && i < size)
    {
        argvs[i++] = tmp->content; /* content 是 char* */
        tmp = tmp->next;
    }

    /* argv 规定必须以 NULL 结尾 */
    argvs[i] = NULL;

    /* 释放链表节点（只 free 节点，不 free content） */
    tmp = argv_cmd;
    free_t_cmd_node(tmp);

    return (argvs);
}
