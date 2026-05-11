#include "proc.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC];
static int nr_proc = 0;
PCB *current = NULL;

uintptr_t loader(_Protect *as, const char *filename);

void load_prog(const char *filename) {
  int i = nr_proc ++;
  _protect(&pcb[i].as);
  current = &pcb[i];

  uintptr_t entry = loader(&pcb[i].as, filename);

  _Area ustack;
  ustack.start = pcb[i].stack;
  ustack.end = pcb[i].stack + sizeof(pcb[i].stack);

  _Area kstack = ustack;

  pcb[i].tf = _umake(&pcb[i].as, ustack, kstack, (void *)entry, NULL, NULL);
  Log("%s:%d load_prog: pcb=%d entry=%p ustack=[%p,%p) tf=%p tf.eip=%p iret_esp=%p",
      __FILE__, __LINE__, i, (void *)entry, ustack.start, ustack.end,
      pcb[i].tf, (void *)pcb[i].tf->eip,
      (void *)((uintptr_t)pcb[i].tf + sizeof(_RegSet)));
  current = NULL;
}

_RegSet* schedule(_RegSet *prev) {
  if (current != NULL) {
    Log("%s:%d schedule: save current=%p prev=%p prev.eip=%p prev.esp=%p",
        __FILE__, __LINE__, current, prev, (void *)prev->eip, (void *)prev->esp);
    current->tf = prev;
  }
  else {
    Log("%s:%d schedule: first switch, keep boot trap frame prev=%p prev.eip=%p prev.esp=%p",
        __FILE__, __LINE__, prev, (void *)prev->eip, (void *)prev->esp);
  }

  current = &pcb[0];
  _switch(&current->as);
  Log("%s:%d schedule: switch to current=%p as=%p tf=%p tf.eip=%p tf.esp=%p",
      __FILE__, __LINE__, current, current->as.ptr, current->tf,
      (void *)current->tf->eip, (void *)current->tf->esp);
  return current->tf;
}
