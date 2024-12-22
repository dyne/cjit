#include "tet.h"
#include "utils.c"
#include "font.c"

#define VBUFLEN 40000

unsigned main_prog_id;
GLuint main_vao;
GLuint main_vbo;
float vbuf[VBUFLEN];
int vbuf_n;
float color_r, color_g, color_b;

// render a line of text optionally with a %d value in it
void text(char *fstr, int value)
{
        if (!fstr) return;
        char str[100];
        snprintf(str, 99, fstr, value);
        font_begin(win_x, win_y);
        font_add_text(str, text_x, text_y, 3 * bs / 4);
        font_end(1, 1, 1);
        text_y += bs * 125 / 100 + (fstr[strlen(fstr) - 1] == ' ' ? bs : 0);
}

void draw_setup()
{
        fprintf(stderr, "GLSL version on this system is %s\n", (char *)glGetString(GL_SHADING_LANGUAGE_VERSION));
        unsigned int vertex = file2shader(GL_VERTEX_SHADER, "examples/tetris/shaders/main.vert");
        unsigned int fragment = file2shader(GL_FRAGMENT_SHADER, "examples/tetris/shaders/main.frag");
        main_prog_id = glCreateProgram();
        glAttachShader(main_prog_id, vertex);
        glAttachShader(main_prog_id, fragment);
        glLinkProgram(main_prog_id);
        check_program_errors(main_prog_id, "main");
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        glGenVertexArrays(1, &main_vao);
        glGenBuffers(1, &main_vbo);
}

void vertex(float x, float y, float r, float g, float b)
{
        if (vbuf_n >= VBUFLEN - 5) return;
        vbuf[vbuf_n++] = x;
        vbuf[vbuf_n++] = y;
        vbuf[vbuf_n++] = r;
        vbuf[vbuf_n++] = g;
        vbuf[vbuf_n++] = b;
}

void rect(float x, float y, float w, float h)
{
        vertex(x    , y    , color_r, color_g, color_b);
        vertex(x + w, y    , color_r, color_g, color_b);
        vertex(x    , y + h, color_r, color_g, color_b);
        vertex(x    , y + h, color_r, color_g, color_b);
        vertex(x + w, y    , color_r, color_g, color_b);
        vertex(x + w, y + h, color_r, color_g, color_b);
}

void draw_start()
{
        glViewport(0, 0, win_x, win_y);
        glClear(GL_COLOR_BUFFER_BIT);
}

void draw_end()
{
        glUseProgram(main_prog_id);

        float near = -1.f;
        float far = 1.f;
        float x = 1.f / (win_x / 2.f);
        float y = -1.f / (win_y / 2.f);
        float z = -1.f / ((far - near) / 2.f);
        float tz = -(far + near) / (far - near);
        float ortho[] = {
                x, 0, 0,  0,
                0, y, 0,  0,
                0, 0, z,  0,
               -1, 1, tz, 1,
        };
        glUniformMatrix4fv(glGetUniformLocation(main_prog_id, "proj"), 1, GL_FALSE, ortho);

        glBindVertexArray(main_vao);
        glBindBuffer(GL_ARRAY_BUFFER, main_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof vbuf, vbuf, GL_STATIC_DRAW);

        // show GL where the position data is
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
        glEnableVertexAttribArray(0);

        // show GL where the color data is
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(2 * sizeof(float)));
        glEnableVertexAttribArray(1); 

        glDrawArrays(GL_TRIANGLES, 0, vbuf_n / 5);
        if (vbuf_n > VBUFLEN * 3 / 4)
                fprintf(stderr, "vbuf fullness (%d/%d)\n", vbuf_n, VBUFLEN);
        vbuf_n = 0;
}

// set the current draw color to the color assoc. with a shape
void set_color_from_shape(int shape, int shade)
{
        color_r = MAX((colors[shape] >> 16 & 0xFF) + shade, 0) / 255.f;
        color_g = MAX((colors[shape] >>  8 & 0xFF) + shade, 0) / 255.f;
        color_b = MAX((colors[shape] >>  0 & 0xFF) + shade, 0) / 255.f;
}

void set_color(int r, int g, int b)
{
        color_r = r / 255.f;
        color_g = g / 255.f;
        color_b = b / 255.f;
}

