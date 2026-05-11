#include "proc.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC];
static int nr_proc = 0;
PCB *current = NULL;

uintptr_t loader(_Protect *as, const char *filename);

void load_prog(const char *filename) {
  int i = nr_proc ++;
  assert(i < MAX_NR_PROC);
  _protect(&pcb[i].as);
  current = &pcb[i];

  uintptr_t entry = loader(&pcb[i].as, filename);

  _Area stack;
  stack.start = pcb[i].stack;
  stack.end = stack.start + sizeof(pcb[i].stack);

  pcb[i].tf = _umake(&pcb[i].as, stack, stack, (void *)entry, NULL, NULL);
  current = NULL;
}

static int current_game = 0;
static int foreground_count = 0;

void switch_current_game(void) {
  current_game = 1 - current_game;
  foreground_count = 0;
  Log("current_game = %d", current_game);
}

_RegSet* schedule(_RegSet *prev) {
  if (current != NULL) {
    current->tf = prev;
  }

  if (nr_proc == 0) {
    return prev;
  }

  if (nr_proc == 1) {
    current = &pcb[0];
  }
  else {
    static const int frequency = 1000;
    int background = 1 - current_game;

    if (current == NULL) {
      current = &pcb[current_game];
    }
    else if (current == &pcb[current_game]) {
      foreground_count ++;
      if (foreground_count >= frequency) {
        current = &pcb[background];
        foreground_count = 0;
      }
    }
    else {
      current = &pcb[current_game];
    }
  }

  _switch(&current->as);
  return current->tf;
}
