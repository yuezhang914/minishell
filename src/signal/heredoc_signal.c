// 这文件解决什么？
// heredoc 期间（<< 输入多行）信号要更“干净”：
// heredoc 读取时 Ctrl-C：应该只中断 heredoc，然后回主 prompt
// 通常 heredoc 会 fork 出一个子进程读输入：
// 父进程：临时忽略信号，安静等子进程结束
// 子进程：用 heredoc 的 handler（只设置 g_signal）

#include "minishell.h"
#include "signals.h"

/* save_signals：把当前三个信号的“配置”保存到 old
 * sigaction(SIGINT, NULL, &old->sigint)
 * - 第二个参数 NULL：表示“不要修改”，只读取当前设置
 */
void save_signals(t_saved_signals *old)
{
    sigaction(SIGINT, NULL, &old->sigint);
    sigaction(SIGQUIT, NULL, &old->sigquit);
    sigaction(SIGTSTP, NULL, &old->sigtstp);
}

/* ignore_heredoc_signals：临时忽略三个信号
 * 常见用法：父进程在 heredoc fork 之后，等待子进程期间调用
 * 目的：避免父进程也被 Ctrl-C 影响，导致状态混乱
 */
void ignore_heredoc_signals(void)
{
    struct sigaction sa_ignore;

    sa_ignore.sa_handler = SIG_IGN;      /* 忽略 */
    sigemptyset(&sa_ignore.sa_mask);
    sa_ignore.sa_flags = 0;

    sigaction(SIGINT, &sa_ignore, NULL);
    sigaction(SIGQUIT, &sa_ignore, NULL);
    sigaction(SIGTSTP, &sa_ignore, NULL);
}

/* restore_signals：恢复成之前 save_signals 保存的状态 */
void restore_signals(t_saved_signals *old)
{
    sigaction(SIGINT, &old->sigint, NULL);
    sigaction(SIGQUIT, &old->sigquit, NULL);
    sigaction(SIGTSTP, &old->sigtstp, NULL);
}

/* sigint_heredoc：heredoc 模式下 Ctrl-C 的 handler
 * 你这里非常“安全”的做法：
 * - 不调用 readline 那套复杂函数
 * - 只设置一个全局标志 g_signal
 *
 * heredoc 的读取循环会检查 g_signal==SIGINT，然后清理并退出
 */
static void sigint_heredoc(int sig)
{
    (void)sig;
    g_signal = SIGINT;
}

/* setup_heredoc_signals：heredoc 模式信号配置
 * - SIGINT：交给 sigint_heredoc（只设置 g_signal）
 * - SIGTSTP/SIGQUIT：忽略（避免 heredoc 输入时挂起或 core dump）
 */
void setup_heredoc_signals(void)
{
    struct sigaction sa;

    sa.sa_handler = sigint_heredoc;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    sa.sa_handler = SIG_IGN;
    sigaction(SIGTSTP, &sa, NULL);

    sa.sa_handler = SIG_IGN;
    sigaction(SIGQUIT, &sa, NULL);
}
