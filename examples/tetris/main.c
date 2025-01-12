// Tet -- http://tinyc.games -- (c) 2023 Jer Wilson
//
// Tet is tiny implementation of a fully-featured Tetris clone.

#pragma comment(lib, "SDL2")

#ifdef _WIN32
#pragma comment(lib, "opengl32")
#elif LINUX
#pragma comment(lib, "OpenGL")
#else
#pragma comment(lib, "OpenGL")
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "tet.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

// common tinyc.games
#include "utils.c"
#include "audio.c"
#include "font.c"

// tetris
#include "graphics.c"
#include "input.c"
#include "timer.h"

int get_interval()
{
        static int then = 0;
        int now = SDL_GetTicks();
        int diff = now - then;
        then = now;
        return diff;
}

// the entry point and main game loop
int main(int argc, char **argv)
{
        float frame_times[10] = {0.f};
        int frame_time_idx = 0;
        setup();
        float accum_msec = 0.f;
        for (;;)
        {
                accum_msec += (float)get_interval();

                if (accum_msec < 7.3333f)
                {
                        int sleep_for = 8 - (int)accum_msec;
                        TIMECALL(SDL_Delay, (sleep_for));
                        //fprintf(stderr, "Slept for %dms\n", sleep_for);
                }

                accum_msec += (float)get_interval();

                for (; accum_msec > 8.3333f; accum_msec -= 8.3333f)
                {
                        TIMECALL(do_events, ());
                        TIMER(update_player);
                        for (p = play; p < play + nplay; p++) update_player();
                        accum_msec += (float)get_interval();
                        TIMECALL(update_particles, ());
                        // MAX_LOOPS?
                }

                TIMECALL(draw_start, ());
                TIMECALL(draw_menu, ());
                TIMER(draw_player);
                for (p = play; p < play + nplay; p++) draw_player();
                TIMECALL(draw_particles, ());
                TIMECALL(draw_end, ());
                TIMECALL(SDL_GL_SwapWindow, (win));

                tick++;

                unsigned long long now = SDL_GetPerformanceCounter();
                static unsigned long long then;
                float diff = (float)(now - then) / (float)SDL_GetPerformanceFrequency() * 1000.f;
                frame_times[frame_time_idx++] = diff;
                frame_time_idx %= 10;
                then = now;

                if (SHOW_FPS && frame_time_idx == 0)
                {
                        float sum = 0.f;
                        char timings_buf[8000];
                        timer_print(timings_buf, 8000, true);
                        fprintf(stderr, "%s", timings_buf);
                        for (int i = 0; i < 10; i++)
                                sum += frame_times[i];
                        fprintf(stderr, "avg frame time %.2fms = %.1f fps\n",
                                        sum / 10.f, 10000.f / sum);
                }
        }
}

void do_events()
{
        static int joy_tick = 0;

        while (SDL_PollEvent(&event)) switch (event.type)
        {
                case SDL_QUIT:                 exit(0);
                case SDL_KEYDOWN:              key_down();      break;
                case SDL_KEYUP:                key_up();        break;
                case SDL_CONTROLLERBUTTONDOWN: joy_down();      break;
                case SDL_CONTROLLERBUTTONUP:   joy_up();        break;
                case SDL_JOYDEVICEADDED:
                case SDL_JOYDEVICEREMOVED:     joy_tick = tick; break;
                case SDL_WINDOWEVENT:
                        if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                                resize(event.window.data1, event.window.data2);
                        break;
        }

        if (joy_tick == tick - 1) joy_setup();
}

#ifndef __APPLE__
void GLAPIENTRY
MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                GLsizei length, const GLchar* message, const void* userParam)
{
        if (type != GL_DEBUG_TYPE_ERROR) return; // too much yelling
        fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
                        ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
                        type, severity, message );
        exit(-7);
}
#endif

// initial setup to get the window and rendering going
void setup()
{
        srand(time(NULL));
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO);

        win = SDL_CreateWindow("Tet", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                win_x, win_y, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
        if (!win) exit(fprintf(stderr, "%s\n", SDL_GetError()));
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        ctx = SDL_GL_CreateContext(win);
        if (!ctx) exit(fprintf(stderr, "Could not create GL context\n"));

        SDL_GL_SetSwapInterval(0); // vsync?
        glClearColor(0.18f, 0.18f, 0.18f, 1.f);

        #ifndef __APPLE__
        glewExperimental = GL_TRUE;
        glewInit();
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, 0);
        #endif

        resize(win_x, win_y);
        audio_init();
        font_init();
        draw_setup();
}

