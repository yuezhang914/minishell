#include "minishell.h"
#include "exec.h"

/*
 * dup_env_path
 * 作用：在 msh->envp 里找 "PATH=..."，复制出 PATH 的值（新字符串）
 * 返回：
 * - 找到：返回 ft_strdup(line + 5)（需要 free）
 * - 没有 PATH：返回 NULL
 */
static char	*dup_env_path(t_minishell *msh)
{
	int		i;
	char	*line;

	i = 0;
	line = NULL;
	if (!msh || !msh->envp)
		return (NULL);

	while (msh->envp[i])
	{
		line = msh->envp[i];
		if (ft_strncmp(line, "PATH=", 5) == 0)
			return (ft_strdup(line + 5));  /* 复制 PATH 的值部分 */
		i = i + 1;
	}
	return (NULL);
}

/*
 * free_paths
 * 作用：释放 msh->paths（PATH 切出来的目录数组）
 * 例如 PATH="/bin:/usr/bin"
 * msh->paths 可能是 ["bin","/usr/bin",NULL]
 */
void	free_paths(t_minishell *msh)
{
	int	i;

	i = 0;
	if (!msh || !msh->paths)
		return ;

	while (msh->paths[i])
	{
		free(msh->paths[i]);
		i = i + 1;
	}
	free(msh->paths);
	msh->paths = NULL;
}

/*
 * exec_refresh_paths
 * 作用：重新从 envp 读取 PATH，并更新 msh->paths
 * 为什么要做？
 * - builtin export PATH=... 后，找命令必须用新的 PATH
 *
 * 返回：
 * - 成功 0
 * - 内存失败 1
 * - 没有 PATH：也算成功（0），只是 msh->paths 保持 NULL
 */
int	exec_refresh_paths(t_minishell *msh)
{
	char	*raw;

	raw = NULL;
	if (!msh)
		return (1);

	free_paths(msh);              /* 先释放旧 paths */

	raw = dup_env_path(msh);      /* 复制 PATH 值 */
	if (!raw)
		return (0);               /* 没 PATH：不算错误 */

	msh->paths = ft_split(raw, ':');  /* 按 ':' 切开目录列表 */
	free(raw);

	if (!msh->paths)
		return (1);
	return (0);
}

/*
 * ensure_paths_ready
 * 作用：懒加载——第一次需要 PATH 时才生成 msh->paths
 */
int	ensure_paths_ready(t_minishell *msh)
{
	if (!msh)
		return (1);
	if (msh->paths)
		return (0);
	return (exec_refresh_paths(msh));
}
