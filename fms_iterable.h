// fms_iterable.h - iterators with operator bool() const
#pragma once
#include <concepts>
#include <functional>
#include <iterator>
#include <limits>
#include <span>

namespace fms::iterable {

	template<class I>
	concept has_operator_bool = requires(I i) {
		{ i.operator bool() } -> std::same_as<bool>;
	};

	template<class I>
	concept has_end = requires(I i) {
		{ i.end() } -> std::convertible_to<I>;
	};

	template<class I>
	concept has_last = requires(I i) {
		{ i.last() } -> std::convertible_to<I>;
	};

	template<class I>
	concept has_empty = requires(I i) {
		{ i.empty() } -> std::convertible_to<bool>;
	};

	template<class I>
	concept is_iterable = std::input_or_output_iterator<I> && has_operator_bool<I>;

	// turn begin/end into an iterable
	template<class I>
	class interval {
		I b, e;
	public:
		constexpr interval(I b, I e)
			: b{ b }, e{ e }
		{ }
		constexpr I begin() const
		{
			return b;
		}
		constexpr I end() const
		{
			return e;
		}

		constexpr operator bool() const
		{
			return b != e;
		}
		constexpr auto operator*() const
		{
			return *b;
		}
		constexpr interval& operator++()
		{
			if (b != e)
				++b;

			return *this;
		}
		constexpr interval operator++(int)
		{
			interval tmp{ *this };
			operator++();

			return tmp;
		}
	};
	namespace {
		constexpr bool test_interval()
		{
			constexpr int c[3] = { 1, 2, 3 };
			//constexpr auto i = interval(c, c + 3);
			return true;
		}
		static_assert(test_interval);
	}

	// assumes lifetime of C
	template<class C>
	constexpr auto container(C& c)
	{
		return interval(std::begin(c), std::end(c));
	}
	/*
	namespace {
		constexpr std::array aa{ c };
		static_assert(*container(aa) == 1);
		static_assert(*++container(aa) == 2);
		static_assert(*++ ++container(aa) == 3);
		static_assert(!(++++ ++container(aa)));
	}
	*/

	template<is_iterable I, is_iterable J>
	constexpr auto operator<=>(I i, J j)
	{
		while (i && j) {
			auto cmp = *i <=> *j;
			if (cmp != 0) {
				return cmp;
			}
			++i;
			++j;
		}

		return i ? 1 : j ? -1 : 0;
	}
	// i starts with all of j
	template<is_iterable I, is_iterable J>
	constexpr bool starts_with(I i, J j)
	{
		return i <=> j == 1;
	}

	template<is_iterable I>
	constexpr I begin(I i)
	{
		return i;
	}
	template<is_iterable I>
	constexpr I end(I i)
		requires has_end<I>
	{
		return i.end();
	}

	// unsafe pointer
	template<class T>
	class ptr {
		T* p;
	public:
		constexpr ptr(T* p)
			: p{ p }
		{}
		constexpr operator bool() const
		{
			return true; // !!!unsafe
		}
		constexpr operator T* ()
		{
			return p;
		}
		constexpr operator const T* () const
		{
			return p;
		}
		constexpr T& operator*() const
		{
			return *p;
		}
		constexpr ptr& operator++()
		{
			++p;
			return *this;
		}
		constexpr ptr operator++(int)
		{
			ptr tmp{ *this };
			++p;
			return tmp;
		}
		// operatator--(), operator--(int), operator+(int), operator-(int), operator+=(int), operator-=(int)
	};
	namespace {
		constexpr int a[3] = { 1, 2, 3 };
		static_assert(*ptr(a) == 1);
		static_assert(*++ptr(a) == 2);
	}

	// safer null terminated pointer
	template<class T>
	class null_ptr {
		T* p;
	public:
		constexpr null_ptr(T* p)
			: p{ p }
		{}
		constexpr operator bool() const
		{
			return p != nullptr and *p != 0;
		}
		// do not provide implicit conversion to T* or const T*
		constexpr explicit operator T* ()
		{
			return p;
		}
		constexpr explicit operator const T* () const
		{
			return p;
		}
		constexpr T& operator*() const
		{
			return *p;
		}
		constexpr null_ptr& operator++()
		{
			++p;
			return *this;
		}
		constexpr null_ptr operator++(int)
		{
			null_ptr tmp{ *this };
			++p;
			return tmp;
		}
		// operatator--(), operator--(int), operator+(int), operator-(int), operator+=(int), operator-=(int)
	};
	namespace {
		constexpr int b[3] = { 1, 2, 0 };
		static_assert(*null_ptr(b) == 1);
		static_assert(*++null_ptr(b) == 2);
		static_assert(!++ ++null_ptr(b));
	}

