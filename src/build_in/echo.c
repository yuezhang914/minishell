// echo.c（echo + 支持 -n/-nnn）
// 文件用途
// parse_echo_flags：识别一个或多个 -n（支持 -nnnn）
// echo_print_args：打印参数并控制空格
// ft_echo：组合

#include "minishell.h"
#include "build_in.h"

/* parse_echo_flags
 * 作用：解析 echo 的 -n 选项（支持 -n -nn -nnnn）
 * 输出：
 * - *start_index：从 argv 的哪个位置开始打印内容
 * 返回：
 * - print_newline：1 表示末尾要打印换行；0 表示不打印换行
 */
static int parse_echo_flags(char **argv, int *start_index)
{
	int i;
	int j;
	int print_newline;

	i = 1;              /* 从 argv[1] 开始看选项（argv[0] 是 "echo"） */
	print_newline = 1;  /* 默认要换行 */

	while (argv[i] && argv[i][0] == '-')
	{
		j = 1;          /* 从 '-' 后一位开始检查 */
		if (argv[i][j] == '\0')
			break;      /* 参数只是 "-"：当普通字符串，不当选项 */

		while (argv[i][j] == 'n')
			j++;

		if (argv[i][j] != '\0')
			break;      /* 出现了非 n 字符：例如 -nx，停止选项解析 */

		/* 走到这里说明 argv[i] 是合法的 -n 或 -nnnn */
		print_newline = 0;
		i++;
	}

	*start_index = i;   /* i 指向第一个非选项参数 */
	return print_newline;
}

/* echo_print_args
 * 作用：从 start_index 开始打印所有参数
 * 规则：
 * - 参数之间打印一个空格
 * - 最后一个参数后不打印空格
 */
static void echo_print_args(char **argv, int start_index)
{
	int i;

	i = start_index;
	while (argv[i])
	{
		printf("%s", argv[i]);
		if (argv[i + 1])
			printf(" ");
		i++;
	}
}

/* ft_echo
 * 作用：echo 总入口
 */
int ft_echo(char **argv)
{
	int print_newline;
	int start_index;

	print_newline = parse_echo_flags(argv, &start_index);
	echo_print_args(argv, start_index);

	if (print_newline)
		printf("\n");

	return 0;
}
