#ifndef __COMMON_COMMON_H__
#define __COMMON_COMMON_H__

typedef enum _tLockOp {
	LOCKOP_NULL = 0,
	LOCKOP_ACQUIRE,
	LOCKOP_RELEASE,
	LOCKOP_LIST,
	LOCKOP_CLEAR,
	LOCKOP_MAX
} LockOp;

LockOp lockop_from_name(const char * s);
const char * lock_op_name(LockOp op);

#endif /* common/Common.h */
