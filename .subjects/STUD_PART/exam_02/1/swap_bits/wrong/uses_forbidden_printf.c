/* wrong: leftover debug printf, which is not allowed */
#include <stdio.h>

unsigned char	swap_bits(unsigned char octet)
{
	printf("octet = %d\n", octet);
	return ((octet >> 4) | (octet << 4));
}
