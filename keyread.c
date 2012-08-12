#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/input.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>

void
usage (void)
{
	fprintf (stderr, "usage: keyread hostname\n");
	exit (1);
}

void
process (char *buf)
{
	int value, code;

	if (sscanf (buf, "kbd %d %d", &value, &code) == 2)
		printf ("%d, %d\n", value, code);
}

int
main (int argc, char **argv)
{
	struct sockaddr_in addr;
	struct hostent *hp;
	int c, sock, win, port, off;
	char ch;
	char buf[1000];
	char *hostname;

	while ((c = getopt (argc, argv, "")) != EOF) {
		switch (c) {
		default:
			usage ();
		}
	}

	if (optind >= argc)
		usage ();

	hostname = argv[optind++];

	if (optind != argc)
		usage ();

	hp = gethostbyname (hostname); 
	if (hp == NULL) {
		fprintf (stderr, "%s not found\n", hostname);
		exit (1);
	}

	sock = socket (AF_INET, SOCK_STREAM, 0);
	win = 0;
	for (port = 9195; port <= 9200; port++) {
		memset (&addr, 0, sizeof addr);
		addr.sin_family = AF_INET;
		memcpy (&addr.sin_addr, hp->h_addr, sizeof addr.sin_addr);
		addr.sin_port = htons (port);
		if (connect (sock,
			     (struct sockaddr *) &addr, sizeof addr) >= 0) {
			win = 1;
			break;
		}

	}

	if ( ! win) {
		fprintf (stderr, "cannot connect to %s\n", hostname);
		exit (1);
	}

	off = 0;
	while (1) {
		if (read (sock, &ch, 1) != 1)
			break;

		buf[off++] = ch;
		if (off + 5 >= sizeof buf) {
			fprintf (stderr, "protocol error\n");
			exit (1);
		}

		if (ch == '\n') {
			buf[off-1] = 0;
			process (buf);
			off = 0;
		}
	}

	return (0);
}
