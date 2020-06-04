/*
 * Copyright (C) 2017-2018 Willy Tarreau <w@1wt.eu>
 * SPDX-License-Identifier: LGPL-2.1 OR MIT
 */

/*
 * This file was originally retrieved from: http://git.1wt.eu/git/nolibc.git
 * An old copy of this file was merged into the Kernel and can be found at:
 * https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/tools/include/nolibc/nolibc.h
 *
 * To update this file, pull any new changes and diff them against old version (diffing against
 * this file won't work, as this has been changed to fit the nlib conventions).
 */

#if !defined(NLIB_LINUX_H_INCLUDE_GUARD)

/*
 * This file is designed to be used as a libc alternative for minimal programs
 * with very limited requirements. It consists of a small number of syscall and
 * type definitions, and the minimal startup code needed to call main().
 * All syscalls are declared as static functions so that they can be optimized
 * away by the compiler when not used.
 *
 * Syscalls are split into 2 levels:
 *   - The lower level is the arch-specific syscall() definition, consisting in
 *     assembly code in compound expressions. These are called linuxMakeSysCall0() to
 *     linuxMakeSysCall6() depending on the number of arguments. The MIPS
 *     implementation is limited to 5 arguments. All input arguments are cast
 *     to a long stored in a register. These expressions always return the
 *     syscall's return value as a signed long value which is often either a
 *     pointer or the negated errno value.
 *
 *   - The second level is mostly architecture-independent. It is made of
 *     static functions called sys_<name>() which rely on linuxMakeSysCallN()
 *     depending on the syscall definition. These functions are responsible
 *     for exposing the appropriate types for the syscall arguments (int,
 *     pointers, etc) and for setting the appropriate return type (often int).
 *     A few of them are architecture-specific because the syscalls are not all
 *     mapped exactly the same among architectures. For example, some archs do
 *     not implement select() and need pselect6() instead, so the sys_select()
 *     function will have to abstract this.
 *
 * Some macro definitions like the O_* values passed to open(), and some
 * structures like the sys_stat struct depend on the architecture.
 *
 * The definitions start with the architecture-specific parts, which are picked
 * based on what the compiler knows about the target architecture, and are
 * completed with the generic code. Since it is the compiler which sets the
 * target architecture, cross-compiling normally works out of the box without
 * having to specify anything.
 *
 * A simple static executable may be built this way :
 *      $ gcc -fno-asynchronous-unwind-tables -fno-ident -s -Os -nostdlib \
 *            -static -include nolibc.h -lgcc -o hello hello.c
 *
 * A very useful calling convention table may be found here :
 *      http://man7.org/linux/man-pages/man2/syscall.2.html
 *
 * This doc is quite convenient though not necessarily up to date :
 *      https://w3challs.com/syscalls/
 *
 */

/* Some archs (at least aarch64) don't expose the regular syscalls anymore by
 * default, either because they have an "_at" replacement, or because there are
 * more modern alternatives. For now we'd rather still use them.
 */
#define __ARCH_WANT_SYSCALL_NO_AT
#define __ARCH_WANT_SYSCALL_NO_FLAGS
#define __ARCH_WANT_SYSCALL_DEPRECATED

#include <linux/unistd.h>
#include <linux/ioctl.h>
#include <asm/ioctls.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/loop.h>
#include <linux/signal.h>
#include <linux/types.h>

global_variable thread_local S32 errno;

typedef   signed long       ssize_t;
typedef   struct timeval    timeval;

/* for stat() */
typedef unsigned int          dev_t;
typedef unsigned long         ino_t;
typedef unsigned int          mode_t;
typedef   signed int          pid_t;
typedef unsigned int          uid_t;
typedef unsigned int          gid_t;
typedef unsigned long         nlink_t;
typedef   signed long         offset_t;
typedef   signed long         blksize_t;
typedef   signed long         blkcnt_t;
typedef   signed long         time_t;

/* for poll() */
typedef struct pollfd {
    S32 fd;
    S16 events;
    S16 revents;
} pollfd;

/* for getdents64() */
typedef struct dirent64 {
    U64  d_ino;
    S64  d_off;
    U16  d_reclen;
    Byte d_type;
    Char d_name[];
} dirent64;

/* commonly an fd_set represents 256 FDs */
#define FD_SETSIZE 256
typedef struct { U32 fd32[FD_SETSIZE/32]; } fd_set;

/* needed by wait4() */
typedef struct rusage {
    timeval ru_utime;
    timeval ru_stime;
    S64        ru_maxrss;
    S64        ru_ixrss;
    S64        ru_idrss;
    S64        ru_isrss;
    S64        ru_minflt;
    S64        ru_majflt;
    S64        ru_nswap;
    S64        ru_inblock;
    S64        ru_oublock;
    S64        ru_msgsnd;
    S64        ru_msgrcv;
    S64        ru_nsignals;
    S64        ru_nvcsw;
    S64        ru_nivcsw;
} rusage;

/* stat flags (WARNING, octal here) */
#define S_IFDIR       0040000
#define S_IFCHR       0020000
#define S_IFBLK       0060000
#define S_IFREG       0100000
#define S_IFIFO       0010000
#define S_IFLNK       0120000
#define S_IFSOCK      0140000
#define S_IFMT        0170000

#define S_ISDIR(mode)  (((mode) & S_IFDIR) == S_IFDIR)
#define S_ISCHR(mode)  (((mode) & S_IFCHR) == S_IFCHR)
#define S_ISBLK(mode)  (((mode) & S_IFBLK) == S_IFBLK)
#define S_ISREG(mode)  (((mode) & S_IFREG) == S_IFREG)
#define S_ISFIFO(mode) (((mode) & S_IFIFO) == S_IFIFO)
#define S_ISLNK(mode)  (((mode) & S_IFLNK) == S_IFLNK)
#define S_ISSOCK(mode) (((mode) & S_IFSOCK) == S_IFSOCK)

#define DT_UNKNOWN 0
#define DT_FIFO    1
#define DT_CHR     2
#define DT_DIR     4
#define DT_BLK     6
#define DT_REG     8
#define DT_LNK    10
#define DT_SOCK   12

/* all the *at functions */
#ifndef AT_FDWCD
#define AT_FDCWD             -100
#endif

/* lseek */
#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2

/* reboot */
#define LINUX_REBOOT_MAGIC1         0xfee1dead
#define LINUX_REBOOT_MAGIC2         0x28121969
#define LINUX_REBOOT_CMD_HALT       0xcdef0123
#define LINUX_REBOOT_CMD_POWER_OFF  0x4321fedc
#define LINUX_REBOOT_CMD_RESTART    0x01234567
#define LINUX_REBOOT_CMD_SW_SUSPEND 0xd000fce2


/* The format of the struct as returned by the libc to the application, which
 * significantly differs from the format returned by the stat() syscall flavours.
 */
struct stat {
    dev_t         st_dev;     /* ID of device containing file */
    ino_t         st_ino;     /* inode number */
    mode_t        st_mode;    /* protection */
    nlink_t       st_nlink;   /* number of hard links */
    uid_t         st_uid;     /* user ID of owner */
    gid_t         st_gid;     /* group ID of owner */
    dev_t         st_rdev;    /* device ID (if special file) */
    offset_t      st_size;    /* total size, in bytes */
    blksize_t  st_blksize; /* blocksize for file system I/O */
    blkcnt_t st_blocks;  /* number of 512B blocks allocated */
    time_t        st_atime;   /* time of last access */
    time_t        st_mtime;   /* time of last modification */
    time_t        st_ctime;   /* time of last status change */
};

#define WEXITSTATUS(status)   (((status) & 0xff00) >> 8)
#define WIFEXITED(status)     (((status) & 0x7f) == 0)

/* Below comes the architecture-specific code. For each architecture, we have
 * the syscall declarations and the _start code definition. This is the only
 * global part. On all architectures the kernel puts everything in the stack
 * before jumping to _start just above us, without any return address (_start
 * is not a function but an entry pint). So at the stack pointer we find argc.
 * Then argv[] begins, and ends at the first NULL. Then we have envp which
 * starts and ends with a NULL as well. So envp=argv+argc+1.
 */

#if defined(__x86_64__)
/* Syscalls for x86_64 :
 *   - registers are 64-bit
 *   - syscall number is passed in rax
 *   - arguments are in rdi, rsi, rdx, r10, r8, r9 respectively
 *   - the system call is performed by calling the syscall instruction
 *   - syscall return comes in rax
 *   - rcx and r8..r11 may be clobbered, others are preserved.
 *   - the arguments are cast to S64 and assigned into the target registers
 *     which are then simply passed as registers to the asm code, so that we
 *     don't have to experience issues with register constraints.
 *   - the syscall number is always specified last in order to allow to force
 *     some registers before (gcc refuses a %-register at the last position).
 */

#define linuxMakeSysCall0(num)                                          \
    ({                                                                  \
        S64 _ret;                                                       \
        register S64 _num  __asm__("rax") = (num);                      \
                                                                        \
        __asm__ volatile (                                              \
                          "syscall\n"                                   \
                          : "=a" (_ret)                                 \
                          : "0"(_num)                                   \
                          : "rcx", "r8", "r9", "r10", "r11", "memory", "cc" \
                                                                );      \
        _ret;                                                           \
    })

#define linuxMakeSysCall1(num, arg1)                                    \
    ({                                                                  \
        S64 _ret;                                                       \
        register S64 _num  __asm__("rax") = (num);                      \
        register S64 _arg1 __asm__("rdi") = (S64)(arg1);                \
                                                                        \
        __asm__ volatile (                                              \
                          "syscall\n"                                   \
                          : "=a" (_ret)                                 \
                          : "r"(_arg1),                                 \
                            "0"(_num)                                   \
                          : "rcx", "r8", "r9", "r10", "r11", "memory", "cc" \
                                                                );      \
        _ret;                                                           \
    })

