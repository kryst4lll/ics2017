#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

int fs_open(const char *pathname, int flags, int mode){
  for(int i = 0; i < NR_FILES; i++){
    if(strcmp(file_table[i].name, pathname) == 0){
      file_table[i].open_offset = 0;
      // assert(0);
      return i;
    }
  }
  assert(0);
  return -1;
}

size_t fs_filesz(int fd) {
  if(fd < 0 || fd>=NR_FILES){
    assert(0);
  }
  return file_table[fd].size;
}

void ramdisk_read(void *buf, off_t offset, size_t len);

ssize_t fs_read(int fd, void *buf, size_t len){
  if(fd < 0 || fd >= NR_FILES){
    assert(0);
  }
  
  Finfo * file = &file_table[fd];
  size_t left = file->size - file->open_offset;
  size_t read_len = (len > left) ? left : len;
  ramdisk_read(buf, file->disk_offset + file->open_offset, read_len);
  file->open_offset += read_len;

  return read_len;
}

int fs_close(int fd){
  return 0;
}

extern void ramdisk_write(const void *buf, off_t offset, size_t len);
off_t disk_offset(int fd) {
  assert(fd>=0 && fd<NR_FILES);
  return file_table[fd].disk_offset;
}
off_t get_open_offset(int fd) {
  assert(fd>=0 && fd<NR_FILES);
  return file_table[fd].open_offset;
}
void set_open_offset(int fd,off_t n) {
  assert(fd>=0 && fd<NR_FILES);
  assert(n>=0);
  if(n>file_table[fd].size)
    n = file_table[fd].size;
  file_table[fd].open_offset = n;
}
ssize_t fs_write(int fd, const void *buf, size_t len){
  assert(fd>=0 && fd<NR_FILES);
	if(fd < 3 || fd == FD_DISPINFO) {
    Log("arg invalid: fd < 3 || fd == FD_DISPINFO");
    return 0;
  }
  int n = fs_filesz(fd) - get_open_offset(fd);
  if(n > len)
    n = len;

  ramdisk_write(buf,disk_offset(fd)+get_open_offset(fd),n);
  set_open_offset(fd,get_open_offset(fd)+n);
	return n;
}

off_t fs_lseek(int fd, off_t offset, int whence){
  if(fd < 0 || fd >= NR_FILES){
    assert(0);
  }

  Finfo *file = &file_table[fd];
  off_t new_offset = 0;

  switch (whence)
  {
  case SEEK_SET:
    new_offset = offset;
    break;
  case SEEK_CUR:
    new_offset = file->open_offset + offset;
    break;
  case SEEK_END:
    new_offset = file->size + offset;
    break;

  default:
    assert(0);
    break;
  }

  if(new_offset < 0){
    new_offset = 0;
  }
  else if(new_offset > file->size){
    new_offset = file->size;
  }

  file->open_offset = new_offset;
  return new_offset;
}
