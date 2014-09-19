//=== softboundcets.h - headers for functions introduced by SoftBound+CETS--*- C -*===// 
// Copyright (c) 2014 Santosh Nagarakatte, Milo M. K. Martin. All rights reserved.

// Developed by: Santosh Nagarakatte, Milo M.K. Martin,
//               Jianzhou Zhao, Steve Zdancewic
//               Department of Computer and Information Sciences,
//               University of Pennsylvania
//               http://www.cis.upenn.edu/acg/softbound/

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal with the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

//   1. Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimers.

//   2. Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimers in the
//      documentation and/or other materials provided with the distribution.

//   3. Neither the names of Santosh Nagarakatte, Milo M. K. Martin,
//      Jianzhou Zhao, Steve Zdancewic, University of Pennsylvania, nor
//      the names of its contributors may be used to endorse or promote
//      products derived from this Software without specific prior
//      written permission.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// WITH THE SOFTWARE.
//===---------------------------------------------------------------------===//


#ifndef __SOFTBOUNDCETS_H__
#define __SOFTBOUNDCETS_H__

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>


#if 0
#define __SOFTBOUNDCETS_SPATIAL_TEMPORAL 1
#endif

/* Trie represented by the following by a structure with four fields
 * if both __SOFTBOUNDCETS_SPATIAL and __SOFTBOUNDCETS_TEMPORAL are
 * specified. It has key and lock with size_t
 */

typedef struct {

#ifdef __SOFTBOUNDCETS_SPATIAL
  void* base;
  void* bound;  

#define __SOFTBOUNDCETS_METADATA_NUM_FIELDS 2
#define __BASE_INDEX 0
#define __BOUND_INDEX 1
#define __KEY_INDEX 10000000
#define __LOCK_INDEX 10000000

#elif __SOFTBOUNDCETS_TEMPORAL  
  size_t key;
  void* lock;
#define __SOFTBOUNDCETS_METADATA_NUM_FIELDS 2
#define __KEY_INDEX 0
#define __LOCK_INDEX 1
#define __BASE_INDEX  10000000
#define __BOUND_INDEX 10000000

#elif __SOFTBOUNDCETS_SPATIAL_TEMPORAL

  void* base;
  void* bound;
  size_t key;
  void* lock;
#define __SOFTBOUNDCETS_METADATA_NUM_FIELDS 4

#define __BASE_INDEX 0
#define __BOUND_INDEX 1
#define __KEY_INDEX 2
#define __LOCK_INDEX 3

#else 

  void* base;
  void* bound;
  size_t key;
  void* lock;  

#define __SOFTBOUNDCETS_METADATA_NUM_FIELDS 4

#define __BASE_INDEX 0
#define __BOUND_INDEX 1
#define __KEY_INDEX 2
#define __LOCK_INDEX 3

#endif

} __softboundcets_trie_entry_t;


#if defined(__APPLE__)
#define SOFTBOUNDCETS_MMAP_FLAGS (MAP_ANON|MAP_NORESERVE|MAP_PRIVATE)
#else
#define SOFTBOUNDCETS_MMAP_FLAGS (MAP_PRIVATE|MAP_ANONYMOUS|MAP_NORESERVE)
#endif


#ifdef __SOFTBOUNDCETS_DEBUG
#undef __SOFTBOUNDCETS_DEBUG
static const int __SOFTBOUNDCETS_DEBUG = 1;
#define __SOFTBOUNDCETS_NORETURN 
#else 
static const int __SOFTBOUNDCETS_DEBUG = 0;
#define __SOFTBOUNDCETS_NORETURN __attribute__((__noreturn__))
#endif

#ifdef __SOFTBOUNDCETS_PREALLOCATE_TRIE
#undef __SOFTBOUNDCETS_PREALLOCATE_TRIE
static const int __SOFTBOUNDCETS_PREALLOCATE_TRIE = 1;
#else
static const int __SOFTBOUNDCETS_PREALLOCATE_TRIE = 0;
#endif

#ifdef __SOFTBOUNDCETS_SPATIAL_TEMPORAL 
#define __SOFTBOUNDCETS_FREE_MAP
#endif

#ifdef __SOFTBOUNDCETS_TEMPORAL
#define __SOFTBOUNDCETS_FREE_MAP
#endif

#ifdef __SOFTBOUNDCETS_FREE_MAP
#undef __SOFTBOUNDCETS_FREE_MAP
static const int __SOFTBOUNDCETS_FREE_MAP = 1;
#else 
static const int __SOFTBOUNDCETS_FREE_MAP = 0;
#endif



// check if __WORDSIZE works with clang on both Linux and MacOSX
/* Allocating one million entries for the temporal key */
#if __WORDSIZE == 32
static const size_t __SOFTBOUNDCETS_N_TEMPORAL_ENTRIES = ((size_t) 4 * (size_t) 1024 * (size_t) 1024); 
static const size_t __SOFTBOUNDCETS_LOWER_ZERO_POINTER_BITS = 2;
static const size_t __SOFTBOUNDCETS_N_STACK_TEMPORAL_ENTRIES = ((size_t) 1024 * (size_t) 64);
static const size_t __SOFTBOUNDCETS_N_GLOBAL_LOCK_SIZE = ((size_t) 1024 * (size_t) 32);
// 2^23 entries each will be 8 bytes each 
static const size_t __SOFTBOUNDCETS_TRIE_PRIMARY_TABLE_ENTRIES = ((size_t) 8*(size_t) 1024 * (size_t) 1024);
static const size_t __SOFTBOUNDCETS_SHADOW_STACK_ENTRIES = ((size_t) 128 * (size_t) 32 );
/* 256 Million simultaneous objects */
static const size_t __SOFTBOUNDCETS_N_FREE_MAP_ENTRIES = ((size_t) 32 * (size_t) 1024* (size_t) 1024);
// each secondary entry has 2^ 22 entries 
static const size_t __SOFTBOUNDCETS_TRIE_SECONDARY_TABLE_ENTRIES = ((size_t) 4 * (size_t) 1024 * (size_t) 1024); 

