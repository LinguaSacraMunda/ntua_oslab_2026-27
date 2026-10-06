#define _GNU_SOURCE
#include <err.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#define die(str) {fprintf(stderr, "%s\n", (str)); exit(EXIT_FAILURE);}
#define err_chk(func, err_str) {if ((func) == -1) die((err_str))};

#define FORK
#define C0

/*
 * O_RDONLY: 0
 * O_WRONLY: 1
 * O_RDWR: 2
 * O_CREAT: 2
 * O_TRUNC: 512
 */

int main(int argc, char **argv) {
#ifdef FORK 
#if defined(C0) || defined(C1)
    char *msg = "hello there :D";

    int fd;
    err_chk((fd = open("./.hello_there", O_WRONLY | O_CREAT | O_TRUNC, 0444)), "open");
    err_chk(write(fd, msg, strlen(msg)), "write");
    err_chk(close(fd), "close");

#endif

	pid_t p;
	if ((p = fork()) == -1) {
		perror("fork");
		exit(1);
	}

	if (p == 0) {
		char *const args[] = {NULL};
		execv("./riddle", args);
	}
#endif


#ifdef FORK
	int wstatus;
	waitpid(p, &wstatus, 0);
#endif
	return 0;
}
