#ifndef BIGINT_HPP
#define BIGINT_HPP

#include <iostream>
#include <string>

// Arbitrary precision unsigned integer, stored as base 10 digits,
// most significant first, without leading zeros ("0" for zero).
class bigint
{
	private:
		std::string digits;

		void trim();

	public:
		bigint();
		bigint(unsigned int n);
		bigint(const bigint &other);
		bigint &operator=(const bigint &other);
		~bigint();

		const std::string &str() const;

		bigint operator+(const bigint &other) const;
		bigint &operator+=(const bigint &other);
		bigint &operator++();
		bigint operator++(int);

		bigint operator<<(unsigned int n) const;
		bigint operator>>(unsigned int n) const;
		bigint &operator<<=(unsigned int n);
		bigint &operator>>=(unsigned int n);
		bigint operator<<(const bigint &n) const;
		bigint operator>>(const bigint &n) const;
		bigint &operator<<=(const bigint &n);
		bigint &operator>>=(const bigint &n);

		bool operator==(const bigint &other) const;
		bool operator!=(const bigint &other) const;
		bool operator<(const bigint &other) const;
		bool operator>(const bigint &other) const;
		bool operator<=(const bigint &other) const;
		bool operator>=(const bigint &other) const;
};

std::ostream &operator<<(std::ostream &os, const bigint &n);

#endif
