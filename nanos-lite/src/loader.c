#include "proc.h"
#include "fs.h"

#define DEFAULT_ENTRY ((void *)0x8048000)

uintptr_t loader(_Protect *as, const char *filename) {

  if (filename == NULL) {
    filename = "/bin/bmptest";
  }

  int fd = fs_open(filename, 0, 0);
  size_t img_size = fs_filesz(fd);
  Log("%s:%d loader: filename=%s entry=%p img_size=%d as=%p",
      __FILE__, __LINE__, filename, DEFAULT_ENTRY, img_size, as);

  if (as == NULL) {
    size_t nread = fs_read(fd, DEFAULT_ENTRY, img_size);
    assert(nread == img_size);
  }
  else {
    for (size_t off = 0; off < img_size; off += PGSIZE) {
      void *pa = new_page();
      memset(pa, 0, PGSIZE);

      size_t len = img_size - off;
      if (len > PGSIZE) {
        len = PGSIZE;
      }
      size_t nread = fs_read(fd, pa, len);
      assert(nread == len);

      _map(as, DEFAULT_ENTRY + off, pa);
    }

    if (current != NULL) {
      uintptr_t brk = (uintptr_t)DEFAULT_ENTRY + img_size;
      current->cur_brk = brk;
      current->max_brk = PGROUNDUP(brk);
      Log("%s:%d loader: brk=%p max_brk=%p",
          __FILE__, __LINE__, (void *)current->cur_brk,
          (void *)current->max_brk);
    }
  }

  fs_close(fd);

  return (uintptr_t)DEFAULT_ENTRY;
}
