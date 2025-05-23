#include "common.h"
#include "memory.h"

#define DEFAULT_ENTRY ((void *)0x8048000)

extern size_t get_ramdisk_size(void);
extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void ramdisk_write(const void *buf, off_t offset, size_t len);

extern int fs_open(const char *pathname, int flags, int mode);
extern ssize_t fs_read(int fd, void *buf, size_t len);
extern int fs_close(int fd);
extern size_t fs_filesz(int fd);

uintptr_t loader(_Protect *as, const char *filename) {
  //TODO();
  // size_t len = get_ramdisk_size();
  // ramdisk_read(DEFAULT_ENTRY, 0, len);
  int fd = fs_open(filename, 0, 0);
  
  size_t size = fs_filesz(fd);
  // ssize_t read_len = fs_read(fd, DEFAULT_ENTRY, file_size);

  // if(read_len < 0){
  //   assert(0);
  // }
  int pgnum = size / PGSIZE;
  if(size % PGSIZE != 0) {
    pgnum++;
  }
  void *pa = NULL;
  void *va = DEFAULT_ENTRY;
  for(int i = 0;i < pgnum;i++) {
    pa = new_page();
    _map(as,va,pa);
    fs_read(fd,pa,PGSIZE);
    va += PGSIZE;
  }  
  fs_close(fd);

  return (uintptr_t)DEFAULT_ENTRY;
}
