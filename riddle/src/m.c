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

#define die(str) {fprintf(stderr, "%s\n", (str)); exit(EXIT_FAILURE);}
#define err_chk(func, err_str) {if ((func) == -1) die((err_str))};

#define FORK
#define C9

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
    // supported by ext4. This solution should may not be portable.
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
    // Establish a socket to localhost
    // https://www.cs.dartmouth.edu/~campbell/cs50/socketprogramming.html

    /*
    // riddle calls: socket(2, 1, 0)
    printf("AF_UNIX: %x\nAF_LOCAL: %x\nAF_INET: %x\n", AF_UNIX, AF_LOCAL, AF_INET);
    printf("SOCK_STREAM: %x\nSOCK_DGRAM: %x\nSOCK_SEQPACKET: %x\n", SOCK_STREAM, SOCK_DGRAM, SOCK_SEQPACKET);
    printf("SOCK_RAW: %x\nSOCK_RDM: %x\nSOCK_PACKET: %x\n", SOCK_RAW, SOCK_RDM, SOCK_PACKET);
        printf("SOCK_NONBLOCK: %x\nSOCK_CLOEXEC: %x\n",SOCK_NONBLOCK,SOCK_CLOEXEC);
     */

    int fd_c9;
    err_chk((fd_c9 = socket(AF_INET, SOCK_STREAM, 0)), "socket C9");

    struct in_addr inp;
    char localhost[] = "127.0.0.1";

    err_chk(inet_aton(localhost, &inp), "inet_aton C9");

    struct sockaddr addr;
    addr.sa_family = AF_INET;

    connect(fd_c9, const struct sockaddr *addr, (socklen_t) 16);
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
