#include "common.h"
#include "syscall.h"
#include "fs.h"
#include "proc.h"

int mm_brk(uint32_t new_brk);

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  static int syscall_log_cnt = 0;
  if (syscall_log_cnt < 80 || a[0] == SYS_brk || a[0] == SYS_write) {
    Log("%s:%d syscall: id=%d a1=%p a2=%p a3=%p eip=%p trap_esp=%p",
        __FILE__, __LINE__, (int)a[0], (void *)a[1], (void *)a[2],
        (void *)a[3], (void *)r->eip, (void *)r);
  }
  if ((a[0] == SYS_read || a[0] == SYS_write) && current != NULL) {
    uintptr_t buf_start = a[2];
    uintptr_t buf_end = buf_start + a[3];
    uintptr_t stack_start = (uintptr_t)current->stack;
    uintptr_t stack_end = stack_start + sizeof(current->stack);
    bool in_heap = buf_start >= 0x8048000 && buf_end <= current->max_brk;
    bool in_stack = buf_start >= stack_start && buf_end <= stack_end;
    if (!in_heap && !in_stack) {
      Log("%s:%d syscall OUT_OF_RANGE: id=%d buf=[%p,%p) max_brk=%p stack=[%p,%p) eip=%p",
          __FILE__, __LINE__, (int)a[0], (void *)buf_start,
          (void *)buf_end, (void *)current->max_brk,
          (void *)stack_start, (void *)stack_end, (void *)r->eip);
    }
  }
  syscall_log_cnt++;

  switch (a[0]) {
    case SYS_none:
      r->eax = 1;
      break;
    case SYS_open:  r->eax = fs_open((const char *)a[1], a[2], a[3]); break;
    case SYS_read:  r->eax = fs_read(a[1], (void *)a[2], a[3]); break;
    case SYS_write: r->eax = fs_write(a[1], (const void *)a[2], a[3]); break;
    case SYS_lseek: r->eax = fs_lseek(a[1], a[2], a[3]); break;
    case SYS_close: r->eax = fs_close(a[1]); break;
    case SYS_exit:
      _halt((int)a[1]);
      break;
    case SYS_brk:
      r->eax = mm_brk((uint32_t)a[1]);
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return r;
}
