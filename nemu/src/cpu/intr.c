#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  //TODO();
  memcpy(&t0, &cpu.eflags, sizeof(cpu.eflags));

  cpu.eflags.IF = 0;

  rtl_push(&t0);
  rtl_push(&cpu.cs);
  rtl_li(&t0, ret_addr);
  rtl_push(&t0);

  // printf("%d,%d\n",cpu.idtr.base, NO*8);
  uint32_t offset_low = vaddr_read(cpu.idtr.base + NO*8, 2);
  uint32_t offset_high = vaddr_read(cpu.idtr.base + NO*8 + 6, 2);
  uint32_t offset = offset_low + (offset_high << 16);

  decoding.jmp_eip = offset;
  decoding.is_jmp = true;

}

void dev_raise_intr() {
  cpu.INTR = true;
}
