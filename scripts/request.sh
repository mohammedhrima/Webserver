#!/bin/bash

# curl -X GET --http1.1 --header "Host: name1" --output file.txt --data @request.txt http://localhost:17000
echo "" > response.log
counter=0
while [ $counter -lt 100 ]; do
    # Send HTTP request using curl
    response=$(curl -s -X GET localhost:17000/003.txt)
    # curl -s localhost:17000/001.txt

    # printf "%04d: %s\n" $i "$response" >> response.log
    
    # Increment counter
    ((counter++))
    echo "$response" >> response.log
    echo "" >> response.log
    echo $counter
done

cat response.log