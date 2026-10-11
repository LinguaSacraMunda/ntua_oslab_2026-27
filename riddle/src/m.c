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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include <errno.h>

#define die(str) {fprintf(stderr, "errno: %s\n%s\n", strerror(errno), (str)); exit(EXIT_FAILURE);}
#define err_chk(func, err_str) {if ((func) == -1) die((err_str))};

#define FORK
#define C9

// Establish a TCP Server through localhost
// https://www.cs.dartmouth.edu/~campbell/cs50/socketprogramming.html
int setup_TCP(void) {
    const int MAXLINE = 4096;
    const int SERV_PORT = 0;
    const int LISTENQ = 8;

    int listenfd, connfd, n;
    socklen_t clilen;
    char buf[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;
    char localhost[] = "127.0.0.1";

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&servaddr, 0 ,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    // Endianess shouldn't matter since SERV_PORT = 0
    // ergo, let the kernel decide
    servaddr.sin_port = htons(SERV_PORT);   
    inet_aton(localhost, &servaddr.sin_addr);

    err_chk(
        bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)),
        "bind C9"
    );
    listen(listenfd, LISTENQ);


    return 0;
}


/*
 * O_RDONLY: 0
 * O_WRONLY: 1
 * O_RDWR: 2
 * O_CREAT: 64
 * O_TRUNC: 512
 */

int main(int argc, char **argv) {
/* ===========================================================================
 *                              PRE-FORK
 * =========================================================================== */
#if defined(C0) || defined(C1) || defined(C7)
    int fd_c0;
    if (access("./hello_there", F_OK) == -1) {
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
    const int fd_c6_cnt = 16;
    int fd_c6[fd_c6_cnt][2];

    for (int i = 0; i < fd_c6_cnt; i++) {
        err_chk((pipe(fd_c6[i])), "pipe C6");
        err_chk((dup2(fd_c6[i][0], 33 + 20 * i)), "dup2 C6 read end");
        err_chk((dup2(fd_c6[i][1], 34 + 20 * i)), "dup2 C6 write end");
    }
#endif

#ifdef C8
    // We shall cheat (!) by creating sparse files,
    // supported by ext4. This solution may not be portable.
    // In vfat file systems the empty space is zero-filled.
    // Equivalent of: dd if=/dev/zero of=bf00 bs=1k seek=1048576 count=1
    const int fd_c8_cnt = 16;
    int fd_c8[fd_c8_cnt];

    // The riddle reads/writes 16 bytes
    size_t rwlen = 16;              
    off_t offset = 0x40000000LL;

    // Message at file footer
    char msg[] = "XXXXXXXXXXXXXXXX";

    char buf[5];
    for (int i = 0; i < fd_c8_cnt; i++) {
        sprintf(buf, "bf0%d", i);
        err_chk((fd_c8[i] = open(buf, O_CREAT | O_RDWR | O_TRUNC, 0777)), "open C8");
        err_chk(ftruncate(fd_c8[i], offset + rwlen), "ftruncate C8");
        err_chk(pwrite(fd_c8[i], msg, strlen(msg) * sizeof(char), offset), "pwrite C8");
        err_chk(close(fd_c8[i]), "close C8");
    }
#endif

#ifdef C9
    // Establish a TCP Server through localhost
    // https://www.cs.dartmouth.edu/~campbell/cs50/socketprogramming.html

    const int MAXLINE = 4096;
    const int SERV_PORT = 8080;
    const int LISTENQ = 8;

    int listenfd, connfd;
    socklen_t clilen;
    char buf[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;
    char localhost[] = "127.0.0.1";

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&servaddr, 0 ,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    // Endianess shouldn't matter since SERV_PORT = 0
    // ergo, let the kernel decide
    servaddr.sin_port = htons(SERV_PORT);   
    inet_aton(localhost, &servaddr.sin_addr);

    err_chk(
        bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)),
        "bind C9"
    );
    listen(listenfd, LISTENQ);
#endif

/* ===========================================================================
 *                                 FORK
 * =========================================================================== */
#ifdef FORK 
	pid_t p;
	if ((p = fork()) == -1) {
		perror("fork");
		exit(1);
	}

	if (p == 0) {
#ifdef C9
        err_chk(close(listenfd), "close C9");
#endif
		char *const args[] = {NULL};
		execv("./riddle", args);
	}
#endif

/* ===========================================================================
 *                              POST-FORK
 * =========================================================================== */


#ifdef C2
    // Make sure the riddle is running and has set up the alarm
    sleep(1);
    printf("sending SIGCONT signal to %d\n", p);
    kill(p, SIGCONT);
#endif

#ifdef C9
    const int step_lim = 16;
    int step = 0, n;


    while (step < step_lim) {
        clilen = sizeof(cliaddr);
        err_chk(
            (connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen)),
            "accept C9"
        );

        while ((n = recv(connfd, buf, MAXLINE, 0)) > 0) {
            printf("%s","String received from and resent to the client:");
            puts(buf);
            send(connfd, buf, n, 0);
        }
        
        if (n < 0) {
            die("recv C9");
        }

        close(connfd);

        usleep(100);
        step++;
    }

    close(listenfd);
#endif



/* ===========================================================================
 *                             CLEANUP 
 * =========================================================================== */
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
    for (int i = 0; i < fd_c6_cnt; i++) {
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
