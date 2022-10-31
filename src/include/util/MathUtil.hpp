//
// Created by BianZheng on 2022/10/29.
//

#ifndef REVERSE_K_RANKS_MATHUTIL_HPP
#define REVERSE_K_RANKS_MATHUTIL_HPP

#include <limits>

namespace ReverseMIPS {
    double constexpr sqrtNewtonRaphson(double x, double curr, double prev) {
        return curr == prev
               ? curr
               : sqrtNewtonRaphson(x, 0.5 * (curr + x / curr), curr);
    }

    /*
    * Constexpr version of the square root
    * Return value:
    *	- For a finite and non-negative value of "x", returns an approximation for the square root of "x"
    *   - Otherwise, returns NaN
    */
    double constexpr sqrt(double x) {
        return x >= 0 && x < std::numeric_limits<double>::infinity()
               ? sqrtNewtonRaphson(x, x, 0)
               : std::numeric_limits<double>::quiet_NaN();
    }
}
#endif //REVERSE_K_RANKS_MATHUTIL_HPP
