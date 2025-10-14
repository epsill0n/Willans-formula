// Willans'_formula - WF.hpp
// Copyright (c) 2025 epsill0n epsill0n
// Non-commercial use only. Redistribution or modification prohibited without permission.
// See LICENSE.txt for full terms.


#pragma once

#    define GCC_O3_OPTIM

#include <cmath>
#include <gmpxx.h>
#include <mpreal.h>
#include <thread>
#include <vector>

using ul = unsigned long;

#   define MPFR_PRECISION 256 // bits for FP (~77 decimal digits)

#   ifdef GCC_O3_OPTIM
// #       pragma message("Using GCC O3 optimization.")
#       pragma GCC push_options
#       pragma GCC optimize("O3")
#   endif

namespace WF {

    namespace seq{};
    namespace conc{ inline mpz_class innerSumIsCumulative_concurrent(const mpz_class& i); };

    inline bool isConcurrent;
    inline bool isCumulative;

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

    inline mpz_class (*modFact) (const mpz_class&, const mpz_class&);
    inline mpz_class (*innerSum) (const mpz_class& i);

    inline mpz_class modFactIsCumulative(const mpz_class& j, const mpz_class& mod) {
        thread_local mpz_t r;
        thread_local mpz_class last_j;
        thread_local mpz_class last_mod;
        thread_local bool initialized = false;

        if (!initialized) {
            mpz_init_set_ui(r, 1);
            last_j = 0;
            last_mod = mod;
            initialized = true;
        }

        if (mod != last_mod) {
            mpz_set_ui(r, 1);
            last_j = 0;
            last_mod = mod;
        }

        if (mod.fits_ulong_p() && j.fits_ulong_p()) {
            unsigned long rl = mpz_get_ui(r);
            unsigned long ml = mod.get_ui();

            for (unsigned long w = last_j.get_ui() + 1; w <= j.get_ui(); ++w) {
                rl = (rl * w) % ml;
            }

            mpz_set_ui(r, rl);
        } else {
            for (mpz_class w = last_j + 1; w <= j; ++w) {
                mpz_mul(r, r, mpz_class(w).get_mpz_t());
                mpz_mod(r, r, mod.get_mpz_t());
            }
        }

        last_j = j;
        return mpz_class(r);
    }

    inline mpz_class modFactNotCumulative(const mpz_class& j, const mpz_class& i) {
        mpz_class r = 1;

        if (j.fits_ulong_p() && i.fits_ulong_p()) {
            const unsigned long jl = j.get_ui();
            const unsigned long il = i.get_ui();
            for (unsigned long w = 2; w <= jl; r = (r * w++) % il);
        } else for (mpz_class w = 2; w <= j; r = (r * w++) % i);

        return r;
    }

    inline mpz_class innerSumIsCumulative_last_i = 0;
    inline mpz_class innerSumIsCumulative_inner_sum_prev = 0;

    inline mpz_class innerSumIsCumulative(const mpz_class& i) {
        mpz_class& last_i = innerSumIsCumulative_last_i;
        mpz_class& inner_sum_prev = innerSumIsCumulative_inner_sum_prev;

        for (mpz_class j = last_i + 1; j <= i; ++j) {
            mpz_class mf = modFact(j - 1, j);

            mpfr::mpreal mf_r = to_mpreal(mf);
            mpfr::mpreal j_r  = to_mpreal(j);
            mpfr::mpreal arg  = ((mf_r + 1) / j_r) * mpfr::const_pi();
            mpfr::mpreal cosv = mpfr::cos(arg);
            mpfr::mpreal val  = mpfr::floor(cosv * cosv);

            inner_sum_prev += to_mpz(val);
        }

        last_i = i;
        return inner_sum_prev;
    }

    inline mpz_class innerSumNotCumulative(const mpz_class& i) {
        mpz_class mf, inner_sum;
        mpfr::mpreal arg, cosv, ratio, root_val;

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

        return inner_sum;
    }

    inline void init_cumulative(bool _isConcurrent = true, bool _isCumulative = true) {
        isConcurrent = _isConcurrent;
        isCumulative = _isCumulative;

        if (_isCumulative) {
            WF::modFact = WF::modFactIsCumulative;
            if (_isConcurrent)
                WF::innerSum = WF::conc::innerSumIsCumulative_concurrent;
            else WF::innerSum = WF::innerSumIsCumulative;
        } else {
            WF::modFact = WF::modFactNotCumulative;
            WF::innerSum = WF::innerSumNotCumulative;
        }
    }

    inline void reset_innerSumCumulative() {
        innerSumIsCumulative_inner_sum_prev = 0;
        innerSumIsCumulative_last_i = 0;
    }

    namespace seq {

        inline mpz_class nthPrime(int n) { 
            if (isCumulative)
                WF::reset_innerSumCumulative();       

            mpz_class outer_sum = 1;
            mpz_class limit = (mpz_class(1) << n) + 1;
            mpz_class mf, inner_sum;

            mpfr::mpreal n_r = mpfr::mpreal(n);
            mpfr::mpreal arg, cosv, ratio, root_val;
            
            for (mpz_class i = 1; i < limit; ++i) {
                inner_sum = innerSum(i);

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

        thread_local mpz_class innerSum_last_i = 0;
        thread_local mpz_class innerSum_prev_sum = 0;

        inline void reset_thread_innerSum() {
            innerSum_last_i = 0;
            innerSum_prev_sum = 0;
        }

        inline mpz_class innerSumIsCumulative_concurrent(const mpz_class& i) {
            for (mpz_class j = innerSum_last_i + 1; j <= i; ++j) {
                mpz_class mf = WF::modFact(j - 1, j);

                mpfr::mpreal arg = ((WF::to_mpreal(mf) + 1) / WF::to_mpreal(j)) * mpfr::const_pi();
                mpfr::mpreal cosv = mpfr::cos(arg);
                mpfr::mpreal val  = mpfr::mpreal(mpfr::floor(cosv * cosv));

                innerSum_prev_sum += WF::to_mpz(val);
            }

            innerSum_last_i = i;
            return innerSum_prev_sum;
        }

        inline mpz_class compute_row(const mpz_class& i, const int& n) {
            mpz_class inner_sum = WF::innerSum(i);  // could be cumulative or not

            if (inner_sum == 0) return mpz_class(0);

            mpfr::mpreal n_r(n);
            mpfr::mpreal inner_r = WF::to_mpreal(inner_sum);
            mpfr::mpreal ratio = n_r / inner_r;
            mpfr::mpreal root = mpfr::pow(ratio, mpfr::mpreal(1) / n_r);
            mpfr::mpreal flo = mpfr::floor(root);

            return WF::to_mpz(flo);
        }
        
        inline mpz_class nthPrime(int n) {
            mpz_class total_rows = mpz_class(1) << n;

            unsigned int num_threads = std::thread::hardware_concurrency();
            if (num_threads == 0) num_threads = 4;

            std::vector<mpz_class> partial_sums(num_threads, 0);

            auto worker = [&](unsigned int tid, mpz_class start, mpz_class end) {
                reset_thread_innerSum();
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