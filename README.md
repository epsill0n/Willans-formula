<h1>Implementation of Willan's formula</h1>

In 1964, Willans gave the formula:

$\displaystyle p_n = 1 + \sum_{i=1}^{2^n} \left\lfloor \left(\frac{n}{\sum_{j=1}^{i} \left\lfloor \left(\cos \frac{(j-1)! + 1}{j} \pi \right)^2 \right\rfloor}\right)^{1/n} \right\rfloor$

for generating the $n$-th prime number.

By Wilson's theorem, $n+1$ is prime if and only if $n! \equiv n \pmod{n+1}$. Thus, when $n+1$ is prime, the first factor in the product becomes one, and the formula produces the prime number $n+1$. But when $n+1$ is not prime, the first factor becomes zero and the formula produces the prime number $2$. This formula is not an efficient way to generate prime numbers because evaluating $n! \pmod{n+1}$ requires about $n-1$ multiplications and reductions modulo $n+1$.