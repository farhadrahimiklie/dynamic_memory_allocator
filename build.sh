#!/bin/bash

# Colors (no Bold)
RED="\033[31m"
GREEN="\033[32m"
RESET="\033[0m"

print_time(){
    local title="$1"
    local DATE=$(date "+%A, %d %B %Y")
    local TIME=$(date "+%I:%M:%S %p")
    
    echo -e "$title $DATE | $TIME"
}

# Start Line
print_time "Compilation ${GREEN}Started${RESET} at"

# Compilte the Program
gcc -Wall -Wextra -Wpedantic Allocator.c main.c -o main

# Run the Program
./main

# move file Content to other file.
# ./main > data.dot


# Finish Line
print_time "Compilation ${RED}Finished${RESET} at"
