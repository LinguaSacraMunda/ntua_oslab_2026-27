#include <signal.h>
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
#define C4

/*
 * O_RDONLY: 0
 * O_WRONLY: 1
 * O_RDWR: 2
 * O_CREAT: 64
 * O_TRUNC: 512
 */

int main(int argc, char **argv) {
#ifdef FORK 
#if defined(C0) || defined(C1)
    int fd_c0;
    err_chk((fd_c0 = open("./.hello_there", O_RDONLY | O_CREAT | O_TRUNC, 0444)), "open C0");
    err_chk(close(fd), "close C0");

#endif

#ifdef C3
    setenv("ANSWER", "42", 1);
#endif

#ifdef C4
    int fd_c4;
    err_chk((fd_c4 = mkfifo("./magic_mirror", 0666)), "mkfifo C4");
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


#ifdef C4
    err_chk(close(fd_c4), "close C4");
#endif
#ifdef C2
    // Make sure the riddle is running and has set up the alarm
    sleep(1);
    printf("sending SIGCONT signal to %d\n", p);
    kill(p, SIGCONT);
#endif


#ifdef FORK
	int wstatus;
	waitpid(p, &wstatus, 0);
#endif
	return 0;
}
