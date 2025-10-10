// Willans'_formula - WF_impl.hpp
// Copyright (c) 2025 epsill0n epsill0n
// Non-commercial use only. Redistribution or modification prohibited without permission.
// See LICENSE.txt for full terms.


#pragma once

#include "WF_base.hpp"

#include <cmath>
#include <gmpxx.h>
#include <mpreal.h>
#include <thread>
#include <vector>

#   ifdef GCC_O3_OPTIM
#       pragma GCC push_options
#       pragma GCC optimize("O3")
#   endif

namespace WF {

    inline void init_mpfr(long prec = MPFR_PRECISION) {
        mpfr::mpreal::set_default_prec(mpfr_prec_t(prec));
    }

    // Convert mpfr::mpreal to mpz_class exactly
    inline mpz_class to_mpz(const mpfr::mpreal& x) {
        mpz_t z;
        mpz_init(z);
        mpfr_get_z(z, x.mpfr_srcptr(), MPFR_RNDZ);
        mpz_class result(z);
        mpz_clear(z);
        return result;
    }

    // Convert mpz_class to mpfr::mpreal exactly
    inline mpfr::mpreal to_mpreal(const mpz_class& z) {
        mpfr::mpreal result;
        mpfr_set_z(result.mpfr_ptr(), z.get_mpz_t(), MPFR_RNDN);
        return result;
    }

    namespace seq {

        inline mpz_class nthPrime(int n) {        

            mpz_class outer_sum = 1;
            mpz_class limit = (mpz_class(1) << n) + 1;
            mpz_class mf, inner_sum;

            mpfr::mpreal n_r = mpfr::mpreal(n);
            mpfr::mpreal arg, cosv, ratio, root_val;
            
            for (mpz_class i = 1; i < limit; ++i) {
                inner_sum = 0;
                
                for (mpz_class j = 1; j <= i; ++j) {
                    mf = modFact(j - 1, j);

                    mpfr::mpreal mf_r = to_mpreal(mf);
                    mpfr::mpreal j_r = to_mpreal(j);

                    arg = ((mf_r + mpfr::mpreal(1)) / j_r) * mpfr::const_pi();
                    cosv = mpfr::cos(arg);

                    mpfr::mpreal cos_sq = cosv * cosv;
                    mpfr::mpreal floor_val = mpfr::floor(cos_sq);

                    inner_sum += to_mpz(floor_val);
                }

                if (inner_sum != 0) {
                    mpfr::mpreal inner_r = to_mpreal(inner_sum);

                    ratio = n_r / inner_r;
                    root_val = mpfr::pow(ratio, mpfr::mpreal(1) / n_r);

                    mpfr::mpreal flo = mpfr::floor(root_val);
                    outer_sum += to_mpz(flo);
                }
            }

            return outer_sum;
        }

    }

    namespace conc {

        inline mpz_class compute_row(const mpz_class& i, const int& n) {
            mpz_class inner_sum = 0;

            for (mpz_class j = 1; j <= i; ++j) {
                mpz_class mf = modFact(j - 1, j);

                mpfr::mpreal mf_r = to_mpreal(mf);
                mpfr::mpreal j_r = to_mpreal(j);

                mpfr::mpreal arg = ((mf_r + mpfr::mpreal(1)) / j_r) * mpfr::const_pi();
                mpfr::mpreal cosv = mpfr::cos(arg);
                mpfr::mpreal cos_sq = cosv * cosv;
                mpfr::mpreal floor_val = mpfr::floor(cos_sq);

                inner_sum += to_mpz(floor_val);
            }

            if (inner_sum == 0) return mpz_class(0);

            mpfr::mpreal n_r(n);
            mpfr::mpreal inner_r = to_mpreal(inner_sum);
            mpfr::mpreal ratio = n_r / inner_r;
            mpfr::mpreal root = mpfr::pow(ratio, mpfr::mpreal(1) / n_r);
            mpfr::mpreal flo = mpfr::floor(root);

            return to_mpz(flo);
        }

        inline mpz_class nthPrime(int n) {
            mpz_class total_rows = mpz_class(1) << n;

            unsigned int num_threads = std::thread::hardware_concurrency();
            if (num_threads == 0) num_threads = 4; // fallback

            std::vector<mpz_class> partial_sums(num_threads, mpz_class(0));

            auto worker = [&](unsigned int tid, mpz_class start, mpz_class end) {
                mpz_class local_sum = 0;
                for (mpz_class i = start; i < end; ++i) {
                    local_sum += compute_row(i + 1, n);
                }
                partial_sums[tid] = local_sum;
            };

            std::vector<std::thread> threads;
            mpz_class chunk_size = total_rows / num_threads;
            mpz_class remainder = total_rows % num_threads;

            mpz_class start = 0;
            for (unsigned int t = 0; t < num_threads; ++t) {
                mpz_class end = start + chunk_size + (t < remainder ? 1 : 0);
                threads.emplace_back(worker, t, start, end);
                start = end;
            }

            for (auto &th : threads) th.join();

            mpz_class outer_sum = 1;
            for (auto &s : partial_sums) outer_sum += s;

            return outer_sum;
        }

    }

}
#ifdef GCC_O3_OPTIM
#   pragma GCC pop_options
#endif