void draw_menu()
{
        if (state != MAIN_MENU && state != NUMBER_MENU) return;

        menu_pos = MAX(menu_pos, 0);
        menu_pos = MIN(menu_pos, state == NUMBER_MENU ? 3 : 2);
        p = play; // just grab first player :)

        set_color(0, 0, 0);
        rect(p->held.x,
             p->held.y + p->box_w + bs2 + line_height * (menu_pos + 1),
             p->board_w,
             line_height);
        draw_end();

        text_x = p->held.x;
        text_y = p->held.y + p->box_w + bs2;
        if (state == MAIN_MENU)
        {
                text("Main Menu"        , 0);
                text("Endless"          , 0);
                text("Garbage Race"     , 0);
                text("Quit"             , 0);
        }
        else if (state == NUMBER_MENU)
        {
                text("How many players?", 0);
                text("1"                , 0);
                text("2"                , 0);
                text("3"                , 0);
                text("4"                , 0);
        }
}

void draw_particles()
{
        set_color(254, 254, 254);
        for (int i = 0; i < NPARTS; i++)
        {
                if (parts[i].r <= 0.5f)
                        continue;
                rect(parts[i].x, parts[i].y, parts[i].r, parts[i].r);
        }
}

// draw a single mino (square) of a shape
void draw_mino(int x, int y, int shape, int outline, int part)
{
        if (!part) return;
        int bw = MAX(1, outline ? bs / 10 : bs / 6);
        set_color_from_shape(shape, -50);
        rect(x, y, bs, bs);
        set_color_from_shape(shape, outline ? -255 : 0);
        rect( // horizontal band
                        x + (part & 8 ? 0 : bw),
                        y + bw,
                        bs - (part & 8 ? 0 : bw) - (part & 2 ? 0 : bw),
                        bs - bw - bw);
        rect( // vertical band
                        x + (part & 32 ? 0 : bw),
                        y + (part & 1 ? 0 : bw),
                        bs - (part & 32 ? 0 : bw) - (part & 16 ? 0 : bw),
                        bs - (part & 1 ? 0 : bw) - (part & 4 ? 0 : bw));
}

#define CENTER 1
#define OUTLINE 2

void draw_shape(int x, int y, int color, int rot, int flags)
{
        for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++)
                draw_mino(
                        x + bs * i + ((flags & CENTER) ? bs2 + bs2 * center[2 * color    ] : 0),
                        y + bs * j + ((flags & CENTER) ? bs  + bs2 * center[2 * color + 1] : 0),
                        color,
                        flags & OUTLINE,
                        is_solid_part(color, rot, i, j)
                );
}

