// exit.c（exit：设置 should_exit，让主循环收尾退出）
// 文件用途
// is_numeric：判断参数是不是合法整数文本
// to_long：转 long
// builtin_exit：实现 exit 的规则，并写入 msh->should_exit
// 你实现的关键点
// 非数字：打印错误，退出码 2，并设置 should_exit=1
// 参数过多：打印错误，不退出，返回 1
// 正常：把参数转成 unsigned char（0~255）作为退出码
// 交互模式打印 exit\n

#include "build_in.h"
#include "minishell.h"

/* is_numeric
 * 作用：判断字符串是不是“纯数字格式”（允许前导 + 或 -）
 * 例：
 * - "123"  -> 是
 * - "-42"  -> 是
 * - "+7"   -> 是
 * - "12a"  -> 否
 * - ""     -> 否
 */
static int is_numeric(const char *s)
{
	int i;

	i = 0;
	if (!s || !s[0])
		return (0);

	/* 可选符号位 */
	if (s[i] == '+' || s[i] == '-')
		i++;

	/* 只有 '+' 或 '-' 不算数字 */
	if (!s[i])
		return (0);

	while (s[i])
	{
		if (!ft_isdigit((unsigned char)s[i]))
			return (0);
		i++;
	}
	return (1);
}

/* to_long
 * 作用：把字符串转 long（假设已经通过 is_numeric）
 */
static long to_long(const char *s)
{
	int i;
	int sign;
	long val;

	i = 0;
	sign = 1;
	val = 0;

	if (s[i] == '+' || s[i] == '-')
	{
		if (s[i] == '-')
			sign = -1;
		i++;
	}

	while (s[i])
	{
		val = val * 10 + (s[i] - '0');
		i++;
	}
	return (val * sign);
}

/* builtin_exit
 * 作用：exit 总入口
 * 重要：你这里不是直接 exit()，而是：
 * - 设置 msh->should_exit = 1
 * - 主循环检测 should_exit 后退出
 * 这样可以统一清理资源（ms_clear 等）
 */
int builtin_exit(char **argv, t_minishell *msh)
{
	long code;
	int ret;

	code = 0;
	ret = 0;

	if (argv && argv[1])
	{
		/* 规则1：不是数字 -> 报错，退出码 2，并要求退出 */
		if (!is_numeric(argv[1]))
		{
			ms_put3("minishell: exit: ", argv[1],
					": numeric argument required\n");
			ret = 2;

			if (msh)
			{
				msh->last_exit_status = ret;
				msh->exit_code = ret;
				msh->should_exit = 1;
			}
			return (ret);
		}

		/* 规则2：参数过多 -> 不退出，返回 1 */
		if (argv[2])
		{
			ms_put3("minishell: exit: ", "too many arguments", "\n");
			if (msh)
				msh->last_exit_status = 1;
			return (1);
		}

		/* 规则3：合法数字 -> 转 long -> 取 unsigned char（0~255） */
		code = to_long(argv[1]);
		ret = (unsigned char)code;
	}
	else
	{
		/* 无参数：用上一次命令退出码 */
		ret = (msh != NULL) ? msh->last_exit_status : 0;
	}

	/* 设置退出信息：交给主循环收尾 */
	if (msh)
	{
		msh->last_exit_status = ret;
		msh->exit_code = ret;
		msh->should_exit = 1;
	}

	/* 交互终端：打印 exit（像 bash） */
	if (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO))
		write(STDOUT_FILENO, "exit\n", 5);

	return (ret);
}
