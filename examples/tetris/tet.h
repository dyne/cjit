#pragma once
#include <SDL2/SDL.h>

#define GL3_PROTOTYPES 1

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

#define BWIDTH 10  // board width, height
#define BHEIGHT 25
#define VHEIGHT 20 // visible height
#define BAG_SZ 7   // bag size
#define GARB_LVLS 4 // levels of queued garbage
#define NPLAY 4
#define NPARTS 1000
#define NFLOWS 20
#define CTDN_TICKS 96
#define SHOW_FPS 0

// collision test results
enum { NONE = 0, WALL, NORMAL };

enum { TSPIN_NONE = 0, TSPIN_FULL, TSPIN_MINI };

// Bits in each letter indicate which sides connect when drawing
// A 1000001    - up
// B 1000010    - right
// D 1000100    - down
// H 1001000    - left
//   1010000    - connect left corner where up or down connects - for square piece
//   1100000    - ...     right ...

char shapes[] =
        ".... D... ..D. .Vl. .... BL.. .FH. ;D;. "
        ".... CJH. BJI. .Si. BJJH .CH. BI.. BKH. "
        ".... .... .... .... .... .... .... ,.,. "
        ".... .... .... .... .... .... .... .... "

        ".... .FH. .D.. .Vl. ..D. ..D. .D.. ,D;. "
        ".... .E.. .E.. .Si. ..E. .FI. .CL. .GH. "
        ".... .A.. .CH. .... ..E. .A.. ..A. ,A;. "
        ".... .... .... .... ..A. .... .... .... "

        ".... .... .... .Vl. .... .... .... ,.,. "
        ".... BJL. FJH. .Si. .... BL.. .FH. BNH. "
        ".... ..A. A... .... BJJH .CH. BI.. ;A;. "
        ".... .... .... .... .... .... .... .... "

        ".... .D.. BL.. .Vl. .D.. .D.. D... ;D,. "
        ".... .E.. .E.. .Si. .E.. FI.. CL.. BM.. "
        ".... BI.. .A.. .... .E.. A... .A.. ;A,. "
        ".... .... .... .... .A.. .... .... .... ";

struct shadow { int x, w, y; } shadows[4][8] = { // pre-computed shadow positions for each piece
        {{0,0,0}, {0,3,1}, {0,3,1}, {1,2,0}, {0,4,1}, {0,3,1}, {0,3,1}, {0,3,1}},
        {{0,0,0}, {1,2,0}, {1,2,2}, {1,2,0}, {2,1,0}, {1,2,1}, {1,2,1}, {1,2,1}},
        {{0,0,0}, {0,3,1}, {0,3,1}, {1,2,0}, {0,4,2}, {0,3,2}, {0,3,2}, {0,3,1}},
        {{0,0,0}, {0,2,2}, {0,2,0}, {1,2,0}, {1,1,0}, {0,2,1}, {0,2,1}, {0,2,1}},
};

// helps center shapes in preview box
int center[] = {0,0, 1,1, 1,1, 0,1, 0,0, 1,1, 1,1, 1,1};

int colors[] = {
        0x000000, // unused
        0x1983c4, // J
        0xfa8333, // L
        0xffca39, // square
        0x1be7ff, // line
        0xff5a5f, // Z
        0x89c926, // S
        0x88488f, // T
        0xffffff, // shine color
        0x6f7866, // garbage colors
        0x9fa896,
        0xffca39,
        0xff5a5f,
};

int kicks[] = {   // clockwise                            counterclockwise
        0,0,  -1, 0,  -1, 1,   0,-2,  -1,-2,     0,0,   1, 0,   1, 1,   0,-2,   1,-2, // rot 0
        0,0,  -1, 0,  -1,-1,   0, 2,  -1, 2,     0,0,  -1, 0,  -1,-1,   0, 2,  -1, 2, // rot 1
        0,0,   1, 0,   1, 1,   0,-2,   1,-2,     0,0,  -1, 0,  -1, 1,   0,-2,  -1,-2, // rot 2
        0,0,   1, 0,   1,-1,   0, 2,   1, 2,     0,0,   1, 0,   1,-1,   0, 2,   1, 2, // rot 3
                // line-clockwise                       line-counterclockwise
        0,0,   2, 0,  -1, 0,   2,-1,  -1, 2,     0,0,   1, 0,  -2, 0,   1, 2,  -2,-1, // rot 0
        0,0,  -2, 0,   1, 0,  -2, 1,   1,-2,     0,0,   1, 0,  -2, 0,   1, 2,  -2,-1, // rot 1
        0,0,  -1, 0,   2, 0,  -1,-2,   2, 1,     0,0,  -2, 0,   1, 0,  -2, 1,   1,-2, // rot 2
        0,0,   2, 0,  -1, 0,   2,-1,  -1, 2,     0,0,  -1, 0,   2, 0,  -1,-2,   2, 1, // rot 3
};

