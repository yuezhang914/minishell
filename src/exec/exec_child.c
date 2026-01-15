// exec_child.c（子进程真正执行：重定向+dup2+execve；以及 126/127 规则）
// 这里会出现你要的“新函数/宏”和“errno 映射”：
// errno
// EACCES / EISDIR
// execve
// #! shebang + “bad interpreter”模拟
// close_all_heredoc_fds 为什么要做

#include "exec.h"
#include "minishell.h"

/*
 * read_shebang
 * 作用：读取文件前 199 字节，判断是不是以 "#!" 开头（脚本）
 * 参数：
 * - path：文件路径
 * - buf：外部提供的缓冲区，用于保存读到的内容
 * 返回：
 * - 是 "#!" 开头返回 1，否则 0
 *
 * 细节：
 * - read(fd, buf, 199)：最多读 199 字节
 * - buf[n] = '\0'：补结束符，方便当字符串分析
 */
static int	read_shebang(const char *path, char *buf)
{
	int		fd;
	ssize_t	n;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return (0);

	n = read(fd, buf, 199);
	close(fd);

	if (n < 2)
		return (0);

	buf[n] = '\0';
	return (buf[0] == '#' && buf[1] == '!');
}

/*
 * try_print_bad_interpreter
 * 场景：execve 失败，errno == EACCES（没权限执行）
 * 你这里额外模拟 bash 的一种提示：
 * - 如果文件是脚本（#! 开头）
 * - 并且脚本写了一个解释器路径（比如 #!/bin/xx）
 * - 那个解释器可能不存在/不可用
 * 于是打印 "bad interpreter: xxx"
 *
 * 返回：
 * - 打印了这种特殊错误 → 返回 1（后面 exit 126）
 * - 不适用 → 返回 0（走普通错误打印）
 */
static int	try_print_bad_interpreter(const char *cmd, const char *path,
		int err)
{
	char	buf[200];
	int		i;
	int		j;

	if (!cmd || !path || err != EACCES)
		return (0);

	if (!read_shebang(path, buf))
		return (0);

	/* 跳过 "#!" 后面的空格/制表符 */
	i = 2;
	while (buf[i] == ' ' || buf[i] == '\t')
		i++;

	/* j 向后走，拿到解释器路径的结尾（遇到空格/换行停止） */
	j = i;
	while (buf[j] && buf[j] != ' ' && buf[j] != '\t' && buf[j] != '\n')
		j++;

	if (j == i)
		return (0);

	buf[j] = '\0';                     /* 让 buf[i..j-1] 成为一个独立字符串 */
	ms_err_bad_interpreter(cmd, &buf[i]);
	return (1);
}

/*
 * handle_exec_exit
 * 作用：execve 失败后的“统一善后出口”
 * 关键：
 * - errno 保存着 execve 为什么失败
 * - 退出码遵循 bash 习惯：
 *   - 127：command not found（这里主要由上层 find_cmd_path==NULL 处理）
 *   - 126：found but cannot execute（EACCES / EISDIR 等）
 */
static void	handle_exec_exit(t_minishell *msh, t_ast *root, char *path,
		char **argv)
{
	int	err;
	int	code;

	err = errno;

	/* 特判：脚本 bad interpreter（你这里模仿 bash 的错误信息） */
	if (try_print_bad_interpreter(argv[0], path, err))
	{
		free(path);
		ms_child_exit(msh, root, 126);
	}

	/* 普通 exec 错误：比如 Permission denied / Exec format error 等 */
	ms_err_exec(argv[0], err);

	code = 127;                 /* 默认认为像“找不到/不可运行” */
	if (err == EACCES || err == EISDIR)
		code = 126;              /* 无权限 / 是目录：明确是“找到了但不能执行” */

	free(path);
	ms_child_exit(msh, root, code);
}

/*
 * child_exec_external
 * 作用：子进程执行外部命令
 * 流程：
 * 1) change_envp：把 env 链表同步成 envp 数组（execve 需要 envp）
 * 2) find_cmd_path：用 PATH 找到可执行文件路径
 * 3) path_is_dir：如果是目录，报错并 exit 126
 * 4) execve：变身（成功不会返回）
 * 5) execve 返回说明失败 → handle_exec_exit
 */
