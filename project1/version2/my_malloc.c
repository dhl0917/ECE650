#include "my_malloc.h"
#include "assert.h"

void* ff_malloc(size_t size){

}

void ff_free(void* ptr){

}

void* bf_malloc(size_t size){

}

void bf_free(void* ptr){

}




block_t* newBlock(size_t size){

}



block_t* getBlock(void* ptr){
  block_t* b = (block_t*)((char*)ptr - sizeof(block_t));
  return b;
}

unsigned long get_data_segment_size(){

}

unsigned long get_data_segment_free_space_size(){

}