// draw everything in the game on the screen for current player
void draw_player()
{
        if (state == MAIN_MENU || state == NUMBER_MENU) return;

        int x = p->board_x + bs * p->shake_x;
        int y = p->board_y + bs * p->shake_y;

        // draw background, black boxes
        set_color(16, 26, 24);
        rect(p->held.x, p->held.y, p->box_w, p->box_w);
        rect(x, y, p->board_w, bs * VHEIGHT);

        // find ghost piece position
        int ghost_y = p->it.y;
        while (ghost_y < BHEIGHT && !collide(p->it.x, ghost_y + 1, p->it.rot))
                ghost_y++;

        // draw shadow
        if (p->it.color)
        {
                struct shadow shadow = shadows[p->it.rot][p->it.color];
                int top = MAX(0, p->it.y + shadow.y - 5);
                set_color(8, 13, 12);
                rect(x + bs * (p->it.x + shadow.x),
                     y + bs * top,
                     bs * shadow.w,
                     MAX(0, bs * (ghost_y - top + shadow.y - 5)));
        }

        // draw hard drop beam
        float loss = .1f * (tick - p->beam_tick);
        if (loss < 1.f && p->beam.color)
        {
                struct shadow shadow = shadows[p->beam.rot][p->beam.color];
                int rw = bs * shadow.w;
                int rh = bs * (p->beam.y + shadow.y - 5);
                int lossw = (1.f - ((1.f - loss) * (1.f - loss))) * rw;
                int lossh = loss < .5f ? 0.f : (1.f - ((1.f - loss) * (1.f - loss))) * rh;
                set_color(66, 74, 86);
                rect(x + bs * (p->beam.x + shadow.x) + lossw / 2,
                     y + lossh,
                     rw - lossw,
                     rh - lossh);
        }

        // draw pieces on board
        for (int i = 0; i < BWIDTH; i++) for (int j = 0; j < BHEIGHT; j++)
                draw_mino(x + bs * i, y + bs * (j-5) - p->row[j].offset,
                                p->row[j].col[i].color, 0, p->row[j].col[i].part);

        // draw queued garbage
        p->top_garb = y + bs * VHEIGHT;
        int garb_height = 0;
        for (int i = 0; i < GARB_LVLS; i++)
                for (int j = 0; j < p->garbage[i]; j++)
                {
                        draw_mino(x - 3 * bs2, (p->top_garb -= bs), (3 - i) + 9, 0, '@');
                        garb_height++;
                }
        if (p->flash > 8)
        {
                int flash_sq = p->flash * p->flash;
                set_color(flash_sq, flash_sq, flash_sq);
                rect(x - 3 * bs2 - bs4, p->top_garb - bs4, bs + bs4 + bs4, garb_height * bs + bs2);
        }
        p->top_garb = y + bs * VHEIGHT;
        for (int i = 0; i < GARB_LVLS; i++)
                for (int j = 0; j < p->garbage[i]; j++)
                        draw_mino(x - 3 * bs2, (p->top_garb -= bs), (3 - i) + 9, 0, '@');

        // draw falling piece & ghost
        draw_shape(x + bs * p->it.x, y + bs * (ghost_y - 5), p->it.color, p->it.rot, OUTLINE);
        draw_shape(x + bs * p->it.x, y + bs * (p->it.y - 5), p->it.color, p->it.rot, 0);

        // draw next pieces
        for (int n = 0; n < 5; n++)
                draw_shape(p->preview_x, p->preview_y + 3 * bs * n, p->next[n], 0, CENTER);

        // draw held piece
        draw_shape(p->held.x, p->held.y, p->held.color, 0, CENTER);

        draw_end();

        // draw scores etc
        text_x = p->held.x;
        text_y = p->held.y + p->box_w + bs2;
        text("%d pts ", p->score);
        text("%d lines ", p->lines);

        int secs = p->ticks / 120 % 60;
        int mins = p->ticks / 120 / 60 % 60;
        char minsec[80];
        sprintf(minsec, "%d:%02d.%02d ", mins, secs, p->ticks % 120 * 1000 / 1200);
        text(minsec, 0);
        text(p->dev_name, 0);
        if (p->combo > 1) text("%d combo ", p->combo);
        if (p->tspin == TSPIN_FULL)
                text("T-SPIN", 0);
        else if (p->tspin == TSPIN_MINI)
                text("T-SPIN MINI", 0);

        if (p->reward)
        {
                text_x = p->reward_x - bs;
                text_y = p->reward_y--;
                text("%d", p->reward);
        }

        text_x = x + bs2;
        text_y = y + bs2 * 19;
        if (p->countdown_time > 0)
                text(countdown_msg[p->countdown_time / CTDN_TICKS], 0);

        if (state == ASSIGN)
                text(p >= play + assign_me ? "Press button to join" : p->dev_name, 0);

        if (state == GAMEOVER) text("Game over", 0);
}

void reflow()
{
        float strength = 0.0005f * (1 + rand() % 10);
        for (int n = 0; n < NFLOWS; n++)
        {
                flows[n].x = rand() % win_x;
                flows[n].y = rand() % win_y;
                flows[n].r = rand() % 100 + 100;
                flows[n].vx = (rand() % 10 - 5) * strength;
                flows[n].vy = (rand() % 10 - 5) * strength;
        }
}

// recalculate sizes and positions on resize
void resize(int x, int y)
{
        win_x = x;
        win_y = y;
        bs = MIN(win_x / (nplay * 22), win_y / 24);
        bs2 = bs / 2;
        bs4 = bs / 4;
        line_height = bs * 125 / 100;
        int n = 0;
        for (p = play; p < play + nplay; p++, n++)
        {
                p->board_x = (x / (nplay * 2)) * (2 * n + 1) - bs2 * BWIDTH;
                p->board_y = (y / 2) - bs2 * VHEIGHT;
                p->board_w = bs * 10;
                p->box_w = bs * 5;
                p->held.x = p->board_x - p->box_w - bs2;
                p->held.y = p->board_y;
                p->preview_x = p->board_x + p->board_w + bs2;
                p->preview_y = p->board_y;
        }
        reflow();
}
