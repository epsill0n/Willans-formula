import math
import numpy as np
import multiprocessing as mp

def modFact(j: int, i: int) -> int:
    r = 1
    for w in range(2, j + 1):
        r = (r * w) % i
    
    return r

def compute_row(i: int, n: int) -> int:
    j_values = np.arange(1, i + 1)

    mods = np.array([modFact(j - 1, j) for j in j_values])

    cos_vals = np.cos(((mods + 1) / j_values) * np.pi) ** 2
    floor_vals = np.floor(cos_vals)
    inner_sum = int(np.sum(floor_vals))

    if inner_sum == 0:
        return 0
    return int(np.floor((n / inner_sum) ** (1 / n)))

def nthPrime(n: int) -> int:
    row_indices = list(range(1, 2 ** n + 1))
    with mp.Pool(processes = mp.cpu_count()) as pool:
        row_results = pool.starmap(compute_row, [(i, n) for i in row_indices])
    
    return 1 + sum(row_results)