unsigned garb_rand()
{
        return (p->seed1 = (1103515245 * p->seed1 + 12345) % 2147483648);
}

unsigned bag_rand()
{
        return (p->seed2 = (1103515245 * p->seed2 + 13456) % 2147483648);
}

void age_garbage()
{
        p->garbage[0] += p->garbage[1];
        for (int i = 1; i < GARB_LVLS; i++)
                p->garbage[i] = p->garbage[i + 1]; // last loop looks like it goes oob, but there's room!
        p->garbage_tick = tick;
}

void receive_garbage()
{
        int gap = garb_rand() % 10;
        int max_at_once = garbage_race ? 10 : 6; // no more than 6, 10 garbage at a time
        for (int i = 0; p->garbage[0] && i < max_at_once; p->garbage[0]--, i++)
        {
                memmove(p->row, p->row + 1, (BHEIGHT - 1) * sizeof *p->row);
                memset(&p->row[BHEIGHT - 1], 0, sizeof *p->row);
                p->row[BHEIGHT - 1].special = 1; // special = garbage
                p->row[BHEIGHT - 1].offset = (p->countdown_time ? 0 : -bs);
                p->garbage_remaining++;

                for (int i = 0; i < 10; i++)
                        if (gap != i && (!garbage_race || garb_rand() % 20))
                        {
                                p->row[BHEIGHT - 1].col[i] = (struct spot){9, '@'};
                                p->row[BHEIGHT - 1].fullness++;
                        }

                if (garbage_race) gap = garb_rand() % 10;
                audio_tone(SQUARE, A1, D1, 50, 5, 50, 200);
        }
}

void new_particle(int x, int y, int opponent, int garbage_bits)
{
        parts[npart++] = (struct particle){
                p->board_x + x * bs,
                p->board_y + (y + VHEIGHT - BHEIGHT) * bs + bs2,
                bs * 0.8f,
                (rand() % 10 - 5) * 0.02f,
                (rand() % 30 + 30) * 0.02f,
                opponent,
                garbage_bits,
        };
        if (npart >= NPARTS) npart = 0;
}

void kill_lines()
{
        int opponent = -1; // target if we end up sending garbage

        p->lines += p->shiny_lines;
        p->level = p->lines / 10;
        p->reward = combo_bonus[MIN(MAX_COMBO, p->combo)] * rewards[p->shiny_lines];
        p->combo++;
        if (p->tspin == TSPIN_FULL)
                p->reward *= 2;
        p->score += p->reward;
        int sendable = p->reward / 400; // garbage to send
        int garbage_bits = sendable * 12 / p->shiny_lines;
        p->shiny_lines = 0;

        if (sendable && nplay > 1 && !garbage_race)
        {
                opponent = rand() % (nplay - 1);
                if (play + opponent >= p)
                        opponent++;
        }

        // clean up sliced pieces
        for (int y = 0; y < BHEIGHT; y++)
        {
                p->row[y].offset = 0;
                if (p->row[y].fullness == 10)
                        for (int x = 0; x < BWIDTH; x++)
                                new_particle(x, y, opponent, garbage_bits);

                if (y > 0 && p->row[y - 1].fullness == 10)
                        for (int x = 0; x < BWIDTH; x++)
                                p->row[y].col[x].part &= ~1;

                if (y < BHEIGHT - 1 && p->row[y + 1].fullness == 10)
                        for (int x = 0; x < BWIDTH; x++)
                                p->row[y].col[x].part &= ~4;
        }

        for (int y = 0; y < BHEIGHT; y++)
        {
                if (p->row[y].fullness != 10) continue;

                if (p->row[y].special) p->garbage_remaining--;

                for (int j = y; j > 0; j--)
                {
                        p->row[j] = p->row[j - 1];
                        p->row[j].offset += bs;
                }

                memset(&p->row[0], 0, sizeof p->row[0]);
        }

        reflow();

        if (garbage_race && p->garbage_remaining == 0)
        {
                age_garbage();
                receive_garbage();
                if (p->garbage_remaining == 0)
                        state = GAMEOVER;
        }
}

