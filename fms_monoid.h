// fms_monoid.h - monoids
#pragma once
#ifdef _DEBUG
#include <cassert>
#endif // _DEBUG
#include <algorithm>
#include <concepts>
#include <functional>
#include <initializer_list>
#include <limits>
#include <numeric>

namespace fms::statistics {
	
	// associative binary operation with identity element
	template<class T>
	concept monoid = requires(T a, T b) {
		{ T::monoid_op() } -> std::same_as<T>;
		{ T::monoid_op(a, b) } -> std::same_as<T>;
	};

	template<class T, class Op, T Id>
	struct monoid_op {
		[[nodiscard]] constexpr T operator()() const
		{
			return Id;
		}
		[[nodiscard]] constexpr T operator()(const T& a) const
		{
			return a;
		}
		template<class... Ts>
		[[nodiscard]] constexpr T operator()(const T& a, const Ts&... as) const
		{
			return Op()(a, operator()(as...));
		}
		[[nodiscard]] constexpr T operator()(const std::initializer_list<T>& as) const
		{
			// left fold
			return std::accumulate(as.begin(), as.end(), Id, Op());
		}
	};
	template<class T>
	struct max_op {
		constexpr T operator()(const T& a, const T& b) const
		{
			return std::max(a, b);
		}
	};
	template<class T>
	struct min_op {
		constexpr T operator()(const T& a, const T& b) const
		{
			return std::min(a, b);
		}
	};
	template<class T> using monoid_plus = monoid_op<T, std::plus<T>, T(0)>;
	template<class T> using monoid_multiplies = monoid_op<T, std::multiplies<T>, T(1)>;
	template<class T> using monoid_max = monoid_op<T, max_op<T>, -std::numeric_limits<T>::max()>;
	template<class T> using monoid_min = monoid_op<T, min_op<T>, std::numeric_limits<T>::max()>;

#ifdef _DEBUG
#define MONOID_TEST(X) \
	X(plus, 0) \
	X(plus, 1, 1) \
	X(plus, 3, 1, 2) \
	X(plus, 6, 1, 2, 3) \
	X(multiplies, 1) \
	X(multiplies, 1, 1) \
	X(multiplies, 2, 1, 2 ) \
	X(multiplies, 6, 1, 2, 3) \
	X(max, -INT_MAX) \
	X(max, 1, 1) \
	X(max, 2, 1, 2 ) \
	X(max, 3, 1, 2, 3) \
	X(min, INT_MAX) \
	X(min, 1, 1) \
	X(min, 1, 1, 2) \
	X(min, 1, 1, 2, 3) \

#define TEST_MONOID(Op, v, ...) static_assert(v == monoid_##Op<int>()(__VA_ARGS__)); 
	MONOID_TEST(TEST_MONOID);
#undef TEST_MONOID
#undef MONOID_TEST
#endif // _DEBUG

	// Average monoid
	template<std::floating_point T>
	class mean_op {
		T t_;
		size_t n;
	public:
		constexpr mean_op()
			: t_{}, n{ 0 }
		{}
		constexpr mean_op(const T& t)
			: t_{ t }, n{ 1 }
		{}
		constexpr mean_op(const T& t, size_t n)
			: t_{ t }, n{ n }
		{}

		[[nodiscard]] constexpr T operator()(const mean_op<T>& a, const mean_op<T>& b) const
		{
			return mean_op(a.t_ + b.t_, a.n + b.n);
		}
	};

}