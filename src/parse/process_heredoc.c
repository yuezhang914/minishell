#include "expander.h"
#include "minishell.h"
#include "parse.h"

/*
 * process_heredoc_line（static）
 * 作用：处理刚读到的一行 heredoc 内容：
 * - 判断是否等于 delimiter（等于就结束）
 * - 若 quoted==0：可以做变量展开
 * - 把最终内容写入 pipe 的写端 write_fd
 *
 * 返回：1 表示匹配 delimiter（应该结束）；0 表示继续
 */
static int process_heredoc_line(char **line, char **full_line,
                                const char *delimiter, int write_fd,
                                t_minishell *msh, int quoted)
{
    char *expanded;

    (void)msh;

    /* line 为空就没啥可处理 */
    if (!line || !*line)
        return 0;

    /* end_line / extract_line：你用来处理换行符的辅助（在 parse.h 声明） */

    /* 如果这一行是 delimiter：说明 heredoc 结束 */
    if (ft_strncmp(*line, delimiter, ft_strlen(delimiter)) == 0
        && end_line(*line)) /* 你的 end_line 应该判断这一行是否正好结束 */
    {
        free(*line);
        *line = NULL;
        free(*full_line);
        *full_line = NULL;
        return 1;
    }

    /* quoted==0：允许变量展开；quoted==1：不展开 */
    if (!quoted)
    {
        /* expand_line：来自 expander 模块（你 parse 只负责调用） */
        expanded = expand_line(*line, msh);
        if (expanded)
        {
            free(*line);
            *line = expanded;
        }
    }

    /* 把处理后的行写进管道（执行时从管道读） */
    write(write_fd, *line, ft_strlen(*line));

    free(*line);
    *line = NULL;

    return 0;
}

/*
 * read_and_process_line（static）
 * 作用：读取一行并交给 process_heredoc_line 处理
 * 这里还有一段逻辑：full_line 用来拼接长行（你可能在处理换行时做了分段）
 *
 * 返回值：
 *  0：继续读
 * -1：被 SIGINT 中断
 *  1：EOF 且没有任何内容
 *  2：匹配到 delimiter（结束）
 */
static int read_and_process_line(char **full_line, const char *delimiter,
                                 int write_fd, t_minishell *msh, int quoted)
{
    char *line;

    /* 读一行（可能带 \n） */
    line = get_next_line(STDIN_FILENO);

    /* 如果全局信号变量显示 SIGINT（Ctrl-C） */
    if (g_signal == SIGINT)
    {
        free(line);
        free(*full_line);

        /* 打印一个换行，让提示符对齐 */
        write(1, "\n", 1);
        return (-1);
    }

    /* 如果 line==NULL 且 full_line 也为空：说明一开始就 EOF */
    if (!line && *full_line == NULL)
        return (1);

    /* 如果 line==NULL，但 full_line 有内容：把 full_line 当作最后一行处理 */
    if (!line)
    {
        line = *full_line;
        *full_line = NULL;
    }
    else if (*full_line)
    {
        /* join_full_line：把前半段 full_line 和新读的 line 拼起来 */
        line = join_full_line(*full_line, line);
    }

    /* 处理这一行：如果返回 1，说明遇到 delimiter */
    if (process_heredoc_line(&line, full_line, delimiter, write_fd, msh, quoted))
        return (2);

    return (0);
}

/*
 * heredoc_loop
 * 作用：heredoc 核心循环
 * - 打印 "heredoc> " 提示
 * - 不断 get_next_line 读输入
 * - 直到 delimiter 或 EOF 或 SIGINT
 *
 * 返回：
 *  0：正常结束
 * -1：被 SIGINT 中断
 */
int heredoc_loop(int write_fd, const char *delimiter, t_minishell *msh, int quoted)
{
    char *full_line;
    int  status;

    full_line = NULL;

    /* heredoc 输入模式下，信号处理要特殊设置（比如 Ctrl-C 只中断 heredoc） */
    setup_heredoc_signals();

    while (1)
    {
        /* 如果 full_line 为空，打印提示符（你写的是 heredoc> ） */
        if (!full_line)
            write(STDOUT_FILENO, "heredoc> ", 9);

        /* 每次读之前把 g_signal 清零 */
        g_signal = 0;

        status = read_and_process_line(&full_line, delimiter, write_fd, msh, quoted);

        /* status != 0 说明要退出循环 */
        if (status != 0)
        {
            if (status == -1)
                return (-1); /* SIGINT */
            return (0);      /* EOF 或 delimiter */
        }
    }
}
