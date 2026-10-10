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
#define C7

/*
 * O_RDONLY: 0
 * O_WRONLY: 1
 * O_RDWR: 2
 * O_CREAT: 64
 * O_TRUNC: 512
 */

int main(int argc, char **argv) {
#ifdef FORK 
#if defined(C0) || defined(C1) || defined(C7)
    int fd_c0;
    if (access("./hello_there", F_OK) == 0) {
        err_chk((fd_c0 = open("./.hello_there", O_RDONLY | O_CREAT | O_TRUNC, 0444)), "open C0");
    }
#endif
#ifdef C7
    err_chk(link("./.hello_there", "./.hey_there"), "link C7");
#endif
#ifdef C3
    setenv("ANSWER", "42", 1);
#endif

#ifdef C4
    int fd_c4;
    err_chk((fd_c4 = mkfifo("./magic_mirror", 0666)), "mkfifo C4");
#endif

#ifdef C5
    int fd_c5;
    err_chk((fd_c5 = open("c5", O_CREAT | O_RDWR | O_TRUNC, 0777)), "open C5");

    // Switch the file descriptor of c5 to 99
    err_chk((dup2(fd_c5, 99)), "dup2 C5");
#endif

#ifdef C6
    // Create pipes with file descriptors {{33,34}, {53,54}, {73,74}, ...}
    const int c6_fd_cnt = 16;
    int fd_c6[c6_fd_cnt][2];

    for (int i = 0; i < c6_fd_cnt; i++) {
        err_chk((pipe(fd_c6[i])), "pipe C6");
        err_chk((dup2(fd_c6[i][0], 33 + 20 * i)), "dup2 C6 read end");
        err_chk((dup2(fd_c6[i][1], 34 + 20 * i)), "dup2 C6 write end");
    }
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


#ifdef C2
    // Make sure the riddle is running and has set up the alarm
    sleep(1);
    printf("sending SIGCONT signal to %d\n", p);
    kill(p, SIGCONT);
#endif

#if defined(C0) || defined(C1) || defined(C7)
    err_chk(close(fd_c0), "close C0");
#endif
#ifdef C4
    err_chk(close(fd_c4), "close C4");
#endif
#ifdef C5
    err_chk(close(fd_c5), "close C5");
#endif
#ifdef C6
    for (int i = 0; i < c6_fd_cnt; i++) {
        err_chk(close(fd_c6[i][0]), "close C6 read end");
        err_chk(close(fd_c6[i][1]), "close C6 write end");
    }
#endif


#ifdef FORK
	int wstatus;
	waitpid(p, &wstatus, 0);
#endif
	return 0;
}
