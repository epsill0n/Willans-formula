# Willans'_formula - nth_prime.py
# Copyright (c) 2025 epsill0n epsill0n
# Non-commercial use only. Redistribution or modification prohibited without permission.
# See LICENSE.txt for full terms.

import sys
import time
from WF import Willans
nthPrime_seq = Willans.seq.nthPrime
nthPrime_conc = Willans.conc.nthPrime
import mpmath

if __name__=='__main__':
    
    if (len(sys.argv)!=3):
        print(f'Usage: [{sys.argv[0]}] [n] [1/0 for concurrency]')
        exit(-1)
    
    print(f'Using mpmath precision of {mpmath.mp.dps} decimals.')
    
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

    print(f'Time: {((end - start) * 1e3):.10f}ms')