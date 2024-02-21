
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/syscall.h>

#ifdef DEBUG
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#define SYSCHK(x) ({          \
  typeof(x) __res = (x);      \
  if (__res == (typeof(x))-1) \
    err(1, "SYSCHK(" #x ")"); \
  __res;                      \
})
#else
#include "syscall.h"
#define SYSCHK(x) x
#define write(...)  syscall(SYS_write,  __VA_ARGS__)
#define exit(...)   syscall(SYS_exit,   __VA_ARGS__)
#define open(...)   syscall(SYS_open,   __VA_ARGS__)
#define read(...)   syscall(SYS_read,   __VA_ARGS__)
#define close(...)  syscall(SYS_close,  __VA_ARGS__)
#define execve(...) syscall(SYS_execve, __VA_ARGS__)
#define ioctl(...)  syscall(SYS_ioctl,  __VA_ARGS__)
#define dup2(...)   syscall(SYS_dup2,   __VA_ARGS__)
#endif


void append(char *p, int x) {
    int i = x;
    do {
        ++p;
    } while (i /= 10);
    *--p = 0;
    do {
		*--p = '0' + x % 10;
	} while (x /= 10);
}

#define PTS_PATH "/dev/pts/"

int main() {
    int fdm, fds, rc, ptn;
    char input[150];
    fdm = SYSCHK(open("/dev/ptmx", O_RDWR));

    int unlock = 0;
    SYSCHK(ioctl(fdm, TIOCSPTLCK, &unlock));

    char ptsname[sizeof(PTS_PATH) + 3] = PTS_PATH;
    ioctl(fdm, TIOCGPTN, &ptn);
    append(ptsname + sizeof(PTS_PATH), ptn);

    fds = SYSCHK(open(ptsname, O_RDWR));

    if (syscall(SYS_fork)) {
        fd_set fd_in;
        close(fds);

        while (1) {
            FD_ZERO(&fd_in);

            FD_SET(0, &fd_in);
            FD_SET(fdm, &fd_in);

            rc = syscall(SYS_select, fdm + 1, &fd_in, 0, 0, 0);
            if (rc == -1) exit(1);
            // If data on standard input
            if (FD_ISSET(0, &fd_in)) {
                rc = read(0, input, sizeof(input));
                if (rc > 0) {
                    // Send data on the master side of PTY
                    write(fdm, input, rc);
                } else if (rc < 0) {
                    exit(1);
                }
            }

            // If data on master side of PTY
            if (FD_ISSET(fdm, &fd_in)) {
                rc = read(fdm, input, sizeof(input));
                if (rc > 0) {
                    // Send data on standard output
                    write(1, input, rc);
                } else if (rc < 0) {
                    exit(1);
                }
            }
        }
    } else {
        close(fdm);
        dup2(fds, 0);
        dup2(fds, 1);
        dup2(fds, 2);
        close(fds);

        SYSCHK(syscall(SYS_setsid));

        SYSCHK(ioctl(0, TIOCSCTTY, 1));
        SYSCHK(execve("/bin/sh", ((char *[]){"sh", 0}), 0));
    }
}
