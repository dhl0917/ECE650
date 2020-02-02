## Report#2: Thread-Safe Malloc

##### Hongliang Dong (hd97)

### 1. Implementation

In project1, I used a straightforward design with a double linked list to record all the allocated and free memory blocks. It worked for most of the test cases. However, it broke down in some corner cases, which is not acceptable. Besides, the previous implementation is not efficient as every time it needs to traverse the whole linked list, which can be very time consuming when the linked list grows large. In project2, I redesigned my structure with a free list for the malloc and free functions. The new data structure and implementation are briefly introduced in 1.1 and 1.2. The two thread-safe malloc implementations are described in 1.3.

#### 1.1 Data Structure

The definition of a memory block `block_t` is shown as below. It has two fields. The first one `size` is used to store how many bytes allocated in the memory block. The second one `free` is a pointer pointing to the next free `block_t` when the `block_t` itself is free. Otherwise, `free` is set to `NULL` to indicate memory allocated. 

```c
struct block_tag{
  size_t size;
  struct block_tag* free;
};
typedef struct block_tag block_t;
```

#### 1.2 Details of Implementation 

Instead of a double linked list linking all the allocated memory blocks regardless they are free or not, the new design only uses one linked list named `freelist` to link all the free memory blocks. The head of the `freelist` is `freehead` and is initialized to `NULL` to indicate there are no free blocks. When we call funcitons to allocate memory, `freelist` is traversed to find available free blocks. If there are no free memory blocks or all free memory blocks are not available, `sbrk()` is called to allocate memory from heap. Note that `sbrk()` actually allocates the required size plus the size of `block_t` for memory usage and record. If we locate one block according to certain policy, we delink it from the `freelist` and set its `free` to `NULL`. 

For the function to free memory blocks, the memory block to be free is linked in the `freelist` according to address order. 

The idea to implement first fit (FF) policy and best fit (BF) policy is no difference from project1. The techniques to split and merge memory blocks when necessary are implemented as well. Because they have been described in project1 report thoroughly, the details are obmitted here. To sum up, the new design is different from the previous design that we only need to traverse free memory blocks, which makes the algorithm much more efficient and elegant. 

#### 1.3 Thread-safe Implementations

To make sure malloc/free fucntions are thread-safe, in LOCK version, a mutex from pthread library is first declared as below to help protect critical sections. The idea to implement the LOCK version is simple. The whole functions of malloc and free are  regarded as critical sections. The mutex is locked when we enter malloc/free functions and is unlocked before return from the functions. In this way, when one thread enters malloc/free functions, the mutex is acquired and all other threads calling malloc/free have to wait until the mutex is released. Race conditions are avoided as there is only one thread can manipulate the linked list of memory blocks at one time. 

```c
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
```

In NOLOCK version, the mutex is only used right before and after the `sbrk()` function which is regarded as the critical section. The general idea is to create local copies of `freelist` for each thread. For this purpose, Thread-local storage is used. Now `freehead` is no longer a global variable. It is decleared to be a TLS variable. In this way, each thread maintains its own `freelist`. As threads are independent and there are no overlapping regions, race conditions are avoided. Threads can run concurrently except the `sbrk()` function.

### 2. Results

The experiments were run 10 times for both LOCK version and NOLOCK version. All the experiments passed. Due to the space limitation, three results are shown below.

![0](/Users/donghongliang/Desktop/0.png)

<center>Fig 1. LOCK Version</center>

![1](/Users/donghongliang/Desktop/1.png)

<center>Fig 2. NOLOCK Version</center>

The trend is obvious. NOLOCK version runs faster while LOCK version has smaller data segment size. The results are reasonable. For NOLOCK version, threads maintain their own free lists so they can run simultaneously except calling `sbrk()`. In other words, unless they need to allocate new memory at the same time, they can run concurrently and have no influence on each other. For LOCK version, things become more sequential, since one thread will be blocked if another thread is executing malloc/free regardless of calling `sbrk()` or not. This is why NOLOCK version runs faster. However, as a weakness of maintaining free lists independently, one thread can only see blocks in its own free list. Adjacent free memory blocks blong to different threads cannot merge together, which lowers the possibility to resue those memory space. On the contrary, LOCK version threads can avoid this problem. This can account for the result that LOCK version tends to have smaller data segment size. Here we see the tradeoff between execution time and memory usage. Under conditions that running time is important, NOLOCK version is a good choice. While when memory usage is the dominant factor, LOCK version outperforms NOLOCK version.


