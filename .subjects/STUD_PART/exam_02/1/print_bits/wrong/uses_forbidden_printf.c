/* wrong: prints with printf, which is not allowed */
#include <stdio.h>

void    print_bits(unsigned char octet)
{
   int i;

   for (i = 7; i >= 0; i--)
   {
        if (octet & (1 << i))
            printf("1");
        else
            printf("0");
   }
}