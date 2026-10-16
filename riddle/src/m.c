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
#define err_chk(func, err_str) {if ((func) == -1) die((err_str));}

#define FORK
#define C13


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
    const int SERV_PORT = 49842;        // Provided by the riddle through strace
    const int LISTENQ = 8;

    int listenfd, connfd;
    socklen_t clilen;
    char buf[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;
    char localhost[] = "127.0.0.1";

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&servaddr, 0 ,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);   
    inet_aton(localhost, &servaddr.sin_addr);

    err_chk(
        bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)),
        "bind C9"
    );
    listen(listenfd, LISTENQ);
#endif


#ifdef C10
    int fd_c10;
    err_chk(
        (fd_c10 = open("secret_number", O_RDWR | O_CREAT | O_TRUNC, 0600)),
        "open C0"
    );
    link("secret_number", "not_so_secret_number");
    close(fd_c10);
#endif

#ifdef C12
    // Redirect stdout to a pipe, making reading the riddle's
    // output easier
    int pfd_C12[2];
    err_chk(pipe(pfd_C12), "pipe C12");
    err_chk(dup2(pfd_C12[1], STDERR_FILENO), "dup2 C12");
#endif

#ifdef C13
    // For clarity's sake, we split the file creating from cases
    // C0, C1, C7
    int fd_c13;
    err_chk((fd_c13 = open("./.hello_there", O_RDWR | O_CREAT, 0600)), "open C13");

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
    // Read riddle's message: How much is x + y ?
    // and reply. Make at most step_lim tries before exiting
    const int step_lim = 16;
    int step = 0, n;

    while (step < step_lim) {
        int d1, d2;
        clilen = sizeof(cliaddr);
        err_chk(
            (connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen)),
            "accept C9"
        );

        while ((n = recv(connfd, buf, MAXLINE, 0)) > 0) {
            printf("%s","String received from and resent to the client:");
            sscanf(buf, "How much is %d + %d%s", &d1, &d2, NULL);
            sprintf(buf, "%d", d1 + d2);
            send(connfd, buf, strlen(buf), 0);
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

#ifdef C10
    sleep(1);
    fd_c10 = open("not_so_secret_number", O_RDONLY);
    char buf[4096];
    read(fd_c10, buf, sizeof(buf));

    // The riddle writes: The number I am thinking of right now is [uppercase matters]: XXXXXXXX
    char fmt[] = "The number I am thinking of right now is [uppercase matters]: ";
    char *ptr = strstr(buf, fmt) + strlen(fmt);

    fprintf(stdout, "%s", ptr);

    close(fd_c10);
    unlink("not_so_secret_number");
#endif

#ifdef C11
    pid_t c11;
    err_chk((c11 = fork()), "fork C11");

    if (c11 == 0) {
        char *const args[] = {NULL};
        execv("./C11.sh", args);
    }
    
    int wstatus_c11;
    waitpid(c11, &wstatus_c11, 0);
#endif

#ifdef C12
    // Make sure the riddle has parsed the savegame up to C12
    sleep(2);

    // Obtain offset and msg from riddle output
    // msg example: I want to find the char 'X' at 0x7fc979be306f
    off_t offset;
    char msg[2048], c;
    ssize_t rc;

    while (1) {
        err_chk(
            (rc = read(pfd_C12[0], msg, sizeof(msg)/sizeof(msg[0]))),
            "read C12"
        );
        if (rc == 0) {
            usleep(500);
            continue;
        }

        char *ptr = strstr(msg, "I want to find the char '");
        if (ptr == NULL) {
            usleep(100);
            continue;
        } else {
            sscanf(ptr, "I want to find the char '%c' at %x", &c, (unsigned int*) &offset);
            break;
        }
    }

    // The riddle creates a file: /tmp/riddle-XXXXXX
    // which it maps to its memory
    int fd_c12;
    char tmp_path[64];
    FILE *cmd_fd_c12 = popen("ls -la \"/proc/$(pidof riddle)/map_files\" | awk '/\\/tmp\\/riddle/ { print $NF }'", "r");

    if (cmd_fd_c12 == NULL) {
        die("popen C12");
    }

    fgets(tmp_path, sizeof(tmp_path), cmd_fd_c12);

    // fgets leaves a trailing \n, messing up the path
    tmp_path[strlen(tmp_path) - 1] = '\0';
    err_chk((fd_c12 = open(tmp_path, O_RDWR, 0600)), "open C12");

    // Block size of 0x1000 bytes
    offset %= 0x1000;
    printf("writing '%c' to offset %x in file %s\n", c, (unsigned int) offset, tmp_path);
    
    if (pwrite(fd_c12, &c, sizeof(c), offset) != sizeof(c)) {
        die("pwrite C12");
    }

    /* Print riddle output in full
    usleep(100);
    printf("%s\n", msg);
    err_chk(
        (rc = read(pfd_C12[0], msg, sizeof(msg)/sizeof(msg[0]))),
        "read C12"
    );
    printf("%s\n", msg);
    */
#endif

#ifdef C13
    sleep(3);
    ftruncate(fd_c13, 0x8000);
    fprintf(stderr, "[%d]: truncated .hello_there to 0x8000\n", getpid());
#endif

/* ===========================================================================
 *                             CLEANUP 
 * =========================================================================== */
#if defined(C0) || defined(C1) || defined(C7)
    err_chk(close(fd_c0), "close C0");
    unlink(".hello_there");
    unlink(".hey_there");
#endif

#ifdef C4
    err_chk(close(fd_c4), "close C4");
    unlink("magic_mirror");
#endif

#ifdef C5
    err_chk(close(fd_c5), "close C5");
#endif

#ifdef C6
    for (int i = 0; i < fd_c6_cnt; i++) {
        sprintf(buf, "bf0%d", i);
        err_chk(close(fd_c6[i][0]), "close C6 read end");
        err_chk(close(fd_c6[i][1]), "close C6 write end");
        unlink(buf);
    }
#endif

#ifdef C8
    for (int i = 0; i < fd_c8_cnt; i++) {
        sprintf(buf, "bf0%d", i);
        unlink(buf);
    }
#endif

#ifdef C12
    err_chk(close(pfd_C12[0]), "close C12");
    err_chk(close(pfd_C12[1]), "close C12");

    err_chk(close(fd_c12), "close C12");

    if (pclose(cmd_fd_c12) != 0) {
        die("pclose C12");
    }
#endif

#ifdef C13
    err_chk(close(fd_c13), "close C13");
    err_chk(unlink("./.hello_there"), "unlink C13");
#endif

#ifdef FORK
	int wstatus;
	waitpid(p, &wstatus, 0);
#endif
	return 0;
}
