// fms_iterable.h - iterators with explicit operator bool() const
#pragma once
#ifdef _DEBUG
#include <cassert>
#endif // _DEBUG
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <forward_list>
#include <functional>
#include <iterator>
#include <limits>
#include <span>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace fms::iterable {

#pragma region Concepts

	template<class I>
	concept has_operator_bool = requires(I i) {
		{ i.operator bool() } -> std::same_as<bool>;
	};

	template<class I>
	concept has_end = requires(I i) {
		{ i.end() } -> std::convertible_to<I>;
	};

	template<class I>
	concept has_size = requires(I i) {
		{ i.size() } -> std::convertible_to <size_t> ;
	};

	template<class I>
	concept is_endable = has_end<I> || has_size<I>;

	template<class I>
	concept input_iterable = std::input_iterator<I> && has_operator_bool<I>;
	template<class I>
	concept forward_iterable = std::forward_iterator<I> && has_operator_bool<I>;
	template<class I, class T>
	concept output_iterable = std::output_iterator<I,T> && has_operator_bool<I>;

#pragma endregion
#pragma region Functions

	template<class I>
	constexpr I begin(I i)
	{
		return i;
	}
	
	template<class I>
	constexpr I end(I i)
	{
		if constexpr (has_end<I>) {
			return i.end();
		}
		if constexpr (has_size<I> && std::random_access_iterator<I>) {
			return i.begin() + i.size();
		}
		// !!! could be infinite
		while (i) {
			++i;
		}

		return i; 
	}

	// length(i,j) = length(i) + length(j)
	template<class I> // std::weakly_incrementable???
	constexpr std::size_t length(I i, std::size_t n = 0)
	{
		if constexpr (has_end<I>) {
			return n + std::distance(i, i.end());
		}
		if constexpr (has_size<I>) {
			return n + i.size();
		}

		while (i) { // !!! dangerous, could be infinite ??? throw
			++i;
			++n;
		}

		return n;
	}

	// lexicographic comparison of iterable values
	template<class I, class J>
	constexpr auto compare(I i, J j)
	{
		return std::lexicographical_compare_three_way(i, end(i), j, end(j));
	}
	// spaceship operator helper
	template<class I, class J> // is_endable??
	constexpr bool equal(I i, J j)
	{
		return std::equal(i, end(i), j, end(j));
	}

	template<class I> // std::w_i
	constexpr I drop(I i, std::size_t n = 1)
	{
		if constexpr (has_end<I>) {
			size_t m = std::distance(i, i.end());

			return std::next(i, std::min(n, m));
		}
		if constexpr (has_size<I>) {
			return std::next(i, std::min(n, i.size()));
		}
		while (i && n) {
			++i;
			--n;
		}

		return i;
	}

	template<input_iterable I, std::weakly_incrementable J>
		requires std::indirectly_copyable<I, J> && has_operator_bool<J>
	constexpr J copy(I i, J j)
	{
		while (i and j) {
			*j = *i;
			++i;
			++j;
		}

		return j;
	}
	template<input_iterable I, std::weakly_incrementable J>
		requires std::indirectly_copyable<I, J>&& has_operator_bool<J>
	constexpr J copy_n(I i, std::size_t n, J j)
	{
		while (n and i and j) {
			*j = *i;
			++i;
			--n;
			++j;
		}

		return j;
	}

#pragma endregion

	// turn begin/end into an iterable::iterator
	template<std::input_iterator I>
	class iterator {
		I b, e;
		using It = std::iterator_traits<I>;
	public:
		using iterator_category = It::iterator_category;
		using value_type = It::value_type;
		using difference_type = It::difference_type;
		using pointer = It::pointer;
		using reference = It::reference;

		constexpr iterator()
		{ }
		constexpr iterator(I b, I e)
			: b{ b }, e{ e }
		{ }
		constexpr iterator(const iterator&) = default;
		constexpr iterator& operator=(const iterator&) = default;
		constexpr ~iterator() = default;

		constexpr bool operator==(const iterator& rhs) const
		{
			return b == rhs.b && e == rhs.e;
		}

		constexpr I begin() const
		{
			return b;
		}
		constexpr I end() const
		{
			return e;
		}

		constexpr explicit operator bool() const
		{
			return b != e;
		}
		constexpr value_type operator*() const
		{
			return *b;
		}
		constexpr iterator& operator++()
		{
			if (b != e)
				++b;

			return *this;
		}
		constexpr iterator operator++(int)
		{
			iterator tmp{ *this };
			operator++();

			return tmp;
		}
		// TODO: add based on iterator category: operator--(), operator--(int), operator+(int), operator-(int), operator+=(int), operator-=(int)
	};
#ifdef _DEBUG
	static void interval_test()
	{
		int a[3] = { 1, 2, 3 };
		auto i = iterator(a, a + 3);
		auto i2{ i };
		assert(i == i2);
		i = i2;
		assert(!(i != i2));
		assert(*i == 1);
		assert(*++i == 2);
		++i;
		assert(*i == 3);
		// *i = 4; fails
		assert(!++i);
	}
