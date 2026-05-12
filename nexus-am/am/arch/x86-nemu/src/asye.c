#include <am.h>
#include <x86.h>

static _RegSet* (*H)(_Event, _RegSet*) = NULL;

void vecsys();
void vectrap();
void vectime();
void vecnull();

#define TIMER_IRQ 32

_RegSet* irq_handle(_RegSet *tf) {
  _RegSet *next = tf;
  if (H) {
    _Event ev = {};
    ev.cause = tf->irq;
    switch (tf->irq) {
      case 0x80: ev.event = _EVENT_SYSCALL; break;
      case 0x81: ev.event = _EVENT_TRAP; break;
      case TIMER_IRQ: ev.event = _EVENT_IRQ_TIME; break;
      default: ev.event = _EVENT_ERROR; break;
    }

    next = H(ev, tf);
    if (next == NULL) {
      next = tf;
    }
  }

  return next;
}

static GateDesc idt[NR_IRQ];

void _asye_init(_RegSet*(*h)(_Event, _RegSet*)) {
  // initialize IDT
  for (unsigned int i = 0; i < NR_IRQ; i ++) {
    idt[i] = GATE(STS_TG32, KSEL(SEG_KCODE), vecnull, DPL_KERN);
  }

  // -------------------- system call --------------------------
  idt[0x80] = GATE(STS_TG32, KSEL(SEG_KCODE), vecsys, DPL_USER);
  idt[0x81] = GATE(STS_TG32, KSEL(SEG_KCODE), vectrap, DPL_KERN);
  idt[TIMER_IRQ] = GATE(STS_IG32, KSEL(SEG_KCODE), vectime, DPL_KERN);

  set_idt(idt, sizeof(idt));

  // register event handler
  H = h;
}

_RegSet *_make(_Area stack, void *entry, void *arg) {
  return NULL;
}

void _trap() {
  asm volatile("int $0x81");
}

int _istatus(int enable) {
  return 0;
}
