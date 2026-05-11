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
  ustack.end = (void *)0x0c000000;
  ustack.start = (void *)0x0b800000;

  _Area kstack;
  kstack.start = pcb[i].stack;
  kstack.end = kstack.start + sizeof(pcb[i].stack);

  pcb[i].tf = _umake(&pcb[i].as, ustack, kstack, (void *)entry, NULL, NULL);
  current = NULL;
}

_RegSet* schedule(_RegSet *prev) {
  if (current != NULL) {
    current->tf = prev;
  }

  current = &pcb[0];
  _switch(&current->as);
  return current->tf;
}