static void	child_exec_external(t_minishell *msh, t_ast *node, t_ast *root)
{
	char	**argv;
	char	*path;

	argv = node->argv;
	if (!argv || !argv[0])
		ms_child_exit(msh, root, 0);

	change_envp(msh->env, &msh->envp);

	path = find_cmd_path(msh, argv[0]);
	if (!path)
	{
		ms_err_cmd_not_found(argv[0]);
		ms_child_exit(msh, root, 127);
	}

	if (path_is_dir(path))
	{
		ms_err_exec(argv[0], EISDIR);
		free(path);
		ms_child_exit(msh, root, 126);
	}

	execve(path, argv, msh->envp);

	/* execve 返回了：说明失败 */
	handle_exec_exit(msh, root, path, argv);
}

/*
 * child_exec_one
 * 作用：子进程执行一个命令节点（包含 builtin / external）
 * 参数：
 * - in_fd/out_fd：来自“管道环境”的输入输出口
 * - root：整棵 AST（用于清理与关闭 heredoc fd）
 *
 * 执行顺序（很重要）：
 * 1) heredoc 检查
 * 2) apply_redir_list：计算最终 stdin/stdout 应该接到哪
 * 3) close_all_heredoc_fds(root)：避免 heredoc fd 泄漏给外部程序
 * 4) dup_in_out_or_close：dup2 并 close 旧 fd
 * 5) builtin：直接执行并 exit
 * 6) external：走 PATH + execve
 */
void	child_exec_one(t_minishell *msh, t_ast *node, int in_fd, int out_fd,
		t_ast *root)
{
	int	new_in;
	int	new_out;

	if (!msh || !node || node->type != NODE_CMD)
		ms_child_exit(msh, root, 1);

	if (has_bad_heredoc(node->redir))
		ms_child_exit(msh, root, 1);

	new_in = in_fd;
	new_out = out_fd;

	/*
	 * 这一段是你代码的一个“策略选择”：
	 * 如果当前命令是 builtin，并且 new_in 是管道读端（>2），你把它 close 掉并设为 -1。
	 *
	 * 直观理解：
	 * - 你希望某些 builtin 不去读管道输入，避免出现“没人读导致上游阻塞”这类情况
	 *
	 * 注意（我必须实话实说）：
	 * - bash 里 builtin 在 pipeline 子进程中通常也会从 pipe 读 stdin
	 * - 但你项目里的 builtin（echo/pwd/env/export...）大多数不读 stdin，
	 *   所以这个策略一般不会出问题。
	 * - 如果未来你写了“会读 stdin 的 builtin”，这里就需要重新评估。
	 */
	if (new_in > 2 && node->argv && node->argv[0] && is_builtin(node->argv[0]))
	{
		close(new_in);
		new_in = -1;
	}

	/* 让重定向覆盖/替换掉 new_in/new_out（函数内部会先 close 旧 fd） */
	if (apply_redir_list(node->redir, &new_in, &new_out) < 0)
		ms_child_exit(msh, root, 1);

	/* 超级关键：关闭整棵 AST 的 heredoc_fd，防止泄漏到 execve 的外部程序里 */
	close_all_heredoc_fds(root);

	/* 把 stdin/stdout 接好，并把旧 fd close 掉（避免泄漏 + 避免 pipe EOF 问题） */
	if (dup_in_out_or_close(new_in, new_out) < 0)
		ms_child_exit(msh, root, 1);

	/* builtin：在子进程里跑，不影响父进程 env（pipeline 里这很正常） */
	if (node->argv && node->argv[0] && is_builtin(node->argv[0]))
		ms_child_exit(msh, root, exec_builtin(node, &msh->env, msh));

	/* 外部命令：PATH + execve */
	child_exec_external(msh, node, root);

	/* 理论上不会走到这里（除非上面逻辑异常） */
	ms_child_exit(msh, root, 1);
}
