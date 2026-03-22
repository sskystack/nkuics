#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

//1
static int cmd_si(char *args) {
  uint64_t n = 1;

  if (args != NULL) {
    char *endptr = NULL;
    //获取args中的数字，默认为1，提取结束后endptr指向第一个非数字字符
    n = strtoull(args, &endptr, 10);

    if (endptr == args || n == 0) {
      printf("Usage: si [N]\n");
      return 0;
    }
  }

  cpu_exec(n);
  return 0;
}

//2
static int cmd_info(char *args) {
  //提取args中的第一个子命令，以空格为分隔符
  char *subcmd = (args == NULL ? NULL : strtok(args, " "));

  if (subcmd == NULL) {
    printf("Usage: info r\n");
    return 0;
  }

  if (strcmp(subcmd, "r") == 0) {
    int i;
    for (i = 0; i < 8; i ++) {
      //寄存器名称、16进制值、10进制值
      printf("%s\t0x%08x\t%u\n", regsl[i], reg_l(i), reg_l(i));
    }
    printf("eip\t0x%08x\t%u\n", cpu.eip, cpu.eip);
    return 0;
  }

  if (strcmp(subcmd, "w") == 0) {
    print_wp();
    return 0;
  }

  printf("Unknown info subcommand '%s'\n", subcmd);
  return 0;
}

static int cmd_w(char *args) {
  WP *wp = new_wp(args);
  if (wp != NULL) {
    printf("Watchpoint %d: %s\n", wp->NO, wp->expr);
  }
  return 0;
}

static int cmd_d(char *args) {
  if (args == NULL) {
    printf("Usage: d N\n");
    return 0;
  }

  char *endptr = NULL;
  long no = strtol(args, &endptr, 10);
  if (endptr == args) {
    printf("Usage: d N\n");
    return 0;
  }

  if (free_wp((int)no)) {
    printf("Watchpoint %ld deleted.\n", no);
  } else {
    printf("No watchpoint number %ld.\n", no);
  }
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);
static int cmd_p(char *args);
static int cmd_x(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "si", "Step through execution by N instructions (default 1)", cmd_si },
  { "info", "Print program status, e.g. info r", cmd_info },
  { "p", "Evaluate expression, usage: p EXPR", cmd_p },
  { "x", "Scan memory, usage: x N EXPR", cmd_x },
  { "w", "Set a watchpoint, usage: w EXPR", cmd_w },
  { "d", "Delete a watchpoint, usage: d N", cmd_d },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_p(char *args) {
  if (args == NULL || args[0] == '\0') {
    printf("Usage: p EXPR\n");
    return 0;
  }

  bool success = false;
  uint32_t result = expr(args, &success);

  if (success) {
    printf("%u\n", result);
  } else {
    printf("Invalid expression: %s\n", args);
  }

  return 0;
}

static int cmd_x(char *args) {
  if (args == NULL || args[0] == '\0') {
    printf("Usage: x N EXPR\n");
    return 0;
  }

  // Parse the arguments
  char *n_str = strtok(args, " ");
  char *expr = strtok(NULL, " ");

  if (n_str == NULL || expr == NULL) {
    printf("Usage: x N EXPR\n");
    return 0;
  }

  // Convert N to an integer
  char *endptr = NULL;
  int n = strtol(n_str, &endptr, 10);
  if (endptr == n_str || n <= 0) {
    printf("Invalid N: %s\n", n_str);
    return 0;
  }

  // Parse the expression as a hexadecimal number
  uint32_t addr = strtoul(expr, &endptr, 16);
  if (endptr == expr || *endptr != '\0') {
    printf("Invalid address expression: %s\n", expr);
    return 0;
  }

  // Print memory content
  printf("Memory content at 0x%x:\n", addr);
  for (int i = 0; i < n; i++) {
    uint32_t data = vaddr_read(addr + i * 4, 4); // Read 4 bytes at a time
    printf("0x%08x: 0x%08x\n", addr + i * 4, data);
  }

  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