float combo_bonus[] = {
        1.f, 1.5f, 2.f, 3.f, 4.f, 5.f, 6.f, 8.f, 10.f, 12.f, 15.f, 20.f,
        25.f, 30.f, 40.f, 50.f, 75.f, 100.f
};
#define MAX_COMBO ((sizeof combo_bonus / sizeof *combo_bonus) - 1)

int rewards[] = {0, 100, 250, 500, 1000}; // points for clearing 0,1,2,3,4 lines

int speeds[] = {100, 80, 70, 60, 52, 46, 40, 35, 30, 26, 22, 18, 15, 12, 10, 8, 6, 5, 4, 3, 2};
#define MAX_SPEED ((sizeof speeds / sizeof *speeds) - 1)

char countdown_msg[][20] = {"        Go!", "       - 1 -", "       - 2 -", "       - 3 -"};

struct piece { int x, y, rot, color; };
struct spot { int color, part; };
struct row {
        struct spot col[BWIDTH];
        int fullness;
        int special;
        int offset;
};

struct player {
        struct row row[BHEIGHT];        // the board, excluding the falling piece
        int left, right, down;          // true when holding a direction
        int move_cooldown;              // cooldown before hold-to-repeat movement
        struct piece it;                // current falling piece - "it"
        struct piece beam;              // hard drop beam
        struct piece held;              // shape in the hold box
        int beam_tick;                  // tick that beam was created
        int hold_uses;                  // have we swapped with the hold already?
        int bag[BAG_SZ];                // "bag" of upcoming pieces
        int bag_idx;                    // last position used up in bag
        int next[5];                    // next pieces in preview (take from bag)
        int grounded;                   // is piece on the ground?
        int grounded_moves;             // how many moves have we made on the ground?
        int last_dx_tick;               // tick of most recent left/right movement
        int lines, score, best;         // scoring
        int combo;                      // clears in-a-row
        int reward, reward_x, reward_y; // for hovering points indicator
        int garbage[GARB_LVLS + 1];     // queued garbage, e.g. received from opponents
        int garbage_tick;               // keeps track of when to age garbage
        int garbage_remaining;          // how many lines of garbage remain to clear to win
        int garbage_bits;               // fractions of garbage attached to each particle
        int top_garb;                   // highest position of garbage stack drawn
        int level;                      // difficultly level (lines/10)
        int countdown_time;             // ready-set-go countdown
        int idle_time;                  // how long the player has been idle in ticks
        int shiny_lines;
        int shine_time;                 // delay in ticks before clearing line(s)
        int dead_time;                  // delay in ticks after game over
        int board_x, board_y, board_w;  // positions and sizes of things
        int preview_x, preview_y;       // position of preview
        int box_w;                      // width of hold box / preview box
        int ticks;                      // counts up while game is going
        int seed1, seed2;               // make garbage and bags fair
        float shake_x, shake_y;         // amount the board is offset by shaking
        int flash;                      // flashing from receiving garbage
        int tspin;
        int device;                     // SDL's input device id
        char dev_name[80];              // input device "name"
} play[NPLAY], *p;                      // one per player

struct particle { float x, y, r, vx, vy; int opponent, bits; };
struct particle parts[NPARTS];
struct particle flows[NFLOWS];

enum state { MAIN_MENU = 0, NUMBER_MENU, ASSIGN, PLAY, GAMEOVER} state;
int win_x = 1000;         // window size
int win_y = 750;
int bs, bs2, bs4;         // individual block size, in half, in quarters
int tick;                 // counts up one per frame
int nplay = 1;            // number of players
int assign_me;            // who is getting an input device assigned?
int menu_pos;             // current position in menu
int text_x, text_y;       // position of text drawing
int line_height;          // text line height
int garbage_race;
int npart;
int seed;

SDL_GLContext ctx;
SDL_Event event;
SDL_Window *win;
SDL_Renderer *renderer;

void do_events();
void setup();
void update_player();
void move(int dx, int dy, int gravity);
void reset_fall();
void bake();
void new_game();
int is_solid_part(int shape, int rot, int i, int j);
int is_tspin_part(int shape, int rot, int i, int j);
int collide(int x, int y, int rot);
void update_particles();
