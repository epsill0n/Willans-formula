from Willan import nthPrime

if __name__=='__main__':
    n = int(input())
    
    print(f'First {n} prime numbers:')
    for i in range (1, n+1):
        print(f'{nthPrime(i)}', end=' ')