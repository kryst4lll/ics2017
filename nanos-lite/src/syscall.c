#include "common.h"
#include "syscall.h"

int sys_none(){
  return 1;
}

int sys_exit(int code){
  _halt(code);
  return 0;
}

int sys_write(int fd, const void *buf, size_t count){
  if(fd == 1 || fd == 2){
    for(int i = 0; i < count; i++){
      _putc(*((char*)buf+i));
    }
    return count;
  }
  return -1;
}

static uintptr_t current_break;

int sys_brk(uint32_t addr){
  current_break = addr;

  return -1;
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
      SYSCALL_ARG1(r) = sys_exit(a[1]);
      break;
    case SYS_write:
      SYSCALL_ARG1(r) = sys_write(a[1], (void*)a[2], a[3]);
      break;
    case SYS_brk:

      break;

    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
