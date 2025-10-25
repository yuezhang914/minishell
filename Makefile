# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: yzhang2 <yzhang2@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/10/24 15:45:56 by yzhang2           #+#    #+#              #
#    Updated: 2025/10/25 04:24:44 by yzhang2          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME         := minishell
CC           := gcc
CFLAGS       := -Wall -Wextra -Werror -g -fno-pie
LDFLAGS      := -no-pie
INCLUDE_DIR  := include
LIBFT_DIR    := libft
LIBFT        := $(LIBFT_DIR)/libft.a
LDLIBS       := -lreadline

SRC_DIRS     := src \
                src/lexer \
                src/parser \
                src/error \
                src/exec \
                src/cmds \
                src/loop

SRCS         := $(foreach d,$(SRC_DIRS),$(wildcard $(d)/*.c))

OBJS         := $(patsubst src/%.c,obj/%.o,$(SRCS))

OBJDIRS      := $(sort $(dir $(OBJS)))

CPPFLAGS     := -I$(INCLUDE_DIR)

all: $(NAME)

$(NAME): $(LIBFT) $(OBJS)
	@echo "Linking $@ ..."
	$(CC) $(LDFLAGS) $(OBJS) $(LIBFT) $(LDLIBS) -o $@
	@echo "Built $(NAME)"

$(LIBFT):
	@echo "Building libft in $(LIBFT_DIR) ..."
	$(MAKE) -C $(LIBFT_DIR) CFLAGS="$(CFLAGS)" || (echo "libft build failed" && false)

obj/%.o: src/%.c | $(OBJDIRS)
	@echo "CC $<"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(OBJDIRS):
	@mkdir -p $@

clean:
	@echo "Cleaning object files..."
	@rm -rf obj
	@$(MAKE) -C $(LIBFT_DIR) clean

fclean: clean
	@echo "Removing binaries..."
	@rm -f $(NAME)
	@$(MAKE) -C $(LIBFT_DIR) fclean || true

re: fclean all

.PHONY: all bonus clean fclean re
