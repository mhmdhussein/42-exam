/* wrong: prints the bits from the lowest to the highest */
#include <unistd.h>

void    print_bits(unsigned char octet)
{
   int i;

   for (i = 0; i <= 7; i++)
   {
        if (octet & (1 << i))
            write (1, "1", 1);
        else
            write (1, "0", 1);
   }
}