#include "minishell.h"
#include "build_in.h"

/* builtin_pwd
 * 作用：pwd 总入口
 * getcwd：把当前目录写入 cwd 缓冲区
 */
int builtin_pwd()
{
	char cwd[PATH_MAX];

	if (getcwd(cwd, sizeof(cwd)) != NULL)
	{
		printf("%s\n", cwd);
		return 0;
	}
	else
	{
		perror("pwd");
		return 1;
	}
}
