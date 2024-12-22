// Flappy -- http://tinyc.games -- (c) 2020 Jer Wilson
//
// Flappy is an extremely small implementation of the Flappy Bird game.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#define W 480
#define H 600
#define GROUND 80
#define PIPE_W 86
#define PHYS_W (W + PIPE_W + 80)
#define GAP 220
#define GRACE 4
#define RANDOM_PIPE_HEIGHT (rand() % (H - GROUND - GAP - 120) + 60)
#define PLYR_X 80
#define PLYR_SZ 60

enum gamestates {READY, ALIVE, GAMEOVER} gamestate = READY;

float player_y = (H - GROUND)/2;
float player_vel;
int pipe_x[2] = {W, W};
float pipe_y[2];
int score;
int best;
int idle_time = 30;
float frame = 0;

SDL_Event event;
SDL_Renderer *renderer;
SDL_Surface *surf;
SDL_Texture *pillar;
SDL_Texture *background;
SDL_Texture *bird[4];
TTF_Font *font;

void setup();
void new_game();
void update_stuff();
void update_pipe(int i);
void draw_stuff();
void text(char *fstr, int value, int height);

//the entry point and main game loop
int main()
{
        setup();

        for(;;)
        {
                while(SDL_PollEvent(&event)) switch(event.type)
                {
                        case SDL_QUIT:
                                exit(0);
                        case SDL_KEYDOWN:
                        case SDL_MOUSEBUTTONDOWN:
                                if(gamestate == ALIVE)
                                {
                                        player_vel = -11.7f;
                                        frame += 1.0f;
                                }
                                else if(idle_time > 30)
                                {
                                        new_game();
                                }
                }

                update_stuff();
                draw_stuff();
                SDL_Delay(1000 / 60);
                idle_time++;
        }
}

//initial setup to get the window and rendering going
void setup()
{
        srand(time(NULL));

        SDL_Init(SDL_INIT_VIDEO);
        SDL_Window *win = SDL_CreateWindow("Flappy",
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, W, H, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC);
        if(!renderer) exit(fprintf(stderr, "Could not create SDL renderer\n"));
	IMG_Init(IMG_INIT_PNG);
        pillar = IMG_LoadTexture(renderer,"examples/tinyc.games/flappy/pillar.png");
        SDL_SetColorKey(surf, 1, 0xffff00);
        background = IMG_LoadTexture(renderer,"examples/tinyc.games/flappy/background.png");

        for(int i = 0; i < 4; i++)
        {
                char file[80];
                sprintf(file, "examples/tinyc.games/flappy/bird-%d.png", i);
                // surf = SDL_LoadBMP(file);
                // SDL_SetColorKey(surf, 1, 0xffff00);
		// bird[i] = SDL_CreateTextureFromSurface(renderer, surf);
		bird[i] = IMG_LoadTexture(renderer,file);
        }

        TTF_Init();
        font = TTF_OpenFont("examples/tinyc.games/PixelatedEleganceRegular.ttf", 42);
}

//start a new game
void new_game()
{
        gamestate = ALIVE;
        player_y = (H - GROUND)/2;
        player_vel = -11.7f;
        score = 0;
        pipe_x[0] = PHYS_W + PHYS_W/2 - PIPE_W;
        pipe_x[1] = PHYS_W - PIPE_W;
        pipe_y[0] = RANDOM_PIPE_HEIGHT;
        pipe_y[1] = RANDOM_PIPE_HEIGHT;
}

//when we hit something
void game_over()
{
        gamestate = GAMEOVER;
        idle_time = 0;
        if(best < score) best = score;
}

//update everything that needs to update on its own, without input
void update_stuff()
{
        if(gamestate != ALIVE) return;

        player_y += player_vel;
        player_vel += 0.61; // gravity

        if(player_vel > 10.0f)
                frame = 0;
        else
                frame -= (player_vel - 10.0f) * 0.03f; //fancy animation

        if(player_y > H - GROUND - PLYR_SZ)
                game_over();

        for(int i = 0; i < 2; i++)
                update_pipe(i);
}

//update one pipe for one frame
void update_pipe(int i)
{
        if(PLYR_X + PLYR_SZ >= pipe_x[i] + GRACE && PLYR_X <= pipe_x[i] + PIPE_W - GRACE &&
                (player_y <= pipe_y[i] - GRACE || player_y + PLYR_SZ >= pipe_y[i] + GAP + GRACE))
                game_over(); // player hit pipe

        // move pipes and increment score if we just passed one
        pipe_x[i] -= 5;
        if(pipe_x[i] + PIPE_W < PLYR_X && pipe_x[i] + PIPE_W > PLYR_X - 5)
                score++;

        // respawn pipe once far enough off screen
        if(pipe_x[i] <= -PIPE_W)
        {
                pipe_x[i] = PHYS_W - PIPE_W;
                pipe_y[i] = RANDOM_PIPE_HEIGHT;
        }
}

//draw everything in the game on the screen
void draw_stuff()
{
        SDL_Rect dest = {0, 0, W, H};
        SDL_RenderCopy(renderer, background, NULL, &dest);

        //draw pipes
        for(int i = 0; i < 2; i++)
        {
                int lower = pipe_y[i] + GAP;
                SDL_RenderCopy(renderer, pillar, NULL, &(SDL_Rect){pipe_x[i], pipe_y[i] - H, PIPE_W, H});
                SDL_Rect src = {0, 0, 86, H - lower - GROUND};
                SDL_RenderCopy(renderer, pillar, &src, &(SDL_Rect){pipe_x[i], lower, PIPE_W, src.h});
        }

        //draw player
        SDL_RenderCopy(renderer, bird[(int)frame % 4], NULL,
                        &(SDL_Rect){PLYR_X, player_y, PLYR_SZ, PLYR_SZ});

        if(gamestate != READY) text("%d", score, 10);
        if(gamestate == READY) text("Press any key", 0, 150);
        if(gamestate == GAMEOVER) text("High score: %d", best, 150);

        SDL_RenderPresent(renderer);
}

void text(char *fstr, int value, int height)
{
        if(!font) return;
        int w, h;
        char msg[80];
        snprintf(msg, 80, fstr, value);
        TTF_SizeText(font, msg, &w, &h);
        SDL_Surface *msgsurf = TTF_RenderText_Blended(font, msg, (SDL_Color){255, 255, 255});
        SDL_Texture *msgtex = SDL_CreateTextureFromSurface(renderer, msgsurf);
        SDL_Rect fromrec = {0, 0, msgsurf->w, msgsurf->h};
        SDL_Rect torec = {(W - w)/2, height, msgsurf->w, msgsurf->h};
        SDL_RenderCopy(renderer, msgtex, &fromrec, &torec);
        SDL_DestroyTexture(msgtex);
        SDL_FreeSurface(msgsurf);
}
