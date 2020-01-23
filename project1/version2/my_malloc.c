#include "my_malloc.h"
#include "assert.h"

void* ff_malloc(size_t size){
  if(size == 0){// Cannot malloc 0 byte
    return NULL;
  }
  if(base == NULL){// No mallocs before
    block_t* pos = newBlock(size);
    if(pos == NULL){// newBlock failed
      return NULL;
    }
    base = (void*) pos;
    return (char*)pos + sizeof(block_t);
  }
  else{// Did malloc before
    if(freehead == NULL){//No free blocks
      block_t* pos = newBlock(size);
      if(pos == NULL){// newBlock failed
	return NULL;
      }
      return (char*)pos + sizeof(block_t);
    }
    else{// There are free blocks
      // Corner case if the head is available
      if(freehead -> size >= size){
	void* ans = (void*)((char*)freehead + sizeof(block_t));
	if(freehead->size >= size + sizeof(block_t)){// spilt
	  block_t* new = (block_t*)((char*)freehead + sizeof(block_t) + size);
	  new->free = freehead->free;
	  new->size = freehead->size - sizeof(block_t) - size;
	  freehead->free = NULL;
	  freehead = new;
	}
	else{// not split
	  block_t* curr = freehead;
	  freehead = freehead->free;
	  curr->free = NULL;
	}
	return ans;
      }
      // General case if the body block is available
      else{
	block_t* p = freehead;
	while(p->free != NULL){
	  if(p->free->size >= size){// Find the available block
	    void* ans = (char*)(p->free)+sizeof(block_t);
	    if(p->free->size >= size + sizeof(block_t)){// split
	      block_t* new = (block_t*)((char*)(p->free) + sizeof(block_t) +size);
	      new->free = p->free->free;
	      new->size = p->free->size - sizeof(block_t) - size;
	      p->free->free = NULL;
	      p->free = new;
	    }
	    else{// not split
	      block_t* curr = p->free;
	      p->free = p->free->free;
	      curr->free = NULL;
	    }
	    return ans;
	  }
	  p = p->free;
	}
	// No available in the free list
	block_t* pos = newBlock(size);
	if(pos==NULL){// newBlock failed
	  return NULL;
	}
	return (char*)pos + sizeof(block_t);
      }
    }
  }
}

void ff_free(void* ptr){
  if(ptr == NULL){// Cannot free NULL
    return;
  }
  block_t* b = getBlock(ptr);
  assert(b->free == NULL);// Make sure block b is not free
  if(freehead == NULL){// No blocks freed before
    freehead = b;
    return;
  }
  // Have blocks freed before
  if(b<freehead){// b will be the freehead
    b->free = freehead;
    freehead = b;
    if((char*)b + sizeof(block_t) + b->size == (char*)(b->free)){// Merge the next
      b->size += sizeof(block_t) + b->free->size;
      block_t* curr = b->free;
      b->free = b->free->free;
      curr->free =NULL;
    }
    return;
  }
  // b is in the middle
  block_t* p = freehead;
  while(p->free != NULL){
    if(p->free > b){// Find the pointer to pointer
      if((char*)p + sizeof(block_t) + p->size == (char*)b){// merge the previous
	p->size += sizeof(block_t) + b->size;
	b = p;
      }
      else{// no need to merge
	b->free = p->free;
	p->free = b;
      }
      if((char*)b + sizeof(block_t)+b->size == (char*)(b->free)){
	b->size += sizeof(block_t) + b->free->size;
	block_t* curr = b->free;
	b->free = b->free->free;
	curr->free = NULL;
      }
      return;
    }
    p = p->free;
  }
  // b is at the end
  if((char*)p + sizeof(block_t) + p->size == (char*)b){
    p->size += sizeof(block_t) + b->size;
    b = p;
  }
  else{
    b->free = p->free;
    p->free = b;
  }
  return;
}

void* bf_malloc(size_t size){
  if(size == 0){// Cannot malloc 0 byte
    return NULL;
  }
  if(base = NULL){// No mallocs before
    block_t* pos = newBlock(size);
    if(pos == NULL){// newBlock failed
      return NULL;
    }
    base = (void*)pos;
    return (char*)pos + sizeof(block_t);
  }
  // Did malloc before
  else{
    if(freehead == NULL){// No freed blocks
      block_t* pos = newBlock(size);
      if(pos == NULL){// newBlock failed
	return NULL;
      }
      return (char*)pos + sizeof(block_t);
    }
    // Have freed blocks
    else{
      // diff for the freehead
      size_t diff_head = UINT_MAX;
      if(freehead->size >= size){
	diff_head = freehead->size - size;
      }
      // diff for the body
      block_t* p = freehead;
      block_t* mark = NULL;
      size_t diff_body = UINT_MAX;
      while(p->free != NULL){
	if(p->free->size >= size){// potential block
	  if(diff_body > p->free->size - size){
	    diff_body = p->free->size - size;
	    mark = p;
	  }
	}
	p = p->free;
      }
      if(diff_head == UINT_MAX && diff_body == UINT_MAX){// not find
	block_t* pos = newBlock(size);
	if(pos == NULL){// sbrk failed
	  return NULL;
	}
	return (char*)pos + sizeof(block_t);
      }
      if(diff_head <= diff_body){// malloc the freehead
	void* ans = (void*)((char*)freehead + sizeof(block_t));
	if(diff_head >= sizeof(block_t)){// split
	  block_t* new = (block_t*)((char*)freehead + sizeof(block_t) + size);
	  new->free = freehead->free;
	  new->size = freehead->size - sizeof(block_t) - size;
	  freehead->free = NULL;
	  freehead = new;
	}
	else{// not split
	  block_t* curr = freehead;
	  freehead = freehead->free;
	  curr->free = NULL;
	}
	return ans;
      }
      else{// malloc the block in the body
	void* ans = (void*)((char*)(mark->free) + sizeof(block_t));
	if(diff_body >= sizeof(block_t)){// split
	  block_t* new = (block_t*)((char*)(mark->free) + sizeof(block_t) + size);
	  new->free = mark->free->free;
	  new->size = mark->free->size - sizeof(block_t) - size;
	  mark->free->free = NULL;
	  mark->free = new;
	}
	else{// not split
	  block_t* curr = mark->free;
	  mark->free = mark->free->free;
	  curr->free = NULL;
	}
	return ans;
      }
    }
  }
}

void bf_free(void* ptr){
  ff_free(ptr);
  return;
}


block_t* newBlock(size_t size){
  block_t* b = sbrk(0);
  if(sbrk(size+sizeof(block_t))==(void*)-1){//sbrk failed
    return NULL;
  }
  b->size = size;
  b->free = NULL;
  return b;
}



block_t* getBlock(void* ptr){
  block_t* b = (block_t*)((char*)ptr - sizeof(block_t));
  return b;
}

unsigned long get_data_segment_size(){
  void* curr = sbrk(0);
  return curr-base;
}

unsigned long get_data_segment_free_space_size(){
  block_t* curr = freehead;
  unsigned long res = 0;
  while(curr != NULL){
    res += curr->size + sizeof(block_t);
    curr = curr->free;
  }
  return res;
}

