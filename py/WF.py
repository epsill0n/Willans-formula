# Willans'_formula - WF.py
# Copyright (c) 2025 epsill0n epsill0n
# Non-commercial use only. Redistribution or modification prohibited without permission.
# See LICENSE.txt for full terms.

import mpmath
import multiprocessing as mp

mpmath.mp.dps = 77 # decimal digits ~= 256 bits

class Willans:
    @staticmethod
    def modFact(j: int, i: int) -> int:
        r = 1
        for w in range(2, j + 1):
            r = (r * w) % i
        
        return r
    
    class seq:
        @staticmethod
        def nthPrime(n: int) -> int:
            outer_sum = 1
            for i in range(1, 2 ** n + 1):
                inner_sum = 0
                for j in range(1, i + 1):
                    mf = Willans.modFact(j - 1, j)
                    mf_mp = mpmath.mpf(mf)
                    j_mp = mpmath.mpf(j)
                    arg = mpmath.pi * (mf_mp + 1) / j_mp
                    cos_sq = mpmath.cos(arg) ** 2
                    inner_sum += int(mpmath.floor(cos_sq))
                
                if inner_sum != 0:
                    n_mp = mpmath.mpf(n)
                    ratio = n_mp / inner_sum
                    root_val = ratio ** (1 / n_mp)
                    outer_sum += int(mpmath.floor(root_val))
                
            return outer_sum
        
    class conc:
        @staticmethod
        def compute_row(i: int, n: int) -> int:
            inner_sum = 0
            
            for j in range(1, i + 1):
                mf = Willans.modFact(j - 1, j)

                mf_mp = mpmath.mpf(mf)
                j_mp = mpmath.mpf(j)
                arg = mpmath.pi * (mf_mp + 1) / j_mp
                cos_sq = mpmath.cos(arg) ** 2
                inner_sum += int(mpmath.floor(cos_sq))

            if inner_sum == 0:
                return 0
            
            n_mp = mpmath.mpf(n)
            ratio = n_mp / inner_sum
            root_val = ratio ** (1 / n_mp)
            return int(mpmath.floor(root_val))

        @staticmethod
        def nthPrime(n: int) -> int:
            total_rows = 2 ** n
            row_indices = range(1, total_rows + 1)

            with mp.Pool(processes=mp.cpu_count()) as pool:
                row_results = pool.starmap(Willans.conc.compute_row, [(i, n) for i in row_indices])
            
            return 1 + sum(row_results)