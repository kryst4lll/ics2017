#include "nemu.h"
#include "device/mmio.h"
#include "memory/mmu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int mm = is_mmio(addr);
  if(mm != -1){
    return mmio_read(addr, len, mm);
  }
  return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int mm = is_mmio(addr);
  if(mm != -1){
    mmio_write(addr, len, data, mm);
  }else{
    memcpy(guest_to_host(addr), &data, len);
  }
}

#define CR3_PDBR(cr3) ((cr3) & 0xfffff000)
#define PDX(va) (((va)>>22) & 0x3ff)
#define PTX(va) (((va)>>12) & 0x3ff)
#define OFFSET(va) ((va) & 0xfff)

paddr_t page_translate(vaddr_t addr, bool is_write){
  CR0 cr_0 = (CR0)cpu.cr0;
  if(cr_0.paging){
    CR3 cr_3 = (CR3)cpu.cr3;
    uint32_t pde_addr = CR3_PDBR(cr_3.val) + PDX(addr) * 4;
    uint32_t pde_val = paddr_read(pde_addr, 4);
    PDE pde;
    pde.val = pde_val;
    assert(pde.present);

    uint32_t pte_addr = CR3_PDBR(pde.val) + PTX(addr) * 4;
    uint32_t pte_val = paddr_read(pte_addr, 4);
    PTE pte;
    pte.val = pte_val;
    assert(pte.present);

    pde.accessed = 1;
    pte.accessed = 1;
    if(is_write){
      pte.dirty = 1;
    }

    paddr_t paddr = CR3_PDBR(pte.val) + OFFSET(addr);
    return paddr;
  }

  return addr;
}


uint32_t vaddr_read(vaddr_t addr, int len) {
  if((addr&0xfffff000) != ((addr + len - 1)&0xfffff000)){
    assert(0);
  }else{
    paddr_t paddr = page_translate(addr, false);
    return paddr_read(paddr, len);
  }
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if((addr&0xfffff000) != ((addr + len - 1)&0xfffff000)){
    assert(0);
  }else{
    paddr_t paddr = page_translate(addr, true);
    paddr_write(paddr, len, data);
  }
}
