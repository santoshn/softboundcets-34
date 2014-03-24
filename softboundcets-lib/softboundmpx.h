//=== SoftBoundRuntime/softboundmpx.h - headers for functions introduced by SoftBound with MPX style metadata--*- C -*===// 
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


#ifndef __SOFTBOUNDMPX_H__
#define __SOFTBOUNDMPX_H__

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

typedef struct {
  void* base;
  void* bound;  
  void* ptr;
  void* dummy;

#define __SOFTBOUNDMPX_METADATA_NUM_FIELDS 4
#define __BASE_INDEX 0
#define __BOUND_INDEX 1
#define __PTR_INDEX 2
} __softboundmpx_trie_entry_t;


#if defined(__APPLE__)
#define SOFTBOUNDMPX_MMAP_FLAGS (MAP_ANON|MAP_NORESERVE|MAP_PRIVATE)
#else
#define SOFTBOUNDMPX_MMAP_FLAGS (MAP_PRIVATE|MAP_ANONYMOUS|MAP_NORESERVE)
#endif

static const size_t __SOFTBOUNDMPX_LOWER_ZERO_POINTER_BITS = 3;
// 2^23 entries each will be 8 bytes each 
static const size_t __SOFTBOUNDMPX_TRIE_PRIMARY_TABLE_ENTRIES = ((size_t) 8*(size_t) 1024 * (size_t) 1024);
static const size_t __SOFTBOUNDMPX_SHADOW_STACK_ENTRIES = ((size_t) 128 * (size_t) 32 );
// each secondary entry has 2^ 22 entries 
static const size_t __SOFTBOUNDMPX_TRIE_SECONDARY_TABLE_ENTRIES = ((size_t) 4 * (size_t) 1024 * (size_t) 1024); 


#define __WEAK__ __attribute__((__weak__))
#define __WEAK_INLINE __attribute__((__weak__,__always_inline__)) 

#define __METADATA_INLINE __attribute__((__weak__, __always_inline__))
#define __NO_INLINE __attribute__((__noinline__))

extern __softboundmpx_trie_entry_t** __softboundmpx_trie_primary_table;
extern size_t* __softboundmpx_shadow_stack_ptr;

extern void __softboundmpx_init();
extern void __softboundmpx_abort();
extern void __softboundmpx_printf(const char* str, ...);

void* __softboundmpx_safe_calloc(size_t, size_t);
void* __softboundmpx_safe_malloc(size_t);
void __softboundmpx_safe_free(void*);

void * __softboundmpx_safe_mmap(void* addr, 
                                 size_t length, int prot, 
                                 int flags, int fd, 
				off_t offset);

/******************************************************************************/

static __attribute__ ((__constructor__)) void __softboundmpx_global_init();

extern __NO_INLINE void __softboundmpx_stub(void);

