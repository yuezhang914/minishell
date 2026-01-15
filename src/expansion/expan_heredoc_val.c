// expan_heredoc_val.c：heredoc 行的变量展开（另一套实现）
// 这个文件的写法更“教学风格”，逻辑是：
// 遇到 $?：拼上 last_exit_status
// 遇到 $NAME：拼上 env 里的值
// 遇到 $ 后面不是合法变量：输出 $
// 普通字符：原样拼上
// 你项目里现在 process_heredoc.c 调用的是 expand_line(...)（我没在你这批上传里找到它），所以我不能假装知道它实现。
// 但 expand_heredoc_vars() 的行为就是一个可用的 heredoc 展开器，你很可能在别的地方会用到它，或者它就是 expand_line 的旧版本。

#include "expander.h"
#include "minishell.h"

/* search_env：
 * 作用：在 ms->env 链表里找 key 对应的 value
 * 注意：这里返回的是“指向链表内部的指针”，不是 strdup！
 * 所以调用者不要 free 返回值。
 */
char *search_env(t_minishell *ms, const char *key)
{
	t_env *node;

	node = ms->env;

	/* 遍历链表 */
	while (node)
	{
		/* ft_strcmp：字符串完全相等返回 0 */
		if (ft_strcmp(node->key, key) == 0)
			return (node->value);

		node = node->next;
	}

	/* 找不到：按空字符串处理 */
	return ("");
}

/* handle_status：
 * 处理 $?：把 last_exit_status 转成字符串并拼到 result 后面
 * step：告诉外层“我这次吃掉了 '?' 这个字符”
 */
static char *handle_status(t_minishell *ms, char *result, int *step)
{
	char *value;

	/* int -> string */
	value = ft_itoa(ms->last_exit_status);
	if (!value)
		return (result);

	/* ft_strjoin_free：你 libft 里封装的“拼接并释放旧 result” */
	result = ft_strjoin_free(result, value);

	free(value);

	*step = 1; /* 告诉外层：$? 的 '?' 被我处理了 */
	return (result);
}

/* handle_variable：
 * 处理 $NAME：
 * - 先数出 NAME 的长度（字母数字下划线）
 * - key = ft_substr(...) 复制变量名
 * - value = search_env(...) 找到变量值
 * - 拼接 value 到 result
 * - step=变量名长度
 */
static char *handle_variable(t_minishell *ms, char *target, char *result, int *step)
{
	char *key;
	char *value;
	int length;

	length = 0;

	/* 数变量名长度 */
	while (ft_isalnum(target[length]) || target[length] == '_')
		length++;

	/* 复制变量名 */
	key = ft_substr(target, 0, length);
	if (!key)
		return (result);

	/* 找变量值（注意：不 strdup） */
	value = search_env(ms, key);

	/* 拼接 value 到 result（value 是只读指针） */
	result = ft_strjoin_free(result, value);

	free(key);

	*step = length;
	return (result);
}

/* join_char：
 * 把一个普通字符 c 追加到 result 后面
 */
static char *join_char(char *result, char c)
{
	char buffer[2];

	buffer[0] = c;
	buffer[1] = '\0';

	return (ft_strjoin_free(result, buffer));
}

/* expand_heredoc_vars：
 * heredoc 行展开入口：
 * - source 指向当前扫描位置
 * - result 是输出，起始为 ""
 * - step 用来控制一次跳过多少字符
 */
char *expand_heredoc_vars(t_minishell *ms, char *source)
{
	char *result;
	int  step;

	result = ft_strdup("");
	while (*source)
	{
		step = 0;

		/* $? */
		if (*source == '$' && *(source + 1) == '?')
			result = handle_status(ms, result, &step);

		/* $NAME：NAME 的首字符是字母或 '_' */
		else if (*source == '$'
			&& (ft_isalpha(*(source + 1)) || *(source + 1) == '_'))
			result = handle_variable(ms, source + 1, result, &step);

		/* 单独的 '$'（比如 "$-"）这里当字面量 '$' */
		else if (*source == '$')
			result = ft_strjoin_free(result, "$");

		/* 普通字符 */
		else
			result = join_char(result, *source);

		/* 如果 step>0，说明刚处理了 "$" + (step 个字符)
		 * 所以 source 要跳过：1（$） + step
		 */
		if (step > 0)
			source += step + 1;
		else
			source++;
	}

	return (result);
}
