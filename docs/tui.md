# Terminal User Interface

Believe it or not, one can draw a beautiful terminal user interface
(also called TUI) on a text console, with colors and blinking lights
too. Our choice is to use [termbox2](https://github.com/termbox/termbox2)
whose API improves a lot on older approaches such as ncurses
or slang, by [sucking much less](https://suckless.org) `:^)`

Here is a quick synopsis:

```c
#define TB_IMPL
#include "termbox2.h"

int main(int argc, char **argv) {
    struct tb_event ev;
    int y = 0;

    tb_init();

    tb_printf(0, y++, TB_GREEN, 0, "hello from termbox");
    tb_printf(0, y++, 0, 0, "width=%d height=%d",
              tb_width(), tb_height());
    tb_printf(0, y++, 0, 0, "press any key...");
    tb_present();

    tb_poll_event(&ev);

    y++;
    tb_printf(0, y++, 0, 0, "event type=%d key=%d ch=%c",
              ev.type, ev.key, ev.ch);
    tb_printf(0, y++, 0, 0, "press any key to quit...");
    tb_present();

    tb_poll_event(&ev);
    tb_shutdown();

    return 0;
}
```

The termbox2 basic API is self explanatory:

```c
int tb_init();
int tb_shutdown();

int tb_width();
int tb_height();

int tb_clear();
int tb_present();

int tb_set_cursor(int cx, int cy);
int tb_hide_cursor();

int tb_set_cell(int x, int y, uint32_t ch,
                uintattr_t fg, uintattr_t bg);

int tb_peek_event(struct tb_event *event,
                  int timeout_ms);
int tb_poll_event(struct tb_event *event);

int tb_print(int x, int y,
             uintattr_t fg, uintattr_t bg,
             const char *str);
int tb_printf(int x, int y,
              uintattr_t fg, uintattr_t bg,
              const char *fmt, ...);
```

To see a demo of what it can do, see the
[examples/termbox2.c](https://github.com/dyne/cjit/blob/main/examples/termbox2.c)
example which will draw a keyboard on your terminal and show the keys
you are pressing in real-time. Its code is a great source of knowledge
about what termbox2 can do!
