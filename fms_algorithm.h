// fms_algorithm.h - various statistical algorithms
#pragma once
#include <array>
#include <span>

namespace fms::statistics {

	// Use for error instead of throw.
	template<class X>
	constexpr X NaN = std::numeric_limits<X>::quiet_NaN();

	// Source - https://stackoverflow.com/a/44719219
	constexpr inline size_t binom(size_t n, size_t k) noexcept
	{
		return k > n ? 0
			: k == 0 || k == n ? 1
			: k == 1 || k == n - 1 ? n
			: binom(n - 1, k - 1) * n / k;
	}
#ifdef _DEBUG
	static_assert(binom(2, 5) == 0);
	static_assert(binom(5, 0) == 1);
	static_assert(binom(5, 5) == 1);
	static_assert(binom(5, 2) == 10);
	static_assert(binom(5, 3) == 10);
#endif // _DEBUG

	// Generates the combinations of r indices from {0,...,n-1}, in increasing order.
	// Returns true and advances idx to the next combination, or false if exhausted.
	// Initialize idx to {0,1,...,r-1} for the first combination.
	constexpr bool next_combination(std::span<size_t>& idx, size_t n)
	{
		size_t r = idx.size();
		if (r == 0 || r > n) {
			return false;
		}

		// find rightmost index that can be incremented
		size_t k = r;
		while (k > 0) {
			--k;
			if (idx[k] < n - r + k) {
				break;
			}
			if (k == 0) {
				return false; // all indices are at their max: no more combinations
			}
		}

		++idx[k];
		for (size_t j = k + 1; j < r; ++j) {
			idx[j] = idx[j - 1] + 1;
		}

		return true;
	}
	/*
	//namespace {
		constexpr std::array<size_t, 3> idx = { 0, 1, 2 };
		constexpr std::span<size_t, 3> idx_(idx.data(), idx.size());
		constexpr bool result = next_combination(idx_, 5);
		constexpr std::array<size_t, 3> expected(idx);
		static_assert(result);
		static_assert(idx[0] == expected[0]);
		static_assert(idx[1] == expected[1]);
		static_assert(idx[2] == expected[2]);
	//}
	*/
}