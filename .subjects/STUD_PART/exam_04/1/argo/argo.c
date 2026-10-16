#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>


typedef struct	json {
	enum {
		MAP,
		INTEGER,
		STRING
	} type;
	union {
		struct {
			struct pair	*data;
			size_t		size;
		} map;
		int	integer;
		char	*string;
	};
}	json;

typedef struct	pair {
	char	*key;
	json	value;
}	pair;

void	free_json(json j);
int	argo(json *dst, FILE *stream);

int	peek(FILE *stream)
{
	int	c = getc(stream);
	ungetc(c, stream);
	return c;
}

void	unexpected(FILE *stream)
{
	if (peek(stream) != EOF)
		printf("unexpected token '%c'\n", peek(stream));
	else
		printf("unexpected end of input\n");
}

int	accept(FILE *stream, char c)
{
	if (peek(stream) == c)
	{
		(void)getc(stream);
		return 1;
	}
	return 0;
}

int	expect(FILE *stream, char c)
{
	if (accept(stream, c))
		return 1;
	unexpected(stream);
	return 0;
}

void	free_json(json j)
{
	switch (j.type)
	{
		case MAP:
			for (size_t i = 0; i < j.map.size; i++)
			{
				free(j.map.data[i].key);
				free_json(j.map.data[i].value);
			}
			free(j.map.data);
			break ;
		case STRING:
			free(j.string);
			break ;
		default:
			break ;
	}
}

void	serialize(json j)
{
	switch (j.type)
	{
		case INTEGER:
			printf("%d", j.integer);
			break ;
		case STRING:
			putchar('"');
			for (int i = 0; j.string[i]; i++)
			{
				if (j.string[i] == '\\' || j.string[i] == '"')
					putchar('\\');
				putchar(j.string[i]);
			}
			putchar('"');
			break ;
		case MAP:
			putchar('{');
			for (size_t i = 0; i < j.map.size; i++)
			{
				if (i != 0)
					putchar(',');
				serialize((json){.type = STRING, .string = j.map.data[i].key});
				putchar(':');
				serialize(j.map.data[i].value);
			}
			putchar('}');
			break ;
	}
}

int	parse_value(json *dst, FILE *stream);

/* Reads a string starting at '"'; handles only \ and \" escapes. */
char	*parse_string(FILE *stream)
{
	char	*s = NULL;
	char	*tmp;
	size_t	len = 0;
	int		c;

	if (!expect(stream, '"'))
		return NULL;
	while (1)
	{
		c = getc(stream);
		if (c == EOF)
		{
			unexpected(stream);
			free(s);
			return NULL;
		}
		if (c == '"')
			break ;
		if (c == '\\')
		{
			if (peek(stream) != '"' && peek(stream) != '\\')
			{
				unexpected(stream);
				free(s);
				return NULL;
			}
			c = getc(stream);
		}
		tmp = realloc(s, len + 2);
		if (!tmp)
		{
			free(s);
			return NULL;
		}
		s = tmp;
		s[len++] = c;
	}
	tmp = realloc(s, len + 1);
	if (!tmp)
	{
		free(s);
		return NULL;
	}
	tmp[len] = '\0';
	return tmp;
}

int	parse_map(json *dst, FILE *stream)
{
	pair	*tmp;
	char	*key;

	dst->type = MAP;
	dst->map.data = NULL;
	dst->map.size = 0;
	if (!expect(stream, '{'))
		return -1;
	if (accept(stream, '}'))
		return 1;
	while (1)
	{
		key = parse_string(stream);
		if (!key)
			return -1;
		tmp = realloc(dst->map.data, sizeof(pair) * (dst->map.size + 1));
		if (!tmp)
		{
			free(key);
			return -1;
		}
		dst->map.data = tmp;
		tmp = &dst->map.data[dst->map.size++];
		tmp->key = key;
		tmp->value.type = INTEGER;
		if (!expect(stream, ':') || parse_value(&tmp->value, stream) != 1)
			return -1;
		if (accept(stream, '}'))
			return 1;
		if (!expect(stream, ','))
			return -1;
	}
}

int	parse_value(json *dst, FILE *stream)
{
	int	c = peek(stream);

	dst->type = INTEGER;
	if (c == '"')
	{
		dst->string = parse_string(stream);
		if (!dst->string)
			return -1;
		dst->type = STRING;
		return 1;
	}
	if (c == '{')
		return parse_map(dst, stream);
	if (isdigit(c) || c == '-')
	{
		if (fscanf(stream, "%d", &dst->integer) == 1)
			return 1;
	}
	unexpected(stream);
	return -1;
}

int	argo(json *dst, FILE *stream)
{
	return parse_value(dst, stream);
}

int	main(int argc, char **argv)
{
	if (argc != 2)
		return 1;
	char *filename = argv[1];
	FILE *stream = fopen(filename, "r");
	json	file;
	if (argo (&file, stream) != 1)
	{
		free_json(file);
		return 1;
	}
	serialize(file);
	printf("\n");
}
