/*
 * Simple program to output the data from a natsnames and natsdata file
 * in an understandable way.
 */
# include <stdio.h>
# include <fcntl.h>

main(argc, argv)
int	argc;
char	**argv;
{
  int i = 1, j = 0, fd, size;
  char c;
  FILE *fptr;
  float buf;
	
	if (argc != 4)
	{
		printf ("Usage: nats <header> <data> <strsize>\n");
		exit (0);
	}

	fptr = fopen (argv[1], "r");
	fd = open (argv[2], O_RDONLY);
	size = atoi (argv[3]);

	while (i > 0)
	{
		for (j = 0; j < size; j++)
		{
			i = fscanf (fptr, "%c", &c);
			printf ("%c", c);
		}
		
		i = read (fd, &buf, sizeof (float));
		printf ("     %f\n\n", buf);
	}

# ifdef notdef
	while (i > 0)
	{
		do
		{
			i = fscanf (fptr, "%c", &c);
			if (c != '\0')
			{
				j = 0;
				printf ("%c", c);
			}
		}
		while (c != '\0');

		if (j == 0)
		{
			i = read (fd, &buf, sizeof (float));
			printf ("     %f\n\n", buf);
			j++;
		}
	}
# endif
}

