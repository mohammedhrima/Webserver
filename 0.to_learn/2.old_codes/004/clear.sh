#!/bin/bash

rm -rf server a.out
# Find the process IDs (PIDs) of processes listening on port 17000
pids=$(lsof -t -i :17000)

# Check if there are any processes listening on port 17000
if [ -z "$pids" ]; then
    echo "No processes are listening on port 17000."
else
    # Kill each process
    echo "Killing processes listening on port 17000..."
    for pid in $pids; do
        kill -9 "$pid"
    done
    echo "Processes killed."
fi


# files="main.cpp config.cpp request.cpp utils.cpp connection.cpp"
# files="request.cpp utils.cpp"

# san="-fsanitize=address -g3"
# c++ $files --std=c++98 $san && ./a.out

clear && rm -rf server 
# && c++ $files --std=c++98 $san -o server && ./server ./conf/default.conf

# clear && rm -rf a.out && c++ $files --std=c++98 -g -o server
# gdb --args ./server ./conf/file.conf
