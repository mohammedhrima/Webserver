# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mhrima <mhrima@student.42.fr>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/06/07 16:53:09 by mhrima            #+#    #+#              #
#    Updated: 2024/06/07 16:53:09 by mhrima           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


CC = c++
RM = rm -rf
NAME = webserv
SAN = -fsanitize=address -g3 
CPPFLAGS = -std=c++98 -Wall -Werror -Wextra $(SAN)
SRCS = main.cpp webserver.cpp request.cpp config.cpp utils.cpp debug.cpp

OBJDIR = objects
OBJS = $(addprefix $(OBJDIR)/,$(SRCS:.cpp=.o))

$(shell mkdir -p $(OBJDIR))

$(NAME): $(OBJS) Makefile header.hpp ./include/header.hpp
	$(CC) $(CPPFLAGS) $(OBJS) -o $(NAME)

$(OBJDIR)/%.o: %.cpp
	$(shell mkdir -p $(OBJDIR))
	$(CC) $(CPPFLAGS) -c $< -o $@

all: $(NAME)
	./webserv ./conf/default.conf

clean:
	$(RM) $(OBJDIR)

fclean: clean
	$(RM) $(NAME) wstester

tests:
	c++ ./tester/tester.cpp -o wstester && ./wstester

re: fclean all
