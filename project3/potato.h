#ifndef POTATO_H
#define POTATO_H

struct potato_tag{
  int num_hops;
  int index;
  int trace[512];
  bool GAMEOVER;
};

typedef struct potato_tag potato_t;
#endif




