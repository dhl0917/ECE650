#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>

// struct of a block in the linked list
struct block_tag{
  size_t size;
  struct block_tag* free;
};

typedef struct block_tag block_t;

// Global variables for the head of the linked list and the base address of the first block

block_t* freehead = NULL;
__thread block_t* TLS_freehead = NULL;

void* ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);

void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);

block_t* newBlock(size_t size);
block_t* getBlock(void* ptr);
block_t* newBlock_nolock(size_t size);



#endif
