// repl_run.c（主循环“最关键”：决定执行还是继续等下一行）
// 这个文件就是“主循环的重中之重”
// 它负责把 acc 交给：
// lexer：判断完整性 / 建 token 链表
// parser：建 AST
// heredoc：提前读
// expander：变量展开、去引号
// exec：fork/pipe/dup2/execve
// 并且它决定：命令不完整就别执行，继续读下一行。

#include "minishell.h"
#include "repl.h"

/*
** 作用：执行完一次（或遇到错误）后，把 acc 和 lexer 状态清干净
** 参数：
** - err_code：如果非 0，就把 last_exit_status 设为这个错误码
**
** 清理内容（很关键）：
** - acc：释放并置 NULL
** - raw_line：置 NULL（避免 lexer 误用旧指针）
** - lexer_need_more / lexer_unclosed_quote：复位
*/
static void	run_drop_acc(t_minishell *ms, char **acc, int err_code)
{
	if (err_code != 0)
		ms->last_exit_status = err_code;

	repl_free_acc(acc);

	ms->raw_line = NULL;
	ms->lexer_need_more = 0;
	ms->lexer_unclosed_quote = 0;
}

/*
** 作用：在 lexer 已经成功的前提下，完成 parse/exec 整套流程
** 流程：
** 1) parse_cmdline：token -> AST
** 2) prepare_heredocs：执行前先把 heredoc 内容读完（bash 行为）
** 3) change_envp：同步 env 链表到 envp 数组（execve 用）
** 4) expander_t_ast：变量展开 / 去引号
** 5) exec_t_ast：真正执行
** 6) free_ast + clear_list：清理本轮资源
*/
static void	run_one_cmd(t_minishell *ms)
{
	t_ast	*root;
	t_lexer	*tmp;

	tmp = ms->lexer;
	root = parse_cmdline(&tmp, ms);
	if (!root)
	{
		/* parse 失败：通常是语法错误 */
		ms->last_exit_status = 2;
		clear_list(&ms->lexer);
		return ;
	}

	/* 保存“当前正在执行的 AST”给信号/中断用 */
	ms->cur_t_ast = root;

	/* heredoc 预处理（可能被 Ctrl-C 中断） */
	if (!prepare_heredocs(root, ms))
	{
		ms->cur_t_ast = NULL;
		free_ast(root);
		clear_list(&ms->lexer);
		return ;
	}

	/* env 链表 -> envp 数组（给 execve） */
	change_envp(ms->env, &ms->envp);

	/* 展开：$VAR、$?、去引号等 */
	expander_t_ast(ms, root);

	/* 执行：fork/pipe/dup2/execve/builtin */
	exec_t_ast(ms, root);

	/* 本轮结束：清理 AST 和 lexer token */
	ms->cur_t_ast = NULL;
	free_ast(root);
	clear_list(&ms->lexer);
}

/*
** 作用：决定是否写 history
** 规则（你写得很聪明）：
** - 只有交互终端 isatty(STDIN_FILENO) 才写 history
** - 并且 acc 里要有非空白字符
** - 如果命令里有 heredoc（<<），只把第一行存入 history
**   原因：heredoc 内容可能很长，不应该塞进历史
*/
static void	repl_add_history(t_minishell *ms, char *acc)
{
	char	*hist;

	if (isatty(STDIN_FILENO) && repl_has_text(acc))
	{
		hist = NULL;

		/* 如果 lexer token 里出现过 TOK_HEREDOC：只存第一行 */
		if (lexer_has_heredoc(ms->lexer))
			hist = dup_first_line(acc);
		else
			hist = ft_strdup(acc);

		if (hist)
		{
			add_history(hist); /* readline 的 history */
			free(hist);
		}
	}
}

/*
** 作用：判断 acc 是否“已经是一条完整命令”
** - 空/全空白：直接丢掉（不执行）
** - ms->raw_line 指向 acc：让 handle_lexer 读取
**
** handle_lexer 的返回值（你这里用到的）：
** - LEX_NEED_MORE：引号没闭合等 → 保留 acc，返回等待下一行
** - LEX_OK：完整 → 写 history → 执行一轮
** - 其他：lexer 报错 → drop acc，错误码 2
*/
void	repl_run_acc(t_minishell *ms, char **acc)
{
	int	lex_ret;

	/* acc 没有实际内容：清空并结束本轮 */
	if (!repl_has_text(*acc))
		return (run_drop_acc(ms, acc, 0));

	/* 让 lexer 从 ms->raw_line 读本条命令 */
	ms->raw_line = *acc;

	/* 词法分析：生成 ms->lexer token 链表 或者告诉你“还需要更多输入” */
	lex_ret = handle_lexer(ms);

	if (lex_ret == LEX_NEED_MORE)
	{
		/* 不完整：保留 acc，等待下一行继续拼 */
		ms->raw_line = NULL;
		return ;
	}

	/* lexer 失败：语法/非法字符等，按 2 处理 */
	if (lex_ret != LEX_OK)
		return (run_drop_acc(ms, acc, 2));

	/* lexer 成功：可以把命令写入 history（如果是交互模式） */
	repl_add_history(ms, *acc);

	/* acc 这份字符串之后不再需要：free 掉 */
	ms->raw_line = NULL;
	repl_free_acc(acc);

	/* 执行一整套 parse/heredoc/expand/exec */
	run_one_cmd(ms);

	/* 收尾复位（包含 lexer_need_more 等） */
	run_drop_acc(ms, acc, 0);
}

