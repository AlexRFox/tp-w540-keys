#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/input.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define RB_SIZE 1000

int listen_fd, input_fd;

struct client {
	struct client *next;
	struct client *prev;
	int fd;
	struct sockaddr_in addr;
	char buf[RB_SIZE];
	int in, out, kill;
};

struct client client_head;

void
usage (void)
{
	fprintf (stderr, "usage: rasp\n");
	exit (1);
}

void
cleanup_clients (void)
{
	struct client *cp, *ncp;

	for (cp = client_head.next, ncp = cp->next;
	     cp != &client_head;
	     cp = ncp, ncp = cp->next) {
		if (cp->kill) {
			fprintf (stderr, "killing client\n");
			close (cp->fd);
			cp->prev->next = cp->next;
			cp->next->prev = cp->prev;
			free (cp);
		}
	}
}

void
make_client (void)
{
	int fd;
	socklen_t socklen;
	struct client *cp;
	struct sockaddr_in addr;

	fd = accept (listen_fd, (struct sockaddr *) &addr, &socklen);
	if (fd < 0) {
		fprintf (stderr, "strange accept error: %s\n", strerror (errno));
		return;
	}

	cp = calloc (1, sizeof *cp);

	cp->fd = fd;
	cp->addr = addr;
	socklen = sizeof cp->addr;
	fcntl (cp->fd, F_SETFL, O_NONBLOCK);
	cp->next = &client_head;
	cp->prev = client_head.prev;
	client_head.prev->next = cp;
	client_head.prev = cp;

	printf ("connection from %s:%d fd %d\n",
		inet_ntoa (cp->addr.sin_addr), ntohs (cp->addr.sin_port),
		cp->fd);
}

void
handle_client (struct client *cp)
{
	int thistime, n;
	char inbuf[1000];

	while (1) {
		n = read (cp->fd, inbuf, sizeof inbuf);
		if (n < 0) {
			if (errno == EAGAIN)
				break;

			fprintf (stderr, "strange read error: %s\n",
				 strerror (errno));
			cp->kill = 1;
			return;
		}

		if (n == 0) {
			fprintf (stderr, "EOF from client\n");
			cp->kill = 1;
			return;
		}
	}

	while (cp->in != cp->out) {
		if (cp->out < cp->in) {
			thistime = cp->in - cp->out;
		} else {
			thistime = RB_SIZE - cp->out;
		}

		n = write (cp->fd, cp->buf + cp->out, thistime);

		if (n < 0) {
			cp->kill = 1;
			break;
		}

		if (n == 0)
			break;

		cp->out = (cp->out + n) % RB_SIZE;
	}
}

void
handle_input (void)
{
	struct input_event ev;
	struct client *cp;
	char buf[1000];
	char *p;
	int n, len, used, togo, thistime;

	while (1) {
		n = read (input_fd, &ev, sizeof ev);
		if (n < 0 && errno != EAGAIN) {
			fprintf (stderr, "input error: %s\n", strerror (errno));
			exit (1);
		}

		if (n <= 0)
			return;

		if (n < sizeof ev) {
			fprintf (stderr, "unexpected input buffer size\n");
			exit (1);
		}

		sprintf (buf, "value %d code %d\n", ev.value, ev.code);
		printf ("%s", buf);
		len = strlen (buf);
		for (cp = client_head.next; cp != &client_head; cp = cp->next) {
			used = (cp->out + RB_SIZE - cp->in) % RB_SIZE;
			if (used + len >= RB_SIZE - 1)
				continue;

			togo = len;
			p = buf;

			while (togo > 0) {
				if (cp->in < cp->out) {
					thistime = cp->out - cp->in;
				} else {
					thistime = RB_SIZE - cp->in;
				}

				if (thistime > togo)
					thistime = togo;

				memcpy (cp->buf + cp->in, p, thistime);
				cp->in = (cp->in + thistime) % RB_SIZE;
				p += thistime;
				togo -= thistime;
			}
		}
	}
}

int
main (int argc, char **argv)
{
	struct sockaddr_in addr;
	struct client *cp;
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

	client_head.next = &client_head;
	client_head.prev = &client_head;

	if ((input_fd = open ("/dev/input/event3", O_RDONLY)) < 0) {
		fprintf (stderr, "cannot open keyboard\n");
		exit (1);
	}

	fcntl (input_fd, F_SETFL, O_NONBLOCK);

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
		cleanup_clients ();

		maxfd = 0;

		FD_ZERO (&rset);
		FD_ZERO (&wset);

		if (listen_fd > maxfd)
			maxfd = listen_fd;
		FD_SET (listen_fd, &rset);

		if (input_fd > maxfd)
			maxfd = input_fd;
		FD_SET (input_fd, &rset);

		for (cp = client_head.next; cp != &client_head; cp = cp->next) {
			if (cp->in != cp->out) {
				if (cp->fd > maxfd)
					maxfd = cp->fd;
				FD_SET (cp->fd, &wset);
				FD_SET (cp->fd, &rset);
			}
		}

		if (select (maxfd + 1, &rset, &wset, NULL, NULL) < 0) {
			fprintf (stderr, "select error\n");
			exit (1);
		}

		if (FD_ISSET (listen_fd, &rset))
			make_client ();
		if (FD_ISSET (input_fd, &rset))
			handle_input ();

		for (cp = client_head.next; cp != &client_head; cp = cp->next) {
			handle_client (cp);
		}
	}

	return (0);
}
