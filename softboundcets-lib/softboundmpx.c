//=== softboundmpx.c - Creates the main function for SoftBound+CETS Runtime --*- C -*===// 
// Copyright (c) 2014 Santosh Nagarakatte, Milo M. K. Martin. All rights reserved.
//
// Developed by: Santosh Nagarakatte,
//               Department of Computer Science, Rutgers University
//               https://github.com/santoshn/softboundcets-34/
//               http://www.cs.rutgers.edu/~santosh.nagarakatte/
//
//               in collaboration with 
//
//               Milo M.K. Martin, Jianzhou Zhao, Steve Zdancewic
//               Department of Computer and Information Sciences,
//               University of Pennsylvania


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


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#if defined(__linux__)
#include <malloc.h>
#endif
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/mman.h>
#if !defined(__FreeBSD__)
#include <execinfo.h>
#endif
#include "softboundmpx.h"

__softboundmpx_trie_entry_t** __softboundmpx_trie_primary_table;
size_t* __softboundmpx_shadow_stack_ptr = NULL;
size_t __softboundmpx_deref_check_count = 0;
void* malloc_address = NULL;


void __softboundmpx_abort()
{
  fprintf(stderr, "\nSoftboundmpx: Memory safety violation detected\n\nBacktrace:\n");

  // Based on code from the backtrace man page
  size_t size;
  void *array[100];
  
#if !defined (__FreeBSD__)
  size = backtrace(array, 100);
  backtrace_symbols_fd(array, size, fileno(stderr));
#endif
  
  fprintf(stderr, "\n\n");

  abort();
}

static int softboundmpx_initialized = 0;

__NO_INLINE void __softboundmpx_stub(void) {
  return;
}
void __softboundmpx_init() 
{
  if (softboundmpx_initialized != 0) {
    return;  // already initialized, do nothing
  }
  softboundmpx_initialized = 1;

  assert(sizeof(__softboundmpx_trie_entry_t) >= 16);

  size_t shadow_stack_size = __SOFTBOUNDMPX_SHADOW_STACK_ENTRIES * sizeof(size_t);
  __softboundmpx_shadow_stack_ptr = mmap(0, shadow_stack_size, 
                                          PROT_READ|PROT_WRITE, 
                                          SOFTBOUNDMPX_MMAP_FLAGS, -1, 0);
  assert(__softboundmpx_shadow_stack_ptr != (void*)-1);

  *((size_t*)__softboundmpx_shadow_stack_ptr) = 0; /* prev stack size */
  size_t * current_size_shadow_stack_ptr =  __softboundmpx_shadow_stack_ptr +1 ;
  *(current_size_shadow_stack_ptr) = 0;

  size_t length_trie = (__SOFTBOUNDMPX_TRIE_PRIMARY_TABLE_ENTRIES) * sizeof(__softboundmpx_trie_entry_t*);
  
  __softboundmpx_trie_primary_table = mmap(0, length_trie, 
					   PROT_READ| PROT_WRITE, 
					   SOFTBOUNDMPX_MMAP_FLAGS, -1, 0);
  assert(__softboundmpx_trie_primary_table != (void *)-1);  
  
  return;
}

static void softboundmpx_init_ctype(){  
#if defined(__linux__)

  char* ptr;
  char* base_ptr;

  ptr = (void*) __ctype_b_loc();
  base_ptr = (void*) (*(__ctype_b_loc()));

  __softboundmpx_metadata_store(ptr, ((char*) base_ptr - 129), 
				((char*) base_ptr + 256), base_ptr);
#endif // __linux ends 
}

void __softboundmpx_printf(const char* str, ...)
{
  va_list args;  
  va_start(args, str);
  vfprintf(stderr, str, args);
  va_end(args);
}

extern int softboundmpx_pseudo_main(int argc, char **argv);

int main(int argc, char **argv){

#if __WORDSIZE == 32
  exit(1);
#endif
  
  char** new_argv = argv;
  int i;
  char* temp_ptr;
  int return_value;
#if defined(__linux__)
  mallopt(M_MMAP_MAX, 0);
#endif

  for(i = 0; i < argc; i++) { 
    __softboundmpx_metadata_store(&new_argv[i], 
				  new_argv[i], 
				  new_argv[i] + strlen(new_argv[i]) + 1,
				  new_argv[i]);    
  }
  softboundmpx_init_ctype();

  /* Santosh: Real Nasty hack because C programmers assume argv[argc]
   * to be NULL. Also this NUll is a pointer, doing + 1 will make the
   * size_of_type to fail
   */
  temp_ptr = ((char*) &new_argv[argc]) + 8;
  __softboundmpx_allocate_shadow_stack_space(2);
  __softboundmpx_store_base_shadow_stack(&new_argv[0], 1);
  __softboundmpx_store_bound_shadow_stack(temp_ptr, 1);
  return_value = softboundmpx_pseudo_main(argc, new_argv);
  __softboundmpx_deallocate_shadow_stack_space();
  return return_value;
}

void * __softboundmpx_safe_mmap(void* addr, 
                                 size_t length, int prot, 
                                 int flags, int fd, 
                                 off_t offset){
  return mmap(addr, length, prot, flags, fd, offset);
}

void* __softboundmpx_safe_calloc(size_t nmemb, size_t size){

  return calloc(nmemb, size);
}

void* __softboundmpx_safe_malloc(size_t size){

  return malloc(size);
}
void __softboundmpx_safe_free(void* ptr){

  free(ptr);
}
