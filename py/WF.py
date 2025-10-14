# Willans'_formula - WF.py
# Copyright (c) 2025 epsill0n epsill0n
# Non-commercial use only. Redistribution or modification prohibited without permission.
# See LICENSE.txt for full terms.

import mpmath
import multiprocessing as mp
import threading

mpmath.mp.dps = 77  # decimal digits ~= 256 bits

def _compute_row_worker(args):
    i, n = args
    return Willans.conc.compute_row(i, n)

class Willans:
    _thread_data = threading.local()
    innerSumFunc = None
    modFactFunc = None
    isConcurrent = True
    isCumulative = True

    @staticmethod
    def modFactNotCumulative(j: int, mod: int) -> int:
        r = 1
        for w in range(2, j + 1):
            r = (r * w) % mod
        return r

    @staticmethod
    def modFactCumulative(j: int, mod: int) -> int:
        td = Willans._thread_data
        if not hasattr(td, 'r'):
            td.r = 1
            td.last_j = 0
            td.last_mod = mod
        if mod != getattr(td, 'last_mod', mod):
            td.r = 1
            td.last_j = 0
            td.last_mod = mod
        for w in range(td.last_j + 1, j + 1):
            td.r = (td.r * w) % mod
        td.last_j = j
        td.last_mod = mod
        return td.r

    @staticmethod
    def innerSumNotCumulative(i: int) -> int:
        inner_sum = 0
        for j in range(1, i + 1):
            mf = Willans.modFactFunc(j - 1, j)
            arg = mpmath.pi * (mpmath.mpf(mf) + 1) / mpmath.mpf(j)
            inner_sum += int(mpmath.floor(mpmath.cos(arg) ** 2))
        return inner_sum

    @staticmethod
    def innerSumCumulative(i: int) -> int:
        td = Willans._thread_data
        if not hasattr(td, 'inner_sum_prev'):
            td.inner_sum_prev = 0
            td.last_i = 0
        for j in range(td.last_i + 1, i + 1):
            mf = Willans.modFactFunc(j - 1, j)
            arg = mpmath.pi * (mpmath.mpf(mf) + 1) / mpmath.mpf(j)
            td.inner_sum_prev += int(mpmath.floor(mpmath.cos(arg) ** 2))
        td.last_i = i
        return td.inner_sum_prev

    @staticmethod
    def init_funcs(isConcurrent: int, isCumulative: int):
        Willans.isConcurrent = isConcurrent
        Willans.isCumulative = isCumulative
        Willans.innerSumFunc = Willans.innerSumCumulative if isCumulative else Willans.innerSumNotCumulative
        Willans.modFactFunc = Willans.modFactCumulative if isCumulative else Willans.modFactNotCumulative

    class seq:
        @staticmethod
        def nthPrime(n: int) -> int:
            td = Willans._thread_data
            if Willans.isCumulative:
                td.inner_sum_prev = 0
                td.last_i = 0
                td.r = 1
                td.last_j = 0
                td.last_mod = 0

            outer_sum = 1
            limit = 2 ** n
            for i in range(1, limit + 1):
                inner_sum = Willans.innerSumFunc(i)
                if inner_sum != 0:
                    ratio = mpmath.mpf(n) / mpmath.mpf(inner_sum)
                    root_val = ratio ** (1 / mpmath.mpf(n))
                    outer_sum += int(mpmath.floor(root_val))
            return outer_sum

    class conc:
        @staticmethod
        def compute_row(i: int, n: int) -> int:
            inner_sum = Willans.innerSumFunc(i)
            if inner_sum == 0:
                return 0
            ratio = mpmath.mpf(n) / mpmath.mpf(inner_sum)
            root_val = ratio ** (1 / mpmath.mpf(n))
            return int(mpmath.floor(root_val))

        @staticmethod
        def nthPrime(n: int) -> int:
            total_rows = 2 ** n
            row_indices = [(i, n) for i in range(1, total_rows + 1)]

            with mp.Pool(processes=mp.cpu_count()) as pool:
                row_results = pool.map(_compute_row_worker, row_indices)

            return 1 + sum(row_results)


        