#define linuxMakeSysCall2(num, arg1, arg2)                          \
    ({                                                          \
        S64 _ret;                                              \
        register S64 _num  __asm__("rax") = (num);             \
        register S64 _arg1 __asm__("rdi") = (S64)(arg1);      \
        register S64 _arg2 __asm__("rsi") = (S64)(arg2);      \
                                                                \
        __asm__ volatile (                                      \
            "syscall\n"                                         \
            : "=a" (_ret)                                       \
            : "r"(_arg1), "r"(_arg2),                           \
              "0"(_num)                                         \
            : "rcx", "r8", "r9", "r10", "r11", "memory", "cc"   \
            );                                                  \
        _ret;                                                   \
    })

#define linuxMakeSysCall3(num, arg1, arg2, arg3)                    \
    ({                                                          \
        S64 _ret;                                              \
        register S64 _num  __asm__("rax") = (num);             \
        register S64 _arg1 __asm__("rdi") = (S64)(arg1);      \
        register S64 _arg2 __asm__("rsi") = (S64)(arg2);      \
        register S64 _arg3 __asm__("rdx") = (S64)(arg3);      \
                                                                \
        __asm__ volatile (                                      \
            "syscall\n"                                         \
            : "=a" (_ret)                                       \
            : "r"(_arg1), "r"(_arg2), "r"(_arg3),               \
              "0"(_num)                                         \
            : "rcx", "r8", "r9", "r10", "r11", "memory", "cc"   \
            );                                                  \
        _ret;                                                   \
    })

