#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  rtlreg_t tmp;
  uint32_t idt_addr = cpu.idtr.base + NO * 8;
  uint32_t low = vaddr_read(idt_addr, 4);
  uint32_t high = vaddr_read(idt_addr + 4, 4);
  uint32_t handler = (low & 0x0000ffff) | (high & 0xffff0000);
  uint32_t gate_type = (high >> 8) & 0xf;

  tmp = cpu.eflags;
  rtl_push(&tmp);

  tmp = 8;
  rtl_push(&tmp);

  tmp = ret_addr;
  rtl_push(&tmp);

  // interrupt gate clears IF, trap gate keeps IF unchanged
  if (gate_type == 0xe) {
    cpu.IF = 0;
  }

  cpu.eip = handler;
}

void dev_raise_intr() {
  if (cpu.IF == 0) {
    return;
  }
  raise_intr(0x81, cpu.eip);
}
