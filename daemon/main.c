#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "project_config.h"
#include "storage.h"

static void show_help(const char *progname);

int main(int argc, char *argv[])
{
	if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (argc == 2) {
		if (!strcmp(argv[1], "--version") || !strcmp(argv[1], "-v")) {
			puts(DAEMON_NAME " v" PROJECT_VERSION);
			exit(EXIT_SUCCESS);
		}
		if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")) {
			show_help(argv[0]);
			exit(EXIT_SUCCESS);
		}

		fprintf(stderr, "%s: unknown option '%s'\n", argv[0], argv[1]);

		fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);

		exit(EXIT_FAILURE);
	}

	int                fd;
	struct sockaddr_un addr;

	storage_init(TRAFFIC_PATH);

	unlink(SOCK_PATH);

	fd = socket(AF_UNIX, SOCK_STREAM, 0);

	memset(&addr, 0, sizeof(addr));

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, SOCK_PATH);

	bind(fd, (struct sockaddr *) &addr, sizeof(addr));

	listen(fd, 5);

	printf("daemon listening on %s\n", SOCK_PATH);

	for (;;) {
		int client = accept(fd, NULL, NULL);

		if (client < 0)
			continue;

		char buf[256] = { 0 };

		read(client, buf, sizeof(buf) - 1);

		if (strcmp(buf, "status") == 0) {
			write(client, "running\n", 8);

		} else if (strcmp(buf, "add") == 0) {
			TrafficRecord rec = { 0 };

			rec.timestamp = time(NULL);
			rec.rx_bytes  = rand() % 100000;
			rec.tx_bytes  = rand() % 50000;

			strcpy(rec.ip, "142.250.193.78");

			storage_append(TRAFFIC_PATH, &rec);

			write(client, "record added\n", 13);

		} else if (strcmp(buf, "list") == 0) {
			storage_dump(TRAFFIC_PATH, client);

		} else {
			write(client, "unknown command\n", 16);
		}

		close(client);
	}

	close(fd);

	return 0;
}


static void show_help(const char *progname)
{
	printf(
		"Usage: %s [OPTION...]\n"
		"%s"
		"\n"
		"Options:\n"
		"  -h, --help       Show this help page\n"
		"  -v, --version    Print program version\n"
		"\n"
		"Examples:\n"
		"  %s\n"
		"  %s --help\n"
		"  %s --version\n"
		"\n"
		"Report bugs to %s/issue\n",
		progname,
		project_args_descripation,
		progname,
		progname,
		progname,
		PROJECT_HOMEPAGE_URL
	);
}
