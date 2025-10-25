/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yzhang2 <yzhang2@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/24 15:21:40 by yzhang2           #+#    #+#             */
/*   Updated: 2025/10/25 05:26:11 by yzhang2          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include "cmds.h"
# include "error.h"
# include "exec.h"
# include "lexer.h"
# include "libft.h"
# include "loop.h"
# include "parser.h"

# include <fcntl.h>
# include <readline/history.h>
# include <readline/readline.h>
# include <signal.h>
# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
...

	// 唯一的全局信号指示器
	extern volatile sig_atomic_t g_signal;

typedef struct s_general
{
	// lexer



	// parse
	char	**envp;
	char	**paths;


	// exec
	int		pipes;
	int		*pid;


	// heredoc
	bool	heredoc;


	// error
	int		error_num;


	// loop


}			t_general;

#endif
