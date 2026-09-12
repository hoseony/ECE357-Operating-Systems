#!/bin/bash 

for a in 1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384 32768 65536; do
    TIMEFORMAT="$a, %R, %U, %S"
    (time ./kit -b $a ./test/64MB.txt > /dev/null) 2>&1
done

# just a note for myself in future
# https://www.cyberciti.biz/faq/unix-linux-time-command-examples-usage-syntax/
# https://unix.stackexchange.com/questions/121200/why-my-tabs-are-not-expanding-in-timeformat
