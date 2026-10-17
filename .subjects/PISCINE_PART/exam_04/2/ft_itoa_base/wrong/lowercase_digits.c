/* wrong: uses lowercase letters for digits above 9 */
#include <stdlib.h>

char	*ft_itoa_base(int value, int base)
{
	char			digits[] = "0123456789abcdef";
	char			buf[34];
	char			*s;
	unsigned int	n;
	int				neg;
	int				i;
	int				len;

	neg = (base == 10 && value < 0);
	n = neg ? -(unsigned int)value : (unsigned int)value;
	i = 0;
	do
	{
		buf[i++] = digits[n % base];
		n /= base;
	} while (n);
	if (neg)
		buf[i++] = '-';
	s = malloc(i + 1);
	if (!s)
		return (NULL);
	len = i;
	while (i--)
		s[len - 1 - i] = buf[i];
	s[len] = '\0';
	return (s);
}
