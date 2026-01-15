#include "exec.h"
#include "minishell.h"

/*
 * close_keep_std（这里传 int*）
 * 作用：只关闭“不是 0/1/2”的 fd，并把它置为 -1
 * 为什么传指针？
 * - 你要把外面的变量也改掉，防止后面重复 close（double close）
 */
static void	close_keep_std(int *fd)
{
	if (!fd)
		return ;
	if (*fd > STDERR_FILENO) /* >2 才是“额外 fd” */
	{
		close(*fd);
		*fd = -1;
	}
}

/*
 * open_redir_fd
 * 作用：对单个重定向节点 r，打开对应 fd
 * - <  : 只读打开文件
 * - >  : 创建/覆盖写
 * - >> : 创建/追加写
 * - << : heredoc：你这里不是 open 文件，而是 dup 一份 heredoc_fd
 *
 * 为什么 heredoc 要 dup？
 * - r->heredoc_fd 是预处理阶段准备好的“读端”
 * - dup 一份给本次执行使用：这样执行阶段 close 这个 fd，不会影响原 fd 的生命周期管理
 */
static int	open_redir_fd(t_redir *r)
{
	int	fd;

	fd = -1;
	if (r->type == REDIR_INPUT)
		fd = open(r->filename, O_RDONLY);
	else if (r->type == REDIR_OUTPUT)
		fd = open(r->filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	else if (r->type == REDIR_APPEND)
		fd = open(r->filename, O_CREAT | O_WRONLY | O_APPEND, 0644);
	else if (r->type == HEREDOC && r->heredoc_fd >= 0)
		fd = dup(r->heredoc_fd);
	return (fd);
}

/*
 * apply_redir_list
 * 作用：按链表顺序应用所有重定向，算出最终的 in_fd/out_fd
 *
 * 参数：
 * - r：重定向链表头
 * - in_fd/out_fd：传入“当前环境的输入输出口”（可能来自 pipe）
 *                 函数内会更新成最终要接到 stdin/stdout 的 fd
 *
 * 关键细节：
 * - 遇到新的输入重定向（< 或 <<）：会先关掉旧的 *in_fd（如果不是 0/1/2）
 * - 遇到新的输出重定向（> 或 >>）：会先关掉旧的 *out_fd
 * - 中途 open 失败：打印错误，并把已经打开的 in/out fd 都关掉
 */
int	apply_redir_list(t_redir *r, int *in_fd, int *out_fd)
{
	int	fd;

	while (r)
	{
		/* 这一步要开“新的输入口”还是“新的输出口”？ */
		if (r->type == REDIR_INPUT || r->type == HEREDOC)
			close_keep_std(in_fd);   /* 关闭旧输入口，避免泄漏 */
		else
			close_keep_std(out_fd);  /* 关闭旧输出口 */

		fd = open_redir_fd(r);
		if (fd < 0)
		{
			/* errno：open 失败原因（比如 Permission denied） */
			return (ms_err_redir(r->filename ? r->filename : "redir", errno),
				close_keep_std(in_fd), close_keep_std(out_fd), -1);
		}

		/* 更新最终 in/out */
		if (r->type == REDIR_INPUT || r->type == HEREDOC)
			*in_fd = fd;
		else
			*out_fd = fd;

		r = r->next;
	}
	return (0);
}
