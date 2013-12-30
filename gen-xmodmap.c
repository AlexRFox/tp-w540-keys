#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#define START_F 13
#define END_F 24

void *
xcalloc (unsigned int a, unsigned int b)
{
	void *p;

	if ((p = calloc (a, b)) == NULL) {
		fprintf (stderr, "memory error\n");
		exit (1);
	}

	return (p);
}

int
main (int argc, char **argv)
{
	int n;
	char line[1000], *p, fn[1000], cmd[1000];
	FILE *fp;

	sprintf (fn, "/tmp/xmodmap-XXXXXX");
	close (mkstemp (fn));

	sprintf (cmd, "xmodmap -pke > %s", fn);
	system (cmd);

	fp = fopen (fn, "r");

	n = START_F;

	while (fgets (line, sizeof line, fp)) {
		if (n > END_F) {
			printf ("%s", line);
			continue;
		}

		p = line;

		while (*p && *p != '=')
			p++;

		p++;

		while (*p && isspace (*p))
			p++;

		if (*p == 0) {
			p--;
			*p = 0;
			printf ("%s F%d F%d F%d F%d\n", line, n, n, n, n);
			n++;
		} else {
			printf ("%s", line);
		}
	}

	return (0);
}
