/* wrong: stops at the first space */
int     ft_strlen(char *str)
{
	int		i;

	i = 0;
	while (str[i] != '\0' && str[i] != ' ')
		i++;
	return (i);
}