void new_game()
{
        memset(p->row, 0, sizeof p->row);
        p->garbage_remaining = 0;
        p->bag_idx = BAG_SZ;
        if (p->best < p->score) p->best = p->score;
        p->score = 0;
        p->lines = 0;
        p->level = 0;
        p->held.color = 0;
        p->hold_uses = 0;
        p->countdown_time = 4 * CTDN_TICKS;
        p->seed1 = seed;
        p->seed2 = seed;

        if (garbage_race)
        {
                p->garbage[0] = 10;
                p->garbage[1] = 5;
                p->garbage[2] = 5;
                p->garbage[3] = 5;
                receive_garbage();
        }
}

// set the current piece to the top, middle to start falling
void reset_fall()
{
        p->idle_time = 0;
        p->grounded_moves = 0;
        p->it.x = 3;
        p->it.y = 3;
        p->it.rot = 0;
}

// pick a new next piece from the bag, and put the old one in play
void new_piece()
{
        if (p->bag_idx >= BAG_SZ) // need to make a new bag?
        {
                p->bag_idx = 0;

                for (int i = 0; i < BAG_SZ; i++)
                        p->bag[i] = i % 7 + 1;

                for (int i = 0; i < BAG_SZ; i++)
                        SWAP(p->bag[i], p->bag[bag_rand() % BAG_SZ]);
        }

        p->it.color = p->next[0];
        memmove(p->next, p->next + 1, sizeof *(p->next) * 4);
        p->next[4] = p->bag[p->bag_idx++];
        reset_fall();
}

//move the falling piece left, right, or down
void move(int dx, int dy, int gravity)
{
        if (!gravity)
                p->move_cooldown = p->move_cooldown ? 8 : 18;

        int collision = collide(p->it.x + dx, p->it.y + dy, p->it.rot);

        if (collision == NONE)
        {
                p->it.x += dx;
                p->it.y += dy;

                // reset idle time if you voluntarily move DOWN
                if (dy) p->idle_time = 0;

                // remember last successful x movement time
                if (dx) p->last_dx_tick = tick;

                // reset idle time if piece is grounded, limit grounded moves though
                if (p->grounded && p->grounded_moves < 15)
                {
                        p->idle_time = 0;
                        p->grounded_moves++;
                }

                if (!gravity)
                        audio_tone(TRIANGLE, C3, F3, 1, 1, 1, 1);
        }
        else if (dy && gravity)
        {
                bake();
                receive_garbage();
                if (tick != p->beam_tick)
                        audio_tone(TRIANGLE, C4, F4, 10, 10, 10, 10);
        }
        else if (collision == WALL)
        {
                if (dx == -1 && tick - p->last_dx_tick < 16)
                {
                        p->shake_x -= .25f;
                        audio_tone(TRIANGLE, C2, C2, 25, 5, 5, 25);
                }
                if (dx ==  1 && tick - p->last_dx_tick < 16)
                {
                        p->shake_x += .25f;
                        audio_tone(TRIANGLE, E2, C2, 15, 5, 5, 15);
                }
        }
}

