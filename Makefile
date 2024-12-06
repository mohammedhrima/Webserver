CC = c++
RM = rm -rf
NAME = webserv
SAN = -fsanitize=address -g3 
CPPFLAGS = -std=c++98 -Wall -Werror -Wextra $(SAN)

# Base directories
SRCDIR = server
OBJDIR = objects
INCDIR = $(SRCDIR)/include

# Files
SRCS = $(SRCDIR)/main.cpp $(SRCDIR)/webserver.cpp $(SRCDIR)/request.cpp \
       $(SRCDIR)/config.cpp $(SRCDIR)/utils.cpp $(SRCDIR)/debug.cpp

OBJS = $(addprefix $(OBJDIR)/,$(notdir $(SRCS:.cpp=.o)))

# Rules
all: $(NAME)

$(NAME): $(OBJS) Makefile $(SRCDIR)/header.hpp $(INCDIR)/header.hpp
	$(CC) $(CPPFLAGS) $(OBJS) -o $(NAME)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(shell mkdir -p $(OBJDIR))
	$(CC) $(CPPFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJDIR) ./tester

fclean: clean
	$(RM) $(NAME)

tests:
	$(CC) ./server/tester/tester.cpp -o tester && ./tester

re: fclean all
