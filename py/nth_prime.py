import sys
import time
from conc.Willan import nthPrime as nthPrime_conc
from seq.Willan import nthPrime as nthPrime_seq

if __name__=='__main__':
    
    if (len(sys.argv)!=3):
        print(f'Usage: [{sys.argv[0]}] [n] [1/0 for concurrency] ')
        exit(-1)
    
    n = int(sys.argv[1])
    isConcurrent = int(sys.argv[2])

    if(isConcurrent == 0):
        print('Not using concurrency.')
        nthPrime = nthPrime_seq
    else:
        print('Using concurrency.')
        nthPrime = nthPrime_conc

    start = time.time()
    prime = nthPrime(n)
    end = time.time()

    print(prime)

    print(f'Time: {(end - start) * 1e3}ms')