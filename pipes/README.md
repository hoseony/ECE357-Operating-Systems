# Demonstrating Interrupted System Calls and Short Writes with Pipes

## build
Build with `make`

## Tests (all four combinations)
❯ ./pipe 4096 10000
19 EINTRs
620 SIGUSR1 handled
9981 writes of 4096 bytes
40882176 total bytes written
read returned EOF
1 reads of 211 bytes
1 reads of 291 bytes
1 reads of 374 bytes
73660 reads of 555 bytes
40882176 total bytes read
writer is done
621 SIGUSR1 sent

❯ ./pipe 4096 10000 restart
0 EINTRs
620 SIGUSR1 handled
10000 writes of 4096 bytes
40960000 total bytes written
read returned EOF
1 reads of 445 bytes
73801 reads of 555 bytes
40960000 total bytes read
writer is done
621 SIGUSR1 sent

❯ ./pipe 5550 10000
7 EINTRs
770 SIGUSR1 handled
8 writes of 1454 bytes
26 writes of 4096 bytes
9959 writes of 5550 bytes
55390578 total bytes written
read returned EOF
1 reads of 500 bytes
1 reads of 523 bytes
99801 reads of 555 bytes
55390578 total bytes read
writer is done
773 SIGUSR1 sent

❯ ./pipe 5550 10000 restart
0 EINTRs
745 SIGUSR1 handled
5 writes of 1454 bytes
36 writes of 4096 bytes
9959 writes of 5550 bytes
55427176 total bytes written
read returned EOF
1 reads of 436 bytes
99868 reads of 555 bytes
55427176 total bytes read
writer is done
746 SIGUSR1 sent

