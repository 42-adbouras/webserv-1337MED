# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/07/11 11:20:47 by adbouras          #+#    #+#              #
#    Updated: 2025/07/21 15:07:39 by adbouras         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

GRN		=	\033[1;92m
YLW		=	\033[1;93m
RED		=	\033[1;91m
CYN		=	\033[1;96m
RST		=	\033[0;39m
CLN		=	\033[K\r\c

CXX		=	c++

CFLAGS	=	-Wall -Wextra -Werror -std=c++98

NAME	=	webserv

SRC		=	$(shell find sources  -name '*.cpp')

NCLD	=	$(shell find includes -name '*.hpp')

HEADERS	=	-I ./includes -I ./includes/serverHeader

OBJ_DIR =	objects/

OBJ		=	$(addprefix $(OBJ_DIR), $(SRC:.cpp=.o))

all: $(NAME)

$(OBJ_DIR)%.o:%.cpp $(NCLD)
	@mkdir -p $(dir $@)
	@echo "$(YLW)[Genarating object file $@]$(RST)$(CLN)"
	@$(CXX) $(CFLAGS) $(HEADERS) -c $< -o $@ 

$(NAME): $(OBJ)
	@echo "$(YLW)[Compiling $(NAME)]$(RST)$(RST)$(CLN)"
	@$(CXX) $(CFLAGS) $(OBJ) -o $(NAME)
	@echo "$(GRN)[Compiled successfully, to run, execute: ./$(NAME)]$(RST)"

clean:
	@rm -rf $(OBJ_DIR)
	@echo "$(RED)[Objects cleaned]$(RST)"

fclean: clean
	@rm -rf $(NAME)
	@echo "$(RED)[$(NAME) cleaned]$(RST)"

run: all
	@echo "$(CYN)[Running $(NAME)]$(RST)"
	@./$(NAME)

re: fclean all
