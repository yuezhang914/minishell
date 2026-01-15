#include "minishell.h"
#include "signals.h"

/* sigint_prompt：提示符模式下 Ctrl-C 的处理函数
 * 注意：sig 是信号编号，这里不需要用它，所以 (void)sig 消除编译警告
 */
void sigint_prompt(int sig)
{
    (void)sig;

    /* 1) 打印一个换行：让光标跳到下一行
     * 用 write 的原因：简单直接（printf 可能有缓冲）
     */
    write(1, "\n", 1);

    /* 2) 告诉 readline：我们现在在新的一行 */
    rl_on_new_line();

    /* 3) 清空用户已经输入但还没回车的内容 */
    rl_replace_line("", 0);

    /* 4) 重新绘制：显示 prompt 和空输入行 */
    rl_redisplay();

    /* 5) 记录：这次发生了 SIGINT
     * 主循环 repl_step 里会检查 g_signal，然后执行“清空 acc、设 130”等动作
     */
    g_signal = SIGINT;

    /* 6) 关键：让 readline 立刻结束阻塞等待
     * 否则你按 Ctrl-C，有时 readline 还在等输入，不返回
     */
    rl_done = 1;
}

/* setup_child_signals：子进程执行命令时的信号策略
 * - 让子进程回到“默认行为”：
 *   Ctrl-C 会杀掉子进程
 *   Ctrl-\ 会让子进程收到 SIGQUIT（可能 core dump）
 *   Ctrl-Z 会暂停子进程
 *
 * 为什么要默认？
 * - 因为外部命令（cat/grep/vim）就应该像正常程序一样响应信号
 */
void setup_child_signals(void)
{
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
}

/* setup_parent_exec_signals：父进程在执行阶段（wait 子进程时）
 * - 父进程忽略 SIGINT/SIGQUIT/SIGTSTP
 *
 * 为什么忽略？
 * - Ctrl-C 应该主要交给前台子进程处理
 * - 父进程如果也响应 Ctrl-C，就可能把 shell 自己弄乱/退出
 */
void setup_parent_exec_signals(void)
{
    struct sigaction sa;

    sa.sa_handler = SIG_IGN;        /* 忽略信号 */
    sigemptyset(&sa.sa_mask);       /* 处理时不额外屏蔽其它信号 */
    sa.sa_flags = 0;                /* 不开 SA_RESTART：执行期你这里选择不开 */

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTSTP, &sa, NULL);
}

/* setup_prompt_signals：主提示符（readline）阶段
 * - SIGINT：用 sigint_prompt（清空当前输入行，重画 prompt）
 * - SIGQUIT / SIGTSTP：忽略（不让 shell 自己 core dump/暂停）
 *
 * SA_RESTART：
 * - 让一些被信号打断的系统调用自动重启
 * - 对交互体验更顺滑（输入不会莫名其妙失败）
 */
void setup_prompt_signals(void)
{
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    /* Ctrl-C：走我们自定义的 prompt handler */
    sa.sa_handler = sigint_prompt;
    sigaction(SIGINT, &sa, NULL);

    /* Ctrl-\ 和 Ctrl-Z：忽略 */
    sa.sa_handler = SIG_IGN;
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTSTP, &sa, NULL);
}
