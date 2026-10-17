/* wrong: counts the terminating '\0' */
int     ft_strlen(char *str)
{
	int		i;

	i = 0;
	while (str[i] != '\0')
		i++;
	return (i + 1);
}