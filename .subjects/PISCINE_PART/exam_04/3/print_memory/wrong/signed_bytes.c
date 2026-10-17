/* wrong: reads the bytes as signed chars, so bytes above 127 print wrong */
#include <unistd.h>

static void	print_hex_byte(char c)
{
	write(1, &"0123456789abcdef"[(c / 16) & 15], 1);
	write(1, &"0123456789abcdef"[(c % 16) & 15], 1);
}

void	print_memory(const void *addr, size_t size)
{
	const char			*p = addr;
	size_t				line;
	size_t				i;

	line = 0;
	while (line < size)
	{
		i = 0;
		while (i < 16)
		{
			if (line + i < size)
				print_hex_byte(p[line + i]);
			else
				write(1, "  ", 2);
			if (i % 2)
				write(1, " ", 1);
			i++;
		}
		i = 0;
		while (i < 16 && line + i < size)
		{
			if (p[line + i] >= 32 && p[line + i] < 127)
				write(1, &p[line + i], 1);
			else
				write(1, ".", 1);
			i++;
		}
		write(1, "\n", 1);
		line += 16;
	}
}
