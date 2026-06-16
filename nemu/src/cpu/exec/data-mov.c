#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(xchg) {
  rtl_mv(&t0, &id_dest->val);
  operand_write(id_dest, &id_src->val);
  operand_write(id_src, &t0);

  print_asm_template2(xchg);
}

make_EHelper(push) {
  rtl_push(&id_dest->val);

  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&t0);
  operand_write(id_dest, &t0);

  print_asm_template1(pop);
}

make_EHelper(pusha) {
  rtl_li(&t0, cpu.esp);
  rtl_push(&cpu.eax);
  rtl_push(&cpu.ecx);
  rtl_push(&cpu.edx);
  rtl_push(&cpu.ebx);
  rtl_push(&t0);
  rtl_push(&cpu.ebp);
  rtl_push(&cpu.esi);
  rtl_push(&cpu.edi);

  print_asm("pusha");
}

make_EHelper(popa) {
  rtl_pop(&cpu.edi);
  rtl_pop(&cpu.esi);
  rtl_pop(&cpu.ebp);
  rtl_pop(&t0);
  rtl_pop(&cpu.ebx);
  rtl_pop(&cpu.edx);
  rtl_pop(&cpu.ecx);
  rtl_pop(&cpu.eax);

  print_asm("popa");
}

make_EHelper(leave) {
  if (decoding.is_operand_size_16) {
    cpu.esp = (cpu.esp & 0xffff0000) | (cpu.ebp & 0xffff);
    t0 = vaddr_read(cpu.esp, 2);
    cpu.esp = (cpu.esp & 0xffff0000) | ((cpu.esp + 2) & 0xffff);
    reg_w(R_BP) = t0;
  }
  else {
    cpu.esp = cpu.ebp;
    rtl_pop(&t0);
    rtl_sr_l(R_EBP, &t0);
  }

  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    rtl_msb(&t0, &cpu.eax, 2);
    rtl_sub(&t0, &tzero, &t0);
    cpu.edx = t0 & 0xffff;
  }
  else {
    rtl_msb(&t0, &cpu.eax, 4);
    rtl_sub(&t0, &tzero, &t0);
    cpu.edx = t0;
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    // cbtw: sign-extend AL to AX
    rtl_sext(&t0, &cpu.eax, 1);
    reg_w(R_AX) = t0;
  }
  else {
    // cwtl(cwde): sign-extend AX to EAX
    rtl_sext(&t0, &cpu.eax, 2);
    cpu.eax = t0;
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
