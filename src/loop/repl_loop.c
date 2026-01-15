#include "minishell.h"
#include "repl.h"

/*
** 作用：REPL 主循环
** 核心变量：
** - acc：累计输入（多行拼成一条完整命令）
** - stop：repl_step 返回 1 表示要停止循环
**
** 循环终止条件：
** - stop == 1（例如 Ctrl-D 退出）
** - ms->should_exit == 1（builtin exit 设置）
*/
void	repl_loop(t_minishell *ms)
{
	char	*acc;
	int		stop;

	acc = NULL;     /* 起初没有半截命令 */
	stop = 0;

	while (!stop && ms && ms->should_exit == 0)
		stop = repl_step(ms, &acc);

	/* 退出循环后，确保 acc 释放干净 */
	repl_free_acc(&acc);
}
