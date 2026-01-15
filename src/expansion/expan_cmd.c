// expan_cmd.c：展开 CMD 节点（argv + redir）
// 这个文件干的是：
// 把 node->argv[i] 每个字符串都变成展开后的版本
// 把 node->redir 每个重定向的 filename 也处理好
// 重点你要特别懂的两个点

// 点1：为什么 heredoc 的 delimiter 不做变量展开？
// 因为 delimiter 只是“终止标记”，不是内容。并且如果 delimiter 带引号，bash 规定 heredoc 内容不展开变量。
// 你这里的做法是：对 HEREDOC 的 filename（也就是 delimiter）只“去引号”。

// 点2：argv 展开为空就删掉（未加引号）
// 这让你更接近 bash 的体验。

#include "expander.h"
#include "minishell.h"

/* 本文件用到的库函数：
 * - ft_strdup：如果 remove_quotes_flag 返回 NULL，就复制原串
 * - free：释放旧字符串
 * - 还有你 expander_str / word_has_quotes
 */

/* strip_quotes_inplace：
 * 作用：把 *s 去掉引号（如果没有引号就复制一份），并把旧 *s free 掉。
 * “inplace” 意思是：直接修改原来的指针 *s（原地替换）。
 */
static int strip_quotes_inplace(char **s)
{
	char *tmp;
	int had_q;
	int q_s;
	int q_d;

	had_q = 0;
	q_s = 0;
	q_d = 0;

	if (!s || !*s)
		return (1); /* 没东西就当成功 */

	/* 尝试去引号：没引号时返回 NULL */
	tmp = remove_quotes_flag(*s, &had_q, &q_s, &q_d);

	/* 没引号：复制一份（确保返回的是“新分配的字符串”） */
	if (!tmp)
		tmp = ft_strdup(*s);

	if (!tmp)
		return (0);

	/* 释放旧字符串，再把指针指向新字符串 */
	free(*s);
	*s = tmp;

	return (1);
}

/* argv_shift_left：
 * 作用：从 argv[i] 开始，把后面的元素整体左移一格，实现“删除 argv[i]”
 *
 * 例子：
 * argv = ["echo","", "abc", NULL]
 * 删掉 i=1 后：
 * argv = ["echo","abc", NULL]
 */
static void argv_shift_left(char **argv, int i)
{
	while (argv[i])
	{
		argv[i] = argv[i + 1];
		i = i + 1;
	}
}

/* expand_argv_inplace：
 * 作用：逐个展开 node->argv[i]
 *
 * 关键逻辑：
 * - had_q = word_has_quotes(原字符串)
 * - tmp = expander_str(展开 + 去引号 + free旧字符串)
 * - 如果 tmp=="" 且 had_q==0：删除该参数（更像 bash）
 */
static int expand_argv_inplace(t_minishell *msh, t_ast *node)
{
	int   i;
	int   had_q;
	char *tmp;

	i = 0;

	while (node->argv && node->argv[i])
	{
		/* 先记录：原字符串里有没有引号 */
		had_q = word_has_quotes(node->argv[i]);

		/* 展开并去引号：注意 expander_str 会 free 掉旧 node->argv[i] */
		tmp = expander_str(msh, node->argv[i]);
		if (!tmp)
			return (0);

		/* 展开结果是空串，并且原来没有引号：删除这个 argv 元素 */
		if (tmp[0] == '\0' && had_q == 0)
		{
			free(tmp);
			argv_shift_left(node->argv, i);

			/* 注意：这里 i 不加 1，因为左移后新的 argv[i] 还没处理 */
		}
		else
		{
			/* 正常情况：用新字符串替换 argv[i] */
			node->argv[i] = tmp;
			i = i + 1;
		}
	}

	return (1);
}

/* expand_redirs_inplace：
 * 作用：处理重定向链表
 *
 * 规则：
 * - HEREDOC：filename 是 delimiter，只去引号（不做 $ 展开）
 * - 其他重定向：filename 也需要做展开（例如 > $OUTFILE）
 */
static int expand_redirs_inplace(t_minishell *msh, t_redir *r)
{
	char *tmp;

	while (r)
	{
		/* heredoc delimiter：只去引号 */
		if (r->filename && r->type == HEREDOC)
		{
			if (!strip_quotes_inplace(&r->filename))
				return (0);
		}
		else if (r->filename)
		{
			/* 普通重定向文件名：做展开+去引号 */
			tmp = expander_str(msh, r->filename);
			if (!tmp)
				return (0);
			r->filename = tmp;
		}

		r = r->next;
	}

	return (1);
}

/* expander_expand_cmd_node：
 * 作用：对一个 CMD 节点做展开（argv + redir）
 * - node->type 必须是 NODE_CMD
 * - 失败返回 0
 */
int expander_expand_cmd_node(t_minishell *msh, t_ast *node)
{
	if (!msh || !node || node->type != NODE_CMD)
		return (1); /* 非 CMD 节点：这里选择“不需要处理也算成功” */

	if (!expand_argv_inplace(msh, node))
		return (0);

	if (!expand_redirs_inplace(msh, node->redir))
		return (0);

	return (1);
}
