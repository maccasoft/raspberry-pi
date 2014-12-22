/*
 * Copyright (c) 2014 Marco Maccaferri and Others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "emmc.h"
#include "fatfs/ff.h"

#ifdef HAVE_USPI
#include "uspi.h"
#endif

/* Definitions of physical drive number for each drive */
#define MMC     0   /* Map MMC/SD card to drive number 0 */
#define USB     1   /* Map USB drive to drive number 1 */

#define MAX_DESCRIPTORS         64
#define RESERVED_DESCRIPTORS    3

static FATFS fat_fs[_VOLUMES];
static FIL *file_descriptors[MAX_DESCRIPTORS];

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

static int FRESULT_to_errno(FRESULT rc) {
    switch(rc) {
        case FR_OK:                  /* (0) Succeeded */
            return 0;
        case FR_NO_FILE:             /* (4) Could not find the file */
        case FR_NO_PATH:             /* (5) Could not find the path */
            return ENOENT;
        case FR_INVALID_NAME:        /* (6) The path name format is invalid */
        case FR_MKFS_ABORTED:        /* (14) The f_mkfs() aborted due to any parameter error */
        case FR_INVALID_PARAMETER:   /* (19) Given parameter is invalid */
            return EINVAL;
        case FR_WRITE_PROTECTED:     /* (10) The physical drive is write protected */
            return EROFS;
        case FR_DISK_ERR:            /* (1) A hard error occurred in the low level disk I/O layer */
        case FR_INT_ERR:             /* (2) Assertion failed */
        case FR_NOT_READY:           /* (3) The physical drive cannot work */
        case FR_DENIED:              /* (7) Access denied due to prohibited access or directory full */
        case FR_EXIST:               /* (8) Access denied due to prohibited access */
        case FR_INVALID_OBJECT:      /* (9) The file/directory object is invalid */
        case FR_INVALID_DRIVE:       /* (11) The logical drive number is invalid */
        case FR_NOT_ENABLED:         /* (12) The volume has no work area */
        case FR_NO_FILESYSTEM:       /* (13) There is no valid FAT volume */
        case FR_TIMEOUT:             /* (15) Could not get a grant to access the volume within defined period */
        case FR_LOCKED:              /* (16) The operation is rejected according to the file sharing policy */
        case FR_NOT_ENOUGH_CORE:     /* (17) LFN working buffer could not be allocated */
        case FR_TOO_MANY_OPEN_FILES: /* (18) Number of open files > _FS_SHARE */
            return EIO;
    }
    return EIO;
}

int _stat(const char *file, struct stat *st) {
    FILINFO fno;
    struct tm ltm;

    FRESULT rc = f_stat(file, &fno);

    if (rc == FR_OK) {
        ltm.tm_sec = (fno.ftime & 0x1f) << 1;
        ltm.tm_min = (fno.ftime & 0x3e0) >> 5;
        ltm.tm_hour = (fno.ftime & 0xf800) >> 11;
        ltm.tm_mday = (fno.fdate & 0x1f);
        ltm.tm_mon = ((fno.fdate & 0x1e0) >> 5) - 1;
        ltm.tm_year = ((fno.fdate & 0xfe00) >> 9) + 80;

        st->st_dev = 0;
        st->st_ino = 0;
        st->st_mode = 0;
        st->st_nlink = 0;
        st->st_uid = 0;
        st->st_gid = 0;
        st->st_rdev = 0;
        st->st_size = fno.fsize;
        st->st_atime = st->st_mtime = st->st_ctime = mktime(&ltm);
        st->st_blksize = 0;
        st->st_blocks = 0;
    }
    errno = FRESULT_to_errno(rc);

    return rc == FR_OK ? 0 : -1;
}

int _fstat(int file, struct stat *st) {
    if (file < RESERVED_DESCRIPTORS || file >= MAX_DESCRIPTORS || file_descriptors[file] == NULL) {
        errno = EBADF;
        return -1;
    }
    st->st_dev = 0;
    st->st_ino = 0;
    st->st_mode = 0;
    st->st_nlink = 0;
    st->st_uid = 0;
    st->st_gid = 0;
    st->st_rdev = 0;
    st->st_size = file_descriptors[file]->fsize;
    st->st_atime = st->st_mtime = st->st_ctime = 0;
    st->st_blksize = 0;
    st->st_blocks = 0;
    return 0;
}

int _isatty(int file) {
    if (file < RESERVED_DESCRIPTORS) {
        return 1;
    }
    if (file >= MAX_DESCRIPTORS || file_descriptors[file] == NULL) {
        errno = EBADF;
        return -1;
    }
    return 0;
}

int _open(const char *name, int flags, int mode) {
    int file;
    FIL *fp;

    for (file = RESERVED_DESCRIPTORS; file < MAX_DESCRIPTORS; file++) {
        if (file_descriptors[file] == NULL)
            break;
    }
    if (file >= MAX_DESCRIPTORS) {
        errno = ENFILE;
        return -1;
    }

    if ((fp = (FIL *)malloc(sizeof(FIL))) == NULL) {
        errno = ENOMEM;
        return -1;
    }

    BYTE fmode = FA_OPEN_EXISTING;
    if ((flags & O_ACCMODE) == O_RDONLY) {
        fmode = FA_READ;
    }
    else if ((flags & O_ACCMODE) == O_WRONLY) {
        fmode = FA_WRITE;
    }
    else if ((flags & O_ACCMODE) == O_RDWR) {
        fmode = FA_READ | FA_WRITE;
    }
    if ((flags & O_CREAT) != 0) {
        if ((flags & O_TRUNC) != 0)
            fmode |= FA_CREATE_ALWAYS;
        else
            fmode |= FA_OPEN_ALWAYS;
    }

    FRESULT rc = f_open(fp, name, fmode);
    if (rc == FR_OK) {
        file_descriptors[file] = fp;
        if ((flags & O_APPEND) != 0) {
            f_lseek(fp, fp->fsize);
        }
    }
    else {
        free(fp);
        file = -1;
    }

    errno = FRESULT_to_errno(rc);

    return file;
}

