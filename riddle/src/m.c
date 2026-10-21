#include <signal.h>
#define _GNU_SOURCE
#include <err.h>
#include <linux/sched.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include <errno.h>

#define die(str) {fprintf(stderr, "errno: %s\n%s\n", strerror(errno), (str)); exit(EXIT_FAILURE);}
#define err_chk(func, err_str) {if ((func) == -1) die((err_str));}

#define FORK
#define C14

#if defined(C11) || defined(C14)
#define ROOT_ACCESS
#endif

#ifdef C14
#undef FORK
#endif


/*
 * O_RDONLY: 0
 * O_WRONLY: 1
 * O_RDWR: 2
 * O_CREAT: 64
 * O_TRUNC: 512
 */

#ifdef C14
// After creating a new pid_namespace, let the new init (PID=1)
// fork into a PID=32767 and execv into the riddle
int pid_namespace_init(void) {
    printf("innit pid: %d\n", getpid());
    pid_t target_pid = 32767;

    struct clone_args cargs;
    memset(&cargs, 0, sizeof(cargs));

    cargs.set_tid = (intptr_t) &target_pid;
    cargs.set_tid_size = 1;
    cargs.exit_signal = SIGCHLD;

    long riddle_pid = syscall(SYS_clone3, &cargs, sizeof(cargs));

    err_chk(riddle_pid, "clone3 C14");

    if (riddle_pid == 0) {
        char *const args[] = {NULL};
        execv("./riddle", args);
    }

    int wstatus;
    waitpid((pid_t) riddle_pid, &wstatus, 0);
    return 0;
}
#endif

int main(int argc, char **argv) {
/* ===========================================================================
 *                              PRE-FORK
 * =========================================================================== */

#ifdef ROOT_ACCESS
    if (geteuid() != 0) {
        fprintf(stderr, "This solution must be run as root\n");
        exit(EXIT_FAILURE);
    }
#endif

#if defined(C0) || defined(C1) || defined(C7)
    int fd_c0 = -1;
    if (access("./.hello_there", F_OK) == -1) {
        err_chk((fd_c0 = open("./.hello_there", O_RDONLY | O_CREAT | O_TRUNC, 0444)), "open C0");
    }
#endif

#ifdef C1
    err_chk(chmod("./.hello_there", 0444), "chmod C1");
#endif

#ifdef C7
    err_chk(link("./.hello_there", "./.hey_there"), "link C7");
#endif

#ifdef C3
    setenv("ANSWER", "42", 1);
#endif

#ifdef C4
    err_chk((mkfifo("./magic_mirror", 0666)), "mkfifo C4");
#endif

#ifdef C5
    int fd_c5;
    err_chk((fd_c5 = open("c5", O_CREAT | O_RDWR | O_TRUNC, 0777)), "open C5");

    // Switch the file descriptor of c5 to 99
    err_chk((dup2(fd_c5, 99)), "dup2 C5");

    if (fd_c5 != 99) err_chk(close(fd_c5), "close C5 original");
#endif

#ifdef C6
    // Create pipes with file descriptors {33,34} and {53,54}
    const int fd_c6_cnt = 2;
    int fd_c6[fd_c6_cnt][2];

    for (int i = 0; i < fd_c6_cnt; i++) {
        err_chk((pipe(fd_c6[i])), "pipe C6");
        err_chk((dup2(fd_c6[i][0], 33 + 20 * i)), "dup2 C6 read end");
        err_chk((dup2(fd_c6[i][1], 34 + 20 * i)), "dup2 C6 write end");
        
        // Close original descriptors, keep duplicates
        err_chk(close(fd_c6[i][0]), "close C6 original read end");
        err_chk(close(fd_c6[i][1]), "close C6 original write end");
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

#ifdef C14_ns_last_pid
    printf("Are you running this in a VM? (y/n)?\n");
    char ans = getchar();

    if (!(ans == 'y' || ans == 'Y')) {
        fprintf(stderr, "Wrong answer :(\n");
        exit(EXIT_FAILURE);
    }

    // Solution based on:
    // https://efiop-notes.blogspot.com/2014/06/how-to-set-pid-using-nslastpid.html

    int fd_c14;
    err_chk(
        (fd_c14 = open("/proc/sys/kernel/ns_last_pid", O_RDWR | O_CREAT, 06444)),
        "open C14"
    );

    // Place an exclusive lock on ns_last_pid
    if (flock(fd_c14, LOCK_EX)) {
        close(fd_c14);
        fprintf(stderr, "flock C14");
        exit(EXIT_FAILURE);
    }

    // Our target pid is 32767, so we set ns_last_pid to 32767 - 1;
    char target_pid[] = "32766";
    char last_pid[16];

    if (read(fd_c14, last_pid, sizeof(last_pid)/sizeof(last_pid[0])) <= 0) {
        fprintf(stderr, "read C14");
        exit(EXIT_FAILURE);
    }

    printf("\n\nlast_pid = %s\n\n\n", last_pid);

    if (write(fd_c14, target_pid, strlen(target_pid)) != strlen(target_pid)) {
        fprintf(stderr, "write C14");
        exit(EXIT_FAILURE);
    }
    printf("\n\nwrite success\n\n\n");
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

#ifdef C14
    struct clone_args cargs;
    memset(&cargs, 0, sizeof(cargs));

    cargs.flags = CLONE_NEWPID;
    cargs.exit_signal = SIGCHLD;

    printf("Pre-clone check\n");

    long innit = syscall(SYS_clone3, &cargs, sizeof(cargs));

    err_chk(innit, "clone3 C14");

    if (innit == 0) {
        int ret = pid_namespace_init();
        exit(ret);
    }

    int wstatus;
    waitpid((pid_t) innit, &wstatus, 0);
#endif

/* ===========================================================================
 *                              POST-FORK
 * =========================================================================== */

#ifdef C2
    // Make sure the riddle is running and has set up the alarm
    sleep(1);
    printf("sending SIGCONT signal to %d\n", p);
    err_chk(kill(p, SIGCONT), "kill C2");
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

#ifdef C6
    if (p > 0) {
        err_chk(close(33), "close C6 33");
        err_chk(close(34), "close C6 34");
        err_chk(close(53), "close C6 53");
        err_chk(close(54), "close C6 54");
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

#ifdef C14_ns_last_pid
    sleep(3);
    if (write(fd_c14, last_pid, strlen(last_pid)) != strlen(last_pid)) {
        fprintf(stderr, "write C14");
        exit(EXIT_FAILURE);
    }
    // Release the lock
    if (flock(fd_c14, LOCK_UN)) {
        fprintf(stderr, "flock C14");
        exit(EXIT_FAILURE);
    }
    close(fd_c14);
#endif

#ifdef FORK
	int wstatus;
	waitpid(p, &wstatus, 0);
#endif

#if defined(C0) || defined(C1) || defined(C7)
    if (fd_c0 >= 0)
        err_chk(close(fd_c0), "close C0");
#endif

#ifdef C7
    err_chk(unlink(".hey_there"), "unlink C7");
#endif

#ifdef C4
    err_chk(unlink("./magic_mirror"), "unlink C4");
#endif

#ifdef C5
    err_chk(close(99), "close C5 fd 99");
    err_chk(unlink("c5"), "unlink C5");
#endif

	return 0;
}
