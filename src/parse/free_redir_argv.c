#include "minishell.h"
#include "parse.h"

/*
 * free_redir_list
 * 作用：释放整个 redir 链表，并关闭 heredoc_fd（如果有）
 */
void free_redir_list(t_redir *r)
{
    t_redir *next;

    while (r)
    {
        next = r->next;

        /* heredoc_fd 如果是有效 fd，关闭它避免 fd 泄漏 */
        if (r->heredoc_fd >= 0)
            close(r->heredoc_fd);

        /* filename 是 malloc/strdup 出来的，要 free */
        if (r->filename)
            free(r->filename);

        free(r);
        r = next;
    }
}

/*
 * free_argv_list
 * 作用：释放 argv 临时链表（包括每个 content 字符串）
 * 注意：这个函数用于“还没转换成 argv 数组之前”的失败路径。
 */
void free_argv_list(t_cmd *a)
{
    t_cmd *next;

    while (a)
    {
        next = a->next;

        /* content 是 char*，是 ft_strdup 出来的 */
        if (a->content)
            free(a->content);

        free(a);
        a = next;
    }
}

/*
 * free_t_cmd_node
 * 作用：只释放 argv 链表节点结构体，不释放 content
 * 用在 build_argvs 成功把 content 转移到 argv 数组之后
 */
void free_t_cmd_node(t_cmd *argv_cmd)
{
    t_cmd *tmp;
    t_cmd *next;

    tmp = argv_cmd;
    while (tmp)
    {
        next = tmp->next;
        free(tmp);
        tmp = next;
    }
}
