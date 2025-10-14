// Willans'_formula - nth_prime.cpp
// Copyright (c) 2025 epsill0n epsill0n
// Non-commercial use only. Redistribution or modification prohibited without permission.
// See LICENSE.txt for full terms.

#include <iostream>
#   define FAST_IO
#include <iomanip>

#include <cstdlib>

#   define GCC_O3_OPTIM
#include "WF.hpp"

#include <chrono>


__attribute__((constructor)) void init() {
#ifdef FAST_IO
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(0);
#endif
    WF::init_mpfr();
}


int main(int argc, char** argv) {
    if (argc!=4) {
        std::cout << "Usage: [" << argv[0] << "] [n] [1/0 for concurrency] [1/0 for accumulation]\n" << std::flush;
        return -1;
    }

    std::cout << "Using MPFR precision of " << MPFR_PRECISION << " bits.\n";

    int n = std::atoi(argv[1]);
    bool isConcurrent = bool(std::atoi(argv[2]));
    bool isCumulative = bool(std::atoi(argv[3]));

    mpz_class prime;
    std::chrono::system_clock::time_point begin, end;
    std::chrono::duration<double, std::milli> elapsed;

    if (isConcurrent)
        std::cout << "Using concurrency.\n" << std::flush;
    else std::cout << "Not using concurrency.\n" << std::flush;

    if (isCumulative)
        std::cout << "Using accumulation.\n" << std::flush;
    else std::cout << "Not using accumulation.\n" << std::flush;

    WF::init_cumulative(isConcurrent, isCumulative);
    
    if (isConcurrent) {

        begin = std::chrono::high_resolution_clock::now();
        prime = WF::conc::nthPrime(n);
        end = std::chrono::high_resolution_clock::now();
        
    } else {

        begin = std::chrono::high_resolution_clock::now();
        prime = WF::seq::nthPrime(n);
        end = std::chrono::high_resolution_clock::now();

    }

    elapsed = end - begin;
    std::cout << prime << "\n";
    std::cout << "Time: " << std::fixed << std::setprecision(10) <<  (elapsed.count()) << "ms\n";
    

    std::cout << std::flush;
}