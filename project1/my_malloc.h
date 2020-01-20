#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

// struct of a block in the linked list
struct block_tag{
  size_t size;
  int free;
  struct block_tag* next;
  struct block_tag* prev;
};

typedef struct block_tag block_t;

// Global variables for the head and tail of the linked list
block_t* head = NULL;
block_t* tail = NULL;

void* ff_malloc(size_t size);
void ff_free(void *ptr);

void* bf_malloc(size_t size);
void bf_free(void *ptr);

block_t* findBlock_ff(size_t size);
block_t* findBlock_bf(size_t size);
block_t* newBlock(size_t size);
block_t* splitBlock(block_t* pos, size_t size);
block_t* getBlock(void* ptr);
block_t* merge(block_t* b);
unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();

#endif
