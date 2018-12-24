#include <klib.h>
#include "fs.h"
#include "proc.h"

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_DISPINFO, FD_EVENT, FD_TTY};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin", 0, 0, 0, invalid_read, invalid_write},
  {"stdout", 0, 0, 0, invalid_read, serial_write},
  {"stderr", 0, 0, 0, invalid_read, serial_write},
  [FD_FB] = {"/dev/fb", 0, 0, 0, invalid_read, fb_write},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0, 0, dispinfo_read, invalid_write},
  [FD_EVENT] = {"/dev/events", 0, 0, 0, events_read, invalid_write},
  [FD_TTY] = {"/dev/tty", 0, 0, 0, invalid_read, serial_write},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
  int size = screen_height() * screen_width();
  file_table[FD_FB].size = size << 2;
}

int fs_open(const char *pathname, int flags, int mode) {
  for (int i = 0; i < NR_FILES; i++) {
    if (strcmp(pathname, file_table[i].name) == 0) {
      return i;
    }
  }
  panic("File %s not found!\n", pathname);
}

size_t fs_read(int fd, void *buf, size_t len) {
  switch (fd) {
    case FD_STDIN:
    case FD_STDOUT:
    case FD_STDERR:
    case FD_TTY:
    case FD_FB:
    case FD_EVENT: return file_table[fd].read(buf, 0, len);
    case FD_DISPINFO: {
      if (file_table[fd].open_offset + len > file_table[fd].size) {
        len = file_table[fd].size - file_table[fd].open_offset;
      }
      file_table[fd].read(buf, file_table[fd].open_offset, len);
      file_table[fd].open_offset += len;
      return len;
    }
    default: {
      if (file_table[fd].open_offset + len > file_table[fd].size) {
        len = file_table[fd].size - file_table[fd].open_offset;
      }
      ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
      file_table[fd].open_offset += len;
      return len;
    }
  }
}

size_t fs_write(int fd, const void *buf, size_t len) {
  switch (fd) {
    case FD_STDIN:
    case FD_STDOUT:
    case FD_STDERR:
    case FD_TTY:
    case FD_DISPINFO:
    case FD_EVENT: return file_table[fd].write(buf, 0, len);
    case FD_FB: {
      if (file_table[fd].open_offset + len > file_table[fd].size) {
        len = file_table[fd].size - file_table[fd].open_offset;
      }
      file_table[fd].write(buf, file_table[fd].open_offset, len);
      file_table[fd].open_offset += len;
      return len;
    }
    default: {
      if (file_table[fd].open_offset + len > file_table[fd].size) {
        len = file_table[fd].size - file_table[fd].open_offset;
      }
      ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
      file_table[fd].open_offset += len;
      return len;
    }
  }
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  switch (whence) {
    case SEEK_SET: file_table[fd].open_offset = offset; break;
    case SEEK_CUR: file_table[fd].open_offset += offset; break;
    case SEEK_END: file_table[fd].open_offset = file_table[fd].size; break;
    default: panic("Invalid whence:%d\n", whence);
  }
  if (file_table[fd].open_offset > file_table[fd].size) {
    return -1;
  }
  return file_table[fd].open_offset;
}

size_t fs_filesz(int fd) {
  return file_table[fd].size;
}

int fs_close(int fd) {
  return 0;
}
