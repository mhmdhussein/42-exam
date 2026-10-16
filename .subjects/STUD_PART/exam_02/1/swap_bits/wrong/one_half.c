/* wrong: keeps only the high half moved down */
#include <unistd.h>

unsigned char	swap_bits(unsigned char octet)
{
	return (octet >> 4);
}
