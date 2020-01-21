#include "my_malloc.h"
#include "assert.h"

void* ff_malloc(size_t size){
  if(size == 0){//Cannot allocate 0 byte
    return NULL;
  }
  if(head == NULL && tail==NULL){
    head = newBlock(size);
    if(head == NULL){// sbrk failed
      return NULL;
    }
    tail = head;
    return (char*)head + sizeof(block_t);
  }
  else{
    block_t* pos = findBlock_ff(size);
    if(pos != NULL){// Find valid blocks
      if(pos->size >= size + sizeof(block_t)){// Able to spilt
	pos = splitBlock(pos,size);
      }
      else{// Not able to split
	pos->free = 0;
      }
    }
    else{// No valid blocks
      pos = newBlock(size);
      if(pos == NULL){// sbrk failed
	return NULL;
      }
      // sbrk succeed and insert the new block into the linked list
      tail->next = pos;
      pos->prev = tail;
      tail = pos;
    }
    return (char*) pos + sizeof(block_t);
  }
}

void ff_free(void* ptr){
  if(ptr == NULL){
    return;
  }
  block_t* b = getBlock(ptr);// Find the pointer to the block
  assert(b->free == 0);
  b->free = 1;
  // Merging the adjacent free regions into a single free region of memory
  while(b->next != NULL && b->next->free == 1){
    b = merge(b);
  }
  while(b->prev != NULL && b->prev->free == 1){
    b = merge(b->prev);
  }
}

void* bf_malloc(size_t size){
  if(size == 0){//Cannot allocate 0 byte
    return NULL;
  }
  if(head == NULL && tail==NULL){
    head = newBlock(size);
    if(head == NULL){// sbrk failed
      return NULL;
    }
    tail = head;
    return (char*)head + sizeof(block_t);
  }
  else{
    block_t* pos = findBlock_bf(size);
    if(pos != NULL){// Find valid blocks
      if(pos->size >= size + sizeof(block_t)){// Able to spilt
	pos = splitBlock(pos,size);
	//return (char*) pos + sizeof(block_t);
      }
      else{// Not able to split
	pos->free = 0;
	//return (char*) pos + sizeof(block_t);
      }
    }
    else{// No valid blocks
      pos = newBlock(size);
      if(pos == NULL){// sbrk failed
	return NULL;
      }
      // sbrk succeed and insert the new block into the linked list
      tail->next = pos;
      pos->prev = tail;
      tail = pos;
      //return (cahr*)pos+sizeof(block_t);
    }
    return (char*) pos + sizeof(block_t);
  }
}

void bf_free(void* ptr){
  ff_free(ptr);
}

block_t* findBlock_ff(size_t size){
  block_t* curr = head;
  while(curr!=NULL){
    if(curr->free == 1 && curr->size >= size){// Find a valid block
      return curr;
    }
    curr = curr->next;
  }
  // Not find
  return NULL;
}

block_t* findBlock_bf(size_t size){
  block_t* b = NULL;
  unsigned int diff = UINT_MAX;
  block_t* curr = head;
  while(curr!=NULL){
    if(curr->free == 0 || curr->size<size){// Invalid blocks
      curr = curr->next;
      continue;
    }
    if(curr->size == size){// Best fit, early stop
      return curr;
    }
    if(curr->size - size < diff){// Potential valid blocks 
      diff = curr->size - size;
      b = curr;
    }
    curr = curr->next;
  }
  return b;
}


block_t* newBlock(size_t size){
  block_t* b = sbrk(0);// pointer to the start address
  if(sbrk(size+sizeof(block_t))==(void*)-1){// sbrk failed
    return NULL;
  }
  b->size = size;
  b->free = 0;
  b->next = NULL;
  b->prev = NULL;
  return b;
}

block_t* splitBlock(block_t* pos, size_t size){
  block_t* new = (block_t*)((char*)pos + size + sizeof(block_t));
  new->size = pos->size - size - sizeof(block_t);
  new->free = 1;
  new->next = pos->next;
  new->prev = pos;
  if(new->next == NULL){// End of the linked list
    tail = new;
  }
  else{// Not end
    new->next->prev = new;
  }
  pos->next = new;
  pos->size = size;
  pos->free = 0;
  return pos;
}

block_t* merge(block_t* b){
  if(b->next != NULL && b->next->free == 1){
    b->size = b->size + sizeof(block_t) + b->next->size;
    b->next = b->next->next;
    if(b->next == NULL){// End of the linked list
      tail = b;
    }
    else{// Not end
      b->next->prev = b;
    }
  }
  return b;
}

block_t* getBlock(void* ptr){
  block_t* b = (block_t*)((char*)ptr - sizeof(block_t));
  return b;
}

unsigned long get_data_segment_size(){
  unsigned long res = 0;
  block_t* curr = head;
  while(curr != NULL){
    res += sizeof(block_t) + curr->size;
    curr = curr ->next;
  }
  return res;
}

unsigned long get_data_segment_free_space_size(){
  unsigned long res = 0;
  block_t* curr = head;
  while(curr != NULL){
    if(curr->free == 1){
      res += sizeof(block_t) + curr->size;
    }
    curr = curr -> next;
  }
  return res;
}
