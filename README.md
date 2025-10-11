<h1>Implementation of Willans' formula</h1>

In 1964, Willans gave the formula:

<div align="center">

$$
\large \displaystyle
p_n = 1 + \sum_{i=1}^{2^n} \left\lfloor \left(\frac{n}{\displaystyle \sum_{j=1}^{i} \left\lfloor \left(\cos \frac{(j-1)! + 1}{j} \pi \right)^2 \right\rfloor}\right)^{1/n} \right\rfloor
$$

</div>


for generating the $n$-th prime number.

By Wilson's theorem, $n+1$ is prime if and only if $n! \equiv n \pmod{n+1}$. Thus, when $n+1$ is prime, the first factor in the product becomes one, and the formula produces the prime number $n+1$. But when $n+1$ is not prime, the first factor becomes zero and the formula produces the prime number $2$. This formula is not an efficient way to generate prime numbers because evaluating $n! \pmod{n+1}$ requires about $n-1$ multiplications and reductions modulo $n+1$.

> **Precision note:** Throughout this project, "256-bit precision" and "77 decimal digits" refer to the same numerical precision level (since $\log_2(10^{77}) \approx 256$ bits). This equivalence applies to both the **MPFR** (C++) and **mpmath** (Python) implementations, ensuring consistent results across languages.

<h2>Content & Functionality</h2>

```
Willans'_formula/
├── py/
│   ├── WF.py
│   ├── nth_prime.py
│   └── all_until_nth_prime.py
│
├── cpp/
│   ├── WF_base.hpp
│   ├── WF_impl.hpp
│   ├── nth_prime.cpp
│   ├── all_until_nth_prime.cpp
│   ├── nth_prime.exe                                           (need to be built)
│   └── all_until_nth_prime.exe                                 (need to be built)
│
├── CMakeLists.txt
├── README.md
└── License.txt
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



<h3>cpp/</h3>

- `WF_base.hpp`: the base included by `WF_impl.hpp`; contains the `modFact` implementation;
- `WF_impl.hpp`: contains the sequential and concurrent implementations of the formula;
- `nth_prime.cpp`: source code for outputting the $n$-th prime number;
- `all_until_nth_prime.cpp`: source code for outputting all the prime numbers until the $n$-th prime number.

Willans’ formula is implemented inside the `WF` namespace, defined in `WF_impl.hpp`.

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

<h3>Modular Factorial</h3>

Willans' formula requires computing factorials modulo a number. Instead of calculating the full factorial `j!`, which grows extremely fast, we compute it modulo `i`:

$$
\text{modFact}(j, i) = (1 \cdot 2 \cdot 3 \cdot \cdots \cdot j) \bmod i
$$

This keeps intermediate numbers manageable while preserving correctness, since only $(j-1)! + 1 \bmod j$ is required in the formula.

- **C++ Implementation** uses `mpz_class` for arbitrary-precision integers and optimizes for small values with `unsigned long` arithmetic.
- **Python Implementation** uses standard Python integers, which support arbitrary precision by default.

<h4>Explanation</h4>

For the key term in Willans' formula, we compute:

$$
\text{modFact}(j-1, j) = (1 \cdot 2 \cdot \dots \cdot (j-1)) \bmod j
$$

Conceptually:

$$
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

<br>
<br>

**At the start of execution, the precision in decimal places is printed to console. The default precision I have set is 77 decimals (around 256 bits) via**
```py
mpmath.mp.dps = 77
```
**in** `WF.py` **(line 9). You can change it to your liking.**
**In the end, the time it took to do the computations for calculating the prime(s) is printed to console.**

<br>
<br>
<br>
<br>

Example:
```bash
py ./py/nth_prime.py 11 1
```
computes the 11th (*first argument*) prime number **with** concurrency (*second argument*).

Output:
```bash
Using mpmath precision of 77 decimals.
Using concurrency.
31
Time: 16716.9559001923ms
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
Using mpmath precision of 77 decimals.
Not using concurrency.
2 3 5 7 11 13 17 19 23 
Time: 4531.45956993103ms
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

<br>
<br>

**At the start of execution, the *MPFR* floating-point precision (in bits) is printed to console. The default precision I have set is 256 bits (around 77 decimals) via**

