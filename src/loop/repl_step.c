// repl_step.c（每轮一步：读行/处理 Ctrl-C Ctrl-D/拼接/尝试执行）
// 你必须搞懂的分支（超级重要）
// Ctrl-C：g_signal == SIGINT
// → 清空 acc，退出码设 130（bash 习惯），继续循环
// Ctrl-D：line == NULL
// 如果 acc 为空：打印 exit 并退出 shell（返回 1）
// 如果 acc 不空：说明正在续行却 EOF → 报错 “unexpected EOF while looking for matching quote”，退出码 258

#include "minishell.h"
#include "repl.h"

/*
** 作用：当用户按 Ctrl-C 时，清空当前半截命令
** 参数：
** - ms：用来设置 last_exit_status
** - acc：累计输入，要清空
** - line：本轮读到的 line（可能为 NULL）
**
** 为什么 Ctrl-C 的退出码是 130？
** - bash 里 SIGINT 通常映射成 128 + 2 = 130
*/
static int handle_interruption(t_minishell *ms, char **acc, char *line)
{
	ms->last_exit_status = 130;  /* Ctrl-C 的标准退出码 */
	g_signal = 0;                /* 复位信号标志（避免影响下一轮） */

	/* 如果 line 是 NULL：这通常意味着在非交互/异常情况下被中断 */
	if (!line)
	{
		ms_clear(ms);
		exit(ms->last_exit_status);
	}

	/* 正常情况：释放本行 */
	if (line)
		free(line);

	/* 清空累计输入 acc（半截命令不要了） */
	if (acc && *acc)
	{
		free(*acc);
		*acc = NULL;
	}
	return (0); /* 返回 0：继续主循环 */
}

/*
** 作用：续行模式下遇到 EOF（Ctrl-D）：
** - 说明用户没把引号闭合就结束输入了
** - 打印错误，设置退出码 258，并清空 acc
*/
static int step_eof_more(t_minishell *ms, char **acc)
{
	char q;

	/* 1) 判断缺的是哪种引号，默认用 " */
	q = '"';
	if (ms && ms->lexer_unclosed_quote)
		q = ms->lexer_unclosed_quote;

	/* 2) 用你自己的错误函数输出（更统一） */
	ms_err_eof_quote(q);

	/* 3) 维护状态：bash 通常用 258 表示语法错误 */
	if (ms)
	{
		ms->last_exit_status = 258;
		ms->raw_line = NULL;
	}

	/* 4) 清空 acc（半截命令丢掉） */
	if (acc && *acc)
	{
		free(*acc);
		*acc = NULL;
	}
	return (0); /* 继续循环：用户还能再输入新命令 */
}

/*
** 作用：处理 Ctrl-D（line==NULL）
** - acc 为空：直接退出 shell（交互模式还打印 exit）
** - acc 不空：说明在续行中遇到 EOF → 当作错误（step_eof_more）
*/
static int step_handle_eof(t_minishell *ms, char **acc)
{
	if (!acc || *acc == NULL)
	{
		/* 只有在真实终端里才打印 exit（更像 bash） */
		if (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO))
			printf("exit\n");
		return (1); /* 返回 1：让 repl_loop 停止 */
	}
	return (step_eof_more(ms, acc));
}

/*
** 作用：REPL 的“一步”
** 返回：
** - 1：退出 shell（repl_loop 会 stop）
** - 0：继续循环
**
** 步骤：
** 1) 读一行 line
** 2) Ctrl-C：清空
** 3) Ctrl-D：退出或报错
** 4) 拼接到 acc
** 5) repl_run_acc：尝试 lex/parse/exec（不完整就不执行）
*/
int repl_step(t_minishell *ms, char **acc)
{
	char *line;
	int ok;

	/* 读一行：prompt 会根据 *acc 是否为空变化 */
	line = repl_read(*acc);

	/* 如果这一轮发生 Ctrl-C，signals 模块会把 g_signal 设为 SIGINT */
	if (g_signal == SIGINT)
		return (handle_interruption(ms, acc, line));

	/* line==NULL：EOF（Ctrl-D 或管道输入结束） */
	if (!line)
		return (step_handle_eof(ms, acc));

	/* 把这一行拼到累计输入 acc */
	ok = repl_join(acc, line);
	free(line);

	/* 拼接失败（通常是 malloc 失败）
	 * 你这里统一走 step_eof_more 做清理与设置 258
	 */
	if (ok == 0)
		return (step_eof_more(ms, acc));

	/* acc 现在是一条“可能完整”的命令：尝试执行 */
	repl_run_acc(ms, acc);

	/* 刷新 stdout（保证输出立刻显示） */
	fflush(stdout);

	return (0);
}

