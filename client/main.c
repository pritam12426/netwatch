#include <argtable3.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "log.h"
#include "project_config.h"


static struct {
	struct arg_lit *help;
	struct arg_lit *version;
	struct arg_lit *verb;
	struct arg_str *level;
	struct arg_end *end;

	Log_level_t log_level;
} G_Arguments = {
	.log_level = LOG_LEVEL_INFO,
};


static void print_global_help(const char *progname, void **argtable)
{
	fprintf(stderr, "Usage: %s", progname);
	arg_print_syntax(stderr, argtable, " ");
	fputs("<command> [args]\n", stderr);
	fputs(project_args_descripation, stderr);
	fprintf(stderr, "\nCommands:\n");
	fprintf(stderr, "  %-25s %s\n", "reset", "Reset stored statistics");
	fprintf(stderr, "  %-25s %s\n", "today", "Show today's traffic usage");
	fprintf(stderr, "  %-25s %s\n", "live", "Show real-time traffic monitor");
	fprintf(stderr, "  %-25s %s\n", "top", "Show top traffic consumers");
	fprintf(stderr, "  %-25s %s\n", "month", "Show current month's traffic usage");
	fprintf(stderr, "  %-25s %s\n", "status", "Show daemon status");
	fprintf(stderr, "\nOptions:\n");
	arg_print_glossary(stderr, argtable, "  %-25s %s\n");
	fputs("\nReport bugs to " PROJECT_HOMEPAGE_URL "/issue\n", stderr);
}


static bool process_log_level(const char *log_level, struct arg_end *end)
{
	if (strcmp(log_level, "error")      == 0)  log_set_level(LOG_LEVEL_ERROR);
	else if (strcmp(log_level, "warn")  == 0)  log_set_level(LOG_LEVEL_WARN);
	else if (strcmp(log_level, "info")  == 0)  log_set_level(LOG_LEVEL_INFO);
	else if (strcmp(log_level, "debug") == 0)  log_set_level(LOG_LEVEL_DEBUG);
	else {
		fprintf(stderr, "%s: invalid log level '%s' (expected error|warn|info|debug)\n", CLIENT_NAME, log_level);
		return false;
	}
	G_Arguments.log_level = log_get_level();
	return true;
}


static int find_subcmd_idx(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] != '-') return i;

		if ((strcmp(argv[i], "-L") == 0 || strcmp(argv[i], "--log-level") == 0) && i + 1 < argc
		    && argv[i + 1][0] != '-') {
			i++;
		}
	}
	return -1;
}


static int sub_cmd_reset(int argc, char *argv[], arg_dstr_t res, void *ctx)
{
	struct arg_lit *help       = arg_litn("h", "help", 0, 1, "Display this help");
	struct arg_end *end        = arg_end(20);
	void           *argtable[] = { help, end };

	int nerrors = arg_parse(argc, argv, argtable);
	if (help->count > 0) {
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		arg_dstr_catf(res, "Usage: %s reset\n  Reset stored statistics.\n", CLIENT_NAME);
		goto done;
	}
	if (nerrors > 0) {
		arg_print_errors_ds(res, end, argv[0]);
		goto done;
	}

	// TODO: actual reset logic

	arg_dstr_cat(res, "Statistics reset.\n");

done:
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}


static int sub_cmd_today(int argc, char *argv[], arg_dstr_t res, void *ctx)
{
	struct arg_lit *help       = arg_litn("h", "help", 0, 1, "Display this help");
	struct arg_int *limit      = arg_intn("n", "limit", "COUNT", 0, 1, "Limit number of displayed records");
	struct arg_end *end        = arg_end(20);
	void           *argtable[] = { help, limit, end };

	int nerrors = arg_parse(argc, argv, argtable);
	if (help->count > 0) {
		arg_dstr_catf(res, "Usage: %s today [-n COUNT]\n  Show today's traffic usage.\n", CLIENT_NAME);
		arg_print_glossary_ds(res, argtable, "  %-25s %s\n");
		goto done;
	}
	if (nerrors > 0) {
		arg_print_errors_ds(res, end, argv[0]);
		goto done;
	}
	int n = (limit->count > 0) ? limit->ival[0] : -1;
	printf("%d ", G_Arguments.log_level);

	// TODO: actual today logic

	arg_dstr_catf(res, "Today's traffic (limit=%d).\n", n);

done:
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}


