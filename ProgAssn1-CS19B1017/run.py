#!/usr/local/bin/python3
import subprocess
import sys
import argparse


def run(n, m):
    inpfile = open("inp-params.txt", "w")
    inpfile.write("{} {}".format(n, m))
    inpfile.close()

    subprocess.call("./primes.out")

    TimeFile = open("Times.txt", "r+")
    times = [float(time)for time in TimeFile.read().split(" ")]
    # file1 = open("Primes-DAM.txt","r+")
    # print(len(file1.read().split(" "))-1,"\t")
    # file1 = open("Primes-SAM1.txt","r+")
    # print(len(file1.read().split(" "))-1,"\t")
    # file1 = open("Primes-SAM2.txt","r+")
    # print(len(file1.read().split(" "))-1,"\t")
    # file1 = open("Primes-Seq.txt","r+")
    # print(len(file1.read().split(" "))-1,"\t")
    # file1.close()
    return times


def cleanup():
    subprocess.call("rm -f inp-params.txt", shell=True)
    subprocess.call("rm -f Primes-DAM.txt", shell=True)
    subprocess.call("rm -f Primes-SAM1.txt", shell=True)
    subprocess.call("rm -f Primes-SAM2.txt", shell=True)
    subprocess.call("rm -f Primes-Seq.txt", shell=True)
    subprocess.call("rm -f Times.txt", shell=True)
    subprocess.call("rm -f primes.out", shell=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='Python Script to Run Assignment-1')
    parser.add_argument('-n', type=int, default=8,
                        help='Fixed n when varying M')
    parser.add_argument('-m', type=int, default=10,
                        help='Fixed m when varying N')
    parser.add_argument('-g', '--graph', action='store_true',
                        help='Show a graph of the results')
    parser.add_argument('-c', '--cleanup', action='store_true', default=False,
                        help='Cleanup the files')
    parser.add_argument('-mrange', nargs=3, type=int,
                        default=[5, 40, 5], help='The range of m values to test in the form [min, max, step]')
    parser.add_argument('-nrange', nargs=3, type=int,
                        default=[3, 9, 1], help='The range of n values to test in the form [min, max, step]')
    args = parser.parse_args()
    args = parser.parse_args()

    cmd = "Src-CS19B1017.cpp"
    subprocess.call(["g++", cmd, "-o", "primes.out"])

    varyM = [run(args.n, m)
             for m in range(args.mrange[0], args.mrange[1], args.mrange[2])]
    varyN = [run(n, args.m)
             for n in range(args.nrange[0], args.nrange[1], args.nrange[2])]

    print("Varying M from {} to {} keeping n {}".format(
        args.mrange[0], args.mrange[1], args.n), "\n", varyM)
    print("Varying N from {} to {} keeping m {}".format(
        args.nrange[0], args.nrange[1], args.m), "\n", varyN)

    if(args.graph):
        import numpy as np
        import matplotlib.pyplot as plt

        varyMx = [m for m in range(args.mrange[0], args.mrange[1], args.mrange[2])]
        data = np.array(varyM).transpose()
        plt.plot(varyMx, data[0], label="DAM")
        plt.plot(varyMx, data[1], label="SAM1")
        plt.plot(varyMx, data[2], label="SAM2")
        plt.plot(varyMx, data[3], label="Seq")
        plt.xlabel("M")
        plt.ylabel("Time in Seconds")
        plt.title("Varying number of threads")
        plt.legend()
        plt.show()

        varyNx = [n for n in range(args.nrange[0], args.nrange[1], args.nrange[2])]
        data = np.array(varyN).transpose()
        plt.plot(varyNx, data[0], label="DAM")
        plt.plot(varyNx, data[1], label="SAM1")
        plt.plot(varyNx, data[2], label="SAM2")
        plt.plot(varyNx, data[3], label="Seq")
        plt.xlabel("N")
        plt.ylabel("Time in Seconds")
        plt.title("Varying the Limit n")
        plt.legend()
        plt.show()
    if(args.cleanup):
        cleanup()
    sys.exit(0)