	template<class T, size_t N = std::dynamic_extent>
	struct span {
		std::span<T, N> s;
	public:
		constexpr span(std::span<T, N> s)
			: s{ s }
		{}
		constexpr operator bool() const
		{
			return !s.empty();
		}
		constexpr T operator*() const
		{
			return s.front();
		}
		constexpr span& operator++() {
			s = s.subspan(1);

			return *this;
		}
		constexpr span operator++(int) {
			span tmp{ *this };
			operator++();

			return tmp;
		}
	};

	template<class I>
	constexpr I drop(I i, std::size_t n = 1)
	{
		if constexpr (has_end<I>) {
			size_t m = std::distance(i, i.end());

			return std::next(i, std::min(n, m));
		}
		while (i && n) {
			++i;
			--n;
		}

		return i;
	}

	template<is_iterable I>
	class take : public I {
		std::size_t n;
	public:
		constexpr take(I i, std::size_t n)
			: I{ i }, n{ n }
		{}
		constexpr operator bool() const
		{
			return n > 0 && I::operator bool();
		}
		constexpr take& operator++()
		{
			--n;
			++static_cast<I&>(*this);

			return *this;
		}
		constexpr take operator++(int)
		{
			take tmp{ *this };
			++*this;

			return tmp;
		}
	};

	// t, t + 1, ...
	template<class T>
	class iota {
		T t;
	public:
		constexpr iota(T t = T(0))
			: t{ t }
		{}
		constexpr operator bool() const
		{
			return true;
		}
		constexpr T operator*() const
		{
			return t;
		}
		constexpr iota& operator++() {
			++t;

			return *this;
		}
		constexpr iota operator++(int) {
			iota tmp{ *this };
			++t;

			return tmp;
		}
		// operatator--(), operator--(int), operator+(int), operator-(int), operator+=(int), operator-=(int)

	};
	static_assert(is_iterable<iota<int>>);
	static_assert(iota<int>().operator bool());
	static_assert(iota(0).operator bool());
	static_assert(*iota(0) == 0);
	static_assert(*iota(1) == 1);
	static_assert(*++iota(0) == 1);

	// init, init + step, ..., init + (size - 1) * step
	template<class T>
	class stride {
		T init, step;
		size_t size;
	public:
		constexpr stride(T init, T step = 1, size_t size = std::numeric_limits<size_t>::max())
			: init{ init }, step{ step }, size{ size }
		{}
		constexpr operator bool() const
		{
			return size > 0;
		}
		constexpr T operator*() const
		{
			return init;
		}
		constexpr stride& operator++() {
			init += step;
			--size;
			return *this;
		}
		constexpr stride operator++(int) {
			stride tmp{ *this };
			operator++();
			return tmp;
		}
	};
	static_assert(is_iterable<stride<int>>);
	static_assert(stride(0, 1).operator bool());
	static_assert(*stride(0, 1) == 0);
	static_assert(*stride(1, 1) == 1);
	static_assert(*++stride(0, 1) == 1);
	static_assert(*++stride(2, 3) == 5);
	static_assert(*++stride(2, -3) == -1);
	static_assert(*++stride(-2, -3) == -5);


	template<is_iterable I>
	constexpr std::size_t length(I i, std::size_t n = 0)
	{
		if constexpr (has_end<I>) {
			return std::distance(i, i.end());
		}

		while (i) {
			++i;
			++n;
		}

		return n;
	}

	template<class T>
	constexpr auto empty()
	{
		return take(iota<T>(), 0);
	}
	static_assert(!empty<int>().operator bool());
	static_assert(!empty<int>());
	static_assert(length(empty<int>()) == 0);

	template<class T>
	constexpr auto singleton(T t)
	{
		return take(iota<T>(t), 1);
	}
	static_assert(singleton(1).operator bool());
	static_assert(singleton(1));
	static_assert(*singleton(1) == 1);
	static_assert(!++singleton(1));

	// [a, b)
	template<class T>
	constexpr auto range(T a, T b)
	{
		return take(iota<T>(a), a <= b ? b - a : 0);
	}

}
