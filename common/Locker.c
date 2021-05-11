#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#include "Locker.h"

#define LOCKFILE_PATHLEN_MAX 256
#define READBUF 4096
#define HOLDLOCKMAX 10

static const char * TC_RED = "\033[1;31m";
static const char * TC_GREEN = "\033[1;32m";
static const char * TC_NULL = "\033[0m";

static int clear(const char * name, int shared, int wait)
{
	fprintf(stdout, "Start to clear %slock \"%s\" (%s wait).\n", shared ? "shared " : "", name, wait ? "Need" : "No");
	return 0;
}

static int check_lock_shared(int fd)
{
	struct stat buf;
	fstat(fd, &buf);
	return S_ISLNK(buf.st_mode);
}

static int read_locks_from(int fd, int lockedby[], int ppid)
{
	char buf[READBUF] = {0};
	ssize_t readsize;
	int soff, eoff, readoff = 0, arrayind = 0, lockedbyme = 0;
	do {
		readsize = read(fd, buf + readoff, READBUF - readoff);
		soff = eoff = 0;
		while (1) {
			while (eoff < READBUF + 1 && buf[eoff] != '\n' && buf[eoff] != '\0')	eoff++;
			if (eoff == READBUF + 1)	break;
			if (buf[eoff] == '\0') {
				int pid = atoi(buf + soff);
				lockedby[arrayind] = pid;
				lockedbyme = lockedbyme || (pid == ppid);
				return lockedbyme;
			}
			buf[eoff] = '\0';
			int pid = atoi(buf + soff);
			lockedby[arrayind++] = pid;
			lockedbyme = lockedbyme || (pid == ppid);
			if (arrayind == HOLDLOCKMAX)	return lockedbyme;
			
			soff = eoff + 1;
			eoff = soff;
		}
		readoff = eoff - soff;
		memcpy(buf, buf + soff, readoff);
	} while (readsize == READBUF);
	return lockedbyme;
}

static int acquire(const char * name, int shared, int wait)
{
	fprintf(stdout, "Start to acquire %slock \"%s\" (%s wait).\n", shared ? "shared " : "", name, wait ? "Need" : "No");

	// Make sure directory exists
	if (mkdir("/tmp/reslock", S_IRWXU) < 0 && errno != EEXIST) {
			printf("Fatal: Create temp lock directory failed: %s\n", strerror(errno));
			return errno;
	}

	pid_t ppid = getppid();
	char lockfile[LOCKFILE_PATHLEN_MAX];
	snprintf(lockfile, LOCKFILE_PATHLEN_MAX, "/tmp/reslock/%s.lock", name);

	if (shared) {
		char sharedlockfile[LOCKFILE_PATHLEN_MAX];
		snprintf(sharedlockfile, LOCKFILE_PATHLEN_MAX, "/tmp/reslock/%s.sharedlock", name);
		
	} else {
		int failcnt = 0;
		while (1) {
			int lockfd = open(lockfile, O_EXCL | O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
			if (lockfd < 0) {
				if (errno == EEXIST) {
					// already locked by another process
					// open again: read which process holds the lock
					lockfd = open(lockfile, O_RDONLY);
					// exlusively locks the file to read failed
					if (lockfd < 0 || flock(lockfd, LOCK_EX) < 0) {
						fprintf(stderr, "Fatal: No permission to process lock for %s: %s\n", name, strerror(errno));
						return errno;
					}
					int lockedby[HOLDLOCKMAX] = {0};
					int lockedbyme = read_locks_from(lockfd, lockedby, ppid);
					int shared = check_lock_shared(lockfd);
					flock(lockfd, LOCK_UN);
					close(lockfd);
	
					if (lockedbyme) {
						fprintf(stderr, "Already locked by this process.\n");
						return 0;
					}
						
					fprintf(stderr, "%sAlready %slocked by process%s", wait ? "\r" : "", shared ? "SHARED " : "", shared ? "(es)" : "");
					int alldead = 1;
					for (int i = 0; lockedby[i]; i++) {
						int pid = lockedby[i];
						if (kill(pid, 0) < 0) {
							// process with this pid is not running
							fprintf(stderr, " %s%d(D)%s", TC_RED, pid, TC_NULL);
						} else {
							alldead = 0;
							fprintf(stderr, " %s%d(R)%s", TC_GREEN, pid, TC_NULL);
						}
					}
					
					if (alldead && isatty(STDIN_FILENO)) {
						fprintf(stderr, "\nAll processes locking is dead. clear it and continue? (Auto cancel in 30 seconds) (y/N): ");
						fd_set fdset;
						struct timeval timeout = {30, 0};
						FD_ZERO(&fdset);
						FD_SET(STDIN_FILENO, &fdset);
						if (select(STDIN_FILENO + 1, &fdset, NULL, NULL, &timeout) <= 0) return -1;
						char * line = NULL;
						size_t size;
						getline(&line, &size, stdin);
						if (!strcmp(line, "y\n") || !strcmp(line, "Y\n")) {
							clear(name, shared, wait);
							return acquire(name, shared, wait);
						}
						fprintf(stderr, "\n");
						return -1;
					}

					if (wait) {
						fprintf(stderr, ". Waiting ... (already waited for %d seconds)", failcnt);
						failcnt++;
						sleep(1);
					} else {
						fprintf(stderr, "\n");
						return -1;
					}
				} else {
					// create lock failed.
					printf("Fatal: Could not create lock for %s: %s\n", name, strerror(errno));
					return errno;
				}
			} else {
				// acquire lock success (Non-shared)
				char pidstr[10];
				snprintf(pidstr, 10, "%d", ppid);
				write(lockfd, pidstr, strlen(pidstr));
				close(lockfd);
				break;
			}
		}
	}
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
