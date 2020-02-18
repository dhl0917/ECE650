#ifndef POTATO_H
#define POTATO_H

struct potato_tag{
  int num_hops;
  int index;
  int trace[512];
  int GAMEOVER; // 1 is over, 0 is not over
};

typedef struct potato_tag potato_t;
#endif




