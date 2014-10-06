/* Support files for GNU libc. */

#include <errno.h>
#include <sys/stat.h>

void * __dso_handle;

static char * heap_end = 0;

caddr_t _sbrk(int incr) {
    extern char __heap_start; /* Defined by the linker */
    extern char __heap_end; /* Defined by the linker */
    char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &__heap_start;
    }
    prev_heap_end = heap_end;

    if (heap_end + incr > &__heap_end) {
        /* Heap and stack collision */
        return (caddr_t) 0;
    }

    heap_end += incr;

    return (caddr_t) prev_heap_end;
}

int _fstat(int file, struct stat *st) {
    return 0;
}

int _isatty(int file) {
    return 1;
}

int _open(const char *name, int flags, int mode) {
    return -1;
}

int _read(int file, char *ptr, int len) {
    return -1;
}

int _write(int file, char *ptr, int len) {
    return len;
}

int _lseek(int file, int ptr, int dir) {
    return 0;
}

int _close(int file) {
    return -1;
}

void _exit(int status) {
    while(1);
}

int _getpid(int n) {
    return 1;
}

int _kill(int pid, int sig) {
    while(1);
    return 0;
}

int _unlink (const char *path) {
    return -1;
}

int _system (const char *s) {
    if (s == NULL) {
        return 0;
    }
    errno = ENOSYS;
    return -1;
}

int _rename (const char * oldpath, const char * newpath) {
    errno = ENOSYS;
    return -1;
}

void _fini() {
    while(1)
        ;
}