static int sub_cmd_live(int argc, char *argv[], arg_dstr_t res, void *ctx)
{
	struct arg_lit *help       = arg_litn("h", "help", 0, 1, "Display this help");
	struct arg_end *end        = arg_end(20);
	void           *argtable[] = { help, end };

	int nerrors = arg_parse(argc, argv, argtable);
	if (help->count > 0) {
		arg_dstr_catf(res, "Usage: %s live\n  Show real-time traffic monitor.\n", CLIENT_NAME);
		goto done;
	}
	if (nerrors > 0) {
		arg_print_errors_ds(res, end, argv[0]);
		goto done;
	}

	// TODO: actual live monitor logic

	arg_dstr_cat(res, "Starting live monitor...\n");

done:
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}


static int sub_cmd_top(int argc, char *argv[], arg_dstr_t res, void *ctx)
{
	struct arg_lit *help = arg_litn("h", "help", 0, 1, "Display this help");
	struct arg_int *limit
	    = arg_intn("n", "limit", "COUNT", 0, 1, "Limit number of displayed records");
	struct arg_end *end        = arg_end(20);
	void           *argtable[] = { help, limit, end };

	int nerrors = arg_parse(argc, argv, argtable);
	if (help->count > 0) {
		arg_dstr_catf(res, "Usage: %s top [-n COUNT]\n  Show top traffic consumers.\n", CLIENT_NAME);
		arg_print_glossary_ds(res, argtable, "  %-25s %s\n");
		goto done;
	}
	if (nerrors > 0) {
		arg_print_errors_ds(res, end, argv[0]);
		goto done;
	}
	int n = (limit->count > 0) ? limit->ival[0] : -1;

	// TODO: actual top logic

	arg_dstr_catf(res, "Top traffic consumers (limit=%d).\n", n);

done:
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}


static int sub_cmd_month(int argc, char *argv[], arg_dstr_t res, void *ctx)
{
	struct arg_lit *help       = arg_litn("h", "help", 0, 1, "Display this help");
	struct arg_int *limit      = arg_intn("n", "limit", "COUNT", 0, 1, "Limit number of displayed records");
	struct arg_end *end        = arg_end(20);
	void           *argtable[] = { help, limit, end };

	int nerrors = arg_parse(argc, argv, argtable);
	if (help->count > 0) {
		arg_dstr_catf(res, "Usage: %s month [-n COUNT]\n  Show current month's traffic usage.\n", CLIENT_NAME);
		arg_print_glossary_ds(res, argtable, "  %-25s %s\n");
		goto done;
	}
	if (nerrors > 0) {
		arg_print_errors_ds(res, end, argv[0]);
		goto done;
	}
	int n = (limit->count > 0) ? limit->ival[0] : -1;

	// TODO: actual month logic

	arg_dstr_catf(res, "This month's traffic (limit=%d).\n", n);

done:
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}


static int sub_cmd_status(int argc, char *argv[], arg_dstr_t res, void *ctx)
{
	struct arg_lit *help       = arg_litn("h", "help", 0, 1, "Display this help");
	struct arg_end *end        = arg_end(20);
	void           *argtable[] = { help, end };

	int nerrors = arg_parse(argc, argv, argtable);
	if (help->count > 0) {
		arg_dstr_catf(res, "Usage: %s status\n  Show daemon status.\n", CLIENT_NAME);
		goto done;
	}
	if (nerrors > 0) {
		arg_print_errors_ds(res, end, argv[0]);
		goto done;
	}

	// TODO: actual status logic

	arg_dstr_cat(res, "Daemon is running.\n");

done:
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}


