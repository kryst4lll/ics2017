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

static int cmd_q(char *args) {
  return -1;
}

static int cmd_si(char *args) {
  int steps = 1;
  if(args){
    steps = atoi(args);
    if(steps <= 0){
      printf("invalid steps!\n");  
    }
  }
  cpu_exec(steps);
  return 0;
}

static int cmd_info(char *args) {
  if(strcmp(args, "r") == 0){
    // printf("The string is: %s\n", args);
    for(int i = 0; i < 8; i++){
      printf("%u\n",cpu.gpr[i]._32);
    }
  }else{
    printf("invalid input!\n");
  }
  return 0;
}

uint32_t mem_read(uint32_t addr) {
  // static uint8_t memory[0x200000] = {0}; 
  return *(uint32_t *)(addr);
}

static int cmd_x(char* args) {
  // printf("The string is: %s\n", args);
  int n;
  uint32_t addr;
  if(sscanf(args, "%d 0x%x", &n, &addr) != 2){
    printf("invalid arguments!");
    return 0;
  }
  if (n <= 0) {
    printf("Invalid number of units: %d\n", n);
    return 0;
  }
  // printf("data: %x\n",addr);
  // uint32_t * addr_t = (uint32_t *)(addr);
  // printf("data: %x\n",*addr_t);
  // for(int i = 0; i < n; i++){
  //   printf("data: %x\n",*(uint32_t*)(addr+i*4));
  // }
  // 输出内存数据
  for (int i = 0; i < n; i++) {
    uint32_t value = vaddr_read(addr + i * 4, 4); // 读取 4 字节
    printf("0x%08x: 0x%08x\n", addr + i * 4, value);
  }

  return 0;
}

static int cmd_help(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Execute N steps", cmd_si},
  { "info", "Print regs' status", cmd_info},
  { "x", "Scan memory: x N 0xADDR", cmd_x},
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