#define linuxMakeSysCall4(num, arg1, arg2, arg3, arg4)              \
    ({                                                          \
        S64 _ret;                                              \
        register S64 _num  __asm__("rax") = (num);             \
        register S64 _arg1 __asm__("rdi") = (S64)(arg1);      \
        register S64 _arg2 __asm__("rsi") = (S64)(arg2);      \
        register S64 _arg3 __asm__("rdx") = (S64)(arg3);      \
        register S64 _arg4 __asm__("r10") = (S64)(arg4);      \
                                                                \
        __asm__ volatile (                                      \
            "syscall\n"                                         \
            : "=a" (_ret), "=r"(_arg4)                          \
            : "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4),   \
              "0"(_num)                                         \
            : "rcx", "r8", "r9", "r11", "memory", "cc"          \
            );                                                  \
        _ret;                                                   \
    })

#define linuxMakeSysCall5(num, arg1, arg2, arg3, arg4, arg5)                \
    ({                                                                  \
        S64 _ret;                                                      \
        register S64 _num  __asm__("rax") = (num);                     \
        register S64 _arg1 __asm__("rdi") = (S64)(arg1);              \
        register S64 _arg2 __asm__("rsi") = (S64)(arg2);              \
        register S64 _arg3 __asm__("rdx") = (S64)(arg3);              \
        register S64 _arg4 __asm__("r10") = (S64)(arg4);              \
        register S64 _arg5 __asm__("r8")  = (S64)(arg5);              \
                                                                        \
        __asm__ volatile (                                              \
            "syscall\n"                                                 \
            : "=a" (_ret), "=r"(_arg4), "=r"(_arg5)                     \
            : "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4), "r"(_arg5), \
              "0"(_num)                                                 \
            : "rcx", "r9", "r11", "memory", "cc"                        \
            );                                                          \
        _ret;                                                           \
    })

#define linuxMakeSysCall6(num, arg1, arg2, arg3, arg4, arg5, arg6)          \
    ({                                                                  \
        S64 _ret;                                                      \
        register S64 _num  __asm__("rax") = (num);                     \
        register S64 _arg1 __asm__("rdi") = (S64)(arg1);              \
        register S64 _arg2 __asm__("rsi") = (S64)(arg2);              \
        register S64 _arg3 __asm__("rdx") = (S64)(arg3);              \
        register S64 _arg4 __asm__("r10") = (S64)(arg4);              \
        register S64 _arg5 __asm__("r8")  = (S64)(arg5);              \
        register S64 _arg6 __asm__("r9")  = (S64)(arg6);              \
                                                                        \
        __asm__ volatile (                                              \
            "syscall\n"                                                 \
            : "=a" (_ret), "=r"(_arg4), "=r"(_arg5)                     \
            : "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4), "r"(_arg5), \
              "r"(_arg6), "0"(_num)                                     \
            : "rcx", "r11", "memory", "cc"                              \
            );                                                          \
        _ret;                                                           \
    })

/* startup code */
__asm__(".section .text\n"
        ".global _start\n"
        "_start:\n"
        "pop %rdi\n"                // argc   (first arg, %rdi)
        "mov %rsp, %rsi\n"          // argv[] (second arg, %rsi)
        "lea 8(%rsi,%rdi,8),%rdx\n" // then a NULL then envp (third arg, %rdx)
        "and $-16, %rsp\n"          // x86 ABI : esp must be 16-byte aligned when
        "sub $8, %rsp\n"            // entering the callee
        "call main\n"               // main() returns the status code, we'll exit with it.
        "movzb %al, %rdi\n"         // retrieve exit code from 8 lower bits
        "mov $60, %rax\n"           // NR_exit == 60
        "syscall\n"                 // really exit
        "hlt\n"                     // ensure it does not return
        "");

/* fcntl / open */
#define O_RDONLY            0
#define O_WRONLY            1
#define O_RDWR              2
#define O_CREAT          0x40
#define O_EXCL           0x80
#define O_NOCTTY        0x100
#define O_TRUNC         0x200
#define O_APPEND        0x400
#define O_NONBLOCK      0x800
#define O_DIRECTORY   0x10000

/* The struct returned by the stat() syscall, equivalent to stat64(). The
 * syscall returns 116 bytes and stops in the middle of __unused.
 */
struct sys_stat_struct {
    U64 st_dev;
    U64 st_ino;
    U64 st_nlink;
    U32  st_mode;
    U32  st_uid;

    U32  st_gid;
    U32  __pad0;
    U64 st_rdev;
    S64          st_size;
    S64          st_blksize;

    S64          st_blocks;
    U64 st_atime;
    U64 st_atime_nsec;
    U64 st_mtime;

    U64 st_mtime_nsec;
    U64 st_ctime;
    U64 st_ctime_nsec;
    S64          __unused[3];
};

#elif defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__)
/* Syscalls for i386 :
 *   - mostly similar to x86_64
 *   - registers are 32-bit
 *   - syscall number is passed in eax
 *   - arguments are in ebx, ecx, edx, esi, edi, ebp respectively
 *   - all registers are preserved (except eax of course)
 *   - the system call is performed by calling int $0x80
 *   - syscall return comes in eax
 *   - the arguments are cast to S64 and assigned into the target registers
 *     which are then simply passed as registers to the asm code, so that we
 *     don't have to experience issues with register constraints.
 *   - the syscall number is always specified last in order to allow to force
 *     some registers before (gcc refuses a %-register at the last position).
 *
 * Also, i386 supports the old_select syscall if newselect is not available
 */
#define __ARCH_WANT_SYS_OLD_SELECT

#define linuxMakeSysCall0(num)                              \
    ({                                                  \
        S64 _ret;                                      \
        register S64 _num __asm__("eax") = (num);      \
                                                        \
        __asm__ volatile (                              \
            "int $0x80\n"                               \
            : "=a" (_ret)                               \
            : "0"(_num)                                 \
            : "memory", "cc"                            \
            );                                          \
        _ret;                                           \
    })

#define linuxMakeSysCall1(num, arg1)                                \
    ({                                                          \
        S64 _ret;                                              \
        register S64 _num __asm__("eax") = (num);              \
        register S64 _arg1 __asm__("ebx") = (S64)(arg1);      \
                                                                \
        __asm__ volatile (                                      \
            "int $0x80\n"                                       \
            : "=a" (_ret)                                       \
            : "r"(_arg1),                                       \
              "0"(_num)                                         \
            : "memory", "cc"                                    \
            );                                                  \
        _ret;                                                   \
    })

#define linuxMakeSysCall2(num, arg1, arg2)                          \
    ({                                                          \
        S64 _ret;                                              \
        register S64 _num __asm__("eax") = (num);              \
        register S64 _arg1 __asm__("ebx") = (S64)(arg1);      \
        register S64 _arg2 __asm__("ecx") = (S64)(arg2);      \
                                                                \
        __asm__ volatile (                                      \
            "int $0x80\n"                                       \
            : "=a" (_ret)                                       \
            : "r"(_arg1), "r"(_arg2),                           \
              "0"(_num)                                         \
            : "memory", "cc"                                    \
            );                                                  \
        _ret;                                                   \
    })

#define linuxMakeSysCall3(num, arg1, arg2, arg3)                    \
    ({                                                          \
        S64 _ret;                                              \
        register S64 _num __asm__("eax") = (num);              \
        register S64 _arg1 __asm__("ebx") = (S64)(arg1);      \
        register S64 _arg2 __asm__("ecx") = (S64)(arg2);      \
        register S64 _arg3 __asm__("edx") = (S64)(arg3);      \
                                                                \
        __asm__ volatile (                                      \
            "int $0x80\n"                                       \
            : "=a" (_ret)                                       \
            : "r"(_arg1), "r"(_arg2), "r"(_arg3),               \
              "0"(_num)                                         \
            : "memory", "cc"                                    \
            );                                                  \
        _ret;                                                   \
    })

#define linuxMakeSysCall4(num, arg1, arg2, arg3, arg4)              \
    ({                                                          \
        S64 _ret;                                              \
        register S64 _num __asm__("eax") = (num);              \
        register S64 _arg1 __asm__("ebx") = (S64)(arg1);      \
        register S64 _arg2 __asm__("ecx") = (S64)(arg2);      \
        register S64 _arg3 __asm__("edx") = (S64)(arg3);      \
        register S64 _arg4 __asm__("esi") = (S64)(arg4);      \
                                                                \
        __asm__ volatile (                                      \
            "int $0x80\n"                                       \
            : "=a" (_ret)                                       \
            : "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4),   \
              "0"(_num)                                         \
            : "memory", "cc"                                    \
            );                                                  \
        _ret;                                                   \
    })

#define linuxMakeSysCall5(num, arg1, arg2, arg3, arg4, arg5)                \
    ({                                                                  \
        S64 _ret;                                                      \
        register S64 _num __asm__("eax") = (num);                      \
        register S64 _arg1 __asm__("ebx") = (S64)(arg1);              \
        register S64 _arg2 __asm__("ecx") = (S64)(arg2);              \
        register S64 _arg3 __asm__("edx") = (S64)(arg3);              \
        register S64 _arg4 __asm__("esi") = (S64)(arg4);              \
        register S64 _arg5 __asm__("edi") = (S64)(arg5);              \
                                                                        \
        __asm__ volatile (                                              \
            "int $0x80\n"                                               \
            : "=a" (_ret)                                               \
            : "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4), "r"(_arg5), \
              "0"(_num)                                                 \
            : "memory", "cc"                                            \
            );                                                          \
        _ret;                                                           \
    })

/* startup code */
__asm__(".section .text\n"
        ".global _start\n"
        "_start:\n"
        "pop %eax\n"                // argc   (first arg, %eax)
        "mov %esp, %ebx\n"          // argv[] (second arg, %ebx)
        "lea 4(%ebx,%eax,4),%ecx\n" // then a NULL then envp (third arg, %ecx)
        "and $-16, %esp\n"          // x86 ABI : esp must be 16-byte aligned when
        "push %ecx\n"               // push all registers on the stack so that we
        "push %ebx\n"               // support both regparm and plain stack modes
        "push %eax\n"
        "call main\n"               // main() returns the status code in %eax
        "movzbl %al, %ebx\n"        // retrieve exit code from lower 8 bits
        "movl   $1, %eax\n"         // NR_exit == 1
        "int    $0x80\n"            // exit now
        "hlt\n"                     // ensure it does not
        "");

/* fcntl / open */
#define O_RDONLY            0
#define O_WRONLY            1
#define O_RDWR              2
#define O_CREAT          0x40
#define O_EXCL           0x80
#define O_NOCTTY        0x100
#define O_TRUNC         0x200
#define O_APPEND        0x400
#define O_NONBLOCK      0x800
#define O_DIRECTORY   0x10000

/* The struct returned by the stat() syscall, 32-bit only, the syscall returns
 * exactly 56 bytes (stops before the unused array).
 */
struct sys_stat_struct {
    U64  st_dev;
    U64  st_ino;
    unsigned short st_mode;
    unsigned short st_nlink;
    unsigned short st_uid;
    unsigned short st_gid;

    U64  st_rdev;
    U64  st_size;
    U64  st_blksize;
    U64  st_blocks;

    U64  st_atime;
    U64  st_atime_nsec;
    U64  st_mtime;
    U64  st_mtime_nsec;

    U64  st_ctime;
    U64  st_ctime_nsec;
    U64  __unused[2];
};

#elif defined(__ARM_EABI__)
/* Syscalls for ARM in ARM or Thumb modes :
 *   - registers are 32-bit
 *   - stack is 8-byte aligned
 *     ( http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka4127.html)
 *   - syscall number is passed in r7
 *   - arguments are in r0, r1, r2, r3, r4, r5
 *   - the system call is performed by calling svc #0
 *   - syscall return comes in r0.
 *   - only lr is clobbered.
 *   - the arguments are cast to S64 and assigned into the target registers
 *     which are then simply passed as registers to the asm code, so that we
 *     don't have to experience issues with register constraints.
 *   - the syscall number is always specified last in order to allow to force
 *     some registers before (gcc refuses a %-register at the last position).
 *
 * Also, ARM supports the old_select syscall if newselect is not available
 */
#define __ARCH_WANT_SYS_OLD_SELECT

#define linuxMakeSysCall0(num)                              \
    ({                                                  \
        register S64 _num __asm__("r7") = (num);       \
        register S64 _arg1 __asm__("r0");              \
                                                        \
        __asm__ volatile (                              \
            "svc #0\n"                                  \
            : "=r"(_arg1)                               \
            : "r"(_num)                                 \
            : "memory", "cc", "lr"                      \
            );                                          \
        _arg1;                                          \
    })

#define linuxMakeSysCall1(num, arg1)                                \
    ({                                                          \
        register S64 _num __asm__("r7") = (num);               \
        register S64 _arg1 __asm__("r0") = (S64)(arg1);       \
                                                                \
        __asm__ volatile (                                      \
            "svc #0\n"                                          \
            : "=r"(_arg1)                                       \
            : "r"(_arg1),                                       \
              "r"(_num)                                         \
            : "memory", "cc", "lr"                              \
            );                                                  \
        _arg1;                                                  \
    })

#define linuxMakeSysCall2(num, arg1, arg2)                          \
    ({                                                          \
        register S64 _num __asm__("r7") = (num);               \
        register S64 _arg1 __asm__("r0") = (S64)(arg1);       \
        register S64 _arg2 __asm__("r1") = (S64)(arg2);       \
                                                                \
        __asm__ volatile (                                      \
            "svc #0\n"                                          \
            : "=r"(_arg1)                                       \
            : "r"(_arg1), "r"(_arg2),                           \
              "r"(_num)                                         \
            : "memory", "cc", "lr"                              \
            );                                                  \
        _arg1;                                                  \
    })

#define linuxMakeSysCall3(num, arg1, arg2, arg3)                    \
    ({                                                          \
        register S64 _num __asm__("r7") = (num);               \
        register S64 _arg1 __asm__("r0") = (S64)(arg1);       \
        register S64 _arg2 __asm__("r1") = (S64)(arg2);       \
        register S64 _arg3 __asm__("r2") = (S64)(arg3);       \
                                                                \
        __asm__ volatile (                                      \
            "svc #0\n"                                          \
            : "=r"(_arg1)                                       \
            : "r"(_arg1), "r"(_arg2), "r"(_arg3),               \
              "r"(_num)                                         \
            : "memory", "cc", "lr"                              \
            );                                                  \
        _arg1;                                                  \
    })

#define linuxMakeSysCall4(num, arg1, arg2, arg3, arg4)              \
    ({                                                          \
        register S64 _num __asm__("r7") = (num);               \
        register S64 _arg1 __asm__("r0") = (S64)(arg1);       \
        register S64 _arg2 __asm__("r1") = (S64)(arg2);       \
        register S64 _arg3 __asm__("r2") = (S64)(arg3);       \
        register S64 _arg4 __asm__("r3") = (S64)(arg4);       \
                                                                \
        __asm__ volatile (                                      \
            "svc #0\n"                                          \
            : "=r"(_arg1)                                       \
            : "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4),   \
              "r"(_num)                                         \
            : "memory", "cc", "lr"                              \
            );                                                  \
        _arg1;                                                  \
    })

#define linuxMakeSysCall5(num, arg1, arg2, arg3, arg4, arg5)                \
    ({                                                                  \
        register S64 _num __asm__("r7") = (num);                       \
        register S64 _arg1 __asm__("r0") = (S64)(arg1);               \
        register S64 _arg2 __asm__("r1") = (S64)(arg2);               \
        register S64 _arg3 __asm__("r2") = (S64)(arg3);               \
        register S64 _arg4 __asm__("r3") = (S64)(arg4);               \
        register S64 _arg5 __asm__("r4") = (S64)(arg5);               \
                                                                        \
        __asm__ volatile (                                              \
            "svc #0\n"                                                  \
            : "=r" (_arg1)                                              \
            : "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4), "r"(_arg5), \
              "r"(_num)                                                 \
            : "memory", "cc", "lr"                                      \
            );                                                          \
        _arg1;                                                          \
    })

/* startup code */
__asm__(".section .text\n"
        ".global _start\n"
        "_start:\n"
#if defined(__THUMBEB__) || defined(__THUMBEL__)
        /* We enter here in 32-bit mode but if some previous functions were in
         * 16-bit mode, the assembler cannot know, so we need to tell it we're in
         * 32-bit now, then switch to 16-bit (is there a better way to do it than
         * adding 1 by hand ?) and tell the asm we're now in 16-bit mode so that
         * it generates correct instructions. Note that we do not support thumb1.
         */
        ".code 32\n"
        "add     r0, pc, #1\n"
        "bx      r0\n"
        ".code 16\n"
#endif
        "pop {%r0}\n"                 // argc was in the stack
        "mov %r1, %sp\n"              // argv = sp
        "add %r2, %r1, %r0, lsl #2\n" // envp = argv + 4*argc ...
        "add %r2, %r2, $4\n"          //        ... + 4
        "and %r3, %r1, $-8\n"         // AAPCS : sp must be 8-byte aligned in the
        "mov %sp, %r3\n"              //         callee, an bl doesn't push (lr=pc)
        "bl main\n"                   // main() returns the status code, we'll exit with it.
        "and %r0, %r0, $0xff\n"       // limit exit code to 8 bits
        "movs r7, $1\n"               // NR_exit == 1
        "svc $0x00\n"
        "");

/* fcntl / open */
#define O_RDONLY            0
#define O_WRONLY            1
#define O_RDWR              2
#define O_CREAT          0x40
#define O_EXCL           0x80
#define O_NOCTTY        0x100
#define O_TRUNC         0x200
#define O_APPEND        0x400
#define O_NONBLOCK      0x800
#define O_DIRECTORY    0x4000

/* The struct returned by the stat() syscall, 32-bit only, the syscall returns
 * exactly 56 bytes (stops before the unused array). In big endian, the format
 * differs as devices are returned as short only.
 */
struct sys_stat_struct {
#if defined(__ARMEB__)
    unsigned short st_dev;
    unsigned short __pad1;
#else
    U64  st_dev;
#endif
    U64  st_ino;
    unsigned short st_mode;
    unsigned short st_nlink;
    unsigned short st_uid;
    unsigned short st_gid;
#if defined(__ARMEB__)
    unsigned short st_rdev;
    unsigned short __pad2;
#else
    U64  st_rdev;
#endif
    U64  st_size;
    U64  st_blksize;
    U64  st_blocks;
    U64  st_atime;
    U64  st_atime_nsec;
    U64  st_mtime;
    U64  st_mtime_nsec;
    U64  st_ctime;
    U64  st_ctime_nsec;
    U64  __unused[2];
};

#elif defined(__aarch64__)
/* Syscalls for AARCH64 :
 *   - registers are 64-bit
 *   - stack is 16-byte aligned
 *   - syscall number is passed in x8
 *   - arguments are in x0, x1, x2, x3, x4, x5
 *   - the system call is performed by calling svc 0
 *   - syscall return comes in x0.
 *   - the arguments are cast to S64 and assigned into the target registers
 *     which are then simply passed as registers to the asm code, so that we
 *     don't have to experience issues with register constraints.
 *
 * On aarch64, select() is not implemented so we have to use pselect6().
 */
#define __ARCH_WANT_SYS_PSELECT6

#define linuxMakeSysCall0(num)                              \
    ({                                                  \
        register S64 _num  __asm__("x8") = (num);      \
        register S64 _arg1 __asm__("x0");              \
                                                        \
        __asm__ volatile (                              \
            "svc #0\n"                                  \
            : "=r"(_arg1)                               \
            : "r"(_num)                                 \
            : "memory", "cc"                            \
            );                                          \
        _arg1;                                          \
    })

#define linuxMakeSysCall1(num, arg1)                                \
    ({                                                          \
        register S64 _num  __asm__("x8") = (num);              \
        register S64 _arg1 __asm__("x0") = (S64)(arg1);       \
                                                                \
        __asm__ volatile (                                      \
            "svc #0\n"                                          \
            : "=r"(_arg1)                                       \
            : "r"(_arg1),                                       \
              "r"(_num)                                         \
            : "memory", "cc"                                    \
            );                                                  \
        _arg1;                                                  \
    })

#define linuxMakeSysCall2(num, arg1, arg2)                          \
    ({                                                          \
        register S64 _num  __asm__("x8") = (num);              \
        register S64 _arg1 __asm__("x0") = (S64)(arg1);       \
        register S64 _arg2 __asm__("x1") = (S64)(arg2);       \
                                                                \
        __asm__ volatile (                                      \
            "svc #0\n"                                          \
            : "=r"(_arg1)                                       \
            : "r"(_arg1), "r"(_arg2),                           \
              "r"(_num)                                         \
            : "memory", "cc"                                    \
            );                                                  \
        _arg1;                                                  \
    })

#define linuxMakeSysCall3(num, arg1, arg2, arg3)                    \
    ({                                                          \
        register S64 _num  __asm__("x8") = (num);              \
        register S64 _arg1 __asm__("x0") = (S64)(arg1);       \
        register S64 _arg2 __asm__("x1") = (S64)(arg2);       \
        register S64 _arg3 __asm__("x2") = (S64)(arg3);       \
                                                                \
        __asm__ volatile (                                      \
            "svc #0\n"                                          \
            : "=r"(_arg1)                                       \
            : "r"(_arg1), "r"(_arg2), "r"(_arg3),               \
              "r"(_num)                                         \
            : "memory", "cc"                                    \
            );                                                  \
        _arg1;                                                  \
    })

#define linuxMakeSysCall4(num, arg1, arg2, arg3, arg4)              \
    ({                                                          \
        register S64 _num  __asm__("x8") = (num);              \
        register S64 _arg1 __asm__("x0") = (S64)(arg1);       \
        register S64 _arg2 __asm__("x1") = (S64)(arg2);       \
        register S64 _arg3 __asm__("x2") = (S64)(arg3);       \
        register S64 _arg4 __asm__("x3") = (S64)(arg4);       \
                                                                \
        __asm__ volatile (                                      \
            "svc #0\n"                                          \
            : "=r"(_arg1)                                       \
            : "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4),   \
              "r"(_num)                                         \
            : "memory", "cc"                                    \
            );                                                  \
        _arg1;                                                  \
    })

#define linuxMakeSysCall5(num, arg1, arg2, arg3, arg4, arg5)                \
    ({                                                                  \
        register S64 _num  __asm__("x8") = (num);                      \
        register S64 _arg1 __asm__("x0") = (S64)(arg1);               \
        register S64 _arg2 __asm__("x1") = (S64)(arg2);               \
        register S64 _arg3 __asm__("x2") = (S64)(arg3);               \
        register S64 _arg4 __asm__("x3") = (S64)(arg4);               \
        register S64 _arg5 __asm__("x4") = (S64)(arg5);               \
                                                                        \
        __asm__ volatile (                                              \
            "svc #0\n"                                                  \
            : "=r" (_arg1)                                              \
            : "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4), "r"(_arg5), \
              "r"(_num)                                                 \
            : "memory", "cc"                                            \
            );                                                          \
        _arg1;                                                          \
    })

#define linuxMakeSysCall6(num, arg1, arg2, arg3, arg4, arg5, arg6)          \
    ({                                                                  \
        register S64 _num  __asm__("x8") = (num);                      \
        register S64 _arg1 __asm__("x0") = (S64)(arg1);               \
        register S64 _arg2 __asm__("x1") = (S64)(arg2);               \
        register S64 _arg3 __asm__("x2") = (S64)(arg3);               \
        register S64 _arg4 __asm__("x3") = (S64)(arg4);               \
        register S64 _arg5 __asm__("x4") = (S64)(arg5);               \
        register S64 _arg6 __asm__("x5") = (S64)(arg6);               \
                                                                        \
        __asm__ volatile (                                              \
            "svc #0\n"                                                  \
            : "=r" (_arg1)                                              \
            : "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4), "r"(_arg5), \
              "r"(_arg6), "r"(_num)                                     \
            : "memory", "cc"                                            \
            );                                                          \
        _arg1;                                                          \
    })

/* startup code */
__asm__(".section .text\n"
        ".global _start\n"
        "_start:\n"
        "ldr x0, [sp]\n"              // argc (x0) was in the stack
        "add x1, sp, 8\n"             // argv (x1) = sp
        "lsl x2, x0, 3\n"             // envp (x2) = 8*argc ...
        "add x2, x2, 8\n"             //           + 8 (skip null)
        "add x2, x2, x1\n"            //           + argv
        "and sp, x1, -16\n"           // sp must be 16-byte aligned in the callee
        "bl main\n"                   // main() returns the status code, we'll exit with it.
        "and x0, x0, 0xff\n"          // limit exit code to 8 bits
        "mov x8, 93\n"                // NR_exit == 93
        "svc #0\n"
        "");

/* fcntl / open */
#define O_RDONLY            0
#define O_WRONLY            1
#define O_RDWR              2
#define O_CREAT          0x40
#define O_EXCL           0x80
#define O_NOCTTY        0x100
#define O_TRUNC         0x200
#define O_APPEND        0x400
#define O_NONBLOCK      0x800
#define O_DIRECTORY    0x4000

/* The struct returned by the newfstatat() syscall. Differs slightly from the
 * x86_64's stat one by field ordering, so be careful.
 */
struct sys_stat_struct {
    U64   st_dev;
    U64   st_ino;
    U32    st_mode;
    U32    st_nlink;
    U32    st_uid;
    U32    st_gid;

    U64   st_rdev;
    U64   __pad1;
    S64            st_size;
    int             st_blksize;
    int             __pad2;

    S64            st_blocks;
    S64            st_atime;
    U64   st_atime_nsec;
    S64            st_mtime;

    U64   st_mtime_nsec;
    S64            st_ctime;
    U64   st_ctime_nsec;
    U32    __unused[2];
};

#elif defined(__mips__) && defined(_ABIO32)
/* Syscalls for MIPS ABI O32 :
 *   - WARNING! there's always a delayed slot!
 *   - WARNING again, the syntax is different, registers take a '$' and numbers
 *     do not.
 *   - registers are 32-bit
 *   - stack is 8-byte aligned
 *   - syscall number is passed in v0 (starts at 0xfa0).
 *   - arguments are in a0, a1, a2, a3, then the stack. The caller needs to
 *     leave some room in the stack for the callee to save a0..a3 if needed.
 *   - Many registers are clobbered, in fact only a0..a2 and s0..s8 are
 *     preserved. See: https://www.linux-mips.org/wiki/Syscall as well as
 *     scall32-o32.S in the kernel sources.
 *   - the system call is performed by calling "syscall"
 *   - syscall return comes in v0, and register a3 needs to be checked to know
 *     if an error occured, in which case errno is in v0.
 *   - the arguments are cast to S64 and assigned into the target registers
 *     which are then simply passed as registers to the asm code, so that we
 *     don't have to experience issues with register constraints.
 */

#define linuxMakeSysCall0(num)                                              \
    ({                                                                  \
        register S64 _num __asm__("v0") = (num);                       \
        register S64 _arg4 __asm__("a3");                              \
                                                                        \
        __asm__ volatile (                                              \
            "addiu $sp, $sp, -32\n"                                     \
            "syscall\n"                                                 \
            "addiu $sp, $sp, 32\n"                                      \
            : "=r"(_num), "=r"(_arg4)                                   \
            : "r"(_num)                                                 \
            : "memory", "cc", "at", "v1", "hi", "lo",                   \
              "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9" \
            );                                                          \
        _arg4 ? -_num : _num;                                           \
    })

#define linuxMakeSysCall1(num, arg1)                                        \
    ({                                                                  \
        register S64 _num __asm__("v0") = (num);                       \
        register S64 _arg1 __asm__("a0") = (S64)(arg1);               \
        register S64 _arg4 __asm__("a3");                              \
                                                                        \
        __asm__ volatile (                                              \
            "addiu $sp, $sp, -32\n"                                     \
            "syscall\n"                                                 \
            "addiu $sp, $sp, 32\n"                                      \
            : "=r"(_num), "=r"(_arg4)                                   \
            : "0"(_num),                                                \
              "r"(_arg1)                                                \
            : "memory", "cc", "at", "v1", "hi", "lo",                   \
              "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9" \
            );                                                          \
        _arg4 ? -_num : _num;                                           \
    })

#define linuxMakeSysCall2(num, arg1, arg2)                                  \
    ({                                                                  \
        register S64 _num __asm__("v0") = (num);                       \
        register S64 _arg1 __asm__("a0") = (S64)(arg1);               \
        register S64 _arg2 __asm__("a1") = (S64)(arg2);               \
        register S64 _arg4 __asm__("a3");                              \
                                                                        \
        __asm__ volatile (                                              \
            "addiu $sp, $sp, -32\n"                                     \
            "syscall\n"                                                 \
            "addiu $sp, $sp, 32\n"                                      \
            : "=r"(_num), "=r"(_arg4)                                   \
            : "0"(_num),                                                \
              "r"(_arg1), "r"(_arg2)                                    \
            : "memory", "cc", "at", "v1", "hi", "lo",                   \
              "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9" \
            );                                                          \
        _arg4 ? -_num : _num;                                           \
    })

#define linuxMakeSysCall3(num, arg1, arg2, arg3)                            \
    ({                                                                  \
        register S64 _num __asm__("v0")  = (num);                      \
        register S64 _arg1 __asm__("a0") = (S64)(arg1);               \
        register S64 _arg2 __asm__("a1") = (S64)(arg2);               \
        register S64 _arg3 __asm__("a2") = (S64)(arg3);               \
        register S64 _arg4 __asm__("a3");                              \
                                                                        \
        __asm__ volatile (                                              \
            "addiu $sp, $sp, -32\n"                                     \
            "syscall\n"                                                 \
            "addiu $sp, $sp, 32\n"                                      \
            : "=r"(_num), "=r"(_arg4)                                   \
            : "0"(_num),                                                \
              "r"(_arg1), "r"(_arg2), "r"(_arg3)                        \
            : "memory", "cc", "at", "v1", "hi", "lo",                   \
              "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9" \
            );                                                          \
        _arg4 ? -_num : _num;                                           \
    })

#define linuxMakeSysCall4(num, arg1, arg2, arg3, arg4)                      \
    ({                                                                  \
        register S64 _num __asm__("v0") = (num);                       \
        register S64 _arg1 __asm__("a0") = (S64)(arg1);               \
        register S64 _arg2 __asm__("a1") = (S64)(arg2);               \
        register S64 _arg3 __asm__("a2") = (S64)(arg3);               \
        register S64 _arg4 __asm__("a3") = (S64)(arg4);               \
                                                                        \
        __asm__ volatile (                                              \
            "addiu $sp, $sp, -32\n"                                     \
            "syscall\n"                                                 \
            "addiu $sp, $sp, 32\n"                                      \
            : "=r" (_num), "=r"(_arg4)                                  \
            : "0"(_num),                                                \
              "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4)            \
            : "memory", "cc", "at", "v1", "hi", "lo",                   \
              "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9" \
            );                                                          \
        _arg4 ? -_num : _num;                                           \
    })

#define linuxMakeSysCall5(num, arg1, arg2, arg3, arg4, arg5)                \
    ({                                                                  \
        register S64 _num __asm__("v0") = (num);                       \
        register S64 _arg1 __asm__("a0") = (S64)(arg1);               \
        register S64 _arg2 __asm__("a1") = (S64)(arg2);               \
        register S64 _arg3 __asm__("a2") = (S64)(arg3);               \
        register S64 _arg4 __asm__("a3") = (S64)(arg4);               \
        register S64 _arg5 = (S64)(arg5);                             \
                                                                        \
        __asm__ volatile (                                              \
            "addiu $sp, $sp, -32\n"                                     \
            "sw %7, 16($sp)\n"                                          \
            "syscall\n  "                                               \
            "addiu $sp, $sp, 32\n"                                      \
            : "=r" (_num), "=r"(_arg4)                                  \
            : "0"(_num),                                                \
              "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4), "r"(_arg5) \
            : "memory", "cc", "at", "v1", "hi", "lo",                   \
              "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9" \
            );                                                          \
        _arg4 ? -_num : _num;                                           \
    })

/* startup code, note that it's called __start on MIPS */
__asm__(".section .text\n"
        ".set nomips16\n"
        ".global __start\n"
        ".set    noreorder\n"
        ".option pic0\n"
        ".ent __start\n"
        "__start:\n"
        "lw $a0,($sp)\n"              // argc was in the stack
        "addiu  $a1, $sp, 4\n"        // argv = sp + 4
        "sll $a2, $a0, 2\n"           // a2 = argc * 4
        "add   $a2, $a2, $a1\n"       // envp = argv + 4*argc ...
        "addiu $a2, $a2, 4\n"         //        ... + 4
        "li $t0, -8\n"
        "and $sp, $sp, $t0\n"         // sp must be 8-byte aligned
        "addiu $sp,$sp,-16\n"         // the callee expects to save a0..a3 there!
        "jal main\n"                  // main() returns the status code, we'll exit with it.
        "nop\n"                       // delayed slot
        "and $a0, $v0, 0xff\n"        // limit exit code to 8 bits
        "li $v0, 4001\n"              // NR_exit == 4001
        "syscall\n"
        ".end __start\n"
        "");

/* fcntl / open */
#define O_RDONLY            0
#define O_WRONLY            1
#define O_RDWR              2
#define O_APPEND       0x0008
#define O_NONBLOCK     0x0080
#define O_CREAT        0x0100
#define O_TRUNC        0x0200
#define O_EXCL         0x0400
#define O_NOCTTY       0x0800
#define O_DIRECTORY   0x10000

/* The struct returned by the stat() syscall. 88 bytes are returned by the
 * syscall.
 */
struct sys_stat_struct {
    U32  st_dev;
    S64          st_pad1[3];
    U64 st_ino;
    U32  st_mode;
    U32  st_nlink;
    U32  st_uid;
    U32  st_gid;
    U32  st_rdev;
    S64          st_pad2[2];
    S64          st_size;
    S64          st_pad3;
    S64          st_atime;
    S64          st_atime_nsec;
    S64          st_mtime;
    S64          st_mtime_nsec;
    S64          st_ctime;
    S64          st_ctime_nsec;
    S64          st_blksize;
    S64          st_blocks;
    S64          st_pad4[14];
};

#elif defined(__riscv)

#if   __riscv_xlen == 64
#define PTRLOG "3"
#define SZREG  "8"
#elif __riscv_xlen == 32
#define PTRLOG "2"
#define SZREG  "4"
#endif

/* Syscalls for RISCV :
 *   - stack is 16-byte aligned
 *   - syscall number is passed in a7
 *   - arguments are in a0, a1, a2, a3, a4, a5
 *   - the system call is performed by calling ecall
 *   - syscall return comes in a0
 *   - the arguments are cast to S64 and assigned into the target
 *     registers which are then simply passed as registers to the asm code,
 *     so that we don't have to experience issues with register constraints.
 */

#define linuxMakeSysCall0(num)                              \
    ({                                                  \
        register S64 _num  __asm__("a7") = (num);      \
        register S64 _arg1 __asm__("a0");              \
                                                        \
        __asm__ volatile (                              \
            "ecall\n\t"                                 \
            : "=r"(_arg1)                               \
            : "r"(_num)                                 \
            : "memory", "cc"                            \
            );                                          \
        _arg1;                                          \
    })

#define linuxMakeSysCall1(num, arg1)                                \
    ({                                                          \
        register S64 _num  __asm__("a7") = (num);              \
        register S64 _arg1 __asm__("a0") = (S64)(arg1);       \
                                                                \
        __asm__ volatile (                                      \
            "ecall\n"                                           \
            : "+r"(_arg1)                                       \
            : "r"(_num)                                         \
            : "memory", "cc"                                    \
            );                                                  \
        _arg1;                                                  \
    })

#define linuxMakeSysCall2(num, arg1, arg2)                          \
    ({                                                          \
        register S64 _num  __asm__("a7") = (num);              \
        register S64 _arg1 __asm__("a0") = (S64)(arg1);       \
        register S64 _arg2 __asm__("a1") = (S64)(arg2);       \
                                                                \
        __asm__ volatile (                                      \
            "ecall\n"                                           \
            : "+r"(_arg1)                                       \
            : "r"(_arg2),                                       \
              "r"(_num)                                         \
            : "memory", "cc"                                    \
            );                                                  \
        _arg1;                                                  \
    })

#define linuxMakeSysCall3(num, arg1, arg2, arg3)                    \
    ({                                                          \
        register S64 _num  __asm__("a7") = (num);              \
        register S64 _arg1 __asm__("a0") = (S64)(arg1);       \
        register S64 _arg2 __asm__("a1") = (S64)(arg2);       \
        register S64 _arg3 __asm__("a2") = (S64)(arg3);       \
                                                                \
        __asm__ volatile (                                      \
            "ecall\n\t"                                         \
            : "+r"(_arg1)                                       \
            : "r"(_arg2), "r"(_arg3),                           \
              "r"(_num)                                         \
            : "memory", "cc"                                    \
            );                                                  \
        _arg1;                                                  \
    })

#define linuxMakeSysCall4(num, arg1, arg2, arg3, arg4)              \
    ({                                                          \
        register S64 _num  __asm__("a7") = (num);              \
        register S64 _arg1 __asm__("a0") = (S64)(arg1);       \
        register S64 _arg2 __asm__("a1") = (S64)(arg2);       \
        register S64 _arg3 __asm__("a2") = (S64)(arg3);       \
        register S64 _arg4 __asm__("a3") = (S64)(arg4);       \
                                                                \
        __asm__ volatile (                                      \
            "ecall\n"                                           \
            : "+r"(_arg1)                                       \
            : "r"(_arg2), "r"(_arg3), "r"(_arg4),               \
              "r"(_num)                                         \
            : "memory", "cc"                                    \
            );                                                  \
        _arg1;                                                  \
    })

#define linuxMakeSysCall5(num, arg1, arg2, arg3, arg4, arg5)        \
    ({                                                          \
        register S64 _num  __asm__("a7") = (num);              \
        register S64 _arg1 __asm__("a0") = (S64)(arg1);       \
        register S64 _arg2 __asm__("a1") = (S64)(arg2);       \
        register S64 _arg3 __asm__("a2") = (S64)(arg3);       \
        register S64 _arg4 __asm__("a3") = (S64)(arg4);       \
        register S64 _arg5 __asm__("a4") = (S64)(arg5);       \
                                                                \
        __asm__ volatile (                                      \
            "ecall\n"                                           \
            : "+r"(_arg1)                                       \
            : "r"(_arg2), "r"(_arg3), "r"(_arg4), "r"(_arg5),   \
              "r"(_num)                                         \
            : "memory", "cc"                                    \
            );                                                  \
        _arg1;                                                  \
    })

#define linuxMakeSysCall6(num, arg1, arg2, arg3, arg4, arg5, arg6)          \
    ({                                                                  \
        register S64 _num  __asm__("a7") = (num);                      \
        register S64 _arg1 __asm__("a0") = (S64)(arg1);               \
        register S64 _arg2 __asm__("a1") = (S64)(arg2);               \
        register S64 _arg3 __asm__("a2") = (S64)(arg3);               \
        register S64 _arg4 __asm__("a3") = (S64)(arg4);               \
        register S64 _arg5 __asm__("a4") = (S64)(arg5);               \
        register S64 _arg6 __asm__("a5") = (S64)(arg6);               \
                                                                        \
        __asm__ volatile (                                              \
            "ecall\n"                                                   \
            : "+r"(_arg1)                                               \
            : "r"(_arg2), "r"(_arg3), "r"(_arg4), "r"(_arg5), "r"(_arg6), \
              "r"(_num)                                                 \
            : "memory", "cc"                                            \
            );                                                          \
        _arg1;                                                          \
    })

/* startup code */
__asm__(".section .text\n"
        ".global _start\n"
        "_start:\n"
        ".option push\n"
        ".option norelax\n"
        "lla   gp, __global_pointer$\n"
        ".option pop\n"
        "ld    a0, 0(sp)\n"          // argc (a0) was in the stack
        "add   a1, sp, "SZREG"\n"    // argv (a1) = sp
        "slli  a2, a0, "PTRLOG"\n"   // envp (a2) = SZREG*argc ...
        "add   a2, a2, "SZREG"\n"    //             + SZREG (skip null)
        "add   a2,a2,a1\n"           //             + argv
        "andi  sp,a1,-16\n"          // sp must be 16-byte aligned
        "call  main\n"               // main() returns the status code, we'll exit with it.
        "andi  a0, a0, 0xff\n"       // limit exit code to 8 bits
        "li a7, 93\n"                // NR_exit == 93
        "ecall\n"
        "");

/* fcntl / open */
#define O_RDONLY            0
#define O_WRONLY            1
#define O_RDWR              2
#define O_CREAT         0x100
#define O_EXCL          0x200
#define O_NOCTTY        0x400
#define O_TRUNC        0x1000
#define O_APPEND       0x2000
#define O_NONBLOCK     0x4000
#define O_DIRECTORY  0x200000

struct sys_stat_struct {
    U64   st_dev;         /* Device.  */
    U64   st_ino;         /* File serial number.  */
    U32    st_mode;        /* File mode.  */
    U32    st_nlink;       /* Link count.  */
    U32    st_uid;         /* User ID of the file's owner.  */
    U32    st_gid;         /* Group ID of the file's group. */
    U64   st_rdev;        /* Device number, if device.  */
    U64   __pad1;
    S64            st_size;        /* Size of file, in bytes.  */
    int             st_blksize;     /* Optimal block size for I/O.  */
    int             __pad2;
    S64            st_blocks;      /* Number 512-byte blocks allocated. */
    S64            st_atime;       /* Time of last access.  */
    U64   st_atime_nsec;
    S64            st_mtime;       /* Time of last modification.  */
    U64   st_mtime_nsec;
    S64            st_ctime;       /* Time of last status change.  */
    U64   st_ctime_nsec;
    U32    __unused4;
    U32    __unused5;
};

#endif

/* Below are the C functions used to declare the raw syscalls. They try to be
 * architecture-agnostic, and return either a success or -errno. Declaring them
 * static will lead to them being inlined in most cases, but it's still possible
 * to reference them by a pointer if needed.
 */
header_function
void* linuxSysBrk (void *addr)
{
    void *result = (void*)(Uptr)linuxMakeSysCall1(__NR_brk, addr);
    return result;
}

header_function
noreturn
void linuxSysExit (S32 status)
{
    linuxMakeSysCall1(__NR_exit, status & 255);
    while (true); // Warning: function declared 'noreturn' should not return
}

header_function
S32 linuxSysChdir (Char *path)
{
    S32 result = (S32)linuxMakeSysCall1(__NR_chdir, path);
    return result;
}

header_function
S32 linuxSysChmod (Char *path, mode_t mode)
{
    S32 result;
#if defined(__NR_fchmodat)
    result = (S32)linuxMakeSysCall4(__NR_fchmodat, AT_FDCWD, path, mode, 0);
#else
    result = (S32)linuxMakeSysCall2(__NR_chmod, path, mode);
#endif

    return result;
}

header_function
S32 linuxSysChown (Char *path, uid_t owner, gid_t group)
{
    S32 result;
#if defined(__NR_fchownat)
    result = (S32)linuxMakeSysCall5(__NR_fchownat, AT_FDCWD, path, owner, group, 0);
#else
    result = (S32)linuxMakeSysCall3(__NR_chown, path, owner, group);
#endif
    return result;
}

header_function
S32 linuxSysChroot (Char *path)
{
    S32 result = (S32)linuxMakeSysCall1(__NR_chroot, path);
    return result;
}

header_function
S32 linuxSysClose (S32 fd)
{
    S32 result = (S32)linuxMakeSysCall1(__NR_close, fd);
    return result;
}

header_function
S32 linuxSysDup (S32 fd)
{
    S32 result = (S32)linuxMakeSysCall1(__NR_dup, fd);
    return result;
}

header_function
S32 linuxSysDup2 (S32 old, S32 new)
{
    S32 result;
#ifdef __NR_dup3
    result = (S32)linuxMakeSysCall3(__NR_dup3, old, new, 0);
#else
    result = (S32)linuxMakeSysCall2(__NR_dup2, old, new);
#endif
    return result;
}

#ifdef __NR_dup3
header_function
S32 linuxSysDup3 (S32 old, S32 new, S32 flags)
{
    S32 result = (S32)linuxMakeSysCall3(__NR_dup3, old, new, flags);
    return result;
}
#endif

header_function
S32 linuxSysExecve (Char *filename, Char *argv[], Char *envp[])
{
    S32 result = (S32)linuxMakeSysCall3(__NR_execve, filename, argv, envp);
    return result;
}

header_function
pid_t linuxSysFork (void)
{
    pid_t result;
#ifdef __NR_clone
    /* note: some archs only have clone() and not fork(). Different archs
     * have a different API, but most archs have the flags on first arg and
     * will not use the rest with no other flag.
     */
    result = (pid_t)linuxMakeSysCall5(__NR_clone, SIGCHLD, 0, 0, 0, 0);
#else
    result = (pid_t)linuxMakeSysCall0(__NR_fork);
#endif
    return result;
}

header_function
S32 linuxSysFsync (S32 fd)
{
    S32 result = (S32)linuxMakeSysCall1(__NR_fsync, fd);
    return result;
}

header_function
S32 linuxSysGetDEnts64 (S32 fd, struct dirent64 *dirp, S32 count)
{
    S32 result = (S32)linuxMakeSysCall3(__NR_getdents64, fd, dirp, count);
    return result;
}

header_function
pid_t linuxSysGetPGID (pid_t pid)
{
    pid_t result = (pid_t)linuxMakeSysCall1(__NR_getpgid, pid);
    return result;
}

header_function
pid_t linuxSysGetpgrp (void)
{
    pid_t result = (pid_t)linuxSysGetPGID(0);
    return result;
}

header_function
pid_t linuxSysGetPID(void)
{
    pid_t result = (pid_t)linuxMakeSysCall0(__NR_getpid);
    return result;
}

header_function
S32 linuxSysGetTimeOfDay(struct timeval *tv, struct timezone *tz)
{
    S32 result = (S32)linuxMakeSysCall2(__NR_gettimeofday, tv, tz);
    return result;
}

header_function
S32 linuxSysIoctl (S32 fd, U64 req, void *value)
{
    S32 result = (S32)linuxMakeSysCall3(__NR_ioctl, fd, req, value);
    return result;
}

header_function
S32 linuxSysKill (pid_t pid, S32 signal)
{
    S32 result = (S32)linuxMakeSysCall2(__NR_kill, pid, signal);
    return result;
}

header_function
S32 linuxSysLink (Char *old, Char *new)
{
    S32 result;
#ifdef __NR_linkat
    result = (S32)linuxMakeSysCall5(__NR_linkat, AT_FDCWD, old, AT_FDCWD, new, 0);
#else
    result = (S32)linuxMakeSysCall2(__NR_link, old, new);
#endif
    return result;
}

header_function
offset_t linuxSysLseek (S32 fd, offset_t offset, S32 whence)
{
    S32 result = (S32)linuxMakeSysCall3(__NR_lseek, fd, offset, whence);
    return result;
}

header_function
S32 linuxSysMkdir (Char *path, mode_t mode)
{
    S32 result;
#ifdef __NR_mkdirat
    result = (S32)linuxMakeSysCall3(__NR_mkdirat, AT_FDCWD, path, mode);
#else
    result = (S32)linuxMakeSysCall2(__NR_mkdir, path, mode);
#endif
    return result;
}

header_function
S64 linuxSysMknod (Char *path, mode_t mode, dev_t dev)
{
    S64 result;
#ifdef __NR_mknodat
    result = linuxMakeSysCall4(__NR_mknodat, AT_FDCWD, path, mode, dev);
#else
    result = linuxMakeSysCall3(__NR_mknod, path, mode, dev);
#endif
    return result;
}

header_function
Uptr linuxSysMMap (void *start, Size length, S32 protection, S32 flags, S32 fd, offset_t offset)
{
    Uptr result;
#ifdef __NR_mmap2
    // NOTE(naman): This is always 4 KiB by spec (see mmap2(2))
    result = (Uptr)linuxMakeSysCall6(__NR_mmap2, start, length,
                                     protection, flags, fd, offset/KiB(4));
#else
    result = (Uptr)linuxMakeSysCall6(__NR_mmap, start, length, protection, flags, fd, offset);
#endif
    return result;
}

header_function
S32 linuxSysMUnmap (void *start, Size length)
{
    S32 result = (S32)linuxMakeSysCall2(__NR_munmap, start, length);
    return result;
}


header_function
S32 linuxSysMount (Char *src, Char *tgt, Char *fst,
                   U64 flags, void *data)
{
    S32 result = (S32)linuxMakeSysCall5(__NR_mount, src, tgt, fst, flags, data);
    return result;
}

header_function
S32 linuxSysOpen(Char *path, S32 flags, mode_t mode)
{
    S32 result;
#ifdef __NR_openat
    result = (S32)linuxMakeSysCall4(__NR_openat, AT_FDCWD, path, flags, mode);
#else
    result = (S32)linuxMakeSysCall3(__NR_open, path, flags, mode);
#endif
    return result;
}

header_function
S32 linuxSysPivotRoot(Char *new, Char *old)
{
    S32 result = (S32)linuxMakeSysCall2(__NR_pivot_root, new, old);
    return result;
}

header_function
S32 linuxSysPoll(pollfd *fds, S32 nfds, S32 timeout)
{
    S32 result;
#if defined(__NR_ppoll)
    struct timespec t;

    if (timeout >= 0) {
        t.tv_sec  = timeout / 1000;
        t.tv_nsec = (timeout % 1000) * 1000000;
    }
    result = (S32)linuxMakeSysCall4(__NR_ppoll, fds, nfds, (timeout >= 0) ? &t : NULL, NULL);
#else
    result = (S32)linuxMakeSysCall3(__NR_poll, fds, nfds, timeout);
#endif
    return result;
}

header_function
ssize_t linuxSysRead(S32 fd, void *buf, Size count)
{
    ssize_t result = linuxMakeSysCall3(__NR_read, fd, buf, count);
    return result;
}

header_function
ssize_t linuxSysReboot (S32 magic1, S32 magic2, S32 cmd, void *arg)
{
    ssize_t result = linuxMakeSysCall4(__NR_reboot, magic1, magic2, cmd, arg);
    return result;
}

header_function
S32 linuxSysSchedYield (void)
{
    S32 result = (S32)linuxMakeSysCall0(__NR_sched_yield);
    return result;
}

header_function
S32 linuxSysSelect (S32 nfds, fd_set *rfds, fd_set *wfds, fd_set *efds, timeval *timeout)
{
    S32 result;

#if defined(__ARCH_WANT_SYS_OLD_SELECT) && !defined(__NR__newselect)
    struct sel_arg_struct {
        U64 n;
        fd_set *r, *w, *e;
        struct timeval *t;
    } arg = { .n = nfds, .r = rfds, .w = wfds, .e = efds, .t = timeout };
    result = linuxMakeSysCall1(__NR_select, &arg);
#elif defined(__ARCH_WANT_SYS_PSELECT6) && defined(__NR_pselect6)
    struct timespec t;

    if (timeout) {
        t.tv_sec  = timeout->tv_sec;
        t.tv_nsec = timeout->tv_usec * 1000;
    }
    result = linuxMakeSysCall6(__NR_pselect6, nfds, rfds, wfds, efds, timeout ? &t : NULL, NULL);
#else
#ifndef __NR__newselect
#define __NR__newselect __NR_select
#endif
    result = (S32)linuxMakeSysCall5(__NR__newselect, nfds, rfds, wfds, efds, timeout);
#endif

    return result;
}

header_function
S32 linuxSysSetPGID (pid_t pid, pid_t pgid)
{
    S32 result = (S32)linuxMakeSysCall2(__NR_setpgid, pid, pgid);
    return result;
}

header_function
pid_t linuxSysSetSID (void)
{
    pid_t result = (pid_t)linuxMakeSysCall0(__NR_setsid);
    return result;
}

header_function
S32 linuxSysStat (Char *path, struct stat *buf)
{
    struct sys_stat_struct stat;
    S32 result;

#ifdef __NR_newfstatat
    /* only solution for arm64 */
    result = (S32)linuxMakeSysCall4(__NR_newfstatat, AT_FDCWD, path, &stat, 0);
#else
    result = (S32)linuxMakeSysCall2(__NR_stat, path, &stat);
#endif
    buf->st_dev     = (dev_t)stat.st_dev;
    buf->st_ino     = stat.st_ino;
    buf->st_mode    = stat.st_mode;
    buf->st_nlink   = stat.st_nlink;
    buf->st_uid     = stat.st_uid;
    buf->st_gid     = stat.st_gid;
    buf->st_rdev    = (dev_t)stat.st_rdev;
    buf->st_size    = stat.st_size;
    buf->st_blksize = stat.st_blksize;
    buf->st_blocks  = stat.st_blocks;
    buf->st_atime   = (time_t)stat.st_atime;
    buf->st_mtime   = (time_t)stat.st_mtime;
    buf->st_ctime   = (time_t)stat.st_ctime;

    return result;
}


header_function
S32 linuxSysSymlink (Char *old, Char *new)
{
    S32 result;
#ifdef __NR_symlinkat
    result = (S32)linuxMakeSysCall3(__NR_symlinkat, old, AT_FDCWD, new);
#else
    result = (S32)linuxMakeSysCall2(__NR_symlink, old, new);
#endif
    return result;
}

header_function
mode_t linuxSysUmask (mode_t mode)
{
    mode_t result = (mode_t)linuxMakeSysCall1(__NR_umask, mode);
    return result;
}

header_function
S32 linuxSysUmount2 (Char *path, S32 flags)
{
    S32 result = (S32)linuxMakeSysCall2(__NR_umount2, path, flags);
    return result;
}

header_function
S32 linuxSysUnlink (Char *path)
{
    S32 result;
#ifdef __NR_unlinkat
    result = (S32)linuxMakeSysCall3(__NR_unlinkat, AT_FDCWD, path, 0);
#else
    result = (S32)linuxMakeSysCall1(__NR_unlink, path);
#endif
    return result;
}

header_function
pid_t linuxSysWait4 (pid_t pid, S32 *status, S32 options, rusage *rusage)
{
    pid_t result = (pid_t)linuxMakeSysCall4(__NR_wait4, pid, status, options, rusage);
    return result;
}

header_function
pid_t linuxSysWaitPID (pid_t pid, S32 *status, S32 options)
{
    pid_t result = linuxSysWait4(pid, status, options, 0);
    return result;
}

header_function
pid_t linuxSysWait (S32 *status)
{
    pid_t result = (pid_t)linuxSysWaitPID(-1, status, 0);
    return result;
}

header_function
ssize_t linuxSysWrite (S32 fd, void *buf, Size count)
{
    ssize_t result = linuxMakeSysCall3(__NR_write, fd, buf, count);
    return result;
}


/* Below are the libc-compatible syscalls which return x or -1 and set errno.
 * They rely on the functions above. Similarly they're marked static so that it
 * is possible to assign pointers to them if needed.
 */

header_function
S32 brk (void *addr)
{
    void *ret = linuxSysBrk(addr);

    if (!ret) {
        errno = ENOMEM;
        return -1;
    }

    return 0;
}

header_function
noreturn
void exit (S32 status)
{
    linuxSysExit(status);
}

header_function
S32 chdir(Char *path)
{
    S32 ret = linuxSysChdir(path);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
S32 chmod(Char *path, mode_t mode)
{
    S32 ret = linuxSysChmod(path, mode);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
S32 chown(Char *path, uid_t owner, gid_t group)
{
    S32 ret = linuxSysChown(path, owner, group);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
S32 chroot(Char *path)
{
    S32 ret = linuxSysChroot(path);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
S32 close(S32 fd)
{
    S32 ret = linuxSysClose(fd);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

static __attribute__((unused))
S32 dup(S32 fd)
{
    S32 ret = linuxSysDup(fd);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
S32 dup2(S32 old, S32 new)
{
    S32 ret = linuxSysDup2(old, new);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

#ifdef __NR_dup3
static __attribute__((unused))
S32 dup3(S32 old, S32 new, S32 flags)
{
    S32 ret = linuxSysDup3(old, new, flags);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}
#endif

header_function
S32 execve(Char *filename, Char *argv[], Char *envp[])
{
    S32 ret = linuxSysExecve(filename, argv, envp);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
pid_t fork(void)
{
    pid_t ret = linuxSysFork();

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
S32 fsync(S32 fd)
{
    S32 ret = linuxSysFsync(fd);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
S32 getdents64(S32 fd, struct dirent64 *dirp, S32 count)
{
    S32 ret = linuxSysGetDEnts64(fd, dirp, count);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

static __attribute__((unused))
pid_t getpgid(pid_t pid)
{
    pid_t ret = linuxSysGetPGID(pid);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
pid_t getpgrp(void)
{
    pid_t ret = linuxSysGetpgrp();

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
pid_t getpid(void)
{
    pid_t ret = linuxSysGetPID();

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
S32 gettimeofday(struct timeval *tv, struct timezone *tz)
{
    S32 ret = linuxSysGetTimeOfDay(tv, tz);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
S32 ioctl(S32 fd, U64 req, void *value)
{
    S32 ret = linuxSysIoctl(fd, req, value);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
S32 kill(pid_t pid, S32 signal)
{
    S32 ret = linuxSysKill(pid, signal);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
S32 link(Char *old, Char *new)
{
    S32 ret = linuxSysLink(old, new);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
offset_t lseek(S32 fd, offset_t offset, S32 whence)
{
    offset_t ret = linuxSysLseek(fd, offset, whence);

    if (ret < 0) {
        errno = -(Sint)ret;
        ret = -1;
    }
    return ret;
}

header_function
S32 mkdir(Char *path, mode_t mode)
{
    S32 ret = linuxSysMkdir(path, mode);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
S32 mknod(Char *path, mode_t mode, dev_t dev)
{
    S32 ret = (S32)linuxSysMknod(path, mode, dev);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

#define MAP_FAILED ((void *) -1)

#define MAP_SHARED     0x01
#define MAP_PRIVATE    0x02
#define MAP_SHARED_VALIDATE 0x03
#define MAP_TYPE       0x0f
#define MAP_FIXED      0x10
#define MAP_ANON       0x20
#define MAP_ANONYMOUS  MAP_ANON
#define MAP_NORESERVE  0x4000
#define MAP_GROWSDOWN  0x0100
#define MAP_DENYWRITE  0x0800
#define MAP_EXECUTABLE 0x1000
#define MAP_LOCKED     0x2000
#define MAP_POPULATE   0x8000
#define MAP_NONBLOCK   0x10000
#define MAP_STACK      0x20000
#define MAP_HUGETLB    0x40000
#define MAP_SYNC       0x80000
#define MAP_FIXED_NOREPLACE 0x100000
#define MAP_FILE       0

#define MAP_HUGE_SHIFT 26
#define MAP_HUGE_MASK  0x3f
#define MAP_HUGE_64KB  (16 << 26)
#define MAP_HUGE_512KB (19 << 26)
#define MAP_HUGE_1MB   (20 << 26)
#define MAP_HUGE_2MB   (21 << 26)
#define MAP_HUGE_8MB   (23 << 26)
#define MAP_HUGE_16MB  (24 << 26)
#define MAP_HUGE_32MB  (25 << 26)
#define MAP_HUGE_256MB (28 << 26)
#define MAP_HUGE_512MB (29 << 26)
#define MAP_HUGE_1GB   (30 << 26)
#define MAP_HUGE_2GB   (31 << 26)
#define MAP_HUGE_16GB  (34U << 26)

#define PROT_NONE      0
#define PROT_READ      1
#define PROT_WRITE     2
#define PROT_EXEC      4
#define PROT_GROWSDOWN 0x01000000
#define PROT_GROWSUP   0x02000000

#define MS_ASYNC       1
#define MS_INVALIDATE  2
#define MS_SYNC        4

header_function
void* mmap(void *start, Size length, S32 protection, S32 flags, S32 fd, offset_t offset)
{
    if ((U64)offset & (KiB(4) - 1)) {
        errno = EINVAL;
        return MAP_FAILED;
    }

    if (length >= PTRDIFF_MAX) {
        errno = ENOMEM;
        return MAP_FAILED;
    }

    Uptr result = linuxSysMMap(start, length, protection, flags, fd, offset);

    return (void *)result;
}

header_function
S32 munmap (void *start, size_t length)
{
    S32 result = linuxSysMUnmap(start, length);
    return result;
}

header_function
S32 mount(Char *src, Char *tgt,
          Char *fst, U64 flags,
          void *data)
{
    S32 ret = linuxSysMount(src, tgt, fst, flags, data);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
S32 open(Char *path, S32 flags, mode_t mode)
{
    S32 ret = linuxSysOpen(path, flags, mode);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
S32 pivot_root(Char *new, Char *old)
{
    S32 ret = linuxSysPivotRoot(new, old);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
S32 poll(pollfd *fds, S32 nfds, S32 timeout)
{
    S32 ret = linuxSysPoll(fds, nfds, timeout);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
ssize_t read(S32 fd, void *buf, Size count)
{
    ssize_t ret = linuxSysRead(fd, buf, count);

    if (ret < 0) {
        errno = -(S32)ret;
        ret = -1;
    }
    return ret;
}

header_function
S32 reboot(S32 cmd)
{
    S32 ret = (S32)linuxSysReboot((S32)LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, cmd, 0);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
void *sbrk(intptr_t inc)
{
    void *ret;

    /* first call to find current end */
    if ((ret = linuxSysBrk(0)) && (linuxSysBrk((Char*)ret + inc) == (Char*)ret + inc))
        return (Char*)ret + inc;

    errno = ENOMEM;
    return (void *)-1;
}

header_function
S32 sched_yield(void)
{
    S32 ret = linuxSysSchedYield();

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
S32 select(S32 nfds, fd_set *rfds, fd_set *wfds, fd_set *efds, timeval *timeout)
{
    S32 ret = linuxSysSelect(nfds, rfds, wfds, efds, timeout);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
S32 setpgid(pid_t pid, pid_t pgid)
{
    S32 ret = linuxSysSetPGID(pid, pgid);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
pid_t setsid(void)
{
    pid_t ret = linuxSysSetSID();

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
U32 sleep(U32 seconds)
{
    timeval time_value = { seconds, 0 };

    if (linuxSysSelect(0, 0, 0, 0, &time_value) < 0)
        return (U32)time_value.tv_sec + !!time_value.tv_usec;
    else
        return 0;
}

header_function
S32 stat(Char *path, struct stat *buf)
{
    S32 ret = linuxSysStat(path, buf);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
S32 symlink(Char *old, Char *new)
{
    S32 ret = linuxSysSymlink(old, new);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
S32 tcsetpgrp(S32 fd, pid_t pid)
{
    return ioctl(fd, TIOCSPGRP, &pid);
}

header_function
mode_t umask(mode_t mode)
{
    return linuxSysUmask(mode);
}

header_function
S32 umount2(Char *path, S32 flags)
{
    S32 ret = linuxSysUmount2(path, flags);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
S32 unlink(Char *path)
{
    S32 ret = linuxSysUnlink(path);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
pid_t wait4(pid_t pid, S32 *status, S32 options, rusage *rusage)
{
    pid_t ret = linuxSysWait4(pid, status, options, rusage);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
pid_t waitpid(pid_t pid, S32 *status, S32 options)
{
    pid_t ret = linuxSysWaitPID(pid, status, options);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
pid_t wait(S32 *status)
{
    pid_t ret = linuxSysWait(status);

    if (ret < 0) {
        errno = -ret;
        ret = -1;
    }
    return ret;
}

header_function
ssize_t write(S32 fd, void *buf, size_t count)
{
    ssize_t ret = linuxSysWrite(fd, buf, count);

    if (ret < 0) {
        errno = -(S32)ret;
        ret = -1;
    }
    return ret;
}

/* needed by libgcc for divide by zero */
header_function
S32 raise(S32 signal)
{
    return kill(getpid(), signal);
}

/* Here come a few helper functions */

header_function
void FD_SET(S32 fd, fd_set *set)
{
    if (fd < 0 || fd >= FD_SETSIZE) {
        return;
    } else {
        set->fd32[fd / 32] |= 1 << (fd & 31);
        return;
    }
}

Sint main (Sint, Char *[]);

#define NLIB_LINUX_H_INCLUDE_GUARD
#endif
