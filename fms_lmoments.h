// fms_lmoments.h : L-moments
// $\lamabda_r = \frac{1}{r} \sum_{k=0}^{r-1} (-1)^k \binom{r-1}{k} E[X_{r-k:r}]$
#pragma once
#include <algorithm>
#include <array>
#include <numeric>
#include <span>
#include <vector>

namespace fms::statistics {

	// r-th sample L-moment of x
	template<class X>
	constexpr X lmoment(size_t r, std::span<const X> x)
	{
		if (!std::is_sorted(x.begin(), x.end(), std::less_equal<X>{})) {
			return NaN<X>;
		}	

		if (r == 0) {
			return X(0);
		}

		X lr = X(0);
		size_t n = x.size();

		//size_t i[9];
		//std::vector<size_t> i(r);

		if (r == 1) {
			for (size_t i = 0; i < n - r; ++i) {
				lr += x[i];
			}
		}
		else if (r == 2) {
			for (size_t i = 0; i < n - r; ++i) {
				for (size_t j = 0; j < i; ++j) {
					lr += x[i] - x[j];
				}
			}
		}
		else if (r == 3) {
			for (size_t i = 2; i < n; ++i) {
				for (size_t j = 1; j < i; ++j) {
					for (size_t k = 0; k < j; ++k) {
						lr += x[i] - x[j] + x[k];
					}
				}
			}
		}
		else if (r == 4) {
			for (size_t i = 3; i < n; ++i) {
				for (size_t j = 2; j < i; ++j) {
					for (size_t k = 1; k < j; ++k) {
						for (size_t l = 0; l < k; ++l) {
							lr += x[i] - x[j] + x[k] - x[l];
						}
					}
				}
			}
		}
		return lr / ( r * binom(n, r));
	}
	constexpr std::array<double,3> x = {1, 2, 3};
	constexpr std::span xs(x.data(), x.size());
	constexpr double l1 = lmoment<double>(1, x); // 1
	//static_assert(lmoment(1, x) == 2);
	constexpr int lmoment_value = 2;
}