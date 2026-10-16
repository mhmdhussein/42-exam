/* wrong: swaps its local copies of the pointers, not the values */
void	ft_swap(int *a, int *b)
{
	int	*temp;

	temp = a;
	a = b;
	b = temp;
	(void)a;
	(void)b;
}