int main(int argc, char *argv[])
{
	const char progname[]      = CLIENT_NAME;
	int        exitcode        = 0;
	bool       cmd_initialized = false;

	void *argtable[] = {
		G_Arguments.help    = arg_litn("h", "help",      0,          1, "Display this help and exit"),
		G_Arguments.version = arg_litn("v", "version",   0,          1, "Display version info and exit"),
		G_Arguments.verb    = arg_litn("V", "verbose",   0,          1, "Verbose output"),
		G_Arguments.level   = arg_strn("L", "log-level", "LEVEL", 0, 1, "Set log level (error|warn|info|debug)"),
		G_Arguments.end     = arg_end(20),
	};
	;

	/* ── 1. No arguments at all ──────────────────────────────────────── */
	if (argc < 2) {
		print_global_help(progname, argtable);
		exitcode = 1;
		goto exit;
	}

	/* ── 2. Find where the sub-command starts ────────────────────────── */
	int subcmd_idx = find_subcmd_idx(argc, argv);

	/* ── 3. Parse global flags (everything before the sub-command) ───── */
	{
		/* arg_parse needs argc to stop at subcmd_idx so it doesn't try
           to interpret the sub-command name or its own options           */
		int global_argc = (subcmd_idx == -1) ? argc : subcmd_idx;
		int nerrors     = arg_parse(global_argc, argv, argtable);

		/* --help and --version are valid without a sub-command */
		if (G_Arguments.help->count > 0) {
			print_global_help(progname, argtable);
			goto exit;
		}
		if (G_Arguments.version->count > 0) {
			fputs(CLIENT_NAME " v" PROJECT_VERSION "\n", stderr);
			goto exit;
		}
		if (nerrors > 0) {
			arg_print_errors(stderr, G_Arguments.end, progname);
			fprintf(stderr, "Try '%s --help' for more information.\n", progname);
			exitcode = 1;
			goto exit;
		}
	}

	/* ── 4. Apply log level so sub-commands inherit it ───────────────── */
	if (G_Arguments.level->count > 0)
		if (process_log_level(G_Arguments.level->sval[0], G_Arguments.end) != true) {
			goto exit;
		}


	/* ── 5. Must have a sub-command at this point ────────────────────── */
	if (subcmd_idx == -1) {
		print_global_help(progname, argtable);
		exitcode = 1;
		goto exit;
	}

	/* ── 6. Register and dispatch sub-commands ───────────────────────── */
	arg_cmd_init();
	cmd_initialized = true;

	arg_cmd_register("reset",  sub_cmd_reset,  "Reset stored statistics",            NULL);
	arg_cmd_register("today",  sub_cmd_today,  "Show today's traffic usage",         NULL);
	arg_cmd_register("live",   sub_cmd_live,   "Show real-time traffic monitor",     NULL);
	arg_cmd_register("top",    sub_cmd_top,    "Show top traffic consumers",         NULL);
	arg_cmd_register("month",  sub_cmd_month,  "Show current month's traffic usage", NULL);
	arg_cmd_register("status", sub_cmd_status, "Show daemon status",                 NULL);

	{
		// Guard: check the sub-command exists before dispatching
		const char *subcmd_name = argv[subcmd_idx];
		if (arg_cmd_info(subcmd_name) == NULL) {
			fprintf(stderr, "%s: unknown command '%s'. Try '%s --help'.\n", progname, subcmd_name, progname);
			exitcode = 1;
			goto exit;
		}

		arg_dstr_t res = arg_dstr_create();
		int        ret = arg_cmd_dispatch(argv[subcmd_idx], argc - subcmd_idx, argv + subcmd_idx, res);

		/* Always flush sub-command output */
		const char *out = arg_dstr_cstr(res);
		if (out && out[0] != '\0') fputs(out, stdout);

		arg_dstr_destroy(res);

		if (ret != 0) {
			fprintf(stderr, "Unknown command '%s'. Try '%s --help'.\n", argv[subcmd_idx], progname);
			exitcode = 1;
		}
	}

	// TODO: Add the main logic here

exit:
	if (cmd_initialized) arg_cmd_uninit();
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return exitcode;
}
