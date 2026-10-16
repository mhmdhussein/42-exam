#include "vect2.hpp"
#include <iostream>

int main()
{
    vect2 v1; // 0, 0
    vect2 v2(1, 2); // 1, 2
    const vect2 v3(v2); // 1, 2
    vect2 v4 = v2; // 1, 2

    std::cout << "v1: " << v1 << std::endl;
    std::cout << "v1: " << "{" << v1[0] << ", " << v1[1] << "}" << std::endl;
    std::cout << "v2: " << v2 << std::endl;
    std::cout << "v3: " << v3 << std::endl;
    std::cout << "v4: " << v4 << std::endl;
    std::cout << v4++ << std::endl; // 2, 3
    std::cout << ++v4 << std::endl; // 3, 4
    std::cout << v4-- << std::endl; // 2, 3
    std::cout << --v4 << std::endl; // 1, 2
    std::cout << "v4: " << v4 << std::endl;
    v2 += v3; // 2, 4
    v1 -= v2; // -2, -4
    v2 = v3 + v3 * 2; // 3, 6
    v2 = 3 * v2; // 9, 18
    v2 += v2 += v3; // 20, 40
    v1 *= 42; // -84, -168
    v1 = v1 - v1 + v1;
    std::cout << "v1: " << v1 << std::endl;
    std::cout << "v2: " << v2 << std::endl;
    std::cout << "-v2: " << -v2 << std::endl;
    std::cout << "v2: " << v2 << std::endl;
    std::cout << "v1[1]: " << v1[1] << std::endl;
    v1[1] = 12;
    std::cout << "v1[1]: " << v1[1] << std::endl;
    std::cout << "v3[1]: " << v3[1] << std::endl;
    std::cout << "v1 == v3: " << (v1 == v3) << std::endl;
    std::cout << "v1 == v1: " << (v1 == v1) << std::endl;
    std::cout << "v1 != v3: " << (v1 != v3) << std::endl;
    std::cout << "v1 != v1: " << (v1 != v1) << std::endl;

    // extra
    vect2 a(-3, 7), b(5, -2);
    std::cout << "a - b: " << a - b << std::endl;
    std::cout << "a * -1: " << a * -1 << std::endl;
    std::cout << "0 * a: " << 0 * a << std::endl;
    std::cout << "a + b == b + a: " << (a + b == b + a) << std::endl;
    std::cout << "vect2(2,2) * 2 == vect2(4,4): " << (vect2(2, 2) * 2 == vect2(4, 4)) << std::endl;
    vect2 c;
    c = a;
    c -= b;
    c *= -2;
    std::cout << "c: " << c << ", a: " << a << std::endl;
    std::cout << "v3 * 3: " << v3 * 3 << ", v3: " << v3 << std::endl;
    return 0;
}