int _read(int file, char *ptr, int len) {
    UINT br;
    if (file < RESERVED_DESCRIPTORS || file >= MAX_DESCRIPTORS || file_descriptors[file] == NULL) {
        errno = EBADF;
        return -1;
    }
    FRESULT rc = f_read(file_descriptors[file], ptr, len, &br);
    errno = FRESULT_to_errno(rc);
    return rc == FR_OK ? br : -1;
}

int _write(int file, char *ptr, int len) {
    UINT bw;
    if (file < RESERVED_DESCRIPTORS || file >= MAX_DESCRIPTORS || file_descriptors[file] == NULL) {
        errno = EBADF;
        return -1;
    }
    FRESULT rc = f_write(file_descriptors[file], ptr, len, &bw);
    errno = FRESULT_to_errno(rc);
    return rc == FR_OK ? bw : -1;
}

int _lseek(int file, int ptr, int dir) {
    if (file < RESERVED_DESCRIPTORS || file >= MAX_DESCRIPTORS || file_descriptors[file] == NULL) {
        errno = EBADF;
        return -1;
    }

    DWORD offs = ptr;
    if (dir == SEEK_CUR) {
        offs = f_tell(file_descriptors[file]) + ptr;
    }
    else if (dir == SEEK_END) {
        offs = f_size(file_descriptors[file]) - ptr;
    }

    FRESULT rc = f_lseek(file_descriptors[file], offs);
    errno = FRESULT_to_errno(rc);
    return rc == FR_OK ? f_tell(file_descriptors[file]) : -1;
}

int _close(int file) {
    if (file < RESERVED_DESCRIPTORS || file >= MAX_DESCRIPTORS || file_descriptors[file] == NULL) {
        errno = EBADF;
        return -1;
    }
    FRESULT rc = f_close(file_descriptors[file]);
    errno = FRESULT_to_errno(rc);
    if (rc == FR_OK) {
        file_descriptors[file] = NULL;
        return 0;
    }
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
    FRESULT rc = f_unlink(path);
    errno = FRESULT_to_errno(rc);
    return rc == FR_OK ? 0 : -1;
}

int _system (const char *s) {
    if (s == NULL) {
        return 0;
    }
    errno = ENOSYS;
    return -1;
}

int _rename (const char * oldpath, const char * newpath) {
    FRESULT rc = f_rename(oldpath, newpath);
    errno = FRESULT_to_errno(rc);
    return rc == FR_OK ? 0 : -1;
}

int mkdir(const char * path, mode_t mode) {
    FRESULT rc = f_mkdir(path);
    errno = FRESULT_to_errno(rc);
    return rc == FR_OK ? 0 : -1;
}

char * getcwd(char * buf, size_t size) {
    FRESULT rc = f_getcwd(buf, size);
    errno = FRESULT_to_errno(rc);
    return rc == FR_OK ? buf : NULL;
}

int chdir(const char * path) {
    FRESULT rc = f_chdir(path);
    errno = FRESULT_to_errno(rc);
    return rc == FR_OK ? 0 : -1;
}

int ftruncate(int file, off_t length) {
    if (file < RESERVED_DESCRIPTORS || file >= MAX_DESCRIPTORS || file_descriptors[file] == NULL) {
        errno = EBADF;
        return -1;
    }

    FRESULT rc = f_truncate(file_descriptors[file]);
    errno = FRESULT_to_errno(rc);
    return rc == FR_OK ? 0 : -1;
}

static const char * _VOLUME_NAME[_VOLUMES] = {
    _VOLUME_STRS
};

int mount(const char *source) {
    for (int i = 0; i < _VOLUMES; i++) {
        if (!strncasecmp(source, _VOLUME_NAME[i], strlen(_VOLUME_NAME[i]))) {
            FRESULT rc = f_mount(&fat_fs[i], source, 1);
            errno = FRESULT_to_errno(rc);
            return rc == FR_OK ? 0 : -1;
        }
    }

    errno = EINVAL;
    return -1;
}

int umount(const char *target) {
    for (int i = 0; i < _VOLUMES; i++) {
        if (!strncasecmp(target, _VOLUME_NAME[i], strlen(_VOLUME_NAME[i]))) {
            FRESULT rc = f_mount(NULL, target, 1);
            errno = FRESULT_to_errno(rc);
            return rc == FR_OK ? 0 : -1;
        }
    }

    errno = EINVAL;
    return -1;
}

void _fini() {
    while(1)
        ;
}

int _gettimeofday(struct timeval *tv, struct timezone *tz) {
    uint64_t t = *((uint64_t *)0x20003004);  // get uptime in nanoseconds
    tv->tv_sec = t / 1000000;  // convert to seconds
    tv->tv_usec = ( t % 1000000 ) / 1000;  // get remaining microseconds
    return 0;  // return non-zero for error
}

void LogWrite (const char *pSource, unsigned Severity, const char *pMessage, ...)
{
    // Do nothing
}

void uspi_assertion_failed (const char *pExpr, const char *pFile, unsigned nLine)
{
    // Do nothing
}

void DebugHexdump (const void *pBuffer, unsigned nBufLen, const char *pSource /* = 0 */)
{
    // Do nothing
}

void log_printf (const char *ptr, ...)
{
    // Do nothing
}