void __softboundmpx_global_init()
{
  __softboundmpx_init();
  __softboundmpx_stub();
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
  
__WEAK_INLINE void __softboundmpx_allocate_shadow_stack_space(int num_pointer_args){
 

  size_t* prev_stack_size_ptr = __softboundmpx_shadow_stack_ptr + 1;
  size_t prev_stack_size = *((size_t*)prev_stack_size_ptr);

  __softboundmpx_shadow_stack_ptr = __softboundmpx_shadow_stack_ptr + prev_stack_size + 2;

  *((size_t*) __softboundmpx_shadow_stack_ptr) = prev_stack_size;
  size_t* current_stack_size_ptr = __softboundmpx_shadow_stack_ptr + 1;
  
  ssize_t size = num_pointer_args * __SOFTBOUNDMPX_METADATA_NUM_FIELDS;
  *((size_t*) current_stack_size_ptr) = size;
}
   
__WEAK_INLINE void* __softboundmpx_load_base_shadow_stack(int arg_no){
  assert (arg_no >= 0 );
  size_t count = 2 +  arg_no * __SOFTBOUNDMPX_METADATA_NUM_FIELDS + __BASE_INDEX ;
  size_t* base_ptr = (__softboundmpx_shadow_stack_ptr + count); 
  void* base = *((void**)base_ptr);
  return base;
}

__WEAK_INLINE void* __softboundmpx_load_bound_shadow_stack(int arg_no){

  assert (arg_no >= 0 );
  size_t count = 2 + arg_no * __SOFTBOUNDMPX_METADATA_NUM_FIELDS  + __BOUND_INDEX ;
  size_t* bound_ptr = (__softboundmpx_shadow_stack_ptr + count); 

  void* bound = *((void**)bound_ptr);
  
  return bound;
}


__WEAK_INLINE void __softboundmpx_store_base_shadow_stack(void* base, int arg_no){
  
  assert(arg_no >= 0);
  size_t count = 2 +  arg_no * __SOFTBOUNDMPX_METADATA_NUM_FIELDS + __BASE_INDEX ;
  void** base_ptr = (void**)(__softboundmpx_shadow_stack_ptr + count); 

  *(base_ptr) = base;
}

__WEAK_INLINE void __softboundmpx_store_bound_shadow_stack(void* bound, int arg_no){

  assert(arg_no >= 0);
  size_t count = 2 +  arg_no * __SOFTBOUNDMPX_METADATA_NUM_FIELDS + __BOUND_INDEX ;
  void** bound_ptr = (void**)(__softboundmpx_shadow_stack_ptr + count); 

  *(bound_ptr) = bound;
}


__WEAK_INLINE void __softboundmpx_deallocate_shadow_stack_space(){

  size_t* reserved_space_ptr = __softboundmpx_shadow_stack_ptr;
  size_t read_value = *((size_t*) reserved_space_ptr);
  assert((read_value >=0 && read_value <= __SOFTBOUNDMPX_SHADOW_STACK_ENTRIES));
  __softboundmpx_shadow_stack_ptr =  __softboundmpx_shadow_stack_ptr - read_value - 2;
}

__WEAK_INLINE __softboundmpx_trie_entry_t* __softboundmpx_trie_allocate(){
  
  __softboundmpx_trie_entry_t* secondary_entry;
  size_t length = (__SOFTBOUNDMPX_TRIE_SECONDARY_TABLE_ENTRIES) * sizeof(__softboundmpx_trie_entry_t);
  secondary_entry = __softboundmpx_safe_mmap(0, length, PROT_READ| PROT_WRITE, 
					      SOFTBOUNDMPX_MMAP_FLAGS, -1, 0);
  return secondary_entry;
}

__WEAK_INLINE void __softboundmpx_dummy(){
  
  printf("calling abort");

}
__WEAK_INLINE void __softboundmpx_introspect_metadata(void* ptr, 
						       void* base, 
						       void* bound, 
						       int arg_no){
  
  printf("[introspect_metadata]ptr=%p, base=%p, bound=%p, arg_no=%d\n", 
	 ptr, base, bound, arg_no);
}

__METADATA_INLINE 
void __softboundmpx_copy_metadata(void* dest, void* from, 
				   size_t size){
  
  
  size_t dest_ptr = (size_t) dest;
  size_t dest_ptr_end = dest_ptr + size;

  size_t from_ptr = (size_t) from;
  size_t from_ptr_end = from_ptr + size;


  if(from_ptr % 8 != 0){
    return;
  }

  __softboundmpx_trie_entry_t* trie_secondary_table_dest_begin;
  __softboundmpx_trie_entry_t* trie_secondary_table_from_begin;
  
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
      
      __softboundmpx_trie_entry_t* temp_from_strie = __softboundmpx_trie_primary_table[temp_from_pindex];

      if(temp_from_strie == NULL){
        temp_from_strie = __softboundmpx_trie_allocate();
        __softboundmpx_trie_primary_table[temp_from_pindex] = temp_from_strie;
      }
     __softboundmpx_trie_entry_t* temp_to_strie = __softboundmpx_trie_primary_table[temp_to_pindex];

      if(temp_to_strie == NULL){
        temp_to_strie = __softboundmpx_trie_allocate();
        __softboundmpx_trie_primary_table[temp_to_pindex] = temp_to_strie;
      }

      void* dest_entry_ptr = &temp_to_strie[dest_secondary_index];
      void* from_entry_ptr = &temp_from_strie[from_secondary_index];
  
      memcpy(dest_entry_ptr, from_entry_ptr, 32);
    }    
    return;

  }
    
  trie_secondary_table_dest_begin = __softboundmpx_trie_primary_table[dest_primary_index_begin];
  trie_secondary_table_from_begin = __softboundmpx_trie_primary_table[from_primary_index_begin];
  
  if(trie_secondary_table_from_begin == NULL)
    return;

  if(trie_secondary_table_dest_begin == NULL){
    trie_secondary_table_dest_begin = __softboundmpx_trie_allocate();
    __softboundmpx_trie_primary_table[dest_primary_index_begin] = trie_secondary_table_dest_begin;
  }

  size_t dest_secondary_index = ((dest_ptr>> 3) & 0x3fffff);
  size_t from_secondary_index = ((from_ptr>> 3) & 0x3fffff);
  
  assert(dest_secondary_index < __SOFTBOUNDMPX_TRIE_SECONDARY_TABLE_ENTRIES);
  assert(from_secondary_index < __SOFTBOUNDMPX_TRIE_SECONDARY_TABLE_ENTRIES);

  void* dest_entry_ptr = &trie_secondary_table_dest_begin[dest_secondary_index];
  void* from_entry_ptr = &trie_secondary_table_from_begin[from_secondary_index];
  
  memcpy(dest_entry_ptr, from_entry_ptr, 32* (size>> 3));
  return;
}

