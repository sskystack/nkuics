#include "proc.h"

#define MAX_NR_PROC 4
#define HELLO_SCHEDULE_INTERVAL 8

static PCB pcb[MAX_NR_PROC];
static int nr_proc = 0;
static int schedule_count = 0;
PCB *current = NULL;

uintptr_t loader(_Protect *as, const char *filename);

void load_prog(const char *filename) {
  assert(nr_proc < MAX_NR_PROC);
  int i = nr_proc ++;
  _protect(&pcb[i].as);
  current = &pcb[i];

  uintptr_t entry = loader(&pcb[i].as, filename);

  _Area ustack;
  ustack.start = pcb[i].stack;
  ustack.end = pcb[i].stack + sizeof(pcb[i].stack);

  _Area kstack = ustack;

  pcb[i].tf = _umake(&pcb[i].as, ustack, kstack, (void *)entry, NULL, NULL);
  current = NULL;
}

_RegSet* schedule(_RegSet *prev) {
  if (current != NULL) {
    current->tf = prev;
  }

  if (nr_proc <= 1 || current == NULL) {
    current = &pcb[0];
  }
  else {
    schedule_count ++;
    current = (schedule_count % HELLO_SCHEDULE_INTERVAL == 0 ? &pcb[1] : &pcb[0]);
  }

  _switch(&current->as);
  return current->tf;
}
