#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

extern int _read_key();
extern unsigned long _uptime();
size_t events_read(void *buf, size_t len) {
  int key = _read_key();
	bool down = false;
	if (key & 0x8000) {
		key ^= 0x8000;
		down = true;
	}
	if (key == _KEY_NONE) {
		unsigned long t = _uptime();
		sprintf(buf, "t %d\n", t);
	}
	else {
		sprintf(buf, "%s %s\n", down ? "kd" : "ku", keyname[key]);
	}
	return strlen(buf);
}

static char dispinfo[128] __attribute__((used));

extern void get_screen(int *_width, int *_height);
extern void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h);

void dispinfo_read(void *buf, off_t offset, size_t len) {
  strncpy(buf, dispinfo + offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
  int width = 0;
  int height = 0;
  get_screen(&width, &height);

  int x = (offset / 4) % width;
  int y = (offset / 4) / width;
  
  int draw_len = len / 4;
  if(draw_len <= (width - x)){
    _draw_rect(buf, x, y, draw_len, 1);
    return;
  }
  int more_h = 1 + (draw_len - (width - x)) / width;
  if(more_h == 1){
    _draw_rect(buf, x, y, (width - x), 1);
    _draw_rect(buf + (width - x)*4, 0, y + 1, draw_len - (width - x), 1);
    return;
  }
  _draw_rect(buf, x, y, (width - x), 1);
  for(int i = 0; i < more_h - 1; i++){
    _draw_rect(buf + (width - x)*4 + width*i*4, 0, y + 1 + i, width, 1);
  }
  _draw_rect(buf + (width - x)*4 + width*(more_h - 1)*4, 0, y + more_h, draw_len - (width - x) - (more_h-1)*width, 1);

}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  int width = 0;
  int height = 0;
  get_screen(&width, &height);
  sprintf(dispinfo,"WIDTH:%d\nHEIGHT:%d\n",width,height);  
}