__WEAK_INLINE void 
__softboundmpx_shrink_bounds(void* new_base, void* new_bound, 
                              void* old_base, void* old_bound, 
                              void** base_alloca, void** bound_alloca) {

  *(base_alloca) = new_base < old_base ? old_base: new_base;
  *(bound_alloca) = new_bound > old_bound? old_bound : new_bound;
}

__WEAK_INLINE void 
__softboundmpx_spatial_call_dereference_check(void* base, void* bound, 
                                               void* ptr) {


#ifndef __NOSIM_CHECKS
  if ((base != bound) && (ptr != base)) {
    __softboundmpx_abort();
  }
#endif

}

extern void* malloc_address;
__WEAK_INLINE void 
__softboundmpx_spatial_load_dereference_check(void *base, void *bound, 
                                               void *ptr, size_t size_of_type)
{

  if ((ptr < base) || ((void*)((char*) ptr + size_of_type) > bound)) {
    __softboundmpx_abort();
  }
}


__WEAK_INLINE void 
__softboundmpx_spatial_store_dereference_check(void *base, 
                                                void *bound, 
                                                void *ptr, 
                                                size_t size_of_type)
{

  if ((ptr < base) || ((void*)((char*)ptr + size_of_type) > bound)) {
    __softboundmpx_abort();
  }
}

/* Memcopy check, different variants based on spatial, temporal and
   spatial+temporal modes
*/


__WEAK_INLINE void 
__softboundmpx_memcopy_check(void* dest, void* src, size_t size,
                              void* dest_base, void* dest_bound, 
                              void* src_base, void* src_bound) {

  if(size >= LONG_MAX)
    __softboundmpx_abort();
  
  if(dest < dest_base || ((char*)dest > ((char*) dest_bound - size)) || (size > (size_t) dest_bound))
    __softboundmpx_abort();

  if(src < src_base || ((char*) src > ((char*) src_bound - size)) || (size > (size_t) dest_bound))
    __softboundmpx_abort();

}


__WEAK_INLINE void 
__softboundmpx_memset_check(void* dest, size_t size,
                             void* dest_base, void* dest_bound){

  if(size >= LONG_MAX)
    __softboundmpx_abort();
  
  if(dest < dest_base || (char*) dest > ((char*)dest_bound - size) || (size > (size_t)dest_bound))
    __softboundmpx_abort();

}

__METADATA_INLINE void __softboundmpx_metadata_store(void* addr_of_ptr, 
                                                      void* base, 
                                                      void* bound,
						      void* stored_ptr) {

  size_t ptr = (size_t) addr_of_ptr;
  size_t primary_index;
  __softboundmpx_trie_entry_t* trie_secondary_table;
  
  primary_index = (ptr >> 25);
  trie_secondary_table = __softboundmpx_trie_primary_table[primary_index];
   
  if(trie_secondary_table == NULL){
    trie_secondary_table =  __softboundmpx_trie_allocate();
    __softboundmpx_trie_primary_table[primary_index] = trie_secondary_table;
  }    
  
  size_t secondary_index = ((ptr >> 3) & 0x3fffff);
  __softboundmpx_trie_entry_t* entry_ptr =&trie_secondary_table[secondary_index];

  entry_ptr->base = base;
  entry_ptr->bound = bound;
  entry_ptr->ptr = stored_ptr;
  return;
}


__METADATA_INLINE void __softboundmpx_metadata_load(void* addr_of_ptr, 
                                                     void** base, void** bound){   
  
  size_t ptr = (size_t) addr_of_ptr;
  __softboundmpx_trie_entry_t* trie_secondary_table;
  size_t primary_index = ( ptr >> 25);
  trie_secondary_table = __softboundmpx_trie_primary_table[primary_index];
  
  if(trie_secondary_table == NULL) {        
    *((void**) base) = 0;
    *((void**) bound) = 0;      
    return;
  }

  size_t secondary_index = ((ptr >> 3) & 0x3fffff);
  __softboundmpx_trie_entry_t* entry_ptr = &trie_secondary_table[secondary_index];
  
  /* Check for ptr */

  void* stored_base = entry_ptr->base;
  void* stored_bound = entry_ptr->bound;
  
  /* MPX style checking and unbounding */

  void* current_ptr = *((void**)(addr_of_ptr));
  if(current_ptr != entry_ptr->ptr){
    stored_base = NULL;
    size_t unbound_value = 1;
    stored_bound = (void*)(unbound_value << 48);
  }
  
  *((void**) base) = stored_base;
  *((void**) bound) = stored_bound;

}

#endif

