#include <stdio.h>

#include "common/Locker.h"

int main(int argc, char * argv[])
{
	int ret;
	Args args;
	if ((ret = parse_args(&args, argc, argv))) return ret;

	show_config(&args);

	return handle_locks(&args);
}
