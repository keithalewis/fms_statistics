# fms_statistics

Streamable statistical functions.

## Iterable

An _iterable_ is an iterator that has an `explicig operator bool() const` 
instead of a `std::ranges` sentinal. 

This is not a new idea.
It is similar to C# `IEnumerator`s. Implementations of that
interface are required to supply `bool MoveNext()`,
`object Current()`, and `void Reset()`.

In C++ `MoveNext` is replaced by `operator++()` and its return value
is replace by `operator bool()`. `Current` is just `operator*()` and
`Reset()` is replaced by the concept `std::forward_iterator`.
This is ubiqutous in Linq which is notorious for its poor performance.
The superiour expresivness of C++ allows for finer grade control. 

The major design flaw in `std::ranges` is to require sentinals
that may be a different type than an iterator. This leads
to algorithms requiring predicates as arguments instead of
just composing functions.

For example, the function $\exp(x) = \sum_{n=0}^\infty x^n/n!$
can be expressed by
`double exp(double x) { return sum(epsilon(pow(x)/factorial())); }`,

The iterable `pow(x)` provides $1$, $x$, $x^2$, $x^3\ldots$
and `factorial()` yields $1$, $1$, $2$, $6\ldots$. The
function `epsilon` truncates the quotient iterable
when it is less than machine epsilon so the `sum` is finite.
It is not the most efficent way to implement `exp` but it
shows the expressive power of iterables. Since $\exp(x + n)
= \exp(x)\exp(n)$ for any integer $n$, precomputing
$\exp(n)$ allows reducing to $0\le x < 1$.




