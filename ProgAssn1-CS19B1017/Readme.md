# Input Output
Input is taken from a file named inp-params.txt of the form:
```
n m
```
n: 10^n is the limit

m: number of threads

# Output
The program generates 5 txt files:
- Primes-DAM.txt: the list of primes up to 10^n using the DAM method
- Primes-SAM1.txt: the list of primes up to 10^n using the SAM1 method
- Primes-SAM2.txt: the list of primes up to 10^n using the SAM2 method
- Primes-Seq.txt: the list of primes up to 10^n using the sequential method
- Times.txt: the times taken to run each method
```
<DAM_time> <SAM1_time> <SAM2_time> <Seq_time>
```
Note:
Numbers in output are not sorted. 
The times are in seconds
# Compile
```
$ g++ Src-CS19B1017.cpp -o primes.out
```
# Run
```
$ ./primes.out
```

# Cleanup
```
$ rm primes.out *.txt
```
# Script
I have also added a python script for easy use of the program and visualisation of the results.

```
usage: run.py [-h] [-n N] [-m M] [-g] [-c] [-mrange MRANGE MRANGE MRANGE] [-nrange NRANGE NRANGE NRANGE]

Python Script to Run Assignment-1

optional arguments:
  -h, --help            show this help message and exit
  -n N                  Fixed n when varying M
  -m M                  Fixed m when varying N
  -g, --graph           Show a graph of the results
  -c, --cleanup         Cleanup the files
  -mrange MRANGE MRANGE MRANGE
                        The range of m values to test in the form [min, max, step]
  -nrange NRANGE NRANGE NRANGE
                        The range of n values to test in the form [min, max, step]
```
Note to use the --graph flag numpy and matplotlib must be installed.
## Example Usage
```
python3 run.py -g -c -n 6 -m 100 -mrange 5 40 5 -nrange 4 8 1
```





