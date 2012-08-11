#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int listen_fd;

struct client {
	int fd;
	struct sockaddr_in addr;
};

void
usage (void)
{
	fprintf (stderr, "usage: rasp\n");
	exit (1);
}

void
make_client (void)
{
	socklen_t socklen;
	struct client *cp;

	cp = calloc (1, sizeof *cp);

	socklen = sizeof cp->addr;
	cp->fd = accept (listen_fd, (struct sockaddr *) &cp->addr, &socklen);
	
	printf ("connection from %s:%d fd %d\n",
		inet_ntoa (cp->addr.sin_addr), ntohs (cp->addr.sin_port),
		cp->fd);
}

int
main (int argc, char **argv)
{
	struct sockaddr_in addr;
	socklen_t addrlen;
	int c, port, maxfd;
	FILE *f;
	fd_set rset, wset;

	while ((c = getopt (argc, argv, "")) != EOF) {
		switch (c) {
		default:
			usage ();
		}
	}

	if (optind != argc)
		usage ();

	listen_fd = socket (AF_INET, SOCK_STREAM, 0);
	listen (listen_fd, 5);
	addrlen = sizeof addr;
	getsockname (listen_fd, (struct sockaddr *) &addr, &addrlen);
	port = ntohs (addr.sin_port);
	printf ("listening on %d\n", port);

	f = fopen ("PORT", "w");
	fprintf (f, "%d\n", port);
	fclose (f);

	while (1) {
		maxfd = 0;

		FD_ZERO (&rset);
		FD_ZERO (&wset);

		if (listen_fd > maxfd)
			maxfd = listen_fd;

		FD_SET (listen_fd, &rset);

		if (select (maxfd + 1, &rset, &wset, NULL, NULL) < 0) {
			fprintf (stderr, "select error\n");
			exit (1);
		}

		if (FD_ISSET (listen_fd, &rset))
			make_client ();
	}

	return (0);
}