#endif // _DEBUG

	// assumes lifetime of C
	template<class C>
	constexpr auto container(C& c)
	{
		return iterator(c.begin(), c.end());
	}

	// use const ref to extend lifetime of std::initializer_list<T>
	template<class T>
	class list : public iterator<T*> {
		const std::initializer_list<T>& l;
	public:
		constexpr list(std::initializer_list<T> l)
			: iterator<T*>(const_cast<T*>(l.begin()), const_cast<T*>(l.end())), l{ l }
		{ }
		constexpr list(const list&) = delete;
		constexpr list& operator=(const list&) = delete;
		constexpr ~list() = default;
	};
#ifdef _DEBUG
	inline void list_test()
	{
		auto l = list{ 1, 2, 3 };
		assert(*l == 1);
		assert(*++l == 2);
		++l;
		assert(*l == 3);
		assert(!++l);
	}
#endif // _DEBUG

	template<input_iterable I>
	class take : public I {
		std::size_t n;
	public:
		// inherit using definitions from I
		constexpr take(I i, std::size_t n)
			: I{ i }, n{ n }
		{}

		constexpr explicit operator bool() const
		{
			return n > 0 && I::operator bool();
		}
		constexpr I::value_type operator*() const
		{
			return I::operator*();
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
#ifdef _DEBUG
	/*
	// take inherits iterator_category from I
	static_assert(std::same_as<take<empty<int>>::iterator_category, empty<int>::iterator_category>);
	static_assert(0 == length(take(empty<int>(), 1)));
	static_assert(2 == length(take(empty<int>(), 1), 2));
	*/
#endif // _DEBUG

	// on the fly iterable
	template<class T>
	class vector {
		std::vector<T> v;
		using It = std::vector<T>::iterator;
	public:
		using iterator_category = It::iterator_category;
		using value_type = It::value_type;
		using difference_type = It::difference_type;
		using pointer = It::pointer;
		using reference = It::reference;

		constexpr vector(std::initializer_list<T> v)
			: v(v)
		{ }

		constexpr auto begin() const
		{
			return v.begin();
		}
		constexpr auto end() const
		{
			return v.end();
		}
		constexpr std::size_t size() const noexcept
		{
			return v.size();
		}

		constexpr explicit operator bool() const noexcept
		{
			return !v.empty();
		}
		constexpr value_type operator*() const
		{
			return v.front();
		}
		constexpr reference operator*()
		{
			return v.front();
		}
		constexpr vector& operator++()
		{
			if (!v.empty()) {
				v.erase(v.begin());
			}

			return *this;
		}
		constexpr vector operator++(int)
		{
			vector tmp{ *this };
			operator++();
			
			return tmp;
		}
#ifdef _DEBUG
		static void test()
		{
			auto l = vector{ 1, 2, 3 };
			auto l2{ l };
			assert(equal(l, l2));
			l = l2;
			assert(3 == l.size());
			assert(3 == l2.size());
			assert(equal(l, l2));
			assert(*l == 1);
			assert(*++l == 2);
			++l;
			assert(*l == 3);
			assert(!++l);
		}
#endif // _DEBUG
	};

	/*
	namespace {
		constexpr std::array aa{ c };
		static_assert(*container(aa) == 1);
		static_assert(*++container(aa) == 2);
		static_assert(*++ ++container(aa) == 3);
		static_assert(!(++++ ++container(aa)));
	}
	*/
	/*
#ifdef _DEBUG
	static_assert(equal (vector{ 1, 4 }, vector{ 1, 4 });
	static_assert(vector{ 1, 4 } > vector{ 1, 3 });
	static_assert(vector{ 1, 2 } < vector{ 1, 3 });
	static_assert(vector{ 1, 2 } < vector{ 2, 3 }); // 1 < 2
	static_assert(vector{ 1, 2 } < vector{ 1, 3 }); // 2 < 3
#endif // _DEBUG
*/
	// unsafe pointer
	template<class T>
	class ptr {
		T* p;
	public:
		using iterator_category = std::iterator_traits<T*>::iterator_category;
		using value_type = T;
		using difference_type = ptrdiff_t;
		using pointer = T*;
		using reference = T&;

		constexpr ptr(T* p)
			: p{ p }
		{}
		constexpr explicit operator bool() const
		{
			return p != nullptr; // !!!unsafe
		}
		constexpr explicit operator T* ()
		{
			return p;
		}
		constexpr explicit operator const T* () const
		{
			return p;
		}
		constexpr value_type operator*() const
		{
			return *p;
		}
		constexpr reference operator*()
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
		// TODO: operatator--(), operator--(int), operator+(int), operator-(int), operator+=(int), operator-=(int)
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
		using iterator_category = std::iterator_traits<T*>::iterator_category;
		using value_type = T;
		using difference_type = ptrdiff_t;
		using pointer = T*;
		using reference = T&;

		constexpr null_ptr(T* p = nullptr)
			: p{ p }
		{}
		constexpr explicit operator bool() const
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
		static_assert(3 == length(null_ptr("abc")));
		constexpr int b[3] = { 1, 2, 0 };
		static_assert(*null_ptr(b) == 1);
		static_assert(*++null_ptr(b) == 2);
		static_assert(!++ ++null_ptr(b));
	}

	// non-owning view of a contiguous sequence of objects
	template<class T, std::size_t N = std::dynamic_extent>
	struct span {
		std::span<T, N> s;
	public:
		using iterator_category = std::iterator_traits<T*>::iterator_category;
		using value_type = T;
		using difference_type = ptrdiff_t;
		using pointer = T*;
		using reference = T&;

		constexpr span(std::span<T, N> s)
			: s{ s }
		{}
		constexpr explicit operator bool() const
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

	// t, t + 1, ...
	template<class T>
	class iota {
		T t;
	public:
		using iterator_category = std::iterator_traits<T*>::iterator_category;
		using value_type = T;
		using difference_type = ptrdiff_t;
		using pointer = T*;
		using reference = T&;

		constexpr iota(T t = T(0))
			: t{ t }
		{}
		constexpr explicit operator bool() const
		{
			return true;
		}
		constexpr T operator*() const
		{
			return t;
		}
		constexpr iota& operator++() {
			t += T(1);

			return *this;
		}
		constexpr iota operator++(int) {
			iota tmp{ *this };
			++t;

			return tmp;
		}
		// operatator--(), operator--(int), operator+(int), operator-(int), operator+=(int), operator-=(int)

	};
	static_assert(input_iterable<iota<int>>);
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
		using iterator_category = std::iterator_traits<T*>::iterator_category;
		using value_type = T;
		using difference_type = ptrdiff_t;
		using pointer = T*;
		using reference = T&;

		constexpr stride(T init, T step = 1, std::size_t size = std::numeric_limits<size_t>::max())
			: init{ init }, step{ step }, size{ size }
		{ }
		constexpr explicit operator bool() const
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
	static_assert(input_iterable<stride<int>>);
	static_assert(stride(0, 1).operator bool());
	static_assert(*stride(0, 1) == 0);
	static_assert(*stride(1, 1) == 1);
	static_assert(*++stride(0, 1) == 1);
	static_assert(*++stride(2, 3) == 5);
	static_assert(*++stride(2, -3) == -1);
	static_assert(*++stride(-2, -3) == -5);

	template<class T>
	constexpr auto singleton(T t)
	{
		return take(iota<T>(t), 1);
	}
	static_assert(singleton(1).operator bool());
	static_assert(singleton(1));
	static_assert(*singleton(1) == 1);
	static_assert(!++singleton(1));

	// [a, b) TODO: allow a > b?
	template<class T>
	constexpr auto range(T a, T b)
	{
		return take(iota<T>(a), a <= b ? b - a : 0);
	}

	// tuple valued
	template<input_iterable... Is>
	class zip {
		std::tuple<Is...> is;
	public:
		using iterator_category = std::common_type_t<typename std::iterator_traits<Is>::iterator_category...>;
		using value_type = std::common_type_t<typename std::iterator_traits<Is>::value_type...>;
		using difference_type = std::common_type_t<typename std::iterator_traits<Is>::difference_type...>;

		zip(const Is&... is)
			: is{ is... }
		{ }
		// stop when any of the iterables is exhausted
		constexpr explicit operator bool() const
		{
			return std::apply([](auto&&... i) { return (i && ...); }, is);
		}
		constexpr auto operator*() const
		{
			return std::apply([](auto&&... i) { return std::tuple{ *i... }; }, is);
		}
		constexpr zip& operator++() {
			std::apply([](auto&&... i) { (++i, ...); }, is);

			return *this;
		}
		constexpr zip operator++(int) {
			zip tmp{ *this };
			operator++();

			return tmp;
		}
	};
	static_assert(std::is_same_v<double, zip<iota<int>, iota<double>>::value_type>);
/*
#ifdef _DEBUG
	static void zip_test() 
	{
		{
			vector a({ 1, 2, 3 });
			vector b({ 4, 5, 6 });
			auto z = zip(a, b);
			assert(*z == std::tuple(1, 4));
			assert(*++z == std::tuple(2, 5));
			z++;
			assert(*z == std::tuple(3, 6));
			assert(!++z);
		}
		{
			vector a({ 1, 2 });
			vector b({ 4, 5, 6 });
			auto z = zip(a, b);
			assert(*z == std::tuple(1, 4));
			assert(*++z == std::tuple(2, 5));
			z++;
			assert(!z);
		}
		{
			vector a({ 1, 2, 3 });
			vector b({ 4, 5 });
			auto z = zip(a, b);
			assert(*z == std::tuple(1, 4));
			assert(*++z == std::tuple(2, 5));
			z++;
			assert(!z);
		}
	}
#endif // _DEBUG
*/}
