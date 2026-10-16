#include "bigint.hpp"
#include <iostream>

int main()
{
    const bigint a(42);
    bigint b(21), c, d(1337), e(d);

    std::cout << "a = " << a << std::endl;
    std::cout << "b = " << b << std::endl;
    std::cout << "c = " << c << std::endl;
    std::cout << "d = " << d << std::endl;
    std::cout << "e = " << e << std::endl;

    std::cout << "a + b = " << a + b << std::endl;
    std::cout << "(c += a) = " << (c += a) << std::endl;

    std::cout << "b = " << b << std::endl;
    std::cout << "++b = " << ++b << std::endl;
    std::cout << "b++ = " << b++ << std::endl;
    std::cout << "b = " << b << std::endl;

    std::cout << "(b << 10) + 42 = " << ((b << 10) + 42) << std::endl;
    std::cout << "(d <<= 4) = " << (d <<= 4) << std::endl;
    std::cout << "(d >>= 2) = " << (d >>= (const bigint)2) << std::endl;

    std::cout << "a =" << a << std::endl;
    std::cout << "d =" << d << std::endl;

    std::cout << "(d < a) = " << (d < a) << std::endl;
    std::cout << "(d <= a) = " << (d <= a) << std::endl;
    std::cout << "(d > a) = " << (d > a) << std::endl;
    std::cout << "(d >= a) = " << (d >= a) << std::endl;
    std::cout << "(d == a) = " << (d == a) << std::endl;
    std::cout << "(d != a) = " << (d != a) << std::endl;

    // carries and big numbers
    bigint x(4294967295u), y(1);
    std::cout << "x + y = " << x + y << std::endl;
    for (int i = 0; i < 6; i++)
        x += x;
    std::cout << "x * 64 = " << x << std::endl;
    bigint big = (x << 25) + x;
    std::cout << "big = " << big << std::endl;
    std::cout << "big + big = " << big + big << std::endl;
    bigint nines(999999999);
    nines = (nines << 9) + 999999999;
    std::cout << "nines + 1 = " << nines + y << std::endl;
    std::cout << "1337 >> 2 = " << (bigint(1337) >> 2) << std::endl;
    std::cout << "big >> 30 = " << (big >> 30) << std::endl;
    std::cout << "big >> 100 = " << (big >> 100) << std::endl;
    std::cout << "0 << 5 = " << (bigint(0) << 5) << std::endl;
    std::cout << "0 + 0 = " << bigint(0) + bigint() << std::endl;

    // comparisons between numbers of different lengths
    bigint p(9), q(10), r(100), s(99);
    std::cout << (p < q) << (q < p) << (r > s) << (s > r) << (r == r) << (r != s)
              << (s <= s) << (s >= r) << (q == bigint(10)) << std::endl;
    std::cout << (big > nines) << (big < nines) << (big == big + 0) << std::endl;
    bigint t(7);
    t <<= 3;
    t >>= 1;
    std::cout << "t = " << t << std::endl;
    return 0;
}
