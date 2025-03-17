#include "nemu.h"
#include "monitor/monitor.h"
// #include "monitor/watchpoint.h"
#include "/home/yyh/ics2017/nemu/src/monitor/debug/watchpoint.c"

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
    /* TODO: check watchpoints here. */
    /* 检查监视点 */
    WP *wp = head;
    while (wp != NULL) {
      // 对监视点的表达式进行求值
      bool success;
      uint32_t new_value = expr(wp->expr, &success);
      if (!success) {
        printf("Error evaluating expression: %s\n", wp->expr);
        break;
      }
      // 检查值是否发生变化
      if (new_value != wp->value) {
        printf("Watchpoint %d: %s\n", wp->NO, wp->expr);
        printf("Old value = %u\n", wp->value);
        printf("New value = %u\n", new_value);
        printf("Program paused.\n");
        // 暂停程序
        nemu_state = NEMU_STOP;
        return;
      }
      // 更新监视点的值
      wp->value = new_value;
      // 检查下一个监视点
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
