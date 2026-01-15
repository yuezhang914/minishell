// repl_io.c（读一行：交互用 readline，非交互用 read）
// 这文件的“灵魂”
// 如果是交互终端：用 readline(prompt)（支持行编辑、上下键历史）
// 如果不是终端（比如 echo "ls" | ./minishell）：自己用 read() 一字一字读到 \n
// 重点变量 st[2]（很适合新手）
// st[0]：当前 buf 容量 capacity
// st[1]：当前已经写了多少字符 len

#include "minishell.h"
#include "repl.h"

/*
** 作用：buf 不够装了，就扩容为 2 倍
** 参数：
** - buf：旧缓冲区
** - size：容量指针（会被更新为 2 倍）
** - len：旧缓冲区已使用长度（复制这么多字节过去）
*/
static char	*ms_expand_buffer(char *buf, size_t *size, size_t len)
{
	char	*new_buf;

	*size = (*size) * 2;                 /* 容量翻倍 */
	new_buf = (char *)malloc(*size);     /* 新缓冲区 */
	if (!new_buf)
	{
		free(buf);                       /* 内存不足：旧 buf 也别泄漏 */
		return (NULL);
	}
	ft_memcpy(new_buf, buf, len);        /* 把已有内容拷贝过去 */
	free(buf);                            /* 释放旧 buf */
	return (new_buf);
}

/*
** 作用：非交互模式下读一行（不用 readline）
** 行为：
** - 从 fd 里一个字符一个字符读
** - 遇到 '\n' 停止
** - 读到 EOF 且一开始就没读到字符：返回 NULL（表示结束）
*/
static char	*ms_read_line_raw(int fd)
{
	char	*buf;
	size_t	st[2];
	char	ch;

	st[0] = 64;                 /* 初始容量 64 */
	st[1] = 0;                  /* 当前长度 0 */
	buf = (char *)malloc(st[0]);
	if (!buf)
		return (NULL);

	while (read(fd, &ch, 1) > 0 && ch != '\n')
	{
		/* 如果快满了：扩容 */
		if (st[1] + 1 >= st[0])
		{
			buf = ms_expand_buffer(buf, &st[0], st[1]);
			if (!buf)
				return (NULL);
		}
		/* 把读到的字符放进 buf，并 len++ */
		buf[st[1]++] = ch;
	}

	/* 这里是 EOF 情况：
	 * - 如果什么都没读到，并且也不是因为读到 '\n' 才停
	 *   说明是真的 EOF：返回 NULL（主循环会退出）
	 */
	if (st[1] == 0 && ch != '\n')
		return (free(buf), NULL);

	buf[st[1]] = '\0';          /* 补字符串结束符 */
	return (buf);
}

/*
** 作用：根据是否是终端选择读法
** - 终端：readline(prompt)
** - 非终端：ms_read_line_raw(0)
*/
static char	*ms_obtain_line(const char *prompt)
{
	if (isatty(STDIN_FILENO))
		return (readline(prompt));
	return (ms_read_line_raw(STDIN_FILENO));
}

/*
** 作用：根据 acc 是否为空，显示不同 prompt
** - acc 有内容：说明上一行命令没写完 → prompt 用 "> "
** - acc 为空：正常提示符 "minishell$ "
*/
char	*repl_read(char *acc)
{
	char	*prompt;

	prompt = "";
	if (isatty(STDIN_FILENO))
	{
		if (acc && acc[0] != '\0')
			prompt = "> ";
		else
			prompt = "minishell$ ";
	}
	return (ms_obtain_line(prompt));
}

/*
** 作用：判断字符串是否有“非空白字符”
** 用到的 check_space：你项目里用于判断空格/tab 等
** - 全是空白：返回 0
** - 存在任何非空白：返回 1
*/
int	repl_has_text(const char *s)
{
	int	i;

	if (!s)
		return (0);
	i = 0;
	while (s[i])
	{
		if (!check_space((unsigned char)s[i]))
			return (1);
		i++;
	}
	return (0);
}
