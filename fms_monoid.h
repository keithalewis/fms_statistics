// fms_monoid.h - monoids
#pragma once
#include <concepts>
#include <limits>
#include <functional>

namespace fms::statistics {

	template<class T>
	concept monoid = requires(T a, T b) {
		{ T::monoid_op() } -> std::same_as<T>;
		{ T::monoid_op(a, b) } -> std::same_as<T>;
	};

	template<class T, class Op, T Id>
	struct monoid_op {
		static constexpr T monoid_op()
		{
			return Id;
		}
		static constexpr T monoid_op operator()(const T& a, const T& b) const 
		{
			return monoid_op{ Op(a, b);};
		}
	};
	template<class T> using monoid_plus = monoid_op<T, std::plus<T>, T(0)>;	
	template<class T> using monoid_muliplies = monoid_op<T, std::multiplies<T>, T(1)>;
	template<class T> using monoid_max = monoid_op<T, std::max<T>, -std::numeric_limits<T>::infinity()>;
	template<class T> using monoid_min = monoid_op<T, std::min<T>, std::numeric_limits<T>::infinity()>;
}