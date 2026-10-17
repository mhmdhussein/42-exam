#include <stdlib.h>

void	print_memory(const void *addr, size_t size);

// Each argument is a hex byte string ("00ff41"); the bytes are printed with
// print_memory. "subject": the int array from the subject.
int	main(int argc, char **argv)
{
	unsigned char	buf[4096];
	size_t			n;
	char			*s;
	char			hex[3];
	int				tab[10] = {0, 23, 150, 255, 12, 16, 21, 42};

	if (argc == 2 && argv[1][0] == 's')
		return (print_memory(tab, sizeof(tab)), 0);
	n = 0;
	for (int i = 1; i < argc; i++)
		for (s = argv[i]; s[0] && s[1] && n < sizeof(buf); s += 2)
		{
			hex[0] = s[0];
			hex[1] = s[1];
			hex[2] = 0;
			buf[n++] = (unsigned char)strtol(hex, NULL, 16);
		}
	print_memory(buf, n);
	return (0);
}
