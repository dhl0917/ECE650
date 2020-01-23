#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

// struct of a block in the linked list
struct block_tag{
  size_t size;
  struct block_tag* free;
};

typedef struct block_tag block_t;

// Global variables for the head of the linked list and the base address of the first block
void* base = NULL;
block_t* freehead = NULL;

void* ff_malloc(size_t size);
void ff_free(void *ptr);

void* bf_malloc(size_t size);
void bf_free(void *ptr);

block_t* newBlock(size_t size);
block_t* getBlock(void* ptr);

unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();



#endif