#else

static const size_t __SOFTBOUNDCETS_N_TEMPORAL_ENTRIES = ((size_t) 64*(size_t) 1024 * (size_t) 1024); 
static const size_t __SOFTBOUNDCETS_LOWER_ZERO_POINTER_BITS = 3;

static const size_t __SOFTBOUNDCETS_N_STACK_TEMPORAL_ENTRIES = ((size_t) 1024 * (size_t) 64);
static const size_t __SOFTBOUNDCETS_N_GLOBAL_LOCK_SIZE = ((size_t) 1024 * (size_t) 32);

// 2^23 entries each will be 8 bytes each 
static const size_t __SOFTBOUNDCETS_TRIE_PRIMARY_TABLE_ENTRIES = ((size_t) 8*(size_t) 1024 * (size_t) 1024);

static const size_t __SOFTBOUNDCETS_SHADOW_STACK_ENTRIES = ((size_t) 128 * (size_t) 32 );

/* 256 Million simultaneous objects */
static const size_t __SOFTBOUNDCETS_N_FREE_MAP_ENTRIES = ((size_t) 32 * (size_t) 1024* (size_t) 1024);
// each secondary entry has 2^ 22 entries 
static const size_t __SOFTBOUNDCETS_TRIE_SECONDARY_TABLE_ENTRIES = ((size_t) 4 * (size_t) 1024 * (size_t) 1024); 

#endif

#define __WEAK__ __attribute__((__weak__))

#define __WEAK_INLINE __attribute__((__weak__,__always_inline__)) 

#if __WORDSIZE == 32
#define __METADATA_INLINE 
#else
#define __METADATA_INLINE __attribute__((__weak__, __always_inline__))
#endif

#define __NO_INLINE __attribute__((__noinline__))

extern __softboundcets_trie_entry_t** __softboundcets_trie_primary_table;

extern size_t* __softboundcets_shadow_stack_ptr;
extern size_t* __softboundcets_temporal_space_begin;

extern size_t* __softboundcets_stack_temporal_space_begin;
extern size_t* __softboundcets_free_map_table;

extern __SOFTBOUNDCETS_NORETURN void __softboundcets_abort();
extern void __softboundcets_printf(const char* str, ...);
extern size_t* __softboundcets_global_lock; 

void* __softboundcets_safe_calloc(size_t, size_t);
void* __softboundcets_safe_malloc(size_t);
void __softboundcets_safe_free(void*);

void * __softboundcets_safe_mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset);
__WEAK_INLINE void __softboundcets_allocation_secondary_trie_allocate(void* addr_of_ptr);
__WEAK_INLINE void __softboundcets_add_to_free_map(size_t ptr_key, void* ptr) ;

/******************************************************************************/

static __attribute__ ((__constructor__)) void __softboundcets_global_init();

extern __NO_INLINE void __softboundcets_stub(void);

extern void __softboundcets_init(void);

void __softboundcets_global_init()
{
  __softboundcets_init();
  __softboundcets_stub();
}


/* Layout of the shadow stack

  1) size of the previous stack frame
  2) size of the current stack frame
  3) base/bound/key/lock of each argument

  Allocation: read the current stack frames size, increment the
  shadow_stack_ptr by current_size + 2, store the previous size into
  the new prev value, calcuate the allocation size and store in the
  new current stack size field; Deallocation: read the previous size,
  and decrement the shadow_stack_ptr */
  
__WEAK_INLINE void __softboundcets_allocate_shadow_stack_space(int num_pointer_args){
 

  size_t* prev_stack_size_ptr = __softboundcets_shadow_stack_ptr + 1;
  size_t prev_stack_size = *((size_t*)prev_stack_size_ptr);

  __softboundcets_shadow_stack_ptr = __softboundcets_shadow_stack_ptr + prev_stack_size + 2;
  
  *((size_t*) __softboundcets_shadow_stack_ptr) = prev_stack_size;
  size_t* current_stack_size_ptr = __softboundcets_shadow_stack_ptr + 1;
  
  ssize_t size = num_pointer_args * __SOFTBOUNDCETS_METADATA_NUM_FIELDS;
  *((size_t*) current_stack_size_ptr) = size;
}
   
__WEAK_INLINE void* __softboundcets_load_base_shadow_stack(int arg_no){
  assert (arg_no >= 0 );
  size_t count = 2 +  arg_no * __SOFTBOUNDCETS_METADATA_NUM_FIELDS + __BASE_INDEX ;
  size_t* base_ptr = (__softboundcets_shadow_stack_ptr + count); 
  void* base = *((void**)base_ptr);
  return base;
}

__WEAK_INLINE void* __softboundcets_load_bound_shadow_stack(int arg_no){

  assert (arg_no >= 0 );
  size_t count = 2 + arg_no * __SOFTBOUNDCETS_METADATA_NUM_FIELDS  + __BOUND_INDEX ;
  size_t* bound_ptr = (__softboundcets_shadow_stack_ptr + count); 

  void* bound = *((void**)bound_ptr);
  return bound;
}

__WEAK_INLINE size_t __softboundcets_load_key_shadow_stack(int arg_no){

  assert (arg_no >= 0 );
  size_t count = 2 + arg_no * __SOFTBOUNDCETS_METADATA_NUM_FIELDS  + __KEY_INDEX ;
  size_t* key_ptr = (__softboundcets_shadow_stack_ptr + count); 
  size_t key = *key_ptr;
  return key;
}

__WEAK_INLINE void* __softboundcets_load_lock_shadow_stack(int arg_no){

  assert (arg_no >= 0 );
  size_t count = 2 + arg_no * __SOFTBOUNDCETS_METADATA_NUM_FIELDS + __LOCK_INDEX;
  size_t* lock_ptr = (__softboundcets_shadow_stack_ptr + count); 
  void* lock = *((void**)lock_ptr);
  return lock;
}

