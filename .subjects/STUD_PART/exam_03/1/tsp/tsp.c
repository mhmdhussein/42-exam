#include <stdio.h>
#include <math.h>
#include <float.h>

typedef struct s_city
{
	float	x;
	float	y;
}	t_city;

static float	distance(t_city a, t_city b)
{
	return (sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y)));
}

/* The first city stays fixed; try every order of the others. */
static void	solve(t_city *c, int *path, int n, int pos, float len, float *best)
{
	int	i;
	int	tmp;

	if (len >= *best)
		return ;
	if (pos == n)
	{
		len += distance(c[path[n - 1]], c[path[0]]);
		if (len < *best)
			*best = len;
		return ;
	}
	for (i = pos; i < n; i++)
	{
		tmp = path[pos]; path[pos] = path[i]; path[i] = tmp;
		solve(c, path, n, pos + 1, len + distance(c[path[pos - 1]], c[path[pos]]), best);
		tmp = path[pos]; path[pos] = path[i]; path[i] = tmp;
	}
}

int	main(void)
{
	t_city	cities[11];
	int		path[11];
	int		n;
	float	best;

	n = 0;
	while (n < 11 && fscanf(stdin, "%f, %f", &cities[n].x, &cities[n].y) == 2)
	{
		path[n] = n;
		n++;
	}
	if (n < 2)
	{
		printf("0.00\n");
		return (0);
	}
	best = FLT_MAX;
	solve(cities, path, n, 1, 0, &best);
	printf("%.2f\n", best);
	return (0);
}
