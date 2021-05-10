#ifndef _COMMON_ARGS_H_
#define _COMMON_ARGS_H_

#include "Common.h"

#define LOCKS_NUM_MAX 10

typedef struct _tArgs {
	LockOp op;
	const char * locknames[LOCKS_NUM_MAX];
	int locks;
	int shared;
	int wait;
} Args;

int parse_args(Args * args, int argc, char * argv[]);

void show_config(Args * args);

#endif /* common/Args.h */