__WEAK_INLINE void __softboundcets_store_base_shadow_stack(void* base, int arg_no){
  
  assert(arg_no >= 0);
  size_t count = 2 +  arg_no * __SOFTBOUNDCETS_METADATA_NUM_FIELDS + __BASE_INDEX ;
  void** base_ptr = (void**)(__softboundcets_shadow_stack_ptr + count); 

  *(base_ptr) = base;

}

__WEAK_INLINE void __softboundcets_store_bound_shadow_stack(void* bound, int arg_no){

  assert(arg_no >= 0);
  size_t count = 2 +  arg_no * __SOFTBOUNDCETS_METADATA_NUM_FIELDS + __BOUND_INDEX ;
  void** bound_ptr = (void**)(__softboundcets_shadow_stack_ptr + count); 

  *(bound_ptr) = bound;
}

__WEAK_INLINE void __softboundcets_store_key_shadow_stack(size_t key, int arg_no){
  assert(arg_no >= 0);
  size_t count = 2 +  arg_no * __SOFTBOUNDCETS_METADATA_NUM_FIELDS + __KEY_INDEX ;
  size_t* key_ptr = (__softboundcets_shadow_stack_ptr + count); 

  *(key_ptr) = key;

}


__WEAK_INLINE void __softboundcets_store_lock_shadow_stack(void* lock, int arg_no){
  assert(arg_no >= 0);
  size_t count = 2 +  arg_no * __SOFTBOUNDCETS_METADATA_NUM_FIELDS + __LOCK_INDEX ;
  void** lock_ptr = (void**)(__softboundcets_shadow_stack_ptr + count); 

  *(lock_ptr) = lock;
}

__WEAK_INLINE void __softboundcets_deallocate_shadow_stack_space(){

  size_t* reserved_space_ptr = __softboundcets_shadow_stack_ptr;

  size_t read_value = *((size_t*) reserved_space_ptr);

  assert((read_value >=0 && read_value <= __SOFTBOUNDCETS_SHADOW_STACK_ENTRIES));
                                                
  __softboundcets_shadow_stack_ptr =  __softboundcets_shadow_stack_ptr - read_value - 2;
}

__WEAK_INLINE __softboundcets_trie_entry_t* __softboundcets_trie_allocate(){
  
  __softboundcets_trie_entry_t* secondary_entry;
  size_t length = (__SOFTBOUNDCETS_TRIE_SECONDARY_TABLE_ENTRIES) * sizeof(__softboundcets_trie_entry_t);
  secondary_entry = __softboundcets_safe_mmap(0, length, PROT_READ| PROT_WRITE, 
					      SOFTBOUNDCETS_MMAP_FLAGS, -1, 0);
  //assert(secondary_entry != (void*)-1); 
  //printf("snd trie table %p %lx\n", secondary_entry, length);
  return secondary_entry;
}

#if 0

//These are primary used to test and introspect  metadata during testing

__WEAK_INLINE void __softboundcets_print_metadata(void* base, void* bound, void* ptr, size_t key, size_t* lock){
  
  printf("[print metadata] ptr = %p, base=%p, bound=%p, key = %zd, lock = %p, *lock = %zd\n", ptr, base, bound, key, lock, *lock);

}

__WEAK_INLINE void __softboundcets_intermediate(char cmp1, char cmp2, char cmp3, size_t loaded_lock){

  printf("cmp = %d, cmp2 =%d cmp=%d, loaded_lock=%zd\n", cmp1, cmp2, cmp3, loaded_lock);

}

#endif

__WEAK_INLINE void __softboundcets_dummy(){
  
  printf("calling abort");

}
__WEAK_INLINE void __softboundcets_introspect_metadata(void* ptr, 
						       void* base, 
						       void* bound, 
						       int arg_no){
  
  printf("[introspect_metadata]ptr=%p, base=%p, bound=%p, arg_no=%d\n", 
	 ptr, base, bound, arg_no);
}

