#include "proc.h"
#include "memory.h"

static void *pf = NULL;

void* new_page(void) {
  assert(pf < (void *)_heap.end);
  void *p = pf;
  pf += PGSIZE;
  return p;
}

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uint32_t new_brk) {
  assert(current != NULL);

  if (current->cur_brk == 0) {
    Log("mm_brk init: new_brk=%p", (void *)new_brk);
    current->cur_brk = new_brk;
    current->max_brk = PGROUNDUP(new_brk);
  }
  else {
    if (new_brk > current->max_brk) {
      uintptr_t map_start = PGROUNDUP(current->max_brk);
      uintptr_t map_end = PGROUNDUP(new_brk);

      Log("mm_brk grow: cur=%p max=%p new=%p map=[%p, %p)",
          (void *)current->cur_brk, (void *)current->max_brk,
          (void *)new_brk, (void *)map_start, (void *)map_end);

      for (uintptr_t va = map_start; va < map_end; va += PGSIZE) {
        void *pa = new_page();
        memset(pa, 0, PGSIZE);
        _map(&current->as, (void *)va, pa);
      }

      current->max_brk = map_end;
    }

    current->cur_brk = new_brk;
  }

  return 0;
}

void init_mm() {
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
  Log("free physical pages starting from %p", pf);

  _pte_init(new_page, free_page);
}
