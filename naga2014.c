#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/input.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

struct keyboard {
	struct keyboard *next;
	int fd, num;
	char *name;
};

struct keyboard *kbd_head;
Display *dpy;

void
usage (void)
{
	fprintf (stderr, "usage: naga2014 KBD...\n");
	exit (1);
}

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

char *
xstrdup (const char *old)
{
	char *new;

	if ((new = strdup (old)) == NULL) {
		fprintf (stderr, "out of memory\n");
		exit (1);
	}

	return (new);
}

void
valgrind_cleanup (void)
{
	exit (0);
}

void
make_kbd (char *kbd, int kid)
{
	struct keyboard *kp;

	kp = xcalloc (1, sizeof *kp);

	kp->name = xstrdup (kbd);
	kp->num = kid;

	if ((kp->fd = open (kp->name, O_RDONLY)) < 0) {
		fprintf (stderr, "cannot open keyboard: %s\n", kp->name);
		exit (1);
	}

	fcntl (kp->fd, F_SETFL, O_NONBLOCK);

	if (!kbd_head) {
		kbd_head = kp;
	} else {
		kp->next = kbd_head;
		kbd_head = kp;
	}
}

void
handle_input (struct keyboard *kp)
{
	struct input_event ev;
	char buf[1000];
	int n;

	while (1) {
		n = read (kp->fd, &ev, sizeof ev);
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

		if (ev.value > 1 || ev.code == 0) {
			continue;
		}

		sprintf (buf, "kbd%d: %d %d\n", kp->num, ev.value, ev.code);
		printf ("%s", buf);

		if (ev.code == 2 && ev.value == 1) {
			printf ("fake down\n");
			XTestFakeKeyEvent (dpy, 167, True, CurrentTime);
		} else if (ev.code == 2 && ev.value == 0) {
			printf ("fake up\n");
			XTestFakeKeyEvent (dpy, 167, False, CurrentTime);
		}

		XFlush (dpy);
	}
}

int
main (int argc, char **argv)
{
	struct keyboard *kp;
	int c, maxfd, idx, kid;
	char *s;
	fd_set rset, wset;

	dpy = XOpenDisplay(NULL);

	while ((c = getopt (argc, argv, "")) != EOF) {
		switch (c) {
		default:
			usage ();
		}
	}

	if (optind >= argc)
		usage ();

	if (optind == argc)
		usage ();

	kid = 0;
	for (idx = optind; idx < argc; idx++) {
		s = xstrdup (argv[idx]);

		make_kbd (s, kid++);

		free (s);
	}

	while (1) {
		maxfd = 0;

		FD_ZERO (&rset);
		FD_ZERO (&wset);

		for (kp = kbd_head; kp; kp = kp->next) {
			if (kp->fd > maxfd)
				maxfd = kp->fd;
			FD_SET (kp->fd, &rset);
		}

		if (select (maxfd + 1, &rset, &wset, NULL, NULL) < 0) {
			fprintf (stderr, "select error\n");
			exit (1);
		}

		for (kp = kbd_head; kp; kp = kp->next) {
			if (FD_ISSET (kp->fd, &rset))
				handle_input (kp);
		}
	}

	return (0);
}
