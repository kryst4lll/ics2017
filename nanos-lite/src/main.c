#include "common.h"

/* Uncomment these macros to enable corresponding functionality. */
#define HAS_ASYE
#define HAS_PTE

void init_mm(void);
void init_ramdisk(void);
void init_device(void);
void init_irq(void);
void init_fs(void);
uint32_t loader(_Protect *, const char *);

int main() {
#ifdef HAS_PTE
  init_mm();
#endif

  Log("'Hello World!' from Nanos-lite");
  Log("Build time: %s, %s", __TIME__, __DATE__);

  init_ramdisk();

  init_device();

#ifdef HAS_ASYE
  Log("Initializing interrupt/exception handler...");
  init_irq();
#endif

  init_fs();
  extern void load_prog(const char *filename);
  // uint32_t entry = loader(NULL, "/bin/pal");
  // ((void (*)(void))entry)();
  load_prog("/bin/pal");
  load_prog("/bin/hello");
  // load_prog("/bin/dummy");

  _trap();

  panic("Should not reach here");
}
