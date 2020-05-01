#include "my_malloc.h"
#include "assert.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void ts_free_lock(void* ptr){
  pthread_mutex_lock(&lock);
  if(ptr == NULL){// Cannot free NULL
    pthread_mutex_unlock(&lock);
    return;
  }
  block_t* b = getBlock(ptr);
  assert(b->free == NULL);// Make sure block b is not free
  if(freehead == NULL){// No blocks freed before
    freehead = b;
    pthread_mutex_unlock(&lock);
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
    pthread_mutex_unlock(&lock);
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
      pthread_mutex_unlock(&lock);
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
  pthread_mutex_unlock(&lock);
  return;
}

void* ts_malloc_lock(size_t size){
  pthread_mutex_lock(&lock);
  if(size == 0){// Cannot malloc 0 byte
    pthread_mutex_unlock(&lock);
    return NULL;
  }

    if(freehead == NULL){// No freed blocks
      block_t* pos = newBlock(size);
      if(pos == NULL){// newBlock failed
	pthread_mutex_unlock(&lock);
	return NULL;
      }
      pthread_mutex_unlock(&lock);
      return (char*)pos + sizeof(block_t);
    }
    // Have freed blocks
    else{
      // diff for the freehead
      size_t diff_head = UINT_MAX;
      if(freehead->size == size){// early stop
	diff_head = 0;
	void* ans = (void*)((char*)freehead + sizeof(block_t));
	block_t* curr = freehead;
	freehead = freehead->free;
	curr->free = NULL;
	pthread_mutex_unlock(&lock);
	return ans;
      }
      if(freehead->size > size){
	diff_head = freehead->size - size;
      }
      // diff for the body
      block_t* p = freehead;
      block_t* mark = NULL;
      size_t diff_body = UINT_MAX;
      while(p->free != NULL){
	if(p->free->size == size){// early stop
	  diff_body = 0;
	  mark = p;
	  break;
	}
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
	  pthread_mutex_unlock(&lock);
	  return NULL;
	}
	pthread_mutex_unlock(&lock);
	return (char*)pos + sizeof(block_t);
      }
      if(diff_head <= diff_body){// malloc the freehead
	void* ans = (void*)((char*)freehead + sizeof(block_t));
	if(diff_head >= sizeof(block_t)){// split
	  block_t* new = (block_t*)((char*)freehead + sizeof(block_t) + size);
	  new->free = freehead->free;
	  new->size = freehead->size - sizeof(block_t) - size;
	  freehead->size = size;
	  freehead->free = NULL;
	  freehead = new;
	}
	else{// not split
	  block_t* curr = freehead;
	  freehead = freehead->free;
	  curr->free = NULL;
	}
	pthread_mutex_unlock(&lock);
	return ans;
      }
      else{// malloc the block in the body
	void* ans = (void*)((char*)(mark->free) + sizeof(block_t));
	if(diff_body >= sizeof(block_t)){// split
	  block_t* new = (block_t*)((char*)(mark->free) + sizeof(block_t) + size);
	  new->free = mark->free->free;
	  new->size = mark->free->size - sizeof(block_t) - size;
	  mark->free->size = size;
	  mark->free->free = NULL;
	  mark->free = new;
	}
	else{// not split
	  block_t* curr = mark->free;
	  mark->free = mark->free->free;
	  curr->free = NULL;
	}
	pthread_mutex_unlock(&lock);
	return ans;
      }
    }
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


//======================================= no lock version ======================

void ts_free_nolock(void* ptr){
  if(ptr == NULL){// Cannot free NULL
    return;
  }
  block_t* b = getBlock(ptr);
  assert(b->free == NULL);// Make sure block b is not free
  if(TLS_freehead == NULL){// No blocks freed before
    TLS_freehead = b;
    return;
  }
  // Have blocks freed before
  if(b<TLS_freehead){// b will be the freehead
    b->free = TLS_freehead;
    TLS_freehead = b;
    if((char*)b + sizeof(block_t) + b->size == (char*)(b->free)){// Merge the next
      b->size += sizeof(block_t) + b->free->size;
      block_t* curr = b->free;
      b->free = b->free->free;
      curr->free =NULL;
    }
    return;
  }
  // b is in the middle
  block_t* p = TLS_freehead;
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

void* ts_malloc_nolock(size_t size){
  if(size == 0){// Cannot malloc 0 byte
    return NULL;
  }
    if(TLS_freehead == NULL){// No freed blocks
      block_t* pos = newBlock_nolock(size);
      if(pos == NULL){// newBlock failed
	return NULL;
      }
      return (char*)pos + sizeof(block_t);
    }
    // Have freed blocks
    else{
      // diff for the freehead
      size_t diff_head = UINT_MAX;
      if(TLS_freehead->size == size){// early stop
	diff_head = 0;
	void* ans = (void*)((char*)TLS_freehead + sizeof(block_t));
	block_t* curr = TLS_freehead;
	TLS_freehead = TLS_freehead->free;
	curr->free = NULL;
	return ans;
      }
      if(TLS_freehead->size > size){
	diff_head = TLS_freehead->size - size;
      }
      // diff for the body
      block_t* p = TLS_freehead;
      block_t* mark = NULL;
      size_t diff_body = UINT_MAX;
      while(p->free != NULL){
	if(p->free->size == size){// early stop
	  diff_body = 0;
	  mark = p;
	  break;
	}
	if(p->free->size >= size){// potential block
	  if(diff_body > p->free->size - size){
	    diff_body = p->free->size - size;
	    mark = p;
	  }
	}
	p = p->free;
      }
      if(diff_head == UINT_MAX && diff_body == UINT_MAX){// not find
	block_t* pos = newBlock_nolock(size);
	if(pos == NULL){// sbrk failed
	  return NULL;
	}
	return (char*)pos + sizeof(block_t);
      }
      if(diff_head <= diff_body){// malloc the freehead
	void* ans = (void*)((char*)TLS_freehead + sizeof(block_t));
	if(diff_head >= sizeof(block_t)){// split
	  block_t* new = (block_t*)((char*)TLS_freehead + sizeof(block_t) + size);
	  new->free = TLS_freehead->free;
	  new->size = TLS_freehead->size - sizeof(block_t) - size;
	  TLS_freehead->size = size;
	  TLS_freehead->free = NULL;
	  TLS_freehead = new;
	}
	else{// not split
	  block_t* curr = TLS_freehead;
	  TLS_freehead = TLS_freehead->free;
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
	  mark->free->size = size;
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

block_t* newBlock_nolock(size_t size){
  pthread_mutex_lock(&lock);
  block_t* b = sbrk(0);
  if(sbrk(size+sizeof(block_t))==(void*)-1){//sbrk failed
    return NULL;
  }
  pthread_mutex_unlock(&lock);
  b->size = size;
  b->free = NULL;
  return b;
}



