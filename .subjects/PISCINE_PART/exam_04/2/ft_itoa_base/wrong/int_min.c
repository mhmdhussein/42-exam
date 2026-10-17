/* wrong: negates value as an int, which overflows on INT_MIN */
#include <stdlib.h>

char	*ft_itoa_base(int value, int base)
{
	char	digits[] = "0123456789ABCDEF";
	char	*s;
	long	n;
	int		len;
	int		neg;

	neg = (value < 0);
	if (neg && base == 10)
		value = -value;
	n = (base != 10) ? (long)(unsigned int)value : value;
	if (n < 0)
		n = 0;
	len = (neg && base == 10) + 1;
	while (n >= base && ++len)
		n /= base;
	s = malloc(len + 1);
	if (!s)
		return (NULL);
	s[len] = '\0';
	n = (base != 10) ? (long)(unsigned int)value : value;
	if (n < 0)
		n = 0;
	while (len-- > (neg && base == 10))
	{
		s[len] = digits[n % base];
		n /= base;
	}
	if (neg && base == 10)
		s[0] = '-';
	return (s);
}
