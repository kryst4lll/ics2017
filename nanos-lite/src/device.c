#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  return 0;
}

static char dispinfo[128] __attribute__((used));

extern void get_screen(int *_width, int *_height);
extern void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h);

void dispinfo_read(void *buf, off_t offset, size_t len) {
}

void fb_write(const void *buf, off_t offset, size_t len) {
  int width = 0;
  int height = 0;
  get_screen(&width, &height);

    // 计算偏移量对应的坐标
    int x = (offset % (width * 4)) / 4; // 假设每个像素 4 字节
    int y = offset / (width * 4);

    // 计算矩形的宽度和高度
    int rect_width = (len < (width - x) * 4) ? len / 4 : (width - x);
    int rect_height = 1;
    size_t remaining_len = len - rect_width * 4;

    // 更准确地计算矩形高度
    while (remaining_len > 0) {
        rect_height++;
        int available_width = width;
        if (remaining_len < available_width * 4) {
            rect_width = remaining_len / 4;
        }
        remaining_len -= rect_width * 4;
    }

    // 调用 _draw_rect 函数
    _draw_rect((const uint32_t *)buf, x, y, rect_width, rect_height);  
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
}