```cpp
#   define MPFR_PRECISION 256
```

**in** `WF_base.hpp` **(line 15). You can change it to your liking.**

**In the end, the time it took to do the computations for calculating the prime(s) is printed.**

<br>
<br>
<br>
<br>

Example:
```bash
./cpp/nth_prime.exe 11 1
```
computes the 11th (*first argument*) prime number **with** concurrency (*second argument*).

Output:
```bash
Using MPFR precision of 256 bits.
Using concurrency.
31
Time: 2914.0817400000ms
```

<br>
<br>

Example:
```bash
./cpp/all_until_nth_prime.exe 9 0
```
computes the first 9 (*first argument*) prime numbers **without** concurrency (*second argument*).

Output:
```bash
Using MPFR precision of 256 bits.
Not using concurrency.
2 3 5 7 11 13 17 19 23 
Time: 550.0970100000ms
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

| Component        | Specification                       |
|-----------------|------------------------------------|
| CPU              | AMD Ryzen 7 5825U                  |
| RAM              | 16 GiB DDR4 SDRAM                  |
| OS               | openSUSE Tumbleweed                |
| Python Version   | 3.13.3                             |
| C++ Compiler     | g++ (GCC) 14.2.1    |
| Precision        | Python: 77 decimals (~256 bits), C++: 256 bits (~77 decimals) |
| Fast I/O (C++)   | Enabled via `FAST_IO` macro |

---

### `nth_prime` Benchmark (Python vs C++, sequential vs concurrent)

<table>
  <thead>
    <tr>
      <th>n-th prime</th>
      <th>Python seq (ms)</th>
      <th>Python conc (ms)</th>
      <th>C++ seq (ms)</th>
      <th>C++ conc (ms)</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>1</td>
      <td style="color:yellow;">0.322342</td>
      <td style="color:red;">27.131081</td>
      <td style="color:green;">0.120215</td>
      <td style="color:yellow;">0.839459</td>
    </tr>
    <tr>
      <td>2</td>
      <td style="color:yellow;">0.433207</td>
      <td style="color:red;">14.464855</td>
      <td style="color:green;">0.165149</td>
      <td style="color:yellow;">0.627323</td>
    </tr>
    <tr>
      <td>3</td>
      <td style="color:yellow;">1.562834</td>
      <td style="color:red;">16.047955</td>
      <td style="color:green;">0.185036</td>
      <td style="color:yellow;">0.586396</td>
    </tr>
    <tr>
      <td>4</td>
      <td style="color:yellow;">4.510164</td>
      <td style="color:red;">15.815020</td>
      <td style="color:yellow;">0.578381</td>
      <td style="color:green;">0.487442</td>
    </tr>
    <tr>
      <td>5</td>
      <td style="color:yellow;">15.270472</td>
      <td style="color:red;">20.074606</td>
      <td style="color:yellow;">2.020497</td>
      <td style="color:green;">0.633243</td>
    </tr>
    <tr>
      <td>6</td>
      <td style="color:red;">55.511951</td>
      <td style="color:yellow;">26.605129</td>
      <td style="color:yellow;">7.697089</td>
      <td style="color:green;">1.169236</td>
    </tr>
    <tr>
      <td>7</td>
      <td style="color:red;">207.010746</td>
      <td style="color:yellow;">50.860405</td>
      <td style="color:yellow;">21.057605</td>
      <td style="color:green;">3.358850</td>
    </tr>
    <tr>
      <td>8</td>
      <td style="color:red;">834.560394</td>
      <td style="color:yellow;">152.147293</td>
      <td style="color:yellow;">90.869233</td>
      <td style="color:green;">12.753101</td>
    </tr>
    <tr>
      <td>9</td>
      <td style="color:red;">3685.491562</td>
      <td style="color:yellow;">604.141951</td>
      <td style="color:yellow;">428.346209</td>
      <td style="color:green;">63.768299</td>
    </tr>
    <tr>
      <td>10</td>
      <td style="color:red;">18574.532509</td>
      <td style="color:yellow;">2924.258471</td>
      <td style="color:yellow;">2351.954378</td>
      <td style="color:green;">415.306161</td>
    </tr>
    <tr>
      <td>11</td>
      <td style="color:red;">105393.932819</td>
      <td style="color:yellow;">17762.602329</td>
      <td style="color:yellow;">14704.352002</td>
      <td style="color:green;">2789.116357</td>
    </tr>
  </tbody>
</table>

> Notes for filling:  
> - Python seq = `py/nth_prime.py n 0`  
> - Python conc = `py/nth_prime.py n 1`  
> - C++ seq = `cpp/nth_prime.exe n 0`  
> - C++ conc = `cpp/nth_prime.exe n 1`

---

### `all_until_nth_prime` Benchmark (Python vs C++, sequential vs concurrent)

<table>
  <thead>
    <tr>
      <th>n (upto)</th>
      <th>Python seq (ms)</th>
      <th>Python conc (ms)</th>
      <th>C++ seq (ms)</th>
      <th>C++ conc (ms)</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>1</td>
      <td style="color:yellow;">0.312090</td>
      <td style="color:red;">26.793957</td>
      <td style="color:green;">0.119474</td>
      <td style="color:yellow;">0.680676</td>
    </tr>
    <tr>
      <td>2</td>
      <td style="color:yellow;">0.563860</td>
      <td style="color:red;">28.688669</td>
      <td style="color:green;">0.140293</td>
      <td style="color:yellow;">0.989705</td>
    </tr>
    <tr>
      <td>3</td>
      <td style="color:yellow;">1.976252</td>
      <td style="color:red;">47.089338</td>
      <td style="color:green;">0.263123</td>
      <td style="color:yellow;">1.410182</td>
    </tr>
    <tr>
      <td>4</td>
      <td style="color:yellow;">6.392956</td>
      <td style="color:red;">62.517881</td>
      <td style="color:green;">0.882263</td>
      <td style="color:yellow;">1.910691</td>
    </tr>
    <tr>
      <td>5</td>
      <td style="color:yellow;">21.238327</td>
      <td style="color:red;">89.342833</td>
      <td style="color:yellow;">3.111601</td>
      <td style="color:green;">2.551071</td>
    </tr>
    <tr>
      <td>6</td>
      <td style="color:yellow;">75.675488</td>
      <td style="color:red;">119.777679</td>
      <td style="color:yellow;">9.734052</td>
      <td style="color:green;">3.883969</td>
    </tr>
    <tr>
      <td>7</td>
      <td style="color:red;">286.724567</td>
      <td style="color:yellow;">167.028666</td>
      <td style="color:yellow;">29.019844</td>
      <td style="color:green;">6.662405</td>
    </tr>
    <tr>
      <td>8</td>
      <td style="color:red;">1148.036480</td>
      <td style="color:yellow;">339.717865</td>
      <td style="color:yellow;">122.630714</td>
      <td style="color:green;">19.404044</td>
    </tr>
    <tr>
      <td>9</td>
      <td style="color:red;">4930.480242</td>
      <td style="color:yellow;">1031.448364</td>
      <td style="color:yellow;">581.074922</td>
      <td style="color:green;">88.178959</td>
    </tr>
    <tr>
      <td>10</td>
      <td style="color:red;">23646.401882</td>
      <td style="color:yellow;">4098.479033</td>
      <td style="color:yellow;">3077.772619</td>
      <td style="color:green;">521.155381</td>
    </tr>
    <tr>
      <td>11</td>
      <td style="color:red;">126190.231323</td>
      <td style="color:yellow;">22108.228683</td>
      <td style="color:yellow;">18484.995409</td>
      <td style="color:green;">3421.928999</td>
    </tr>
  </tbody>
</table>

> Notes for filling:  
> - Python seq = `py/all_until_nth_prime.py n 0`  
> - Python conc = `py/all_until_nth_prime.py n 1`  
> - C++ seq = `cpp/all_until_nth_prime.exe n 0`  
> - C++ conc = `cpp/all_until_nth_prime.exe n 1`

<br>

> ***Concurrent implementations achieve orders-of-magnitude speedup for larger $n$***


<h2>Changelog</h2>

- C++ sequential implementation;
- C++ concurrent implementation;
- Python uses `mpmath` for precise floating-point operations.



<h2>Soon</h2>

- Memoization.
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
