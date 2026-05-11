#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

#define CR0_PG_MASK 0x80000000
#define PTE_P_MASK  0x001
#define PTE_A_MASK  0x020
#define PTE_D_MASK  0x040
#define PTE_ADDR(x) ((x) & ~PAGE_MASK)
#define PDX(addr)   (((addr) >> 22) & 0x3ff)
#define PTX(addr)   (((addr) >> 12) & 0x3ff)
#define OFF(addr)   ((addr) & PAGE_MASK)

uint32_t paddr_read(paddr_t addr, int len) {
  int map_NO = is_mmio(addr);
  if (map_NO != -1) {
    return mmio_read(addr, len, map_NO);
  }
  return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int map_NO = is_mmio(addr);
  if (map_NO != -1) {
    mmio_write(addr, len, data, map_NO);
    return;
  }
  memcpy(guest_to_host(addr), &data, len);
}

static inline paddr_t page_translate(vaddr_t addr, bool is_write) {
  if ((cpu.cr0.val & CR0_PG_MASK) == 0) {
    return addr;
  }

  paddr_t pdir_base = PTE_ADDR(cpu.cr3.val);
  paddr_t pde_addr = pdir_base + PDX(addr) * sizeof(PDE);
  PDE pde;
  pde.val = paddr_read(pde_addr, sizeof(PDE));
  assert(pde.present);

  if (!pde.accessed) {
    pde.accessed = 1;
    paddr_write(pde_addr, sizeof(PDE), pde.val);
  }

  paddr_t ptab_base = PTE_ADDR(pde.val);
  paddr_t pte_addr = ptab_base + PTX(addr) * sizeof(PTE);
  PTE pte;
  pte.val = paddr_read(pte_addr, sizeof(PTE));
  assert(pte.present);

  pte.accessed = 1;
  if (is_write) {
    pte.dirty = 1;
  }
  paddr_write(pte_addr, sizeof(PTE), pte.val);

  return PTE_ADDR(pte.val) + OFF(addr);
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  uint32_t data = 0;
  for (int i = 0; i < len; i ++) {
    paddr_t paddr = page_translate(addr + i, false);
    data |= paddr_read(paddr, 1) << (i * 8);
  }
  return data;
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  for (int i = 0; i < len; i ++) {
    paddr_t paddr = page_translate(addr + i, true);
    paddr_write(paddr, 1, (data >> (i * 8)) & 0xff);
  }
}
