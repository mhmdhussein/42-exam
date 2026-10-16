#include "bigint.hpp"
#include <sstream>

bigint::bigint() : digits("0") {}

bigint::bigint(unsigned int n)
{
	std::ostringstream os;

	os << n;
	digits = os.str();
}

bigint::bigint(const bigint &other) : digits(other.digits) {}

bigint &bigint::operator=(const bigint &other)
{
	digits = other.digits;
	return *this;
}

bigint::~bigint() {}

void bigint::trim()
{
	std::string::size_type i = digits.find_first_not_of('0');

	if (i == std::string::npos)
		digits = "0";
	else
		digits.erase(0, i);
}

const std::string &bigint::str() const
{
	return digits;
}

bigint bigint::operator+(const bigint &other) const
{
	std::string	res;
	int			i = digits.size() - 1;
	int			j = other.digits.size() - 1;
	int			carry = 0;

	while (i >= 0 || j >= 0 || carry)
	{
		int sum = carry;
		if (i >= 0)
			sum += digits[i--] - '0';
		if (j >= 0)
			sum += other.digits[j--] - '0';
		res.insert(res.begin(), char('0' + sum % 10));
		carry = sum / 10;
	}
	bigint r;
	r.digits = res;
	r.trim();
	return r;
}

bigint &bigint::operator+=(const bigint &other)
{
	*this = *this + other;
	return *this;
}

bigint &bigint::operator++()
{
	return *this += bigint(1);
}

bigint bigint::operator++(int)
{
	bigint old(*this);

	++*this;
	return old;
}

bigint bigint::operator<<(unsigned int n) const
{
	bigint r(*this);

	return r <<= n;
}

bigint bigint::operator>>(unsigned int n) const
{
	bigint r(*this);

	return r >>= n;
}

bigint &bigint::operator<<=(unsigned int n)
{
	if (digits != "0")
		digits.append(n, '0');
	return *this;
}

bigint &bigint::operator>>=(unsigned int n)
{
	if (n >= digits.size())
		digits = "0";
	else
		digits.erase(digits.size() - n);
	return *this;
}

// Shift amounts given as bigint: anything that doesn't fit is huge, so
// left shifts are refused (left as is) and right shifts give 0.
static unsigned int to_uint(const bigint &n, bool &fits)
{
	std::istringstream	is(n.str());
	unsigned int		v = 0;

	fits = n.str().size() <= 9;
	if (fits)
		is >> v;
	return v;
}

bigint bigint::operator<<(const bigint &n) const
{
	bigint r(*this);

	return r <<= n;
}

bigint bigint::operator>>(const bigint &n) const
{
	bigint r(*this);

	return r >>= n;
}

bigint &bigint::operator<<=(const bigint &n)
{
	bool			fits;
	unsigned int	v = to_uint(n, fits);

	if (fits)
		*this <<= v;
	return *this;
}

bigint &bigint::operator>>=(const bigint &n)
{
	bool			fits;
	unsigned int	v = to_uint(n, fits);

	if (fits)
		return *this >>= v;
	digits = "0";
	return *this;
}

bool bigint::operator==(const bigint &other) const
{
	return digits == other.digits;
}

bool bigint::operator!=(const bigint &other) const
{
	return !(*this == other);
}

bool bigint::operator<(const bigint &other) const
{
	if (digits.size() != other.digits.size())
		return digits.size() < other.digits.size();
	return digits < other.digits;
}

bool bigint::operator>(const bigint &other) const
{
	return other < *this;
}

bool bigint::operator<=(const bigint &other) const
{
	return !(other < *this);
}

bool bigint::operator>=(const bigint &other) const
{
	return !(*this < other);
}

std::ostream &operator<<(std::ostream &os, const bigint &n)
{
	return os << n.str();
}
