// Willans'_formula - all_until_nth_prime.cpp
// Copyright (c) 2025 epsill0n epsill0n
// Non-commercial use only. Redistribution or modification prohibited without permission.
// See LICENSE.txt for full terms.

#include <iostream>
#   define FAST_IO
#include <iomanip>

#include <cstdlib>

#   define GCC_O3_OPTIM
#include "WF_impl.hpp"

#include <chrono>


__attribute__((constructor)) void init() {
#ifdef FAST_IO
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(0);
#endif
    WF::init_mpfr(MPFR_PRECISION);
}


int main(int argc, char** argv) {
    if (argc!=3) {
        std::cout << "Usage: [" << argv[0] << "] [n] [1/0 for concurrency]\n" << std::flush;
        return -1;
    }

    std::cout << "Using MPFR precision of " << MPFR_PRECISION << " bits.\n";

    int n = std::atoi(argv[1]);
    mpz_class prime[n];
    std::chrono::system_clock::time_point begin, end;
    std::chrono::duration<double, std::milli> elapsed;
    if (std::atoi(argv[2]) == 0) {
        std::cout << "Not using concurrency.\n" << std::flush;

        begin = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < n; ++i)
            prime[i] = WF::seq::nthPrime(i+1);

        end = std::chrono::high_resolution_clock::now();

    } else {
        std::cout << "Using concurrency.\n" << std::flush;
        begin = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < n; ++i)
            prime[i] = WF::conc::nthPrime(i+1);
        end = std::chrono::high_resolution_clock::now();
    }
    
    elapsed = end - begin;
    for (int i = 0; i < n; ++i)
        std::cout << prime[i] << " ";
    std::cout << "\nTime: " << std::fixed << std::setprecision(10) <<  (elapsed.count()) << "ms\n";
    
    std::cout << std::flush;
}