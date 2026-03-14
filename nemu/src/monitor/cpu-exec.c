#include "nemu.h"
#include "monitor/monitor.h"

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INSTR_TO_PRINT 10

int nemu_state = NEMU_STOP;

void exec_wrapper(bool);

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  if (nemu_state == NEMU_END) {
    printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
    return;
  }
  nemu_state = NEMU_RUNNING;

  bool print_flag = n < MAX_INSTR_TO_PRINT;

  for (; n > 0; n --) {
    /* Execute one instruction, including instruction fetch,
     * instruction decode, and the actual execution. */
    exec_wrapper(print_flag);

#ifdef DEBUG
    /* Check watchpoints here */
    WP *wp = head;
    while (wp != NULL) {
      bool success = false;
      uint32_t new_val = expr(wp->expr, &success);
      if (!success) {
        printf("Failed to evaluate watchpoint %d: %s\n", wp->NO, wp->expr);
        wp = wp->next;
        continue;
      }

      if (new_val != wp->old_val) {
        printf("Watchpoint %d triggered: %s\n", wp->NO, wp->expr);
        printf("Old value = %u\n", wp->old_val);
        printf("New value = %u\n", new_val);
        wp->old_val = new_val;
        nemu_state = NEMU_STOP;
        return;
      }

      wp = wp->next;
    }
#endif

#ifdef HAS_IOE
    extern void device_update();
    device_update();
#endif

    if (nemu_state != NEMU_RUNNING) { return; }
  }

  if (nemu_state == NEMU_RUNNING) { nemu_state = NEMU_STOP; }
}
