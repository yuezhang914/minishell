#ifndef SIGNAL_H
#define SIGNAL_H

/* readline/readline.h：
 * - 你在 sigint_prompt 里用了：
 *   rl_on_new_line / rl_replace_line / rl_redisplay / rl_done
 */
#include <readline/readline.h>

/* unistd.h：
 * - 你在 sigint_prompt 里用了 write(1, "\n", 1)
 */
#include <unistd.h>

/* termios.h：
 * - 这个文件通常用于终端属性（你这份 signals 里没直接用到函数，
 *   但项目里信号/终端控制经常一起出现，所以经常被一起 include）
 */
#include <termios.h>

/* t_saved_signals：保存三种信号的“旧配置”
 * 为什么要保存？
 * - heredoc/执行阶段会临时改信号策略
 * - 结束后必须恢复回“提示符模式”的策略
 *
 * sigaction 结构体里保存了：
 * - sa_handler：当信号来时执行谁
 * - sa_mask：处理时临时屏蔽哪些信号
 * - sa_flags：比如 SA_RESTART
 */
typedef struct s_saved_signals
{
    struct sigaction sigint;   /* 旧的 SIGINT 配置 */
    struct sigaction sigquit;  /* 旧的 SIGQUIT 配置 */
    struct sigaction sigtstp;  /* 旧的 SIGTSTP 配置 */
} t_saved_signals;

/* 提示符模式 Ctrl-C 的 handler（readline 专用） */
void sigint_prompt(int sig);

/* 三种模式的信号配置 */
void setup_prompt_signals(void);        /* 等你输入时 */
void setup_child_signals(void);         /* 子进程执行命令时 */
void setup_parent_exec_signals(void);   /* 父进程等待子进程时 */
void setup_heredoc_signals(void);       /* heredoc 读取时 */

/* 保存/忽略/恢复：主要给 heredoc 使用 */
void save_signals(t_saved_signals *old);
void ignore_heredoc_signals(void);
void restore_signals(t_saved_signals *old);

#endif
