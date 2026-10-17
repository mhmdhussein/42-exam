#include <stdio.h>
#include <stdlib.h>

char	*ft_itoa_base(int value, int base);

// Each pair of arguments is a value and a base.
int	main(int argc, char **argv)
{
	char	*s;
	int		i;

	i = 1;
	while (i + 1 < argc)
	{
		s = ft_itoa_base(atoi(argv[i]), atoi(argv[i + 1]));
		printf("%s\n", s);
		free(s);
		i += 2;
	}
	return (0);
}
