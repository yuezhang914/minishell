#include "minishell.h"
#include "exec.h"

/* join_dir_cmd：把 dir + "/" + cmd 拼起来 */
static char	*join_dir_cmd(const char *dir, const char *cmd)
{
	char	*tmp;
	char	*res;

	tmp = NULL;
	res = NULL;

	tmp = ft_strjoin(dir, "/");
	if (!tmp)
		return (NULL);

	res = ft_strjoin(tmp, cmd);
	free(tmp);
	return (res);
}

/*
 * find_cmd_path
 * 作用：
 * - 如果 cmd 自己包含 '/'：当作路径直接返回 strdup(cmd)
 * - 否则：遍历 msh->paths（由 PATH 环境变量拆出来的目录数组）
 *         依次拼路径并 access(F_OK) 判断是否存在
 *
 * 注意：这里只检查存在性，不检查可执行
 * 让 execve 去触发 EACCES/EISDIR，再映射成 126 更像 bash
 */
char	*find_cmd_path(t_minishell *msh, const char *cmd)
{
	int		i;
	char	*path;

	i = 0;
	path = NULL;

	if (!cmd || !*cmd)
		return (NULL);

	if (ft_strchr(cmd, '/'))
		return (ft_strdup(cmd));

	ensure_paths_ready(msh);

	while (msh->paths && msh->paths[i])
	{
		path = join_dir_cmd(msh->paths[i], cmd);
		if (!path)
			return (NULL);

		if (access(path, F_OK) == 0)
			return (path);

		free(path);
		i = i + 1;
	}
	return (NULL);
}
