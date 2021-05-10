// vim: ts=2 sw=2 smarttab
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Args.h"

void usage(const char * exename)
{
	fprintf(stdout, "Usage: %s [<options>] <operation> <lockname> [<more locknames>]\n\n", exename);
	fprintf(stdout, "    Operations: (Case sensitive)\n");
	fprintf(stdout, "      acquire    Acquire locks for lockname(s).\n");
	fprintf(stdout, "      release    Release locks for lockname(s).\n");
	fprintf(stdout, "      list       List locks for lockname(s).\n");
	fprintf(stdout, "                 If no lock names are given, list all locks.\n");
	fprintf(stdout, "      clear*     FORCE clear locks for lockname(s). Use with caution.\n");
	fprintf(stdout, "\n");
	fprintf(stdout, "    Options:\n");
	fprintf(stdout, "      -h         Show this message and exit.\n");
	fprintf(stdout, "      -s         Use shared locks instead of exclusive locks.\n");
	fprintf(stdout, "      -w         Wait for the locks if some of them are already locked.\n");
	fprintf(stdout, "\n");
}

int parse_args(Args * args, int argc, char * argv[])
{
	int opt;
	args->op = LOCKOP_NULL;
	args->locks = 0;
	args->shared = 0;
	args->wait = 0;
	while ((opt = getopt(argc, argv, "hws")) != -1) {
		switch (opt) {
			case 'h':	usage(argv[0]);	exit(0);
			case 'w':	args->wait = 1;	break;
			case 's': args->shared = 1;	break;
			default:
				fprintf(stderr, "Unknown option: %c\n", optopt);
				return -1;
		}
	}

	if (optind == argc) {
		usage(argv[0]);
		exit(0);
	}

	// positional arguments
	const char * arg = argv[optind];
	// 1. The operation
	// Operation is illegal
	args->op = lockop_from_name(arg);
	if (args->op == LOCKOP_NULL) {
		fprintf(stderr, "Unknown operation: %s\n\n", arg);
		usage(argv[0]);
		return -1;
	}
	optind++;

	// 2. Locknames
	while (optind < argc) {
		arg = argv[optind];

		// Too many locknames?
		if (args->locks == LOCKS_NUM_MAX) {
			fprintf(stderr, "Fatal: Too many locknames.\nMaximum number of locks supported: %d\n", LOCKS_NUM_MAX);
			return -2;
		}
		args->locknames[args->locks++] = arg;
	
		// increase optind by ourselves
		optind++;
	}
	return 0;
}

void show_config(Args * args)
{
	if (!args)	return;
	fprintf(stdout, "===== Configuration =====\n");
	fprintf(stdout, "Op:     %s\n", lock_op_name(args->op));
	fprintf(stdout, "Locks:  ");
	for (int i = 0; i < args->locks; i++) {
		fprintf(stdout, "%s ", args->locknames[i]);
	}
	fprintf(stdout, "\n");
	fprintf(stdout, "Shared: %s\n", args->shared ? "yes": "no");
	fprintf(stdout, "Wait:   %s\n", args->shared ? "yes": "no");
	fprintf(stdout, "=== Configuration End ===\n");
}

