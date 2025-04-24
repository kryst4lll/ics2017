#include "common.h"
#include "syscall.h"

int sys_none(){
  return 1;
}

int sys_exit(int code){
  _halt(code);
  return 0;
}

extern ssize_t fs_write(int fd, const void *buf, size_t len);

int sys_write(int fd, const void *buf, size_t count){
  if(fd == 1 || fd == 2){
    for(int i = 0; i < count; i++){
      _putc(*((char*)buf+i));
    }
    return count;
  }
  else if(fd > 3){
    return fs_write(fd, buf, count);
  }

  return -1;
}

static uintptr_t current_break = 0;

int sys_brk(uint32_t addr){
  if (current_break == 0) {
    extern char end;  
    current_break = (uint32_t)&end;
  }

  current_break = addr;

  return -1;
}

extern int fs_open(const char *pathname, int flags, int mode);

int sys_open(const char *pathname){
  return fs_open(pathname, 0, 0);
}

extern ssize_t fs_read(int fd, void *buf, size_t len);

int sys_read(int fd, void *buf, size_t len){
  return fs_read(fd, buf, len);
}

extern int fs_close(int fd);

int sys_close(int fd){
  return fs_close(fd);
}

off_t fs_leek(int fd, off_t offset, int whence);

int sys_lseek(int fd, off_t offset, int whence){
  return fs_leek(fd, offset, whence);
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
    case SYS_none:
      SYSCALL_ARG1(r) = sys_none();
      break;
    case SYS_exit:
      sys_exit(a[1]);
      break;
    case SYS_write:
      SYSCALL_ARG1(r) = sys_write(a[1], (void *)a[2], a[3]);
      break;
    case SYS_brk:
      SYSCALL_ARG1(r) = sys_brk(a[1]);
      break;
    case SYS_open:
      SYSCALL_ARG1(r) = sys_open((char*)a[1]);
      break;      
    case SYS_read:
      SYSCALL_ARG1(r) = sys_read(a[1], (void *)a[2], a[3]);
      break;
    case SYS_close:
      SYSCALL_ARG1(r) = sys_close(a[1]);
      break;      
    case SYS_lseek:
      SYSCALL_ARG1(r) = sys_lseek(a[1], a[2], a[3]);
      break;

    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
