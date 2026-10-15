# miniShell - PS3P3

## Building
```
cd src
make        #regular build
make debug  #build with debug mode
make clean
```

## Usage
```
./miniShell 
```
or run commands from a script
```
./miniShell script.sh
```

## Tests

### test1.sh
```
$ ./miniShell test1.sh
pid 97562 (child) exited with return 0
Real: 0.005s User: 0.001s Sys: 0.002s
pid 97563 (child) exited with return 0
Real: 0.004s User: 0.001s Sys: 0.002s
total 160
-rw-r--r--@ 1 hoseonyu  staff     77 Oct 13 17:45 err.txt
-rwxr-xr-x@ 1 hoseonyu  staff     14 Oct 13 16:57 lsfd.sh
-rwxr-xr-x@ 1 hoseonyu  staff  35528 Oct 14 19:20 miniShell
-rw-r--r--@ 1 hoseonyu  staff     12 Oct 13 17:45 out.txt
-rw-r--r--@ 1 hoseonyu  staff      7 Oct 13 17:45 out2.txt
-rw-r--r--@ 1 hoseonyu  staff     29 Oct 13 17:53 output.txt
-rwxr-xr-x@ 1 hoseonyu  staff     59 Oct  7 09:22 shell.sh
-rw-r--r--@ 1 hoseonyu  staff      8 Oct  6 13:48 test.txt
-rwxr-xr-x@ 1 hoseonyu  staff    314 Oct  9 15:46 test1.sh
-rwxr-xr-x@ 1 hoseonyu  staff    588 Oct 14 09:42 test2.sh
-rw-r--r--@ 1 hoseonyu  staff     18 Oct 14 20:13 testfile.out
-rw-r--r--@ 1 hoseonyu  staff     18 Oct 14 20:13 testfile2.out
pid 97564 (child) exited with return 0
Real: 0.008s User: 0.001s Sys: 0.003s
$ echo $?
17
$ cat testfile.out
TESTINGTESTING123
$ cat testfile2.out
TESTINGTESTING123

```

### test2.sh 

```
$ ./miniShell test2.sh
/private/tmp
/Users/hoseonyu
pid 4751 (child) exited with return 0
Real: 0.005s User: 0.001s Sys: 0.002s
ERROR: execvp failed: No such file or directory
pid 4752 (child) exited with return 127
Real: 0.004s User: 0.000s Sys: 0.002s
pid 4753 (child) exited with return 0
Real: 0.005s User: 0.001s Sys: 0.002s
Can not open testfile.out: Permission denied
pid 4754 (child) exited with return 1
Real: 0.001s User: 0.000s Sys: 0.000s
$ echo $?
1
```
Note: /tmp is symlink to /private/tmp on macOS

### Additional tests

Simple I/O redirection
```
$ ./miniShell
echo hello >out.txt
pid 8305 (child) exited with return 0
Real: 0.010s User: 0.001s Sys: 0.004s
cat out.txt
hello
pid 8306 (child) exited with return 0
Real: 0.007s User: 0.001s Sys: 0.003s
cat <out.txt
hello
pid 8781 (child) exited with return 0
Real: 0.005s User: 0.001s Sys: 0.002s
$ echo $?
0
```

```
$ ./miniShell
echo one >out.txt
pid 74397 (child) exited with return 0
Real: 0.007s User: 0.001s Sys: 0.003s
echo two >>out.txt
pid 74857 (child) exited with return 0
Real: 0.006s User: 0.001s Sys: 0.003s
cat out.txt
one
two
pid 74858 (child) exited with return 0
Real: 0.007s User: 0.001s Sys: 0.003s
cat < thisFileDoesNotExist.txt
Can not open thisFileDoesNotExist.txt: No such file or directory
pid 75777 (child) exited with return 1
Real: 0.003s User: 0.000s Sys: 0.001s
Ctrl-d
$ echo $?
1
```


I/O redirection with stderr and append
```
$ ./miniShell
ls nothing1 2> err.txt
pid 81700 (child) exited with return 1
Real: 0.005s User: 0.001s Sys: 0.002s
ls nothing2 2>>err.txt
pid 82589 (child) exited with return 1
Real: 0.005s User: 0.001s Sys: 0.003s
cat err.txt
ls: nothing1: No such file or directory
ls: nothing2: No such file or directory
pid 82590 (child) exited with return 0
Real: 0.006s User: 0.001s Sys: 0.003s
exit 10
$ echo $?
10
```

Dumpcore
```
$ ./miniShell
./dumpcore
pid 60918 (child) exited with signal 11 (Segmentation fault: 11)
Real: 0.006s User: 0.001s Sys: 0.003s
exit
$ echo $?
139
```
