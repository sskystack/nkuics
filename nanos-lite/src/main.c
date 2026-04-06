#include "common.h"

/* Uncomment these macros to enable corresponding functionality. */
#define HAS_ASYE
//#define HAS_PTE

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
  Log("Interrupt/exception handler initialized");
#endif

  init_fs();
  Log("File system initialized");

  Log("Loading user program: /bin/pal");
  uint32_t entry = loader(NULL, "/bin/pal");
  Log("Jumping to entry = 0x%x", entry);
  ((void (*)(void))entry)();

  panic("Should not reach here");
}
