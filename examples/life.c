#!/usr/bin/env cjit
#include <stdio.h>
#include <stdlib.h>

#define X 121
#define Y 51

// Conway's Game of Life
// make conway && ./conway
// change X and Y for display area

typedef struct {
  void* cur;
  void* nxt;
} game_t;

void fmt_tick (void* board) {
  int* bd = (int*) board;
  system("clear");
  int i, j, v;
  for (j = 1; j < Y; j++) {
    printf("\n");
    for (i = 0; i < X; i++) {
      printf("%s", bd[j*X+i]? "*": " ");
    }
  }
}

void clearboard (void* board) {
  int* bd = (int*) board;
  int i, j, v;
  for (j = 0; j <= Y; j++) {
    for (i = 0; i <= X; i++) {
      bd[j*X+i] = 0;
    }
  }
}

void tick (game_t* game) {
  int* cur = (int*)game->cur;
  clearboard((void*)game->nxt);
  int* nxt = (int*)game->nxt;
  int i, j, s;
  s = 0;
  for (j = 1; j < (Y-1); j++) {
    for (i = 1; i < (X-1); i++) {
      s = 0;
      s += cur[(j-1)*(X) + (i-1)];
      s += cur[(j-1)*(X) + (i)];
      s += cur[(j-1)*(X) + (i+1)];
      s += cur[j*(X) + (i-1)];
      s += cur[j*(X) + (i+1)];
      s += cur[(j+1)*(X) + (i+1)];
      s += cur[(j+1)*(X) + (i)];
      s += cur[(j+1)*(X) + (i-1)];
      if (cur[j*X+i]) {
        if (s == 2 || s == 3) {
          nxt[j*X+i] = 1;
        }
      } else {
        if (s == 3) {
          nxt[j*X+i] = 1;
        }
      }
    }
  }
  fmt_tick((void*)nxt);
  game->cur = (void*)nxt;
  game->nxt = (void*)cur;
}

void seed (void* board) {
  int* bd = (int*) board;
  int i, j, v;
  for (j = 1; j < Y; j++) {
    for (i = 1; i < X; i++) {
      v = rand() % 2;
      bd[j*X+i] = v? 0: 1;
    }
  }
}

int main () {
  system("clear");
  game_t gm;
  game_t* game = &gm;
  void* seedboard = malloc(X*Y*4);
  seed(seedboard);
  void* gameboard = malloc(X*Y*4);
  clearboard(gameboard);
  game->cur = seedboard;
  game->nxt = gameboard;
  int generations = 0;
  while (1) {
    tick(game);
    generations++;
    printf("\ngeneration: %d\n", generations);
    usleep(75000);
  }
  // Superfluous!
  free(seedboard);
  free(gameboard);
  return 0;
}

