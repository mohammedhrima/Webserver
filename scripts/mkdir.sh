#!/bin/bash

# Create parent directory "tmp"
dir1="/home/ironman/Desktop/webserver/serv/name1/src"
dir2="/home/ironman/Desktop/webserver/serv/name2/src"

mkdir -p $dir1/tmp
mkdir -p $dir2/tmp

mkdir -p $dir1/tmp/0
mkdir -p $dir2/tmp/0
mkdir -p $dir1/tmp/1
mkdir -p $dir2/tmp/1

mkdir -p $dir1/tmp/0/0
mkdir -p $dir2/tmp/0/0
mkdir -p $dir1/tmp/0/1
mkdir -p $dir2/tmp/0/1

mkdir -p $dir1/tmp/1/0
mkdir -p $dir2/tmp/1/0
mkdir -p $dir1/tmp/1/1
mkdir -p $dir2/tmp/1/1

mkdir -p $dir1/tmp/0/0/0
mkdir -p $dir2/tmp/0/0/0
mkdir -p $dir1/tmp/0/0/1
mkdir -p $dir2/tmp/0/0/1
mkdir -p $dir1/tmp/0/1/0
mkdir -p $dir2/tmp/0/1/0
mkdir -p $dir1/tmp/0/1/1
mkdir -p $dir2/tmp/0/1/1

mkdir -p $dir1/tmp/1/0/0
mkdir -p $dir2/tmp/1/0/0
mkdir -p $dir1/tmp/1/0/1
mkdir -p $dir2/tmp/1/0/1
mkdir -p $dir1/tmp/1/1/0
mkdir -p $dir2/tmp/1/1/0
mkdir -p $dir1/tmp/1/1/1
mkdir -p $dir2/tmp/1/1/1

echo "Directories created successfully."