<h1>Implementation of Willans' formula</h1>

In 1964, Willans gave the formula:

<h3 align="center">

$$
\large
p_n = 1 + \sum_{i=1}^{2^n} \left\lfloor \left(\frac{n}{\sum_{j=1}^{i} \left\lfloor \left(\cos \frac{(j-1)! + 1}{j} \pi \right)^2 \right\rfloor}\right)^{1/n} \right\rfloor
$$

</h3>


for generating the $n$-th prime number.

By Wilson's theorem, $n+1$ is prime if and only if $n! \equiv n \pmod{n+1}$. Thus, when $n+1$ is prime, the first factor in the product becomes one, and the formula produces the prime number $n+1$. But when $n+1$ is not prime, the first factor becomes zero and the formula produces the prime number $2$. This formula is not an efficient way to generate prime numbers because evaluating $n! \pmod{n+1}$ requires about $n-1$ multiplications and reductions modulo $n+1$.

<h2>Content</h2>

```
Willans'_formula/
├── py/
│   ├── seq/
│   │   └── Willan.py
│   ├── conc/
│   │   └── Willan.py
│   ├── nth_prime.py
│   └── all_until_nth_prime.py
└── README.md
```   

<h3>/py/</h3>



`Python` implementation of the formula.

- `Willan.py`:
contains the implementation of the formula, which is the function `nthPrime`. This function takes a single argument, the number $n$, since it computes the $n$-th prime number;
- `nth_prime.py`:
outputs the $n$-th prime number;
- `all_until_nth_prime.py`: outputs all the prime numbers until the $n$-th prime number.

`/seq/` contains a simple implementation that executes all computations sequentially, without using concurrency or parallel processing.


`/conc/` contains an implementation including concurrency. It works by:

1. **Parallelizing the outer loop** - instead of creating a separate task for every small computation, each process handles one entire "row" of Willans’ formula, i.e., all computations for a given `i`. This reduces the number of parallel tasks from ~$2^{(2n)}$ to $2^n$, making multiprocessing efficient.

2. **Using NumPy for inner calculations** - for each row, all the cosine, squaring, flooring, and summing operations are performed using NumPy arrays. This leverages vectorized operations for speed instead of Python loops.

3. **Efficient multiprocessing** - a pool of worker processes (`mp.Pool`) runs the row computations in parallel across available CPU cores. Each worker returns its computed row sum, and the results are combined to compute the final nth prime.

Overall, this approach balances concurrency and performance, avoids excessive process overhead, and uses NumPy for fast numerical operations.


<h2>Usage</h2>  

<h3>Python</h3>

The scripts take 2 CLI arguments: 
1. $n$;
2. Indicator whether or not to use concurrency: 0 means that *concurrency will **NOT** be used*; any other value means that *concurrency will indeed **be used***.

**At the end, the time it took to do the computations for calculating the prime(s) is printed to console.**

<br>
<br>

Example:
```bash
py ./py/nth_prime.py 11 1
```
computes the 11th (*first argument*) prime number **with** concurrency (*second argument*).

Output:
```bash
Using concurrency.
31
Time: 20538.02990913391ms
```

<br>
<br>

Example:
```bash
py ./py/all_until_nth_prime.py 9 0
```
computes the first 9 (*first argument*) prime numbers **without** concurrency (*second argument*).

Output:
```bash
Not using concurrency.
2 3 5 7 11 13 17 19 23 
Time: 815.2904510498047ms
```

<h2>Soon</h2>

- Optimized `all_until_nth_prime.py` (avoiding some recalculations).
- `C++` implementation of the formula for CPU - both sequential and concurrent.
- `C++` implementation of the formula for CUDA devices.