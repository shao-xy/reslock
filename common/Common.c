#include <string.h>

#include "Common.h"

static const char * lock_op_names[] = {
	"null",
	"acquire",
	"release",
	"list",
	"clear"
};

LockOp lockop_from_name(const char * s)
{
	if (!s)	return LOCKOP_NULL;
	for (int i = 1; i < (int) LOCKOP_MAX; i++) {
		if (!strcmp(s, lock_op_names[i])) {
			return (LockOp)i;
		}
	}
	return LOCKOP_NULL;
}

const char * lock_op_name(LockOp op)
{
	if ((int) op < 1 || (int) op >= LOCKOP_MAX)	return "null";
	return lock_op_names[(int)op];
}
