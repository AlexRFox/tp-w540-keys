/*
 *
 *  Copyright (c) 2013 Alex Willisson
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

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
int buttons[12];
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

int
fetch_keycode (char *s)
{
	KeyCode code;
	KeySym sym;

	sym = XStringToKeysym (s);

	if (sym == NoSymbol) {
		fprintf (stderr, "Unable to resolve keysym for '%s'\n", s);
		exit (1);
	}

	code = XKeysymToKeycode (dpy, sym);

	return (code);
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

		if (ev.value == 1) {
			XTestFakeKeyEvent (dpy, buttons[ev.code-2],
					   True, CurrentTime);
		} else if (ev.value == 0) {
			XTestFakeKeyEvent (dpy, buttons[ev.code-2],
					   False, CurrentTime);
		}

		XFlush (dpy);
	}
}

int
main (int argc, char **argv)
{
	struct keyboard *kp;
	int c, maxfd, idx, kid, code;
	char *s, *conf, line[1000], *p;
	fd_set rset, wset;
	FILE *fp;

	dpy = XOpenDisplay(NULL);
	conf = "btnmap";

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

	fp = fopen (conf, "r");

	for (idx = 0; idx < 12; idx++) {
		fgets (line, sizeof line, fp);

		if (line == NULL) {
			fprintf (stderr, "invalid config file\n");
			return (1);
		}

		p = line;

		while (*p && *p != '=')
			p++;

		if (*p == 0)
			fprintf (stderr, "invalid line in config file\n");

		*p = 0;
		p++;

		s = p;

		while (*p && *p != '\n')
			p++;

		*p = 0;

		if (strcmp (line, "code") == 0) {
			code = atoi (s);
		} else if (strcmp (line, "sym") == 0) {
			code = fetch_keycode (s);
		} else {
			fprintf (stderr, "invalid config file\n");
			return (1);
		}

		buttons[idx] = code;
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
