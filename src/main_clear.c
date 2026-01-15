// main_clear.c（退出清理：释放 env/envp/paths/lexer）
// 文件用途
// release_matrix(char **m)：释放字符串数组（比如 ms->envp / ms->paths）
// ms_clear(t_minishell *ms)：集中清理 t_minishell 里所有资源

#include "minishell.h"
#include "repl.h"

/*
** 作用：释放一个“字符串数组”
** 例子：m = ["a","b","c",NULL]
** - 先 free 每个 m[i]
** - 再 free 整个 m
*/
void	release_matrix(char **m)
{
	int	i;

	if (!m)
		return ;
	i = 0;
	while (m[i])
	{
		free(m[i]);
		i++;
	}
	free(m);
}

/*
** 作用：程序关门前打扫卫生
** 清理顺序（从代码看）：
** 1) lexer token 链表
** 2) paths（PATH 切出来的目录数组）
** 3) envp（给 execve 用的 char**）
** 4) env（你自己维护的环境变量链表）
*/
void	ms_clear(t_minishell *ms)
{
	if (!ms)
		return ;

	if (ms->lexer)
		clear_list(&ms->lexer);

	if (ms->paths)
	{
		release_matrix(ms->paths);
		ms->paths = NULL;
	}

	if (ms->envp)
	{
		release_matrix(ms->envp);
		ms->envp = NULL;
	}

	if (ms->env)
	{
		free_env(ms->env);
		ms->env = NULL;
	}
}
