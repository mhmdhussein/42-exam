/* wrong: ignores the last element */
int	max(int* tab, unsigned int len)
{
	if (len == 0)
		return (0);
	int res = tab[0];
	for (unsigned int i = 0; i + 1 < len; i += 1)
	{
		if (res < tab[i])
			res = tab[i];
	}
	return (res);
}
