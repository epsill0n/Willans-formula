// Willans'_formula - WF_base.hpp
// Copyright (c) 2025 epsill0n epsill0n
// Non-commercial use only. Redistribution or modification prohibited without permission.
// See LICENSE.txt for full terms.


#pragma once

#    define GCC_O3_OPTIM

#include <gmpxx.h>

using ul = unsigned long;

#   define MPFR_PRECISION 256 // bits for FP (~77 decimal digits)

#   ifdef GCC_O3_OPTIM
#       pragma message("Using GCC O3 optimization.")
#       pragma GCC push_options
#       pragma GCC optimize("O3")
#   endif

namespace WF {

    inline mpz_class modFact(const mpz_class& j, const mpz_class& i) {
        mpz_class r = 1;

        if (j.fits_ulong_p() && i.fits_ulong_p()) {
            const unsigned long jl = j.get_ui();
            const unsigned long il = i.get_ui();
            for (unsigned long w = 2; w <= jl; r = (r * w++) % il);
        } else for (mpz_class w = 2; w <= j; r = (r * w++) % i);

        return r;
    }
}

#ifdef GCC_O3_OPTIM
#   pragma GCC pop_options
#endif