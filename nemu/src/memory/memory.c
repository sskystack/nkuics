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
  Assert(pde.present,
      "invalid PDE: vaddr=0x%08x cr3=0x%08x eip=0x%08x esp=0x%08x pde_idx=%u pde_addr=0x%08x pde=0x%08x",
      addr, cpu.cr3.val, cpu.eip, cpu.esp, PDX(addr), pde_addr, pde.val);

  if (!pde.accessed) {
    pde.accessed = 1;
    paddr_write(pde_addr, sizeof(PDE), pde.val);
  }

  paddr_t ptab_base = PTE_ADDR(pde.val);
  paddr_t pte_addr = ptab_base + PTX(addr) * sizeof(PTE);
  PTE pte;
  pte.val = paddr_read(pte_addr, sizeof(PTE));
  Assert(pte.present,
      "invalid PTE: vaddr=0x%08x cr3=0x%08x eip=0x%08x esp=0x%08x pde_idx=%u pte_idx=%u pte_addr=0x%08x pte=0x%08x",
      addr, cpu.cr3.val, cpu.eip, cpu.esp, PDX(addr), PTX(addr), pte_addr, pte.val);

  pte.accessed = 1;
  if (is_write) {
    pte.dirty = 1;
  }
  paddr_write(pte_addr, sizeof(PTE), pte.val);

  return PTE_ADDR(pte.val) + OFF(addr);
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  assert(len >= 1 && len <= 4);

  uint32_t data = 0;
  int read_bytes = 0;
  while (read_bytes < len) {
    int chunk = PAGE_SIZE - OFF(addr + read_bytes);
    if (chunk > len - read_bytes) {
      chunk = len - read_bytes;
    }

    paddr_t paddr = page_translate(addr + read_bytes, false);
    data |= paddr_read(paddr, chunk) << (read_bytes * 8);
    read_bytes += chunk;
  }
  return data;
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  assert(len >= 1 && len <= 4);

  int written_bytes = 0;
  while (written_bytes < len) {
    int chunk = PAGE_SIZE - OFF(addr + written_bytes);
    if (chunk > len - written_bytes) {
      chunk = len - written_bytes;
    }

    paddr_t paddr = page_translate(addr + written_bytes, true);
    paddr_write(paddr, chunk, (data >> (written_bytes * 8)) &
        (~0u >> ((4 - chunk) << 3)));
    written_bytes += chunk;
  }
}