// update everything related to current player, while the game is running normally
void update_player()
{
        if (state != PLAY) return;

        if (p->countdown_time > 0)
        {
                int note = p->countdown_time > CTDN_TICKS ? A4 : A5;
                if (p->countdown_time % CTDN_TICKS == 0)
                        audio_tone(TRIANGLE, note, note, 80, 50, 5, 20);

                if (--p->countdown_time > CTDN_TICKS)
                        return;
        }

        p->ticks++;

        while (!p->it.color && !p->shine_time && !p->dead_time)
                new_piece();

        p->grounded = collide(p->it.x, p->it.y + 1, p->it.rot);

        if (p->move_cooldown) p->move_cooldown--;

        if (p->move_cooldown < 2)
        {
                if (p->left)  move(-1, 0, 0);
                if (p->right) move( 1, 0, 0);
                if (p->down)  move( 0, 1, 0);
        }

        for (int y = 0; y < BHEIGHT; y++)
                if (p->row[y].offset > 0)
                        p->row[y].offset = MAX(0, p->row[y].offset - bs4);
                else
                        p->row[y].offset = MIN(0, p->row[y].offset + bs4);

        if (p->shine_time > 0 && --p->shine_time == 0)
                kill_lines();

        if (p->dead_time > 0)
        {
                p->dead_time--;
                int x = p->dead_time % BWIDTH;
                int y = BHEIGHT - 1 - p->dead_time / BWIDTH;
                if (p->row[y].col[x].part)
                {
                        new_particle(x, y, -1, 0);
                        p->row[y].col[x].part = 0;
                }
                if (p->dead_time == 0)
                        new_game();
        }

        if (++p->idle_time >= (p->grounded ? 100 : speeds[MIN(MAX_SPEED, p->level)]))
        {
                move(0, 1, 1);
                p->idle_time = 0;
        }

        if (tick - p->garbage_tick > (garbage_race ? 6000 : 400))
                age_garbage();

        // update shaky offsets
        if (p->shake_x < .01f && p->shake_x > -.01f) p->shake_x = .0f;
        if (p->shake_y < .01f && p->shake_y > -.01f) p->shake_y = .0f;
        if (p->shake_x) p-> shake_x *= .98f;
        if (p->shake_y) p-> shake_y *= .98f;

        // decrease flash
        p->flash = CLAMP(0, p->flash - 1, 18);
}

void game_over()
{
        memset(p->bag, 0, sizeof p->bag);
        memset(p->next, 0, sizeof p->next);
        p->dead_time = BHEIGHT * BWIDTH;
}

// check if a line has been completed and act accordingly
int increment_and_check_line(int y)
{
        if (++p->row[y].fullness != 10)
                return 0;

        p->reward = 0; // set up hovering reward number
        p->reward_x = p->board_x + bs * (p->it.x + 2);
        p->reward_y = p->board_y + bs * (p->it.y - 4);
        p->shine_time = 40;
        p->shiny_lines++;

        for (int i = 0; i < BWIDTH; i++)
                p->row[y].col[i].color = 8;
        return 1;
}

// bake the falling piece into the background/board
void bake()
{
        int completed_lines = 0;
        int tsum = 0; // for detecting t-spins
        int stuck = collide(p->it.x    , p->it.y - 1, p->it.rot)
                 && collide(p->it.x - 1, p->it.y    , p->it.rot)
                 && collide(p->it.x + 1, p->it.y    , p->it.rot);

        for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++)
        {
                int world_i = i + p->it.x;
                int world_j = j + p->it.y;
                int part = is_solid_part(p->it.color, p->it.rot, i, j);

                int tpart = is_tspin_part(p->it.color, p->it.rot, i, j);
                if (world_i < 0 || world_i >= BWIDTH || world_j < 0 || world_j >= BHEIGHT ||
                        p->row[world_j].col[world_i].color)
                        tsum += tpart;

                if (!part) continue;

                if (p->row[world_j].col[world_i].color)
                        game_over();

                p->row[world_j].col[world_i].color = p->it.color;
                p->row[world_j].col[world_i].part = part;
                completed_lines += increment_and_check_line(world_j);
        }

        p->it.color = 0;
        p->hold_uses = 0;
        p->tspin = TSPIN_NONE;

        if (stuck && tsum >= 21)
        {
                p->tspin = TSPIN_FULL;
                audio_tone(SQUARE, F2, F2, 2, 2, 2, 2);
                audio_tone(SQUARE, D4, D4, 2, 3, 1, 1);
                audio_tone(SINE, C5, C5, 20, 5, 1, 1);
                audio_tone(SINE, E5, E5, 80, 5, 1, 40);
        }
        else if (stuck && tsum == 12)
        {
                p->tspin = TSPIN_MINI;
                audio_tone(SQUARE, F2, F2, 2, 2, 2, 2);
                audio_tone(SQUARE, D4, D4, 2, 3, 1, 1);
        }
        else if (completed_lines)
        {
                for (int i = 0; i < completed_lines; i++)
                        audio_tone(SINE, G3, G5, 20, 50, 50, 200);
        }
        else
        {
                p->combo = 0; // break combo
        }
}

