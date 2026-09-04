# fms_statistics

Streamable statistics functions.

An _iterable_ is an iterator that has an `operator bool() const` instead
of a `std::ranges` sentinal. This is not a new idea.
They are similar to C# `IEnumerator`s. Implementations of that
interface are required to supply `bool MoveNext()`,
`object Current()`, and `void Reset()`.

In C++ `MoveNext` is replaced by `operator++()` and its return value
is replace by `operator bool()`. `Current` is just `operator*()` and
`Reset()` is replaced by the concept `std::forward_iterator`.

The major design flaw in `std::ranges` is to require sentinals
that may be a different type than an iterator. This leads
to algorithms requiring predicates as arguments instead of
just composing functions.

For example, the function $\exp(x) = \sum_{n=0}^\infty x^n/n!$
can be expressed by
`exp(double x) { return sum(epsilon(pow(x)/factorial())); }`,

The iterable `pow(x)` provided the iterable $1$, $x$, $x^2\ldots$
and factorial is $1$, $1$, $21$, $6\ldots$. THe
function `epsilon` truncates the iterable when the qutient
is below machine epsilon so the `sum` is finite.

