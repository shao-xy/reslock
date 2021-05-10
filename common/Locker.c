#include <stdio.h>
#include <unistd.h>

#include "Locker.h"

static int acquire(const char * name, int shared, int wait)
{
	fprintf(stdout, "Start to acquire %slock \"%s\" (%s wait).\n", shared ? "shared " : "", name, wait ? "Need" : "No");
	return 0;
}

static int release(const char * name, int shared, int wait)
{
	fprintf(stdout, "Start to release %slock \"%s\" (%s wait).\n", shared ? "shared " : "", name, wait ? "Need" : "No");
	return 0;
}

static int list(const char * name, int shared, int wait)
{
	fprintf(stdout, "Start to list %slock \"%s\" (%s wait).\n", shared ? "shared " : "", name, wait ? "Need" : "No");
	return 0;
}

static int clear(const char * name, int shared, int wait)
{
	fprintf(stdout, "Start to clear %slock \"%s\" (%s wait).\n", shared ? "shared " : "", name, wait ? "Need" : "No");
	return 0;
}

static int list_all()
{
	fprintf(stdout, "Start to list all locks.\n");
	return 0;
}

static const int (*func_table[])(const char*, int, int) = {
	acquire, release, list, clear
};

int handle_locks(Args * args)
{
	// no locknames?
	// List all locks if operation is list
	// Do nothing in other conditions
	if (args->locks == 0) {
		return args->op == LOCKOP_LIST ? list_all() : 0;
	}

	// we have locknames, choose corresponding function
	int (*func)(const char*, int, int) = func_table[(int)args->op - 1];
	int ret;
	for (int i = 0; i < args->locks; i++) {
		ret = func(args->locknames[i], args->shared, args->wait);
		if (ret)	return ret;
	}
	return 0;
}