// check if a sub-part of the falling shape is solid at a particular rotation
int is_solid_part(int shape, int rot, int i, int j)
{
        int base = shape * 5 + rot * 5 * 8 * 4;
        int part = shapes[base + j * 5 * 8 + i];
        return part < '@' ? 0 : part;
}

int is_tspin_part(int shape, int rot, int i, int j)
{
        int base = shape * 5 + rot * 5 * 8 * 4;
        int part = shapes[base + j * 5 * 8 + i];
        return part == ',' ? 1 : part == ';' ? 10 : 0;
}

// check if the current piece would collide at a certain position and rotation
int collide(int x, int y, int rot)
{
        int ret = NONE;
        for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++)
                if (is_solid_part(p->it.color, rot, i, j))
                {
                        if (i + x < 0 || i + x >= BWIDTH || j + y >= BHEIGHT)
                                return WALL;
                        if (p->row[j + y].col[i + x].color)
                                ret = NORMAL;
                }

        return ret;
}

int in_rect(int x, int y, int left, int top, int width, int height)
{
        return x >= left && x <= left + width && y >= top && y <= top + height;
}

void update_particles()
{
        for (int i = 0; i < NPARTS; i++)
        {
                struct particle *q = parts + i;
                if (q->r <= 0.1f)
                        continue;
                q->x += q->vx;
                q->y += q->vy;

                if (q->r < 2.f)
                        q->r *= 0.996f + (rand() % 800) * 0.001f;
                else
                        q->r *= 0.992f + (rand() % 400) * 0.00001f;

                struct player *o = q->opponent < 0 ? NULL :
                        play + q->opponent;

                // get contribution from flow nodes
                float flow_vx = 0.f;
                float flow_vy = 0.f;
                for (int n = 0; n < NFLOWS; n++)
                {
                        float xdiff = flows[n].x - q->x;
                        float ydiff = flows[n].y - q->y;
                        float distsq = xdiff * xdiff + ydiff * ydiff;
                        if (distsq > flows[n].r * flows[n].r)
                                continue;
                        flow_vx += flows[n].vx;
                        flow_vy += flows[n].vy;
                }

                // get contribution from homing in on opponent
                float targ_x = 0.f;
                float targ_y = 0.f;
                float homing_vx = 0.f;
                float homing_vy = 0.f;
                float dist = 1000.f;
                if (o)
                {
                        targ_x = o->board_x - 3 * bs2;
                        targ_y = CLAMP(o->top_garb - bs, q->y, o->board_y + bs * VHEIGHT);
                        homing_vx = (targ_x - q->x);
                        homing_vy = (targ_y - q->y);
                        dist = sqrtf(homing_vx * homing_vx + homing_vy * homing_vy);
                        homing_vx /= dist;
                        homing_vy /= dist;
                }

                float normal_r = q->r / bs;
                if (o && q->r && normal_r < .65f + (i % 9) * .07f) // particle has an opponent target
                {
                        if (dist < bs * 4 || normal_r < .5f)
                        {
                                q->vx *= .97f;
                                q->vy *= .97f;
                        }
                        float t = (dist < bs * 4) ? (dist / (bs * 6)) :
                                    .75f;
                        q->vx += flow_vx * t + homing_vx * (1.f - t);
                        q->vy += flow_vy * t + homing_vy * (1.f - t);
                }
                else if (!o && normal_r > .8f) // particle still just falling softly
                {
                        q->vy *= .82f;
                }
                else
                {
                        q->vx += flow_vx;
                        q->vy += flow_vy;
                }

                if (o && in_rect(q->x + q->r * .5f, q->y + q->r * .5f, targ_x, targ_y - bs2, bs, bs))
                {
                        q->r = 0.f;
                        o->flash += 50;
                        o->garbage_bits += q->bits;
                        if (o->garbage_bits >= 120)
                        {
                                audio_tone(SQUARE, G3, G4, 15, 15, 15, 100);
                                o->garbage[GARB_LVLS - 1]++;
                                o->garbage_tick = tick;
                                o->garbage_bits -= 120;
                        }
                        audio_tone(SQUARE, G4, C6, 5, 10, 5, 20);
                }
        }
}