__METADATA_INLINE 
void __softboundcets_copy_metadata(void* dest, void* from, 
				   size_t size){
  
  //  printf("dest=%p, from=%p, size=%zx\n", dest, from, size);
  
  size_t dest_ptr = (size_t) dest;
  size_t dest_ptr_end = dest_ptr + size;

  size_t from_ptr = (size_t) from;
  size_t from_ptr_end = from_ptr + size;


  if(from_ptr % 8 != 0){
    //printf("dest=%p, from=%p, size=%zx\n", dest, from, size);
    return;
    //    from_ptr = from_ptr %8;
    //    dest_ptr = dest_ptr %8;

  }

  //  printf("dest=%p, from=%p, size=%zx\n", dest, from, size);
  __softboundcets_trie_entry_t* trie_secondary_table_dest_begin;
  __softboundcets_trie_entry_t* trie_secondary_table_from_begin;
  
  size_t dest_primary_index_begin = (dest_ptr >> 25);
  size_t dest_primary_index_end = (dest_ptr_end >> 25);

  size_t from_primary_index_begin = (from_ptr >> 25);
  size_t from_primary_index_end =  (from_ptr_end >> 25);


  if((from_primary_index_begin != from_primary_index_end) || 
     (dest_primary_index_begin != dest_primary_index_end)){

    size_t from_sizet = from_ptr;
    size_t dest_sizet = dest_ptr;

    size_t trie_size = size;
    size_t index = 0;

    for(index=0; index < trie_size; index = index + 8){
      
      size_t temp_from_pindex = (from_sizet + index) >> 25;
      size_t temp_to_pindex = (dest_sizet + index) >> 25;

      size_t dest_secondary_index = (((dest_sizet + index) >> 3) & 0x3fffff);
      size_t from_secondary_index = (((from_sizet + index) >> 3) & 0x3fffff);
      
      __softboundcets_trie_entry_t* temp_from_strie = __softboundcets_trie_primary_table[temp_from_pindex];

      if(temp_from_strie == NULL){
        temp_from_strie = __softboundcets_trie_allocate();
        __softboundcets_trie_primary_table[temp_from_pindex] = temp_from_strie;
      }
     __softboundcets_trie_entry_t* temp_to_strie = __softboundcets_trie_primary_table[temp_to_pindex];

      if(temp_to_strie == NULL){
        temp_to_strie = __softboundcets_trie_allocate();
        __softboundcets_trie_primary_table[temp_to_pindex] = temp_to_strie;
      }

      void* dest_entry_ptr = &temp_to_strie[dest_secondary_index];
      void* from_entry_ptr = &temp_from_strie[from_secondary_index];
  
#ifdef __SOFTBOUNDCETS_SPATIAL
      memcpy(dest_entry_ptr, from_entry_ptr, 16);
#elif __SOFTBOUNDCETS_TEMPORAL
      memcpy(dest_entry_ptr, from_entry_ptr, 16);
#elif __SOFTBOUNDCETS_SPATIAL_TEMPORAL
      memcpy(dest_entry_ptr, from_entry_ptr, 32);
#else
      memcpy(dest_entry_ptr, from_entry_ptr, 32);
#endif
    }    
    return;

  }
    
  trie_secondary_table_dest_begin = __softboundcets_trie_primary_table[dest_primary_index_begin];
  trie_secondary_table_from_begin = __softboundcets_trie_primary_table[from_primary_index_begin];
  
  if(trie_secondary_table_from_begin == NULL)
    return;

  if(trie_secondary_table_dest_begin == NULL){
    trie_secondary_table_dest_begin = __softboundcets_trie_allocate();
    __softboundcets_trie_primary_table[dest_primary_index_begin] = trie_secondary_table_dest_begin;
  }

  size_t dest_secondary_index = ((dest_ptr>> 3) & 0x3fffff);
  size_t from_secondary_index = ((from_ptr>> 3) & 0x3fffff);
  
  assert(dest_secondary_index < __SOFTBOUNDCETS_TRIE_SECONDARY_TABLE_ENTRIES);
  assert(from_secondary_index < __SOFTBOUNDCETS_TRIE_SECONDARY_TABLE_ENTRIES);

  void* dest_entry_ptr = &trie_secondary_table_dest_begin[dest_secondary_index];
  void* from_entry_ptr = &trie_secondary_table_from_begin[from_secondary_index];
  
#ifdef __SOFTBOUNDCETS_SPATIAL

  memcpy(dest_entry_ptr, from_entry_ptr, 16* (size>>3));
#elif __SOFTBOUNDCETS_TEMPORAL

  memcpy(dest_entry_ptr, from_entry_ptr, 16* (size>>3));
#elif __SOFTBOUNDCETS_SPATIAL_TEMPORAL

  memcpy(dest_entry_ptr, from_entry_ptr, 32* (size >> 3));
#else

  memcpy(dest_entry_ptr, from_entry_ptr, 32* (size>> 3));
#endif
  return;
}

__WEAK_INLINE void 
__softboundcets_shrink_bounds(void* new_base, void* new_bound, 
                              void* old_base, void* old_bound, 
                              void** base_alloca, void** bound_alloca) {


  *(base_alloca) = new_base < old_base ? old_base: new_base;
  *(bound_alloca) = new_bound > old_bound? old_bound : new_bound;
}

__WEAK_INLINE void 
__softboundcets_spatial_call_dereference_check(void* base, void* bound, 
                                               void* ptr) {

#ifndef __NOSIM_CHECKS
  if ((base != bound) && (ptr != base)) {
    if (__SOFTBOUNDCETS_DEBUG) {
      __softboundcets_printf("In Call Dereference Check, base=%p, bound=%p, ptr=%p\n", base, bound, ptr);
    }
    __softboundcets_abort();
  }
#endif

}

extern void* malloc_address;
__WEAK_INLINE void 
__softboundcets_spatial_load_dereference_check(void *base, void *bound, 
                                               void *ptr, size_t size_of_type)
{

  if ((ptr < base) || ((void*)((char*) ptr + size_of_type) > bound)) {

    __softboundcets_printf("In LDC, base=%zx, bound=%zx, ptr=%zx\n",
    			   base, bound, ptr);    
    __softboundcets_abort();
  }
}


__WEAK_INLINE void 
__softboundcets_spatial_store_dereference_check(void *base, 
                                                void *bound, 
                                                void *ptr, 
                                                size_t size_of_type)
{
  
  if ((ptr < base) || ((void*)((char*)ptr + size_of_type) > bound)) {
    __softboundcets_printf("In Store Dereference Check, base=%p, bound=%p, ptr=%p, size_of_type=%zx, ptr+size=%p\n",
                              base, bound, ptr, size_of_type, (char*)ptr+size_of_type); 
    
    __softboundcets_abort();
  }
}

/* Memcopy check, different variants based on spatial, temporal and
   spatial+temporal modes
*/

#ifdef __SOFTBOUNDCETS_SPATIAL
__WEAK_INLINE void 
__softboundcets_memcopy_check(void* dest, void* src, size_t size,
                              void* dest_base, void* dest_bound, 
                              void* src_base, void* src_bound) {

  if(size >= LONG_MAX)
    __softboundcets_abort();
  
  if(dest < dest_base || dest > ((char*) dest_bound - size) || (size > (size_t) dest_bound))
    __softboundcets_abort();

  if(src < src_base || src > ((char*) src_bound - size) || (size > (size_t) dest_bound))
    __softboundcets_abort();

}
#elif __SOFTBOUNDCETS_TEMPORAL

