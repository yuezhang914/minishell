// 3.1 main.c（程序入口：初始化 → 进入 repl_loop → 清理退出）
// 文件用途
// 检查参数（你这里要求 minishell 不接受参数）
// 初始化 t_minishell
// 初始化环境变量与 PATH
// 进入主循环
// 退出后释放资源并返回 exit code

#include "minishell.h"
#include "repl.h"

/*
** 作用：向屏幕报告错误（写到 stderr=2）。
** 为什么不用 printf？
** - printf 走缓冲，错误信息可能延迟
** - write(2,...) 更直接，且 42 项目常用
*/
static void ms_report_error(const char *s)
{
	int i;

	i = 0;
	/* 计算字符串长度 */
	while (s[i])
		i++;
	/* fd=2 表示标准错误输出 stderr */
	write(2, s, i);
}

/*
** 作用：把系统给你的 envp（char **）转换成你自己的 env 链表 + envp 数组
** 用到的函数：
** - init_env(envp)：把 "KEY=VALUE" 建成链表 ms->env
** - change_envp(ms->env, &ms->envp)：把链表再转回 char**（execve 要用）
*/
static int ms_initial_env(t_minishell *ms, char **envp)
{
	ms->env = init_env(envp);     /* 建立环境变量链表 */
	if (!ms->env)
		return (0);

	ms->envp = NULL;
	change_envp(ms->env, &ms->envp); /* 生成 envp 数组 */
	if (!ms->envp)
		return (0);

	return (1);
}

/*
** 作用：给程序做准备工作：
** - 把结构体指针全部清空（避免野指针）
** - 把状态位全部归零
** - 初始化 env / envp / paths
**
** 你这里初始化的字段（从代码能看到）：
** - env/envp/paths：环境相关
** - lexer：词法链表
** - cur_t_ast：当前正在执行的 AST（常用于信号/中断时清理）
** - raw_line：当前给 lexer 的原始输入指针
** - should_exit：builtin exit 会置 1，让主循环退出
** - last_exit_status：上一个命令退出码（$?）
** - lexer_need_more / lexer_unclosed_quote：续行信息
*/
static int ms_initial_system(t_minishell *ms, char **envp)
{
	ms->env = NULL;
	ms->envp = NULL;
	ms->paths = NULL;
	ms->lexer = NULL;
	ms->cur_t_ast = NULL;
	ms->raw_line = NULL;
	ms->should_exit = 0;
	ms->last_exit_status = 0;
	ms->lexer_need_more = 0;
	ms->lexer_unclosed_quote = 0;

	/* 初始化环境 */
	if (!ms_initial_env(ms, envp))
		return (0);

	/* 准备 PATH 拆分出来的目录数组 ms->paths */
	if (ensure_paths_ready(ms) != 0)
		return (0);

	return (1);
}

/*
** 作用：程序大门
** - 禁止参数：argc 必须为 1
** - 初始化系统
** - 进入 repl_loop 主循环
** - 退出后清理并返回 last_exit_status
*/
int main(int argc, char **argv, char **envp)
{
	t_minishell ms;

	(void)argv; /* 不用 argv，避免编译警告 */

	if (argc != 1)
	{
		ms_report_error("minishell: no argument allowed\n");
		return (1);
	}

	/* 进入“提示符态”的信号策略（Ctrl-C 只换行，不杀掉 shell） */
	setup_prompt_signals();

	if (!ms_initial_system(&ms, envp))
	{
		ms_report_error("minishell: init failed\n");
		ms_clear(&ms);
		return (1);
	}

	/* 主循环：一直读命令、执行，直到 should_exit 或 EOF */
	repl_loop(&ms);

	/* 退出时释放所有资源 */
	ms_clear(&ms);

	/* shell 的退出码：最后一次命令的退出码 */
	return (ms.last_exit_status);
}

