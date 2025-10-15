# Implementation of Willans’ Formula for the n-th Prime

> This repository implements Willans’ formula - a remarkable mathematical expression that generates the *n*-th prime number.
> Implemented in both **Python** (using `mpmath`) and **C++** (using `GMP`/`MPFR`), it demonstrates the use of arbitrary-precision arithmetic, concurrency, and modular optimization.

<h1>Implementation of Willans' formula</h1>

In 1964, Willans gave the formula:

<div align="center">

$$
\Large \displaystyle
p_n = 1 + \sum_{i=1}^{2^n} \left\lfloor \left(\frac{n}{\displaystyle \sum_{j=1}^{i} \left\lfloor \left(\cos \frac{(j-1)! + 1}{j} \pi \right)^2 \right\rfloor}\right)^{1/n} \right\rfloor
$$

</div>


for generating the $n$-th prime number.

By Wilson's theorem, $n+1$ is prime if and only if $n! \equiv n \pmod{n+1}$. Thus, when $n+1$ is prime, the first factor in the product becomes one, and the formula produces the prime number $n+1$. But when $n+1$ is not prime, the first factor becomes zero and the formula produces the prime number $2$. This formula is not an efficient way to generate prime numbers because evaluating $n! \pmod{n+1}$ requires about $n-1$ multiplications and reductions modulo $n+1$.

> **Precision note:** Throughout this project, "256-bit precision" and "77 decimal digits" refer to the same numerical precision level (since $\log_2(10^{77}) \approx 256$ bits). This equivalence applies to both the **MPFR** (C++) and **mpmath** (Python) implementations, ensuring consistent results across languages.

