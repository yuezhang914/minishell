// cd.c（改变当前目录 + 更新 PWD/OLDPWD）
// 文件用途
// cd_get_target：确定目标目录（无参用 HOME）
// cd_get_oldcwd：切换前 getcwd 保存旧目录
// cd_change_dir：chdir 执行切换
// ft_cd：组合以上步骤，并更新 env 链表里的 PWD/OLDPWD
// 行为提醒
// 你用的是 getenv("HOME")（读系统环境），不是你的链表
// cwd 缓冲区你写死成 4096，也可用 PATH_MAX

#include "build_in.h"
#include "minishell.h"

/* cd_get_target
 * 作用：决定 cd 要去哪里
 * - cd（无参数）：去 HOME
 * - cd arg1 arg2：报错 too many arguments
 * - cd path：去 path
 */
static char *cd_get_target(char **argv)
{
	char *target;

	if (!argv[1])
	{
		/* getenv 来自 libc：读进程真实环境变量 HOME */
		target = getenv("HOME");
		if (!target)
		{
			ft_putstr_fd("cd: HOME not set\n", STDERR_FILENO);
			return NULL;
		}
	}
	else if (argv[2])
	{
		ms_put3("cd: ", "too many arguments", "\n");
		return NULL;
	}
	else
	{
		target = argv[1];
	}
	return target;
}

/* cd_get_oldcwd
 * 作用：在 chdir 前把当前目录存起来（用于 OLDPWD）
 * getcwd：把当前工作目录绝对路径写进 cwd 缓冲区
 */
static int cd_get_oldcwd(char *cwd, size_t size)
{
	if (!getcwd(cwd, size))
	{
		perror("getcwd");
		return -1;
	}
	return 0;
}

/* cd_change_dir
 * 作用：执行 chdir
 * chdir 失败常见原因：
 * - 路径不存在
 * - 权限不够
 */
static int cd_change_dir(const char *target)
{
	if (chdir(target) != 0)
	{
		ms_put3("minishell: cd: ", target, "No such file or directory\n");
		return (-1);
	}
	return 0;
}

/* ft_cd
 * 作用：cd builtin 总入口
 * 步骤：
 * 1) target = cd_get_target
 * 2) getcwd 保存旧目录到 cwd（给 OLDPWD）
 * 3) chdir(target)
 * 4) env_set(OLDPWD=旧cwd)
 * 5) 再次 getcwd 得到新目录，env_set(PWD=新cwd)
 */
int ft_cd(char **argv, t_env **env)
{
	char cwd[4096];
	char *target;

	target = cd_get_target(argv);
	if (!target)
		return 1;

	/* 保存旧目录 */
	if (cd_get_oldcwd(cwd, sizeof(cwd)) != 0)
		return 1;

	/* 切换目录 */
	if (cd_change_dir(target) != 0)
		return 1;

	/* 更新 OLDPWD */
	env_set(env, "OLDPWD", cwd);

	/* 重新获取当前目录（新目录） */
	if (!getcwd(cwd, sizeof(cwd)))
	{
		perror("getcwd");
		return 1;
	}

	/* 更新 PWD */
	env_set(env, "PWD", cwd);
	return 0;
}
