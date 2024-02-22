
#include <asm/unistd_32.h>
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

#ifdef __i386__
#undef SYS_select
#define SYS_select SYS__newselect
#endif

static inline void append(char *p, int x) {
    int i = x;
    do {
        ++p;
    } while (i /= 10);
    *--p = 0;
    do {
		*--p = '0' + x % 10;
	} while (x /= 10);
}

__attribute__ ((noreturn)) void main() {
    char dev_buf[sizeof("/dev/ptmx") + 3] = "/dev/ptmx";
    int fdm, rc;
    char input[150];
    fdm = SYSCHK(open(dev_buf, O_RDWR));

    int unlock = 0;
    SYSCHK(ioctl(fdm, TIOCSPTLCK, &unlock));

    if (syscall(SYS_fork)) {
        fd_set fd_in;

        while (1) {
            FD_ZERO(&fd_in);

            FD_SET(0, &fd_in);
            FD_SET(fdm, &fd_in);

            rc = SYSCHK(syscall(SYS_select, fdm + 1, &fd_in, 0, 0, 0));
            // If data on standard input
            if (FD_ISSET(0, &fd_in)) {
                rc = read(0, input, sizeof(input));
                if (rc > 0) {
                    // Send data on the master side of PTY
                    write(fdm, input, rc);
                } else if (rc < 0) {
                    break;
                }
            }

            // If data on master side of PTY
            if (FD_ISSET(fdm, &fd_in)) {
                rc = read(fdm, input, sizeof(input));
                if (rc > 0) {
                    // Send data on standard output
                    write(1, input, rc);
                } else if (rc < 0) {
                    break;
                }
            }
        }
        exit(0);
    } else {
        int fds, ptn;
        char *ptsname = dev_buf;
        ptsname[7] = 's';
        ptsname[8] = '/';
        ioctl(fdm, TIOCGPTN, &ptn);
        close(fdm);

        append(ptsname + sizeof("/dev/pts/"), ptn);
        fds = SYSCHK(open(ptsname, O_RDWR));
        for (int i = 0; i < 3; i++) dup2(fds, i);
        close(fds);

        SYSCHK(syscall(SYS_setsid));

        SYSCHK(ioctl(0, TIOCSCTTY, 1));

        char bin_sh[] = "/bin/sh";
        SYSCHK(execve(bin_sh, ((char *[]){bin_sh, 0}), 0));
    }
}