## Table of Contents
1. [Overview](#implementation-of-willans-formula-for-the-n-th-prime)
2. [Content & Functionality](#content--functionality)
3. [Python Implementation](#python-implementation)
4. [C++ Implementation](#c-implementation)
5. [Optimizations](#optimizations)
6. [Dependencies](#dependencies)
7. [Usage](#usage)
8. [Benchmark](#benchmark)
9. [License](#license)

---

<h2>Content & Functionality</h2>

```
Willans'_formula/
├── py/
│   ├── WF.py
│   ├── nth_prime.py
│   └── all_until_nth_prime.py
│
├── cpp/
│   ├── WF.hpp
│   ├── nth_prime.cpp
│   ├── all_until_nth_prime.cpp
│   ├── nth_prime.exe                 (need to be built)
│   └── all_until_nth_prime.exe       (need to be built)
│
├── CMakeLists.txt
├── README.md
└── LICENSE.txt
```

> **Note:** The compiled C++ binaries intentionally use the `.exe` suffix for naming consistency and clarity across platforms.  
> Although this project builds and runs on Linux, macOS, and Windows alike, the `.exe` extension is retained to clearly mark executable files and maintain uniformity in documentation and build scripts.

<h3>py/</h3>

`Python` implementation of the formula.

- `WF.py`:
contains the implementation of the formula;
- `nth_prime.py`:
outputs the $n$-th prime number;
- `all_until_nth_prime.py`: outputs all the prime numbers until the $n$-th prime number.

<br>

`WF.py` wraps the implementation through the `Willans` class, which wraps around two other classes: `seq` and `conc`. Both contain their own `nthPrime` function.

### 1. Sequential Implementation (`Willans.seq.nthPrime`)

`seq` contains a simple implementation that executes all computations sequentially, without using concurrency or parallel processing.

### 2. Concurrent Implementation (`Willans.conc.nthPrime`)

`conc` contains a concurrent implementation of Willans’ formula for the $n$-th prime. It achieves parallelism and precision through the following design:

1. Arbitrary-precision arithmetic with `mpmath` - all floating-point computations (cosine, division, roots, etc.) are performed using the `mpmath` library with a default precision of 77 decimals (mpmath.mp.dps = 77). This ensures numerical accuracy even for large values of n.

2. Row-wise parallelization - each worker process computes the contribution of one or more outer-sum indices `i` (each corresponding to one "row" in the summation). This means the outer loop over `i` is parallelized, while the inner computations remain serial within each process. This strategy avoids creating millions of tiny tasks and keeps multiprocessing efficient.

3. Efficient process pooling - a `multiprocessing.Pool` is used to distribute rows across all available CPU cores. Each process returns its computed contribution to the final sum, which are then combined to produce the nth prime.

Each process uses a thread-local cache for factorial and inner-sum values, ensuring correct and thread-safe accumulation.

<h3>cpp/</h3>

- `WF.hpp`: contains the implementations of the formula, as well as helper functions;
- `nth_prime.cpp`: source code for outputting the $n$-th prime number;
- `all_until_nth_prime.cpp`: source code for outputting all the prime numbers until the $n$-th prime number.

Willans’ formula is implemented inside the `WF` namespace, defined in `WF.hpp`. This namespace encapsulates two other namespaces, `seq` and `conc`, both containing an implementation of the formula for computing the $n$-th prime number, as well as some helper functions.

The implementation leverages **GMP** (`mpz_class`) for arbitrary-size integers and **MPFR** (`mpfr::mpreal`) for high-precision floating-point arithmetic (256-bit precision by default).

### 1. Sequential Implementation (`WF::seq::nthPrime`)

It computes the $n$-th prime number entirely on a single thread;

### 2. Concurrent Implementation (`WF::conc::nthPrime`)

It parallelizes the outer loop over `i` using `std::thread`:
1. The total number of rows ($2^n$) is divided into approximately equal chunks, one per thread.
2. Each thread independently computes a range of rows using `WF::conc::compute_row(i, n)`, which performs the same per-row calculation as the sequential version.
3. After all threads finish, their partial sums are combined to produce the final result.

This approach:
- Parallelizes coarse-grained work (each row computation) to reduce total runtime.
- Avoids race conditions by storing each thread’s partial result in a separate vector entry.
- Scales with available CPU cores using `std::thread::hardware_concurrency()` for automatic thread count selection.
- Maintains full arbitrary-precision arithmetic using **GMP**/**MPFR** within each thread.

The concurrent implementation achieves significant performance gains on multi-core systems while preserving mathematical accuracy identical to the sequential version.

<h4>Type Conversion Helpers</h4>

The implementation includes two utility functions for converting between high-precision floating-point numbers (`mpfr::mpreal`) and arbitrary-size integers (`mpz_class`):
1. `WF::to_mpz(const mpfr::mpreal& x)`
    - Converts an `mpfr::mpreal` to an exact `mpz_class` integer.
    - Performs a floor-like rounding using **MPFR**’s `MPFR_RNDZ` mode to ensure no fractional part remains.
    - Used when inner or outer sums of Willans’ formula need to be stored as exact integers.

2. `WF::to_mpreal(const mpz_class& z)`
    - Converts an `mpz_class` integer to an exact `mpfr::mpreal`.
    - Ensures that subsequent floating-point operations (division, cosine, root) are performed with high precision.

<h3>Optimizations</h3>

<h4>Modular Factorial</h4>

Willans' formula requires computing factorials modulo a number. Instead of calculating the full factorial `j!`, which grows extremely fast, we compute it modulo `i`:

$$
\Large \displaystyle
\text{modFact}(j, i) = (1 \cdot 2 \cdot 3 \cdot \cdots \cdot j) \bmod i
$$

This keeps intermediate numbers manageable while preserving correctness, since only $(j-1)! + 1 \bmod j$ is required in the formula.

- **C++ Implementation** uses `mpz_class` for arbitrary-precision integers and optimizes for small values with `unsigned long` arithmetic.
- **Python Implementation** uses standard Python integers, which support arbitrary precision by default.

<h5>Explanation</h5>

For the key term in Willans' formula, we compute:

$$
\Large \displaystyle
\text{modFact}(j-1, j) = (1 \cdot 2 \cdot \dots \cdot (j-1)) \bmod j
$$

Conceptually:

$$
\Large \displaystyle
\text{modFact}(j-1, j)
= 1 \cdot 2 \cdot 3 \cdot \cdots \cdot (j-1) \bmod j
= ( (1 \bmod j) \cdot (2 \bmod j) \cdot \cdots \cdot ((j-1) \bmod j) ) \bmod j
$$

- Only the remainder modulo `j` matters at each step.
- This keeps intermediate numbers small, avoiding huge factorials.
- When `j` is prime, by Wilson's theorem:
    - $(j-1)! \equiv -1 (\bmod j)$

> In short: `modFact(j-1, j)` efficiently computes the factorial modulo `j` without ever storing the full gigantic factorial, which is crucial for both sequential and concurrent implementations.

<br>

**Example Implementations**

C++:

```cpp
inline mpz_class modFact(const mpz_class& j, const mpz_class& i) {
    mpz_class r = 1;

    if (j.fits_ulong_p() && i.fits_ulong_p()) {
        const unsigned long jl = j.get_ui();
        const unsigned long il = i.get_ui();
        for (unsigned long w = 2; w <= jl; r = (r * w++) % il);
    } else for (mpz_class w = 2; w <= j; r = (r * w++) % i);

    return r;
}
```

Python:
```py
def modFact(j: int, i: int) -> int:
    r = 1
    for w in range(2, j + 1):
        r = (r * w) % i
    return r
```

> By using modular factorials, both sequential and concurrent implementations avoid huge numbers and maintain full precision, making the computation feasible even for larger $n$.

<h4>Caching & Cumulative Functions</h4>

<h5>Inner Sum</h5>

Let us denote the inner sum as:

$$
\Large \displaystyle
\sum_{j=1}^{i} \left\lfloor \left(\cos \frac{(j-1)! + 1}{j} \pi \right)^2 \right\rfloor = \sum_{j=1}^{i} \varphi(j) = \Phi(i) 
$$

Therefore,

$$
\Large \displaystyle
\sum_{j=1}^{i} \varphi(j) = \sum_{j=1}^{i-1} \varphi(j) + \varphi(i)
$$

$$
\Large \displaystyle
\iff
$$

$$
\Large \displaystyle
\Phi(i) = \Phi(i-1) + \varphi(i)
$$

Because $i$ goes from $1$ to $2^n$ in the outer sum, we first compute $\varphi(i-1)$ and then $\varphi(i)$.

We can avoid recomputations by caching the latest computed value for $\varphi$.

<br>

<h5>Modular Factorial</h5>

Let us denote the modular factorial, as:

$$
\Large \displaystyle
j! \bmod i = \Psi(j, i)
$$

Since

$$
\Large \displaystyle
j! = (j-1)! \cdot j,
$$

follows

$$
\Large \displaystyle
j! \bmod i = \left( (j-1)! \bmod i \right) \cdot \left( j \bmod i \right),
$$

$$
\Large \displaystyle
\iff
$$

$$
\Large \displaystyle
\Psi(j,i) = \Psi(j-1, i) \cdot \left( j \bmod i \right)
$$

Because in the inner sum, $j$ goes from $1$ to $i$, we compute $\Psi(j-1,i)$ $\left(\text{modFact }(j-2,j)\text{ in the code}\right)$ before $\Psi(j-2,i)$ $\left(\text{modFact }(j-3,j)\text{ in the code}\right)$.

We can avoid recomputations by caching the latest computed value for $\Psi$. We also need to save the modulus $i$ to check if the cached modular factorial is relevant.

> Note: Accumulation here refers to using a cumulative cache for inner-sum values (Φ(i)) or modular factorials (Ψ(j, i)), avoiding recomputation and redundant arithmetic.

<h2>Dependencies</h2>

<h3>Python</h3>

The Python version uses the package **mpmath** for precise floating-point arithmetic. 

**mpmath** is a free (BSD licensed) Python library for real and complex floating-point arithmetic with arbitrary precision.

<br>

Install requirements:
```bash
pip install mpmath
```




<h3>C++</h3>

The C++ version uses libraries **GMP**, **MPFR** and **mpreal.h**. 

**GMP** (GNU Multiple Precision Arithmetic Library) is a free library for arbitrary-precision arithmetic, operating on signed integers, rational numbers, and floating-point numbers. There are no practical limits to the precision except the ones implied by the available memory. GMP has a rich set of functions, and the functions have a regular interface.

**GNU MPFR** (GNU Multiple Precision Floating-Point Reliable Library) is a GNU portable C library for arbitrary-precision binary floating-point computation with correct rounding, based on GNU Multiple Precision Arithmetic Library.

**MPREAL** (MPFR C++) is a multiple precision floating point arithmetic library for C++. Thread-safe, cross-platform (MSVC, GCC, ICC), one-header C++ library.

<br>

For downloads, please visit:
- **GMP**: https://gmplib.org/#DOWNLOAD
- **MPFR**: https://www.mpfr.org/mpfr-current/#download
- **MPREAL**: http://www.holoborodko.com/pavel/mpfr/#download

<br>
<br>
<br>
<br>

***Credits go to the developers for making these libraries and I would like to thank them for that. I wouldn't have been able to make this project as such otherwise.***

<h2>Usage</h2>  

<h3>Python</h3>

The scripts take 2 CLI arguments: 
1. $n$;
2. Indicator whether or not to use concurrency: 0 means that *concurrency will **NOT** be used*; any other value means that *concurrency will indeed **be used***.
3. Indicator whether or not to use accumulation: 0 means that *accumulation will **NOT** be used*; any other value means that *accumulation will indeed **be used***.

<br>
<br>

**At the start of execution, the precision in decimal places is printed to console. The default precision I have set is 77 decimals (around 256 bits) via**
```py
mpmath.mp.dps = 77
```
**in** `WF.py` **(line 10). You can change it to your liking.**
**In the end, the time it took to do the computations for calculating the prime(s) is printed to console.**

<br>
<br>
<br>
<br>

Example:
```bash
py ./py/nth_prime.py 11 1 0
```
computes the 11th (*first argument*) prime number **with** concurrency (*second argument*) and **without** accumulation (*third argument*).

Output:
```bash
Using mpmath precision of 77 decimals.
Using concurrency.
Not using accumulation.
31
Time: 17515.6862735748ms
```

<br>
<br>

Example:
```bash
py ./py/all_until_nth_prime.py 9 0 1
```
computes the first 9 (*first argument*) prime numbers **without** concurrency (*second argument*) and **with** accumulation (*third argument*).

Output:
```bash
Using mpmath precision of 77 decimals.
Not using concurrency.
Using accumulation.
2 3 5 7 11 13 17 19 23 
Time: 76.12895965576172ms
```

<h3>C++</h3>

Inside the root directory (`Willans'_formula`) run:
```bash
cmake -S . -B build
```
This creates a `build/` directory containing the generated Makefiles.

To build the executables, run:
```bash
cmake --build build
```
or equivalently:
```bash
cd build
make
```

After a successful build, you'll find the compiled executables here:
```bash
cpp/nth_prime.exe
cpp/all_until_nth_prime.exe
```
The executables take 2 CLI arguments: 
1. $n$;
2. Indicator whether or not to use concurrency: 0 means that *concurrency will **NOT** be used*; any other value means that *concurrency will indeed **be used***.
3. Indicator whether or not to use accumulation: 0 means that *accumulation will **NOT** be used*; any other value means that *accumulation will indeed **be used***.

<br>
<br>

**At the start of execution, the *MPFR* floating-point precision (in bits) is printed to console. The default precision I have set is 256 bits (around 77 decimals) via**

```cpp
#   define MPFR_PRECISION 256
```

**in** `WF.hpp` **(line 19). You can change it to your liking.**

**In the end, the time it took to do the computations for calculating the prime(s) is printed.**

<br>
<br>
<br>
<br>

Example:
```bash
./cpp/nth_prime.exe 11 1 0
```
computes the 11th (*first argument*) prime number **with** concurrency (*second argument*) and **without** accumulation (*third argument*).

Output:
```bash
Using MPFR precision of 256 bits.
Using concurrency.
Not using accumulation.
31
Time: 2604.9122080000ms
```

<br>
<br>

Example:
```bash
./cpp/all_until_nth_prime.exe 9 0 1
```
computes the first 9 (*first argument*) prime numbers **without** concurrency (*second argument*) and **with** accumulation (*third argument*).

Output:
```bash
Using MPFR precision of 256 bits.
Not using concurrency.
Using accumulation.
2 3 5 7 11 13 17 19 23 
Time: 12.3837230000ms
```

<h3>C++ Initialization Note</h3>

The C++ executables include a small initialization function at startup:

```cpp
__attribute__((constructor)) void init() {
#ifdef FAST_IO
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(0);
#endif
    WF::init_mpfr();
}
```

Details:
1. The `__attribute__((constructor))` syntax is **specific to GCC and Clang**. It ensures that `init()` executes automatically before `main()`.
2. On **MSVC (Microsoft Visual C++)**, this attribute is not supported, so the function will not run automatically. MSVC users would need to call **init()** manually if necessary.
3. What `init()` does: 
- Enables fast I/O if `FAST_IO` is defined.
- Initializes MPFR arbitrary-precision arithmetic (`WF::init_mpfr()`), setting the floating-point precision for computations.

> Users do not need to call this function manually on GCC/Clang platforms. It ensures that the program starts in a ready state for both sequential and concurrent computations.


<h2>Benchmark</h2>

### Setup

| Component        | Specification                                                 |
|------------------|----------------------------------------------------------------|
| CPU              | AMD Ryzen 7 5825U                                             |
| RAM              | 16 GiB DDR4 SDRAM                                             |
| OS               | openSUSE Tumbleweed                                           |
| Python Version   | 3.13.3                                                        |
| C++ Compiler     | g++ (GCC) 14.2.1                                              |
| Precision        | Python: 77 decimals (~256 bits), C++: 256 bits (~77 decimals) |
| Fast I/O (C++)   | Enabled via `FAST_IO` macro                                   |

---

### `nth_prime` Benchmark (Python vs C++, sequential vs concurrent)

<table class="tg" style="undefined;table-layout: fixed; width: 1010px"><colgroup>
<col style="width: 200px">
<col style="width: 200px">
<col style="width: 200px">
<col style="width: 200px">
<col style="width: 200px">
<col style="width: 200px">
<col style="width: 200px">
<col style="width: 200px">
<col style="width: 200px">
</colgroup>
<tbody>
  <tr>
    <td class="tg-0pky"></td>
    <td class="tg-c3ow" colspan="4">Non-cumulative</td>
    <td class="tg-c3ow" colspan="4">Cumulative </td>
  </tr>
  <tr>
    <td class="tg-0pky"></td>
    <td class="tg-c3ow" colspan="2">Sequential</td>
    <td class="tg-c3ow" colspan="2">Concurrent</td>
    <td class="tg-c3ow" colspan="2">Sequential</td>
    <td class="tg-c3ow" colspan="2">Concurrent</td>
  </tr>
  <tr>
    <td class="tg-c3ow">n (upto)</td>
    <td class="tg-c3ow">Python</td>
    <td class="tg-c3ow">C++</td>
    <td class="tg-c3ow">Python</td>
    <td class="tg-c3ow">C++</td>
    <td class="tg-c3ow">Python</td>
    <td class="tg-c3ow">C++</td>
    <td class="tg-c3ow">Python</td>
    <td class="tg-c3ow">C++</td>
  </tr>
  <tr>
    <td class="tg-c3ow">1</td>
    <td class="tg-lboi">🟡 0.322342</td>
    <td class="tg-lboi">🟢 0.120215</td>
    <td class="tg-lboi">🟡 27.131081</td>
    <td class="tg-lboi">🟡 0.839459</td>
    <td class="tg-0pky">🟡 0.438928</td>
    <td class="tg-0pky">🟡 0.181741</td>
    <td class="tg-0pky">🔴 42.539119</td>
    <td class="tg-0pky">🟡 1.136843</td>
  </tr>
  <tr>
    <td class="tg-c3ow">2</td>
    <td class="tg-lboi">🟡 0.433207</td>
    <td class="tg-lboi">🟡 0.165149</td>
    <td class="tg-lboi">🟡 14.464855</td>
    <td class="tg-lboi">🟡 0.627323</td>
    <td class="tg-0pky">🟡 0.377178</td>
    <td class="tg-0pky">🟢 0.105228</td>
    <td class="tg-0pky">🔴 24.685382</td>
    <td class="tg-0pky">🟡 0.818946</td>
  </tr>
  <tr>
    <td class="tg-c3ow">3</td>
    <td class="tg-lboi">🟡 1.562834</td>
    <td class="tg-lboi">🟡 0.185036</td>
    <td class="tg-lboi">🟡 16.047955</td>
    <td class="tg-lboi">🟡 0.586396</td>
    <td class="tg-0pky">🟡 1.074552</td>
    <td class="tg-0pky">🟢 0.089237</td>
    <td class="tg-0pky">🔴 26.222229</td>
    <td class="tg-0pky">🟡 0.880562</td>
  </tr>
  <tr>
    <td class="tg-c3ow">4</td>
    <td class="tg-lboi">🟡 4.510164</td>
    <td class="tg-lboi">🟡 0.578381</td>
    <td class="tg-lboi">🟡 15.815020</td>
    <td class="tg-lboi">🟡 0.487442</td>
    <td class="tg-0pky">🟡 1.781463</td>
    <td class="tg-0pky">🟢 0.177162</td>
    <td class="tg-0pky">🔴 27.725458</td>
    <td class="tg-0pky">🟡 0.718188</td>
  </tr>
  <tr>
    <td class="tg-c3ow">5</td>
    <td class="tg-lboi">🟡 15.270472</td>
    <td class="tg-lboi">🟡 2.020497</td>
    <td class="tg-lboi">🟡 20.074606</td>
    <td class="tg-lboi">🟡 0.633243</td>
    <td class="tg-0pky">🟡 3.138542</td>
    <td class="tg-0pky">🟢 0.322215</td>
    <td class="tg-0pky">🔴 28.650760</td>
    <td class="tg-0pky">🟡 0.719690</td>
  </tr>
  <tr>
    <td class="tg-c3ow">6</td>
    <td class="tg-lboi">🔴 55.511951</td>
    <td class="tg-lboi">🟡 7.697089</td>
    <td class="tg-lboi">🟡 26.605129</td>
    <td class="tg-lboi">🟡 1.169236</td>
    <td class="tg-0pky">🟡 5.635499</td>
    <td class="tg-0pky">🟢 0.710593</td>
    <td class="tg-0pky">🟡 33.384323</td>
    <td class="tg-0pky">🟡 0.838283</td>
  </tr>
  <tr>
    <td class="tg-c3ow">7</td>
    <td class="tg-lboi">🔴 207.010746</td>
    <td class="tg-lboi">🟡 21.057605</td>
    <td class="tg-lboi">🟡 50.860405</td>
    <td class="tg-lboi">🟡 3.358850</td>
    <td class="tg-0pky">🟡 11.571407</td>
    <td class="tg-0pky">🟡 1.460400</td>
    <td class="tg-0pky">🟡 41.068792</td>
    <td class="tg-0pky">🟢 0.914786</td>
  </tr>
  <tr>
    <td class="tg-c3ow">8</td>
    <td class="tg-lboi">🔴 834.560394</td>
    <td class="tg-lboi">🟡 90.869233</td>
    <td class="tg-lboi">🟡 152.147293</td>
    <td class="tg-lboi">🟡 12.753101</td>
    <td class="tg-0pky">🟡 26.289463</td>
    <td class="tg-0pky">🟡 3.068867</td>
    <td class="tg-0pky">🟡 51.230430</td>
    <td class="tg-0pky">🟢 1.459137</td>
  </tr>
  <tr>
    <td class="tg-c3ow">9</td>
    <td class="tg-lboi">🔴 3685.491562</td>
    <td class="tg-lboi">🟡 428.346209</td>
    <td class="tg-lboi">🟡 604.141951</td>
    <td class="tg-lboi">🟡 63.768299</td>
    <td class="tg-0pky">🟡 60.386419</td>
    <td class="tg-0pky">🟡 6.386711</td>
    <td class="tg-0pky">🟡 100.171566</td>
    <td class="tg-0pky">🟢 2.365878</td>
  </tr>
  <tr>
    <td class="tg-c3ow">10</td>
    <td class="tg-lboi">🔴 18574.532509</td>
    <td class="tg-lboi">🟡 2351.954378</td>
    <td class="tg-lboi">🟡 2924.258471</td>
    <td class="tg-lboi">🟡 415.306161</td>
    <td class="tg-0pky">🟡 165.998458</td>
    <td class="tg-0pky">🟡 14.297772</td>
    <td class="tg-0pky">🟡 237.833499</td>
    <td class="tg-0pky">🟢 5.514976</td>
  </tr>
  <tr>
    <td class="tg-c3ow">11</td>
    <td class="tg-lboi">🔴 105393.932819</td>
    <td class="tg-lboi">🟡 14704.352002</td>
    <td class="tg-lboi">🟡 17762.602329</td>
    <td class="tg-lboi">🟡 2789.116357</td>
    <td class="tg-0pky">🟡 515.338182</td>
    <td class="tg-0pky">🟡 32.422103</td>
    <td class="tg-0pky">🟡 744.323492</td>
    <td class="tg-0pky">🟢 16.942394</td>
  </tr>
</tbody></table>

---

### `all_until_nth_prime` Benchmark (Python vs C++, sequential vs concurrent)

<table class="tg" style="undefined;table-layout: fixed; width: 1031px"><colgroup>
<col style="width: 200px">
<col style="width: 200px">
<col style="width: 200px">
<col style="width: 200px">
<col style="width: 200px">
<col style="width: 200px">
<col style="width: 200px">
<col style="width: 200px">
<col style="width: 200px">
</colgroup>
<tbody>
  <tr>
    <td class="tg-0pky"></td>
    <td class="tg-c3ow" colspan="4">Non-cumulative</td>
    <td class="tg-c3ow" colspan="4">Cumulative </td>
  </tr>
  <tr>
    <td class="tg-0pky"></td>
    <td class="tg-c3ow" colspan="2">Sequential</td>
    <td class="tg-c3ow" colspan="2">Concurrent</td>
    <td class="tg-c3ow" colspan="2">Sequential</td>
    <td class="tg-c3ow" colspan="2">Concurrent</td>
  </tr>
  <tr>
    <td class="tg-c3ow">n (upto)</td>
    <td class="tg-c3ow">Python</td>
    <td class="tg-c3ow">C++</td>
    <td class="tg-c3ow">Python</td>
    <td class="tg-c3ow">C++</td>
    <td class="tg-c3ow">Python</td>
    <td class="tg-c3ow">C++</td>
    <td class="tg-c3ow">Python</td>
    <td class="tg-c3ow">C++</td>
  </tr>
  <tr>
    <td class="tg-c3ow">1</td>
    <td class="tg-lboi">🟡 0.312090</td>
    <td class="tg-lboi">🟢 0.119474</td>
    <td class="tg-lboi">🔴 26.793957</td>
    <td class="tg-lboi">🟡 0.680676</td>
    <td class="tg-0pky">🟡 0.305414</td>
    <td class="tg-0pky">🟡 0.149080</td>
    <td class="tg-0pky">🟡 25.183200</td>
    <td class="tg-0pky">🟡 0.634730</td>
  </tr>
  <tr>
    <td class="tg-c3ow">2</td>
    <td class="tg-lboi">🟡 0.563860</td>
    <td class="tg-lboi">🟢 0.140293</td>
    <td class="tg-lboi">🔴 28.688669</td>
    <td class="tg-lboi">🟡 0.989705</td>
    <td class="tg-0pky">🟡 0.352144</td>
    <td class="tg-0pky">🟡 0.114765</td>
    <td class="tg-0pky">🟡27.993440</td>
    <td class="tg-0pky">🟡 0.889839</td>
  </tr>
  <tr>
    <td class="tg-c3ow">3</td>
    <td class="tg-lboi">🟡 1.976252</td>
    <td class="tg-lboi">🟢 0.263123</td>
    <td class="tg-lboi">🔴 47.089338</td>
    <td class="tg-lboi">🟡 1.410182</td>
    <td class="tg-0pky">🟡 0.985622</td>
    <td class="tg-0pky">🟡 0.138871</td>
    <td class="tg-0pky">🟡 45.850992</td>
    <td class="tg-0pky">🟡 1.397160</td>
  </tr>
  <tr>
    <td class="tg-c3ow">4</td>
    <td class="tg-lboi">🟡 6.392956</td>
    <td class="tg-lboi">🟢 0.882263</td>
    <td class="tg-lboi">🔴 62.517881</td>
    <td class="tg-lboi">🟡 1.910691</td>
    <td class="tg-0pky">🟡 1.830339</td>
    <td class="tg-0pky">🟡 0.297017</td>
    <td class="tg-0pky">🟡 60.323238</td>
    <td class="tg-0pky">🟡 1.622252</td>
  </tr>
  <tr>
    <td class="tg-c3ow">5</td>
    <td class="tg-lboi">🟡 21.238327</td>
    <td class="tg-lboi">🟡 3.111601</td>
    <td class="tg-lboi">🔴 89.342833</td>
    <td class="tg-lboi">🟡 2.551071</td>
    <td class="tg-0pky">🟡 4.434585</td>
    <td class="tg-0pky">🟡 0.621856</td>
    <td class="tg-0pky">🟡 84.814310</td>
    <td class="tg-0pky">🟢 1.809723</td>
  </tr>
  <tr>
    <td class="tg-c3ow">6</td>
    <td class="tg-lboi">🟡 75.675488</td>
    <td class="tg-lboi">🟡 9.734052</td>
    <td class="tg-lboi">🔴 119.777679</td>
    <td class="tg-lboi">🟡 3.883969</td>
    <td class="tg-0pky">🟡 6.624460</td>
    <td class="tg-0pky">🟡 1.636859</td>
    <td class="tg-0pky">🟡 107.084989</td>
    <td class="tg-0pky">🟢 2.134353</td>
  </tr>
  <tr>
    <td class="tg-c3ow">7</td>
    <td class="tg-lboi">🔴 286.724567</td>
    <td class="tg-lboi">🟡 29.019844</td>
    <td class="tg-lboi">🟡 167.028666</td>
    <td class="tg-lboi">🟡 6.662405</td>
    <td class="tg-0pky">🟡 14.232635</td>
    <td class="tg-0pky">🟡 2.737794</td>
    <td class="tg-0pky">🟡 131.708621</td>
    <td class="tg-0pky">🟢 2.639809</td>
  </tr>
  <tr>
    <td class="tg-c3ow">8</td>
    <td class="tg-lboi">🔴 1148.036480</td>
    <td class="tg-lboi">🟡 122.630714</td>
    <td class="tg-lboi">🟡 339.717865</td>
    <td class="tg-lboi">🟡 19.404044</td>
    <td class="tg-0pky">🟡 32.555818</td>
    <td class="tg-0pky">🟡 6.132860</td>
    <td class="tg-0pky">🟡 178.711175</td>
    <td class="tg-0pky">🟢 3.624857</td>
  </tr>
  <tr>
    <td class="tg-c3ow">9</td>
    <td class="tg-lboi">🔴 4930.480242</td>
    <td class="tg-lboi">🟡 581.074922</td>
    <td class="tg-lboi">🟡 1031.448364</td>
    <td class="tg-lboi">🟡 88.178959</td>
    <td class="tg-0pky">🟡 72.506189</td>
    <td class="tg-0pky">🟡 9.065279</td>
    <td class="tg-0pky">🟡 267.734766</td>
    <td class="tg-0pky">🟢 7.723773</td>
  </tr>
  <tr>
    <td class="tg-c3ow">10</td>
    <td class="tg-lboi">🔴 23646.401882</td>
    <td class="tg-lboi">🟡 3077.772619</td>
    <td class="tg-lboi">🟡 4098.479033</td>
    <td class="tg-lboi">🟡 521.155381</td>
    <td class="tg-0pky">🟡 200.129032</td>
    <td class="tg-0pky">🟡 19.657179</td>
    <td class="tg-0pky">🟡 497.070550</td>
    <td class="tg-0pky">🟢 10.590669</td>
  </tr>
  <tr>
    <td class="tg-c3ow">11</td>
    <td class="tg-lboi">🔴 126190.231323</td>
    <td class="tg-lboi">🟡 18484.995409</td>
    <td class="tg-lboi">🟡 22108.228683</td>
    <td class="tg-lboi">🟡 3421.928999</td>
    <td class="tg-0pky">🟡 545.511722</td>
    <td class="tg-0pky">🟡 39.963135</td>
    <td class="tg-0pky">🟡 1205.855131</td>
    <td class="tg-0pky">🟢 21.491489</td>
  </tr>
</tbody></table>

<br>

### Concurrent vs Sequential Speedup (Factor Range)

| Implementation              | Speedup Range (min–max) |
| --------------------------- | ----------------------- |
| C++ without accumulation    | 0.49× – 2789×           |
| Python without accumulation | 14.46× – 17762×         |
| C++ with accumulation       | 0.63× – 744×            |
| Python with accumulation    | 25.18× – 1205×          |

> Notes:
> - A factor <1× means the concurrent version was slower than sequential.
> - Python sees huge gains from concurrency in non-cumulative mode.
> - C++ gains in concurrent non-cumulative mode are smaller, sometimes <1× for very small n.

<br>

### Cumulative vs Non-cumulative Speedup (Factor Range)

| Implementation | Sequential      | Concurrent     |
| -------------- | --------------- | -------------- |
| C++            | 0.089× – 0.838× | 0.718× – 1.46× |
| Python         | 0.305× – 515×   | 0.634× – 1205× |

> Notes:
> Factor <1× means cumulative is slower than non-cumulative.
> Factor >1× means cumulative is faster.
> In C++ sequential, cumulative is almost always slower.
> In Python concurrent, cumulative can give large speedups for large n.

<br>

### Why Python Concurrent + Cumulative Is Slower

While the C++ concurrent + cumulative implementation shows significant acceleration,
the Python version exhibits slower performance in the same configuration.  
This behavior stems from Python’s **Global Interpreter Lock (GIL)**, which prevents
multiple threads from executing bytecode simultaneously. As a result, CPU-bound
tasks like modular arithmetic and arbitrary-precision operations are serialized.

Additionally, cumulative caching introduces shared-memory overhead and locking
when accessed by multiple threads, further reducing performance.  
To achieve true concurrency in Python, multiprocessing would be required, but that
adds inter-process communication overhead and complicates shared caching.

**In short:** Python threads cannot truly parallelize CPU-bound workloads,
whereas C++ threads execute independently, fully utilizing available CPU cores.

<h2>Changelog</h2>

- Cumulative functions & caching for the modular factorial and inner sum computations, for both C++ and Python.



<h2>Soon</h2>

- C++ implementation of the formula for CUDA devices.
- Fortran implementation of the formula.
- Optimizations via quantum computing using `qiskit` in Python.

<h2>License</h2>

Copyright (c) 2025 epsill0n epsill0n.  
**All rights reserved.**

This project may be used for **personal, academic, or research purposes only**.  
Commercial use, redistribution, or modification is strictly prohibited without explicit written permission from the author.  

You are welcome to **study the code, suggest improvements, or propose collaborations**.  
See the [LICENSE.txt](LICENSE.txt) file for full terms.
