# Willans'_formula - all_until_nth_prime.py
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

    if (len(sys.argv)!=4):
        print(f'Usage: [{sys.argv[0]}] [n] [1/0 for concurrency] [1/0 for accumulation]')
        exit(-1)
    
    n = int(sys.argv[1])
    isConcurrent = int(sys.argv[2])
    isCumulative = int(sys.argv[3])

    print(f'Using mpmath precision of {mpmath.mp.dps} decimals.')

    Willans.init_funcs(isConcurrent, isCumulative)

    if isConcurrent:
        print("Using concurrency.")
        nthPrime = nthPrime_conc
    else:
        print("Not using concurrency.")
        nthPrime = nthPrime_seq

    if isCumulative:
        print("Using accumulation.")
    else:
        print("Not using accumulation.")
    
    primes = [_ for _ in range(n)]
    start = time.time()
    for i in range(1, n+1):
        primes[i-1] = nthPrime(i)
    end = time.time()

    for i in range (1, n+1):
        print(f'{primes[i-1]}', end=' ')
    
    print(f'\nTime: {(end - start) * 1e3}ms')