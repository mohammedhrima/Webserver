#!/bin/bash



commands=(
  "GET /kl/src HTTP/1.1\nHost: name1\n\n"
  "GET / HTTP/1.1\nHost: name2\nContent-Length: 10\n\nHelloWorld"
)

# Iterate over the commands array
for cmd in "${commands[@]}"; do
    # Execute the command using netcat
    printf "$cmd" | nc 127.0.0.1 17000
    echo ""
    echo "========================================================================"
done