__WEAK_INLINE void 
__softboundcets_memcopy_check(void* dest, void* src, size_t size,
                              size_t dest_key, void* dest_lock, 
                              size_t src_key, void* src_lock) {  

  if(size >= LONG_MAX)
    __softboundcets_abort();

  if(dest_key != *((size_t*)(dest_lock))){
    __softboundcets_abort();
  }

  if(src_key != *((size_t*)(src_lock))){
    __softboundcets_abort();
  }

}

#elif __SOFTBOUNDCETS_SPATIAL_TEMPORAL

__WEAK_INLINE void 
__softboundcets_memcopy_check(void* dest, void* src, size_t size,
                              void* dest_base, void* dest_bound, 
                              void* src_base, void* src_bound,
                              size_t dest_key, void* dest_lock, 
                              size_t src_key, void* src_lock) {  

#ifndef __NOSIM_CHECKS

  /* printf("dest=%zx, src=%zx, size=%zx, ulong_max=%zx\n",  */
  /*        dest, src, size, ULONG_MAX); */
  if(size >= LONG_MAX)
    __softboundcets_abort();


  if(dest < dest_base || (char*) dest > ((char*) dest_bound - size) || (size > (size_t) dest_bound))
    __softboundcets_abort();

  if(src < src_base || (char*) src > ((char*) src_bound - size) || (size > (size_t) dest_bound))
    __softboundcets_abort();

  if(dest_key != *((size_t*)(dest_lock))){
    __softboundcets_abort();
  }

  if(src_key != *((size_t*)(src_lock))){
    __softboundcets_abort();
  }

#endif


}
#else

__WEAK_INLINE void 
__softboundcets_memcopy_check(void* dest, void* src, size_t size,
                              void* dest_base, void* dest_bound, 
                              void* src_base, void* src_bound,
                              size_t dest_key, void* dest_lock, 
                              size_t src_key, void* src_lock) {  

  printf("not handled\n");
  __softboundcets_abort();

}
#endif

/* Memset check, different variants based on spatial, temporal and
   spatial+temporal modes */


#ifdef __SOFTBOUNDCETS_SPATIAL
__WEAK_INLINE void 
__softboundcets_memset_check(void* dest, size_t size,
                             void* dest_base, void* dest_bound){


  if(size >= LONG_MAX)
    __softboundcets_abort();
  
  if(dest < dest_base || (char*) dest > ((char*)dest_bound - size) || (size > (size_t)dest_bound))
    __softboundcets_abort();

}
#elif __SOFTBOUNDCETS_TEMPORAL

__WEAK_INLINE void 
__softboundcets_memset_check(void* dest, size_t size,
                             size_t dest_key, void* dest_lock){

  if(size >= LONG_MAX)
    __softboundcets_abort();


  if(size >= LONG_MAX)
    __softboundcets_abort();


  if(dest_key != *((size_t*)(dest_lock))){
    __softboundcets_abort();
  }

}

#elif __SOFTBOUNDCETS_SPATIAL_TEMPORAL

__WEAK_INLINE void 
__softboundcets_memset_check(void* dest, size_t size,
                             void* dest_base, void* dest_bound, 
                             size_t dest_key, void* dest_lock){


#ifndef __NOSIM_CHECKS

  if(size >= LONG_MAX)
    __softboundcets_abort();

  if(dest < dest_base || (char*) dest > ((char*) dest_bound - size) || (size > (size_t) dest_bound))
    __softboundcets_abort();

  if(dest_key != *((size_t*)(dest_lock))){
    __softboundcets_abort();
  }

#endif

}
#else

__WEAK_INLINE void 
__softboundcets_memset_check(void* dest, size_t size,
                             void* dest_base, void* dest_bound, 
                             size_t dest_key, void* dest_lock){
    
  printf("not handled\n");
  __softboundcets_abort();

}
#endif




/* Metadata store parameterized by the mode of checking */

   
#ifdef __SOFTBOUNDCETS_SPATIAL

