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
#include <sys/time.h>
#include <math.h>

#define TIMEOUT 50

double last_bell;

void
usage (void)
{
	fprintf (stderr, "usage: keyread hostname\n");
	exit (1);
}

double
get_secs (void)
{
        struct timeval tv;
	gettimeofday (&tv, NULL);
        return (tv.tv_sec + tv.tv_usec/1e6);
}

void
process (char *buf)
{
	int value, code;
	char cmd[1000];

	if (sscanf (buf, "kbd0 %d %d", &value, &code) == 2) {
		printf ("btn %d %s\n", code, value ? "pressed" : "released");
		switch (code) {
		/* case 10: */
		/* 	system ("notify-send 'doorbell!'"); */
		/* 	printf ("doorbell!\n"); */
		/* 	last_bell = now; */
		/* 	break; */
		/* case 106: */
		/* 	if (value == 1) { */
		/* 		system ("notify-send 'door opened!'"); */
		/* 	} else { */
		/* 		system ("notify-send 'door closed!'"); */
		/* 	} */

		/* 	break; */
		/* case 110: */
		/* 	if (value) */
		/* 		system ("cmus-remote -u"); */
		/* 	break; */
		/* default: */
		/* 	break; */
		case 73:			
			sprintf (cmd, "echo key %d was %s on `date` >> %d.log",
				 code, value ? "pressed" : "released", code);
			system (cmd);
			break;
		case 83:
			sprintf (cmd, "echo key %d was %s on `date` >> %d.log",
				 code, value ? "pressed" : "released", code);
			system (cmd);
			break;
		case 25:
			sprintf (cmd, "echo key %d was %s on `date` >> %d.log",
				 code, value ? "pressed" : "released", code);
			system (cmd);
			break;
		case 50:
			sprintf (cmd, "echo key %d was %s on `date` >> %d.log",
				 code, value ? "pressed" : "released", code);
			system (cmd);
			break;
		case 9:
			sprintf (cmd, "echo key %d was %s on `date` >> %d.log",
				 code, value ? "pressed" : "released", code);
			system (cmd);
			break;
		default:
			break;
		}
	}
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

	last_bell = get_secs ();

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
