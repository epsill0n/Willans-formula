<h1>Implementation of Willans' formula</h1>

In 1964, Willans gave the formula:

<p align="center">

$$

\Large \displaystyle p_n = 1 + \sum_{i=1}^{2^n} \left\lfloor \left(\frac{n}{\sum_{j=1}^{i} \left\lfloor \left(\cos \frac{(j-1)! + 1}{j} \pi \right)^2 \right\rfloor}\right)^{1/n} \right\rfloor

$$

</p>

for generating the $n$-th prime number.

By Wilson's theorem, $n+1$ is prime if and only if $n! \equiv n \pmod{n+1}$. Thus, when $n+1$ is prime, the first factor in the product becomes one, and the formula produces the prime number $n+1$. But when $n+1$ is not prime, the first factor becomes zero and the formula produces the prime number $2$. This formula is not an efficient way to generate prime numbers because evaluating $n! \pmod{n+1}$ requires about $n-1$ multiplications and reductions modulo $n+1$.

<h2>Content</h2>
<h3>Directories</h3>
<h4>Python (/py/)</h4>

This directory is a `Python` implementation of the formula.

- `Willan.py`:
contains the implementation of the formula, which is the function `nthPrime`. This function takes as an argument the number $n$ since it computes the $n$-th prime number;
- `nth_prime.py`:
reads the number $n$ on input and outputs the $n$-th prime number;
- `all_until_nth_prime.py`:
reads the number $n$ on input and outputs all the prime numbers until the $n$-th prime number.

<h4>C++ (/cpp/)</h4>

This directory is a `C++` implementation of the formula.

`Soon to be released.`

<h3>Subdirectories</h3>
<h4>Sequential computation (/seq/)</h4>

This subdirectory contains a sequential implementation of the computation. Namely, every line of code is computed one after the other - no concurrency implemented.

<h4> Concurrent computation (/conc/)</h4>

This directory contains a concurrent implementation of the computation. Namely, some `for` loop calculations are computed in parallel.

`Soon to be released.`