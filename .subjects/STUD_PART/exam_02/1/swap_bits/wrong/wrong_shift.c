/* wrong: shifts right by 3 instead of 4 */
#include <unistd.h>

unsigned char	swap_bits(unsigned char octet)
{
	return ((octet >> 3) | (octet << 4));
}
