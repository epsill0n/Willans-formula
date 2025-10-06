import math as m

def modFact(j: int, i: int) -> int:
    r = 1
    for w in range(2, j + 1):
        r = (r * w) % i
    
    return r

def nthPrime(n: int) -> int:
    outer_sum = 1
    for i in range(1, 2 ** n + 1):
        inner_sum = 0
        for j in range(1, i + 1):
            inner_sum += m.floor(m.cos(((modFact(j-1, j) + 1)/(j)) * m.pi) ** 2)
        
        outer_sum += 0 if inner_sum==0 else m.floor( (n/(inner_sum)) ** float(1/n))
    
    return outer_sum