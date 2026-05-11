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
  ustack.end = (void *)0x0bfe0010;
  ustack.start = (void *)((uintptr_t)ustack.end - STACK_SIZE);

  _Area kstack;
  kstack.start = pcb[i].stack;
  kstack.end = kstack.start + sizeof(pcb[i].stack);

  pcb[i].tf = _umake(&pcb[i].as, ustack, kstack, (void *)entry, NULL, NULL);
}

_RegSet* schedule(_RegSet *prev) {
  if (current != NULL) {
    uintptr_t stack_start = (uintptr_t)current->stack;
    uintptr_t stack_end = stack_start + sizeof(current->stack);
    uintptr_t prev_addr = (uintptr_t)prev;
    if (prev_addr >= stack_start && prev_addr < stack_end) {
      current->tf = prev;
    }
  }

  current = &pcb[0];
  _switch(&current->as);
  return current->tf;
}
