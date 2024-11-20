
SRCS = main.cpp config.cpp request.cpp utils.cpp connection.cpp
SAN  = -fsanitize=address -g3
FLAGS = --std=c++98 $(SAN)

all:
	bash clear.sh && c++ $(SRCS) $(FLAGS) -o server && ./server ./conf/default.conf