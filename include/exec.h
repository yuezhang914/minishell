/* ************************************************************************** */
/* exec.h：执行阶段总头文件
 *
 * 作用：
 * 1) 统一 include 执行阶段需要的库/头文件
 * 2) 定义执行阶段会用到的结构体（比如管道上下文 t_pipe_ctx）
 * 3) 声明执行阶段的函数接口
 */
/* ************************************************************************** */

#ifndef EXEC_H
# define EXEC_H

/* libft：这里会用到
 * - ft_bzero（清零结构体）
 * - ft_strjoin / ft_strdup 等（PATH 拼接）
 */
# include "../libft/libft.h"

/* build_in.h：这里会用到
 * - t_env 环境变量链表（builtin 和 envp 更新）
 */
# include "build_in.h"

/* error.h：这里会用到
 * - ms_err_redir（重定向打开失败时打印错误）
 */
# include "error.h"

/* parse.h：这里会用到
 * - t_ast / t_redir / NODE_PIPE / NODE_CMD / HEREDOC 等结构与枚举
 */
# include "parse.h"

/* signals.h：这里会用到
 * - heredoc 的信号保存/恢复、专用信号行为（你 handle_heredoc 用）
 */
# include "signals.h"

/* 系统库：你 exec 阶段用到的系统调用/宏都来自这些 */
# include <errno.h>      /* errno */
# include <fcntl.h>      /* open 的 O_RDONLY/O_CREAT/... */
# include <stdlib.h>     /* malloc/free */
# include <string.h>     /* strerror（如果你 error 模块用） */
# include <sys/stat.h>   /* stat + S_ISDIR */
# include <sys/wait.h>   /* waitpid + WIFEXITED/WEXITSTATUS/... */
# include <unistd.h>     /* pipe/fork/dup/dup2/close/access */

/* 保存标准输入输出用（跑 builtin 且带重定向时） */
typedef struct s_fd_save
{
	int	in;     /* 备份的 stdin（dup 得到的新 fd） */
	int	out;    /* 备份的 stdout（dup 得到的新 fd） */
}	t_fd_save;

/* 管道执行上下文：把 pipeline 摊平后统一管理 */
typedef struct s_pipe_ctx
{
	t_minishell	*msh;   /* 全局 shell 状态（取 env/退出码/等） */
	t_ast		**arr;  /* pipeline 摊平后的命令数组：[cmd0,cmd1,...] */
	pid_t		*pids;  /* 每个 cmd fork 出来的 pid 记录在这里 */
	t_ast		*root;  /* 整条 pipeline 的根（用于子进程失败时清理） */
	int			n;      /* pipeline 段数（命令个数） */
	int			in_fd;  /* 当前这段命令的输入口（上一段 pipe 的读端） */
	int			out_fd; /* 整条 pipeline 最终输出口（可能是文件/屏幕） */
}	t_pipe_ctx;

/* ===== 下面这些声明里，有一部分实现文件你这批没上传（我已在开头说明） ===== */
int		exec_t_ast(t_minishell *msh, t_ast *root);
int		exec_cmd_root(t_minishell *msh, t_ast *node);
int		exec_pipe_root(t_minishell *msh, t_ast *node);

int		exec_cmd_node(t_minishell *msh, t_ast *node, int in_fd, int out_fd);
int		exec_pipe_node(t_minishell *msh, t_ast *node, int in_fd, int out_fd);

int		pipe_collect(t_ast *root, t_ast ***out_arr, int *out_n);

/* 子进程执行一个命令：通常会在里面做 dup2(in/out) + execve/builtin */
void	child_exec_one(t_minishell *msh, t_ast *node, int in_fd, int out_fd, t_ast *root);

/* ===== 你这批有实现的接口（下面会逐行讲） ===== */
int		apply_redir_list(t_redir *r, int *in_fd, int *out_fd);
int		dup_in_out_or_close(int in_fd, int out_fd);

char	*find_cmd_path(t_minishell *msh, const char *cmd);

int		save_std_fds(t_fd_save *save);
void	restore_std_fds(t_fd_save *save);
void	set_status_from_wait(t_minishell *msh, int st);
int		wait_pair_set_right(t_minishell *msh, pid_t left, pid_t right);

int		prepare_heredocs(t_ast *node, t_minishell *ms);
int		run_builtin_parent_logic(t_minishell *msh, t_ast *node, int in_fd, int out_fd);

int		path_is_dir(const char *path);
int		has_bad_heredoc(t_redir *r);
void	close_all_heredoc_fds(t_ast *node);

void	wait_all_and_set_lt_ast(t_minishell *msh, pid_t *pids, int n);
int		pipe_make(int i, int n, int out_fd, int pfd[2]);
void	handle_parent_pipes(t_pipe_ctx *ctx, int pfd[2]);

#endif