__METADATA_INLINE void __softboundcets_metadata_store(void* addr_of_ptr, 
                                                      void* base, 
                                                      void* bound) {

#elif __SOFTBOUNDCETS_TEMPORAL

__METADATA_INLINE void __softboundcets_metadata_store(void* addr_of_ptr, 
                                                      size_t key, 
                                                      void* lock) {

#elif __SOFTBOUNDCETS_SPATIAL_TEMPORAL

__METADATA_INLINE void __softboundcets_metadata_store(void* addr_of_ptr, 
                                                      void* base, 
                                                      void* bound, 
                                                      size_t key, 
                                                      void* lock) {  
  
#else

__METADATA_INLINE void __softboundcets_metadata_store(void* addr_of_ptr, 
                                                      void* base, 
                                                      void* bound, 
                                                      size_t key, 
                                                      void* lock) {  

#endif 

  size_t ptr = (size_t) addr_of_ptr;
  size_t primary_index;
  __softboundcets_trie_entry_t* trie_secondary_table;
  //  __softboundcets_trie_entry_t** trie_primary_table = __softboundcets_trie_primary_table;
  
  
  primary_index = (ptr >> 25);
  trie_secondary_table = __softboundcets_trie_primary_table[primary_index];
 
 
  if(!__SOFTBOUNDCETS_PREALLOCATE_TRIE) {
    if(trie_secondary_table == NULL){
      trie_secondary_table =  __softboundcets_trie_allocate();
      __softboundcets_trie_primary_table[primary_index] = trie_secondary_table;
    }    
    //    __softboundcetswithss_printf("addr_of_ptr=%zx, primary_index =%zx, trie_secondary_table=%p\n", addr_of_ptr, primary_index, trie_secondary_table);
    assert(trie_secondary_table != NULL);
  }
  
  size_t secondary_index = ((ptr >> 3) & 0x3fffff);
  __softboundcets_trie_entry_t* entry_ptr =&trie_secondary_table[secondary_index];

  if(__SOFTBOUNDCETS_DEBUG){
    //    printf("[metadata_store] base=%p, bound=%p, key=%zx, lock=%p\n", base, bound, key, lock);
  }
  
  
#ifdef __SOFTBOUNDCETS_SPATIAL

  entry_ptr->base = base;
  entry_ptr->bound = bound;
  //  __softboundcets_printf("[metadata_store] addr_of_ptr=%p, base=%p, bound=%p, primary_index=%zx, secondary_index=%zx, trie_entry_addr=%p\n", addr_of_ptr, base, bound, primary_index, secondary_index, entry_ptr);

#elif __SOFTBOUNDCETS_TEMPORAL

  entry_ptr->key = key;
  entry_ptr->lock = lock;        


#elif __SOFTBOUNDCETS_SPATIAL_TEMPORAL
  
  entry_ptr->base = base;
  entry_ptr->bound = bound;
  entry_ptr->key = key;
  entry_ptr->lock = lock;

#else

  entry_ptr->base = base;
  entry_ptr->bound = bound;
  entry_ptr->key = key;
  entry_ptr->lock = lock;

#endif


  return;
}

#ifdef __SOFTBOUNDCETS_SPATIAL_TEMPORAL

 __WEAK_INLINE void* __softboundcets_metadata_map(void* addr_of_ptr){


    size_t ptr = (size_t) addr_of_ptr;
    __softboundcets_trie_entry_t* trie_secondary_table;
    size_t primary_index = ( ptr >> 25);
    trie_secondary_table = __softboundcets_trie_primary_table[primary_index];

#if 0
    /* unnecessary control flow causes performance overhead */
    /* this can cause segfaults with uninitialized pointer reads from memory */
    if(trie_secondary_table == NULL){
      trie_secondary_table = __softboundcets_trie_allocate();
      __softboundcets_trie_primary_table[primary_index] = trie_secondary_table;
    }

#endif
    
    size_t secondary_index = ((ptr >> 3) & 0x3fffff);
    __softboundcets_trie_entry_t* entry_ptr =&trie_secondary_table[secondary_index];

    return (void*) entry_ptr;
    
 }

 __WEAK_INLINE void* __softboundcets_metadata_load_base(void* address){
      
   __softboundcets_trie_entry_t* entry_ptr = (__softboundcets_trie_entry_t*)address;
   return entry_ptr->base;
   
 }

 __WEAK_INLINE void* __softboundcets_metadata_load_bound(void* address){

   __softboundcets_trie_entry_t* entry_ptr = (__softboundcets_trie_entry_t*)address;
   return entry_ptr->bound;


 }

 __WEAK_INLINE size_t __softboundcets_metadata_load_key(void* address){

   __softboundcets_trie_entry_t* entry_ptr = (__softboundcets_trie_entry_t*)address;
   return entry_ptr->key;

 }

 __WEAK_INLINE void* __softboundcets_metadata_load_lock(void* address){

   __softboundcets_trie_entry_t* entry_ptr = (__softboundcets_trie_entry_t*)address;
   return entry_ptr->lock;

 }

#endif



#ifdef __SOFTBOUNDCETS_SPATIAL

__METADATA_INLINE void __softboundcets_metadata_load(void* addr_of_ptr, 
                                                     void** base, void** bound){   

#elif __SOFTBOUNDCETS_TEMPORAL

__METADATA_INLINE void __softboundcets_metadata_load(void* addr_of_ptr, 
                                                     size_t* key, void** lock){   

#elif __SOFTBOUNDCETS_SPATIAL_TEMPORAL

__METADATA_INLINE void __softboundcets_metadata_load(void* addr_of_ptr, void** base, 
                                                     void** bound, size_t* key, void** lock){   

#else
 
__METADATA_INLINE void __softboundcets_metadata_load(void* addr_of_ptr, void** base, 
                                                     void** bound, size_t* key, void** lock){

#endif

  size_t ptr = (size_t) addr_of_ptr;
  __softboundcets_trie_entry_t* trie_secondary_table;
  //    __softboundcets_trie_entry_t** trie_primary_table = __softboundcets_trie_primary_table;
    
  //assert(__softboundcetswithss_trie_primary_table[primary_index] == trie_secondary_table);

  size_t primary_index = ( ptr >> 25);
  trie_secondary_table = __softboundcets_trie_primary_table[primary_index];


  if(!__SOFTBOUNDCETS_PREALLOCATE_TRIE) {      
    if(trie_secondary_table == NULL) {  

#ifdef __SOFTBOUNDCETS_SPATIAL
      *((void**) base) = 0;
      *((void**) bound) = 0;
#elif __SOFTBOUNDCETS_TEMPORAL
      *((size_t*) key ) = 0;
      *((size_t*) lock) = 0;        

#elif __SOFTBOUNDCETS_SPATIAL_TEMPORAL

      *((void**) base) = 0;
      *((void**) bound) = 0;
      *((size_t*) key ) = 0;
      *((size_t*) lock) = 0;        

#else
      *((void**) base) = 0;
      *((void**) bound) = 0;
      *((size_t*) key ) = 0;
      *((size_t*) lock) = 0;                
#endif 
      return;
    }
  } /* PREALLOCATE_ENDS */

    /* MAIN SOFTBOUNDCETS LOAD WHICH RUNS ON THE NORMAL MACHINE */
  size_t secondary_index = ((ptr >> 3) & 0x3fffff);
  __softboundcets_trie_entry_t* entry_ptr = &trie_secondary_table[secondary_index];
    
#ifdef __SOFTBOUNDCETS_SPATIAL
  *((void**) base) = entry_ptr->base;
  *((void**) bound) = entry_ptr->bound;

#elif __SOFTBOUNDCETS_TEMPORAL
  *((size_t*) key) = entry_ptr->key;
  *((void**) lock) = (void*) entry_ptr->lock;

#elif __SOFTBOUNDCETS_SPATIAL_TEMPORAL

  *((void**) base) = entry_ptr->base;
  *((void**) bound) = entry_ptr->bound;
  *((size_t*) key) = entry_ptr->key;
  *((void**) lock) = (void*) entry_ptr->lock;
      
#else
  
  *((void**) base) = entry_ptr->base;
  *((void**) bound) = entry_ptr->bound;
  *((size_t*) key) = entry_ptr->key;
  *((void**) lock) = (void*) entry_ptr->lock;

#endif      
  return;
}
/******************************************************************************/

extern size_t __softboundcets_key_id_counter;
extern size_t* __softboundcets_lock_next_location;
extern size_t* __softboundcets_lock_new_location;

#ifdef __SOFTBOUNDCETS_SPATIAL_TEMPORAL
__WEAK_INLINE void 
__softboundcets_temporal_load_dereference_check(void* pointer_lock, 
                                                size_t key, 
                                                void* base, 
                                                void* bound) {
#else
__WEAK_INLINE void 
__softboundcets_temporal_load_dereference_check(void* pointer_lock, 
                                                size_t key) {
#endif


#if 0  
  /* URGENT: I should think about removing this condition check */
  if(!pointer_lock){
    if(__SOFTBOUNDCETS_DEBUG) {
      __softboundcets_printf("Temporal lock null\n");
    }
    __softboundcets_printf("Temporal lock null\n");
    __softboundcets_abort();
    return;
  }

#endif


  size_t temp = *((size_t*)pointer_lock);
  
  if(temp != key) {
    if(__SOFTBOUNDCETS_DEBUG) {
      __softboundcets_printf("[TLDC] Key mismatch key = %zx, *lock=%zx\n", 
                             key, temp );
    }
    __softboundcets_printf("[TLDC] Key mismatch key = %zx, *lock=%zx, next_ptr =%zx\n", 
                           key, temp, __softboundcets_lock_next_location );
    __softboundcets_abort();    
  }

}

#ifdef __SOFTBOUNDCETS_SPATIAL_TEMPORAL
__WEAK_INLINE void 
__softboundcets_temporal_store_dereference_check(void* pointer_lock, 
                                                 size_t key, 
                                                 void* base, 
                                                 void* bound) {
#else
__WEAK_INLINE void 
__softboundcets_temporal_store_dereference_check(void* pointer_lock, 
                                                 size_t key){
#endif    

#if 0
  if(!pointer_lock){
    __softboundcets_printf("lock null?");
    __softboundcets_abort();    
  }
#endif
  
  size_t temp = *((size_t*)pointer_lock);
  
  if(temp != key) {

    if(__SOFTBOUNDCETS_DEBUG) {
      __softboundcets_printf("[TSDC] Key mismatch, key = %zx, *lock=%zx\n", 
                             key, temp );
    }
    __softboundcets_printf("[TSDC] Key mismatch, key = %zx, *lock=%zx\n", 
                           key, temp );    
    __softboundcets_abort();    
  }
}


__WEAK_INLINE void __softboundcets_stack_memory_deallocation(size_t ptr_key){

  
#ifndef __SOFTBOUNDCETS_CONSTANT_STACK_KEY_LOCK

  __softboundcets_stack_temporal_space_begin--;
  *(__softboundcets_stack_temporal_space_begin) = 0;

#endif

  return;

}

__WEAK_INLINE void 
__softboundcets_memory_deallocation(void* ptr_lock, size_t ptr_key) {

  
  if(__SOFTBOUNDCETS_DEBUG){
    __softboundcets_printf("[Hdealloc] pointer_lock = %p, *pointer_lock=%zx\n", 
                           ptr_lock, *((size_t*) ptr_lock));
  }

#if 0
  if(!ptr_lock)
    return;
#endif
  
  *((size_t*)ptr_lock) = 0;
  *((void**) ptr_lock) = __softboundcets_lock_next_location;
  __softboundcets_lock_next_location = ptr_lock;

}

__WEAK_INLINE void*  __softboundcets_allocate_lock_location() {
  
  void* temp= NULL;
  if(__softboundcets_lock_next_location == NULL) {
    if(__SOFTBOUNDCETS_DEBUG) {
      __softboundcets_printf("[lock_allocate] new_lock_location=%p\n", 
                             __softboundcets_lock_new_location);
      
      if(__softboundcets_lock_new_location  > 
         __softboundcets_temporal_space_begin + __SOFTBOUNDCETS_N_TEMPORAL_ENTRIES){
        __softboundcets_printf("[lock_allocate] out of temporal free entries \n");
        __softboundcets_abort();
      }
    }

    return __softboundcets_lock_new_location++;
  }
  else{

    temp = __softboundcets_lock_next_location;
    if(__SOFTBOUNDCETS_DEBUG){
      __softboundcets_printf("[lock_allocate] next_lock_location=%p\n", temp);
    }

    __softboundcets_lock_next_location = *((void**)__softboundcets_lock_next_location);
    return temp;
  }
}

__WEAK_INLINE void 
__softboundcets_allocation_secondary_trie_allocate_range(void* initial_ptr, 
                                                         size_t size) {

  if(!__SOFTBOUNDCETS_PREALLOCATE_TRIE)
    return;

  void* addr_of_ptr = initial_ptr;
  size_t start_addr_of_ptr = (size_t) addr_of_ptr;
  size_t start_primary_index = start_addr_of_ptr >> 25;
  
  size_t end_addr_of_ptr = (size_t)((char*) initial_ptr + size);
  size_t end_primary_index = end_addr_of_ptr >> 25;
  
  for(; start_primary_index <= end_primary_index; start_primary_index++){
    
    __softboundcets_trie_entry_t* 
      trie_secondary_table = __softboundcets_trie_primary_table[start_primary_index];    
    if(trie_secondary_table == NULL) {
      trie_secondary_table =  __softboundcets_trie_allocate();
      __softboundcets_trie_primary_table[start_primary_index] = trie_secondary_table;
    }
  }
}

__WEAK_INLINE void 
__softboundcets_allocation_secondary_trie_allocate(void* addr_of_ptr) {
  
  /* URGENT: THIS FUNCTION REQUIRES REWRITE */

  if(!__SOFTBOUNDCETS_PREALLOCATE_TRIE)
    return;


  size_t ptr = (size_t) addr_of_ptr;
  size_t primary_index = ( ptr >> 25);
  //  size_t secondary_index = ((ptr >> 3) & 0x3fffff);
  
  __softboundcets_trie_entry_t* 
    trie_secondary_table = __softboundcets_trie_primary_table[primary_index];

  if(trie_secondary_table == NULL) {
    trie_secondary_table =  __softboundcets_trie_allocate();
    __softboundcets_trie_primary_table[primary_index] = trie_secondary_table;
  }

  __softboundcets_trie_entry_t* 
    trie_secondary_table_second_entry = __softboundcets_trie_primary_table[primary_index +1];

  if(trie_secondary_table_second_entry == NULL) {
    __softboundcets_trie_primary_table[primary_index+1] = __softboundcets_trie_allocate();
  }

  if(primary_index != 0 && (__softboundcets_trie_primary_table[primary_index -1] == NULL)){
    __softboundcets_trie_primary_table[primary_index-1] = __softboundcets_trie_allocate();    
  }

  return;
}


__WEAK_INLINE void 
__softboundcets_stack_memory_allocation(void** ptr_lock, size_t* ptr_key) {


#ifdef __SOFTBOUNDCETS_CONSTANT_STACK_KEY_LOCK
  *((size_t*) ptr_key) = 1;
  *((size_t**) ptr_lock) = __softboundcets_global_lock;
#else
  size_t temp_id = __softboundcets_key_id_counter++;
  *((size_t**) ptr_lock) = (size_t*)__softboundcets_stack_temporal_space_begin++;
  *((size_t*)ptr_key) = temp_id;
  **((size_t**)ptr_lock) = temp_id;  
#endif

}

__WEAK_INLINE void 
__softboundcets_memory_allocation(void* ptr, void** ptr_lock, size_t* ptr_key){

  size_t temp_id = __softboundcets_key_id_counter++;

  *((size_t**) ptr_lock) = (size_t*)__softboundcets_allocate_lock_location();  
  *((size_t*) ptr_key) = temp_id;
  **((size_t**) ptr_lock) = temp_id;

  __softboundcets_add_to_free_map(temp_id, ptr);
  //  printf("memory allocation ptr=%zx, ptr_key=%zx\n", ptr, temp_id);
  __softboundcets_allocation_secondary_trie_allocate(ptr);

  if(__SOFTBOUNDCETS_DEBUG) {    
    __softboundcets_printf("[mem_alloc] lock = %p, ptr_key = %p, key = %zx\n", 
                           ptr_lock, ptr_key, temp_id);
  }
}


__WEAK_INLINE void* __softboundcets_get_global_lock(){  
  return __softboundcets_global_lock;
}

__WEAK_INLINE void __softboundcets_add_to_free_map(size_t ptr_key, void* ptr) {

  if(!__SOFTBOUNDCETS_FREE_MAP)
    return;

  assert(ptr!= NULL);

  size_t counter  = 0;
  while(1){
    size_t index = (ptr_key + counter) % __SOFTBOUNDCETS_N_FREE_MAP_ENTRIES;
    size_t* entry_ptr = &__softboundcets_free_map_table[index];
    size_t tag = *entry_ptr;

    if(tag == 0 || tag == 2) {
      //      printf("entry_ptr=%zx, ptr=%zx, key=%zx\n", entry_ptr, ptr, ptr_key);
      *entry_ptr = (size_t)(ptr);
      return;
    }
    if(counter >= (__SOFTBOUNDCETS_N_FREE_MAP_ENTRIES)) {
#ifndef __NOSIM_CHECKS
      __softboundcets_abort();
#else
      break;
#endif
    }
    counter++;
  }
  return;
}


__WEAK_INLINE void __softboundcets_check_remove_from_free_map(size_t ptr_key, void* ptr) {

  if(! __SOFTBOUNDCETS_FREE_MAP){
    return;
  }
  size_t counter = 0;
  while(1) {
    size_t index = (ptr_key + counter) % __SOFTBOUNDCETS_N_FREE_MAP_ENTRIES;
    size_t* entry_ptr = &__softboundcets_free_map_table[index];
    size_t tag = *entry_ptr;

    if(tag == 0) {
#ifndef __NOSIM_CHECKS      
      __softboundcets_abort();
#else
      break;
#endif
    }

    if(tag == (size_t) ptr) {      
      *entry_ptr = 2;
      return;
    }

    if(counter >= __SOFTBOUNDCETS_N_FREE_MAP_ENTRIES) {
      //      printf("free map out of entries\n");
#ifndef __NOSIM_CHECKS
      __softboundcets_abort();
#else
      break;
#endif
    }
    counter++;
  }
  return;
}

 __METADATA_INLINE void __softboundcets_metadata_load_vector(void* addr_of_ptr, 
							     void** base, 
							     void** bound, 
							     size_t* key, 
							     void** lock, 
							     int index){

   size_t val = index * 8;
   size_t addr = (size_t) addr_of_ptr;
   addr = addr + val;

   __softboundcets_metadata_load((void*) addr, base, bound, key, lock);   

 }

 __METADATA_INLINE void __softboundcets_metadata_store_vector(void* addr_of_ptr, 
							      void* base, 
							      void* bound, 
							      size_t key, 
							      void* lock, 
							      int index){
   size_t val = index * 8;
   size_t addr = (size_t) addr_of_ptr;
   addr = addr + val;

   __softboundcets_metadata_store((void*)addr, base, bound, key, lock);
   
 }


#endif

