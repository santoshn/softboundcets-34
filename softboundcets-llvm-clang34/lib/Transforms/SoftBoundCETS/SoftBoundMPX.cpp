//=== SoftBoundCETS/SoftBoundMPX.cpp --*- C++ -*=====///
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
//               http://www.cis.upenn.edu/acg/softbound/
//
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

#include "llvm/Transforms/SoftBoundCETS/SoftBoundMPXPass.h"


cl::opt<bool>
mpx_eliminate_struct_checks
("mpx_eliminate_struct_checks",
 cl::desc("don't perform any spatial checking for structure accesses"),
 cl::init(false));



static cl::opt<bool>
MPXBOUNDSCHECKOPT 
("softboundmpx_bounds_check_opt",
 cl::desc("enable dominator based load dereference check elimination"),
 cl::init(true));


static cl::opt<bool>
MPXCALLCHECKS
("softboundmpx_call_checks",
 cl::desc("introduce call checks"),
 cl::init(true));

static cl::opt<bool>
MPXINDIRECTCALLCHECKS
("softboundmpx_indirect_call_checks",
 cl::desc("introduce indirect call checks"),
 cl::init(false));


char SoftBoundMPXPass:: ID = 0;

static RegisterPass<SoftBoundMPXPass> P ("SoftBoundMPXPass",
                                          "SoftBound Pass for Spatial Safety similar to Intel MPX");



//
// Method: initializeSoftBoundVariables()
//
// Description: 
// This function initializes the Function*'s that will be
// inserted by the SoftBound/CETS Pass
//
// Input:
//
// module: Input module that has either the function definitions or
// the function prototypes for the SoftBound/CETS functions
//

void SoftBoundMPXPass::initializeSoftBoundVariables(Module& module) {

    m_spatial_load_dereference_check = 
      module.getFunction("__softboundmpx_spatial_load_dereference_check");
    assert(m_spatial_load_dereference_check && 
           "__softboundmpx_spatial_load_dereference_check function type null?");
    
    m_spatial_store_dereference_check = 
      module.getFunction("__softboundmpx_spatial_store_dereference_check");
    assert(m_spatial_store_dereference_check && 
           "__softboundmpx_spatial_store_dereference_check function type null?");
    

  m_introspect_metadata = 
    module.getFunction("__softboundmpx_introspect_metadata");
  assert(m_introspect_metadata && 
         "__softboundmpx_introspect_metadata null?");
    
  m_copy_metadata = module.getFunction("__softboundmpx_copy_metadata");
  assert(m_copy_metadata && "__softboundmpx_copy_metadata NULL?");
    
  m_shadow_stack_allocate = 
    module.getFunction("__softboundmpx_allocate_shadow_stack_space");
  assert(m_shadow_stack_allocate && 
         "__softboundmpx_allocate_shadow_stack_space NULL?");

  m_shadow_stack_deallocate = 
    module.getFunction("__softboundmpx_deallocate_shadow_stack_space");
  assert(m_shadow_stack_deallocate && 
         "__softboundmpx_deallocate_shadow_stack_space NULL?");


    m_shadow_stack_base_store = 
      module.getFunction("__softboundmpx_store_base_shadow_stack");
    assert(m_shadow_stack_base_store && 
           "__softboundmpx_store_base_shadow_stack NULL?");
    
    m_shadow_stack_bound_store = 
      module.getFunction("__softboundmpx_store_bound_shadow_stack");
    assert(m_shadow_stack_bound_store && 
           "__softboundmpx_store_bound_shadow_stack NULL?");
  
    
    m_shadow_stack_base_load = 
      module.getFunction("__softboundmpx_load_base_shadow_stack");
    assert(m_shadow_stack_base_load && 
           "__softboundmpx_load_base_shadow_stack NULL?");
    
    m_shadow_stack_bound_load = 
      module.getFunction("__softboundmpx_load_bound_shadow_stack");
    assert(m_shadow_stack_bound_load && 
           "__softboundmpx_load_bound_shadow_stack NULL?");


   
  m_load_base_bound_func = module.getFunction("__softboundmpx_metadata_load");
  assert(m_load_base_bound_func && "__softboundmpx_metadata_load null?");
  
  m_store_base_bound_func = module.getFunction("__softboundmpx_metadata_store");
  assert(m_store_base_bound_func && "__softboundmpx_metadata_store null?");

  m_call_dereference_func = 
    module.getFunction("__softboundmpx_spatial_call_dereference_check");
  assert(m_call_dereference_func && 
         "__softboundmpx_spatial_call_dereference_check function null??");

  m_memcopy_check = 
    module.getFunction("__softboundmpx_memcopy_check");
  assert(m_memcopy_check && 
         "__softboundmpx_memcopy_check function null?");

  m_memset_check = 
    module.getFunction("__softboundmpx_memset_check");
  assert(m_memcopy_check && 
         "__softboundmpx_memset_check function null?");


  m_void_ptr_type = PointerType::getUnqual(Type::getInt8Ty(module.getContext()));
    
  size_t inf_bound;

  if (m_is_64_bit) {
    m_key_type = Type::getInt64Ty(module.getContext());
  } else {
    m_key_type = Type::getInt32Ty(module.getContext());
  }

  if (m_is_64_bit) {
    inf_bound = (size_t) pow(2, 48);
  } else {
    inf_bound = (size_t) (2147483647);
  }
    
  ConstantInt* infinite_bound;

  if (m_is_64_bit) {
    infinite_bound = 
      ConstantInt::get(Type::getInt64Ty(module.getContext()), inf_bound, false);
  } else {
    infinite_bound = 
      ConstantInt::get(Type::getInt32Ty(module.getContext()), inf_bound, false);
  }
    
  m_infinite_bound_ptr = ConstantExpr::getIntToPtr(infinite_bound, 
                                                   m_void_ptr_type);
 
  PointerType* vptrty = dyn_cast<PointerType>(m_void_ptr_type);
  m_void_null_ptr = ConstantPointerNull::get(vptrty);
  
  PointerType* sizet_ptr_ty = NULL; 
  if (m_is_64_bit) {
    sizet_ptr_ty = 
      PointerType::getUnqual(Type::getInt64Ty(module.getContext()));
  } else{
    sizet_ptr_ty = 
      PointerType::getUnqual(Type::getInt32Ty(module.getContext()));
  }

  m_sizet_ptr_type = sizet_ptr_ty;

  m_sizet_null_ptr = ConstantPointerNull::get(sizet_ptr_ty);


  m_constantint32ty_one = 
    ConstantInt::get(Type::getInt32Ty(module.getContext()), 1);

  m_constantint32ty_zero = 
    ConstantInt::get(Type::getInt32Ty(module.getContext()), 0);

  m_constantint64ty_one = 
    ConstantInt::get(Type::getInt64Ty(module.getContext()), 1);

  m_constantint64ty_zero = 
    ConstantInt::get(Type::getInt64Ty(module.getContext()), 0);

  if (m_is_64_bit) {
    m_constantint_one = m_constantint64ty_one;
    m_constantint_zero = m_constantint64ty_zero;
  } else {
    m_constantint_one = m_constantint32ty_one;
    m_constantint_zero = m_constantint32ty_zero;
  }
}

// Method: hasAllocaInst()
//
// Description:
//
// This function checks whether internal function has an alloca
// instruction in the function. This function is useful to determine
// whether we need to allocate a key and a lock for the function or
// not.
// 
bool SoftBoundMPXPass::isAllocaPresent(Function* func){

  for(Function::iterator bb_begin = func->begin(), bb_end = func->end();
      bb_begin != bb_end; ++bb_begin) {
    
    for(BasicBlock::iterator i_begin = bb_begin->begin(), 
	  i_end = bb_begin->end(); i_begin != i_end; ++i_begin){
      
      Instruction* alloca_inst = dyn_cast<Instruction>(i_begin);
      
      if(isa<AllocaInst>(alloca_inst) && m_present_in_original.count(alloca_inst)){
	return true;
      }      
    }
  }
  return false;

}

//
// Method: transformMain()
//
// Description:
//
// This method renames the function "main" in the module as
// pseudo_main. The C-handler has the main function which calls
// pseudo_main. Actually transformation of the main takes places in
// two steps.  Step1: change the name to pseudo_main and Step2:
// Function renaming to append the function name with softboundmpx_
//
// Inputs:
// module: Input module with the function main
//
// Outputs:
//
// Changed module with any function named "main" is changed to
// "pseudo_main"
//
// Comments:
//
// This function is doing redundant work. We should probably use
// renameFunction to accomplish the task. The key difference is that
// transform renames it the function as either pseudo_main or
// softboundmpx_pseudo_main which is subsequently renamed to
// softboundmpx_pseudo_main in the first case by renameFunction
//

void SoftBoundMPXPass::transformMain(Module& module) {
    
  Function* main_func = module.getFunction("main");

  // 
  // If the program doesn't have main then don't do anything
  //
  if (!main_func) return;

  Type* ret_type = main_func->getReturnType();
  const FunctionType* fty = main_func->getFunctionType();
  std::vector<Type*> params;

  SmallVector<AttributeSet, 8> param_attrs_vec;
  const AttributeSet& pal = main_func->getAttributes();

  //
  // Get the attributes of the return value
  //

  if(pal.hasAttributes(AttributeSet::ReturnIndex))
    param_attrs_vec.push_back(AttributeSet::get(main_func->getContext(), pal.getRetAttributes()));

  // Get the attributes of the arguments 
  int arg_index = 1;
  for(Function::arg_iterator i = main_func->arg_begin(), 
        e = main_func->arg_end();
      i != e; ++i, arg_index++) {
    params.push_back(i->getType());

    AttributeSet attrs = pal.getParamAttributes(arg_index);

    if(attrs.hasAttributes(arg_index)){
      AttrBuilder B(attrs, arg_index);
      param_attrs_vec.push_back(AttributeSet::get(main_func->getContext(), params.size(), B));
    }
  }

  FunctionType* nfty = FunctionType::get(ret_type, params, fty->isVarArg());
  Function* new_func = NULL;

  // create the new function 
  new_func = Function::Create(nfty, main_func->getLinkage(), 
                              "softboundmpx_pseudo_main");

  // set the new function attributes 
  new_func->copyAttributesFrom(main_func);
  new_func->setAttributes(AttributeSet::get(main_func->getContext(), param_attrs_vec));
    
  main_func->getParent()->getFunctionList().insert(main_func, new_func);
  main_func->replaceAllUsesWith(new_func);

  // 
  // Splice the instructions from the old function into the new
  // function and set the arguments appropriately
  // 
  new_func->getBasicBlockList().splice(new_func->begin(), 
                                       main_func->getBasicBlockList());
  Function::arg_iterator arg_i2 = new_func->arg_begin();
  for(Function::arg_iterator arg_i = main_func->arg_begin(), 
        arg_e = main_func->arg_end(); 
      arg_i != arg_e; ++arg_i) {      
    arg_i->replaceAllUsesWith(arg_i2);
    arg_i2->takeName(arg_i);
    ++arg_i2;
    arg_index++;
  }  
  //
  // Remove the old function from the module
  //
  main_func->eraseFromParent();
}

//
// Method: isFuncDefSoftBound
//
// Description: 
//
// This function checks if the input function name is a
// SoftBound/CETS defined function
//

bool SoftBoundMPXPass::isFuncDefSoftBound(const std::string &str) {
  if (m_func_def_softbound.getNumItems() == 0) {

    m_func_wrappers_available["system"] = true;
    m_func_wrappers_available["setreuid"] = true;
    m_func_wrappers_available["mkstemp"] = true;
    m_func_wrappers_available["getuid"] = true;
    m_func_wrappers_available["getrlimit"] = true;
    m_func_wrappers_available["setrlimit"] = true;
    m_func_wrappers_available["fread"] = true;
    m_func_wrappers_available["umask"] = true;
    m_func_wrappers_available["mkdir"] = true;
    m_func_wrappers_available["chroot"] = true;
    m_func_wrappers_available["rmdir"] = true;
    m_func_wrappers_available["stat"] = true;
    m_func_wrappers_available["fputc"] = true;
    m_func_wrappers_available["fileno"] = true;
    m_func_wrappers_available["fgetc"] = true;
    m_func_wrappers_available["strncmp"] = true;
    m_func_wrappers_available["log"] = true;
    m_func_wrappers_available["fwrite"] = true;
    m_func_wrappers_available["atof"] = true;
    m_func_wrappers_available["feof"] = true;
    m_func_wrappers_available["remove"] = true;
    m_func_wrappers_available["acos"] = true;
    m_func_wrappers_available["atan2"] = true;
    m_func_wrappers_available["sqrtf"] = true;
    m_func_wrappers_available["expf"] = true;
    m_func_wrappers_available["exp2"] = true;
    m_func_wrappers_available["floorf"] = true;
    m_func_wrappers_available["ceil"] = true;
    m_func_wrappers_available["ceilf"] = true;
    m_func_wrappers_available["floor"] = true;
    m_func_wrappers_available["sqrt"] = true;
    m_func_wrappers_available["fabs"] = true;
    m_func_wrappers_available["abs"] = true;
    m_func_wrappers_available["srand"] = true;
    m_func_wrappers_available["srand48"] = true;
    m_func_wrappers_available["pow"] = true;
    m_func_wrappers_available["fabsf"] = true;
    m_func_wrappers_available["tan"] = true;
    m_func_wrappers_available["tanf"] = true;
    m_func_wrappers_available["tanl"] = true;
    m_func_wrappers_available["log10"] = true;
    m_func_wrappers_available["sin"] = true;
    m_func_wrappers_available["sinf"] = true;
    m_func_wrappers_available["sinl"] = true;
    m_func_wrappers_available["cos"] = true;
    m_func_wrappers_available["cosf"] = true;
    m_func_wrappers_available["cosl"] = true;
    m_func_wrappers_available["exp"] = true;
    m_func_wrappers_available["ldexp"] = true;
    m_func_wrappers_available["tmpfile"] = true;
    m_func_wrappers_available["ferror"] = true;
    m_func_wrappers_available["ftell"] = true;
    m_func_wrappers_available["fstat"] = true;
    m_func_wrappers_available["fflush"] = true;
    m_func_wrappers_available["fputs"] = true;
    m_func_wrappers_available["fopen"] = true;
    m_func_wrappers_available["fdopen"] = true;
    m_func_wrappers_available["fseek"] = true;
    m_func_wrappers_available["ftruncate"] = true;
    m_func_wrappers_available["popen"] = true;
    m_func_wrappers_available["fclose"] = true;
    m_func_wrappers_available["pclose"] = true;
    m_func_wrappers_available["rewind"] = true;
    m_func_wrappers_available["readdir"] = true;
    m_func_wrappers_available["opendir"] = true;
    m_func_wrappers_available["closedir"] = true;
    m_func_wrappers_available["rename"] = true;
    m_func_wrappers_available["sleep"] = true;
    m_func_wrappers_available["getcwd"] = true;
    m_func_wrappers_available["chown"] = true;
    m_func_wrappers_available["isatty"] = true;
    m_func_wrappers_available["chdir"] = true;
    m_func_wrappers_available["strcmp"] = true;
    m_func_wrappers_available["strcasecmp"] = true;
    m_func_wrappers_available["strncasecmp"] = true;
    m_func_wrappers_available["strlen"] = true;
    m_func_wrappers_available["strpbrk"] = true;
    m_func_wrappers_available["gets"] = true;
    m_func_wrappers_available["fgets"] = true;
    m_func_wrappers_available["perror"] = true;
    m_func_wrappers_available["strspn"] = true;
    m_func_wrappers_available["strcspn"] = true;
    m_func_wrappers_available["memcmp"] = true;
    m_func_wrappers_available["memchr"] = true;
    m_func_wrappers_available["rindex"] = true;
    m_func_wrappers_available["strtoul"] = true;
    m_func_wrappers_available["strtod"] = true;
    m_func_wrappers_available["strtol"] = true;
    m_func_wrappers_available["strchr"] = true;
    m_func_wrappers_available["strrchr"] = true;
    m_func_wrappers_available["strcpy"] = true;
    m_func_wrappers_available["abort"] = true;
    m_func_wrappers_available["rand"] = true;
    m_func_wrappers_available["atoi"] = true;
    m_func_wrappers_available["puts"] = true;
    m_func_wrappers_available["exit"] = true;
    m_func_wrappers_available["strtok"] = true;
    m_func_wrappers_available["strdup"] = true;
    m_func_wrappers_available["strcat"] = true;
    m_func_wrappers_available["strncat"] = true;
    m_func_wrappers_available["strncpy"] = true;
    m_func_wrappers_available["strstr"] = true;
    m_func_wrappers_available["signal"] = true;
    m_func_wrappers_available["clock"] = true;
    m_func_wrappers_available["atol"] = true;
    m_func_wrappers_available["realloc"] = true;
    m_func_wrappers_available["calloc"] = true;
    m_func_wrappers_available["malloc"] = true;
    m_func_wrappers_available["mmap"] = true;

    m_func_wrappers_available["putchar"] = true;
    m_func_wrappers_available["times"] = true;
    m_func_wrappers_available["strftime"] = true;
    m_func_wrappers_available["localtime"] = true;
    m_func_wrappers_available["time"] = true;
    m_func_wrappers_available["drand48"] = true;
    m_func_wrappers_available["free"] = true;
    m_func_wrappers_available["lrand48"] = true;
    m_func_wrappers_available["ctime"] = true;
    m_func_wrappers_available["difftime"] = true;
    m_func_wrappers_available["toupper"] = true;
    m_func_wrappers_available["tolower"] = true;
    m_func_wrappers_available["setbuf"] = true;
    m_func_wrappers_available["getenv"] = true;
    m_func_wrappers_available["atexit"] = true;
    m_func_wrappers_available["strerror"] = true;
    m_func_wrappers_available["unlink"] = true;
    m_func_wrappers_available["close"] = true;
    m_func_wrappers_available["open"] = true;
    m_func_wrappers_available["read"] = true;
    m_func_wrappers_available["write"] = true;
    m_func_wrappers_available["lseek"] = true;
    m_func_wrappers_available["gettimeofday"] = true;
    m_func_wrappers_available["select"] = true;
    m_func_wrappers_available["__errno_location"] = true;
    m_func_wrappers_available["__ctype_b_loc"] = true;
    m_func_wrappers_available["__ctype_toupper_loc"] = true;
    m_func_wrappers_available["__ctype_tolower_loc"] = true;

    
    m_func_def_softbound["__softboundmpx_intermediate"]= true;
    m_func_def_softbound["__softboundmpx_dummy"] = true;
    m_func_def_softbound["__softboundmpx_print_metadata"] = true;
    m_func_def_softbound["__softboundmpx_introspect_metadata"] = true;
    m_func_def_softbound["__softboundmpx_copy_metadata"] = true;
    m_func_def_softbound["__softboundmpx_allocate_shadow_stack_space"] = true;

    m_func_def_softbound["__softboundmpx_load_base_shadow_stack"] = true;
    m_func_def_softbound["__softboundmpx_load_bound_shadow_stack"] = true;

    m_func_def_softbound["__softboundmpx_store_base_shadow_stack"] = true;      
    m_func_def_softbound["__softboundmpx_store_bound_shadow_stack"] = true;      

    m_func_def_softbound["__softboundmpx_deallocate_shadow_stack_space"] = true;

    m_func_def_softbound["__softboundmpx_trie_allocate"] = true;
    m_func_def_softbound["__softboundmpx_shrink_bounds"] = true;
    m_func_def_softbound["__softboundmpx_memcopy_check"] = true;

    m_func_def_softbound["__softboundmpx_spatial_load_dereference_check"] = true;

    m_func_def_softbound["__softboundmpx_spatial_store_dereference_check"] = true;
    m_func_def_softbound["__softboundmpx_spatial_call_dereference_check"] = true;

    m_func_def_softbound["__softboundmpx_allocation_secondary_trie_allocate"] = true;
    m_func_def_softbound["__softboundmpx_allocation_secondary_trie_allocate_range"] = true;
    m_func_def_softbound["__softboundmpx_allocate_lock_location"] = true;

    m_func_def_softbound["__softboundmpx_metadata_load"] = true;
    m_func_def_softbound["__softboundmpx_metadata_store"] = true;
    m_func_def_softbound["__hashProbeAddrOfPtr"] = true;
    m_func_def_softbound["__memcopyCheck"] = true;
    m_func_def_softbound["__memcopyCheck_i64"] = true;

    m_func_def_softbound["__softboundmpx_global_init"] = true;      
    m_func_def_softbound["__softboundmpx_init"] = true;      
    m_func_def_softbound["__softboundmpx_abort"] = true;      
    m_func_def_softbound["__softboundmpx_printf"] = true;
    
    m_func_def_softbound["__softboundmpx_stub"] = true;
    m_func_def_softbound["safe_mmap"] = true;
    m_func_def_softbound["safe_calloc"] = true;
    m_func_def_softbound["safe_malloc"] = true;
    m_func_def_softbound["safe_free"] = true;

    m_func_def_softbound["__assert_fail"] = true;
    m_func_def_softbound["assert"] = true;
    m_func_def_softbound["__strspn_c2"] = true;
    m_func_def_softbound["__strcspn_c2"] = true;
    m_func_def_softbound["__strtol_internal"] = true;
    m_func_def_softbound["__stroul_internal"] = true;
    m_func_def_softbound["ioctl"] = true;
    m_func_def_softbound["error"] = true;
    m_func_def_softbound["__strtod_internal"] = true;
    m_func_def_softbound["__strtoul_internal"] = true;
    
    
    m_func_def_softbound["fflush_unlocked"] = true;
    m_func_def_softbound["full_write"] = true;
    m_func_def_softbound["safe_read"] = true;
    m_func_def_softbound["_IO_getc"] = true;
    m_func_def_softbound["_IO_putc"] = true;
    m_func_def_softbound["__xstat"] = true;

    m_func_def_softbound["select"] = true;
    m_func_def_softbound["_setjmp"] = true;
    m_func_def_softbound["longjmp"] = true;
    m_func_def_softbound["fork"] = true;
    m_func_def_softbound["pipe"] = true;
    m_func_def_softbound["dup2"] = true;
    m_func_def_softbound["execv"] = true;
    m_func_def_softbound["compare_pic_by_pic_num_desc"] = true;
     
    m_func_def_softbound["wprintf"] = true;
    m_func_def_softbound["vfprintf"] = true;
    m_func_def_softbound["vsprintf"] = true;
    m_func_def_softbound["fprintf"] = true;
    m_func_def_softbound["printf"] = true;
    m_func_def_softbound["sprintf"] = true;
    m_func_def_softbound["snprintf"] = true;

    m_func_def_softbound["scanf"] = true;
    m_func_def_softbound["fscanf"] = true;
    m_func_def_softbound["sscanf"] = true;   

    m_func_def_softbound["asprintf"] = true;
    m_func_def_softbound["vasprintf"] = true;
    m_func_def_softbound["__fpending"] = true;
    m_func_def_softbound["fcntl"] = true;

    m_func_def_softbound["vsnprintf"] = true;
    m_func_def_softbound["fwrite_unlocked"] = true;
    m_func_def_softbound["__overflow"] = true;
    m_func_def_softbound["__uflow"] = true;
    m_func_def_softbound["execlp"] = true;
    m_func_def_softbound["execl"] = true;
    m_func_def_softbound["waitpid"] = true;
    m_func_def_softbound["dup"] = true;
    m_func_def_softbound["setuid"] = true;
    
    m_func_def_softbound["_exit"] = true;
    m_func_def_softbound["funlockfile"] = true;
    m_func_def_softbound["flockfile"] = true;

    m_func_def_softbound["__option_is_short"] = true;
    

  }

  // Is the function name in the above list?
  if (m_func_def_softbound.count(str) > 0) {
    return true;
  }

  // FIXME: handling new intrinsics which have isoc99 in their name
  if (str.find("isoc99") != std::string::npos){
    return true;
  }

  // If the function is an llvm intrinsic, don't transform it
  if (str.find("llvm.") == 0) {
    return true;
  }

  return false;
}

// 
// Method: identifyFuncToTrans
//
// Description: This function traverses the module and identifies the
// functions that need to be transformed by SoftBound/CETS
//

void SoftBoundMPXPass::identifyFuncToTrans(Module& module) {
    
  for (Module::iterator fb_it = module.begin(), fe_it = module.end(); 
      fb_it != fe_it; ++fb_it) {

    Function* func = dyn_cast<Function>(fb_it);
    assert(func && " Not a function");

    // Check if the function is defined in the module
    if (!func->isDeclaration()) {
      if (isFuncDefSoftBound(func->getName())) 
        continue;
      
      m_func_softboundcets_transform[func->getName()] = true;
      if (hasPtrArgRetType(func)) {
        m_func_to_transform[func->getName()] = true;
      }
    }
  }
}


// 
// Method: castToVoidPtr()
//
// Description: 
// 
// This function introduces a bitcast instruction in the IR when an
// input operand that is a pointer type is not of type i8*. This is
// required as all the SoftBound/CETS handlers take i8*s
//

Value* 
SoftBoundMPXPass:: castToVoidPtr(Value* operand, Instruction* insert_at) {

  Value* cast_bitcast = operand;
  if (operand->getType() != m_void_ptr_type) {
    cast_bitcast = new BitCastInst(operand, m_void_ptr_type,
                                   "bitcast",
                                   insert_at);
  }
  return cast_bitcast;
}


//
// Method: hasPtrArgRetType()
//
// Description:
//
// This function checks if the function has either pointer arguments
// or returns a pointer value. This function is used to determine
// whether shadow stack loads/stores need to be introduced for
// metadata propagation.
//

bool SoftBoundMPXPass::hasPtrArgRetType(Function* func) {
   
  const Type* ret_type = func->getReturnType();
  if (isa<PointerType>(ret_type))
    return true;

  for (Function::arg_iterator i = func->arg_begin(), e = func->arg_end(); 
      i != e; ++i) {
      
    if (isa<PointerType>(i->getType()))
      return true;
  }
  return false;
}

//
// Method: addStoreBaseBoundFunc
//
// Description:
//
// This function inserts metadata stores into the bitcode whenever a
// pointer is being stored to memory.
//
// Inputs:
//
// pointer_dest: address where the pointer being stored
//
// pointer_base, pointer_bound, pointer_key, pointer_lock: metadata
// associated with the pointer being stored
//
// pointer : pointer being stored to memory
//
// size_of_type: size of the access
//
// insert_at: the insertion point in the bitcode before which the
// metadata store is introduced.
//
void SoftBoundMPXPass::addStoreBaseBoundFunc(Value* pointer_dest, 
                                              Value* pointer_base, 
                                              Value* pointer_bound, 
                                              Value* pointer,
                                              Value* size_of_type, 
                                              Instruction* insert_at) {

  Value* pointer_base_cast = NULL;
  Value* pointer_bound_cast = NULL;

  
  Value* pointer_dest_cast = castToVoidPtr(pointer_dest, insert_at);

  pointer_base_cast = castToVoidPtr(pointer_base, insert_at);
  pointer_bound_cast = castToVoidPtr(pointer_bound, insert_at);
    
  SmallVector<Value*, 8> args;

  args.push_back(pointer_dest_cast);

  args.push_back(pointer_base_cast);
  args.push_back(pointer_bound_cast);
 
  Value* pointer_cast = castToVoidPtr(pointer, insert_at);  
  args.push_back(pointer_cast);

  CallInst::Create(m_store_base_bound_func, args, "", insert_at);
}

//
// The metadata propagation for PHINode occurs in two passes. In the
// first pass, SoftBound/CETS transformation just creates the metadata
// PHINodes and records it in the maps maintained by
// SoftBound/CETS. In the second pass, it populates the incoming
// values of the PHINodes. This two pass approach ensures that every
// incoming value of the original PHINode will have metadata in the
// SoftBound/CETS maps
// 

//
// Method: handlePHIPass1()
//
// Description:
//
// This function creates a PHINode for the metadata in the bitcode for
// pointer PHINodes. It is important to note that this function just
// creates the PHINode and does not populate the incoming values of
// the PHINode, which is handled by the handlePHIPass2.
//

void SoftBoundMPXPass::handlePHIPass1(PHINode* phi_node) {

  // Not a Pointer PHINode, then just return
  if (!isa<PointerType>(phi_node->getType()))
    return;

  unsigned num_incoming_values = phi_node->getNumIncomingValues();

  PHINode* base_phi_node = PHINode::Create(m_void_ptr_type,
					   num_incoming_values,
					   "phi.base",
					   phi_node);
  
  PHINode* bound_phi_node = PHINode::Create(m_void_ptr_type, 
					    num_incoming_values,
					    "phi.bound", 
					    phi_node);
  
  Value* base_phi_node_value = base_phi_node;
  Value* bound_phi_node_value = bound_phi_node;
  
  associateBaseBound(phi_node, base_phi_node_value, bound_phi_node_value);
}



//
// Method: handlePHIPass2()
//
// Description: This pass fills the incoming values for the metadata
// PHINodes inserted in the first pass. There are four cases that
// needs to be handled for each incoming value.  First, if the
// incoming value is a ConstantPointerNull, then base, bound, key,
// lock will be default values.  Second, the incoming value can be an
// undef which results in default metadata values.  Third, Global
// variables need to get the same base and bound for each
// occurence. So we maintain a map which maps the base and boundfor
// each global variable in the incoming value.  Fourth, by default it
// retrieves the metadata from the SoftBound/CETS maps.

// Check if we need separate global variable and constant expression
// cases.

void SoftBoundMPXPass::handlePHIPass2(PHINode* phi_node) {

  // Work to be done only for pointer PHINodes.
  if (!isa<PointerType>(phi_node->getType())) 
    return;

  PHINode* base_phi_node = NULL;
  PHINode* bound_phi_node  = NULL;


  // Obtain the metada PHINodes 

    base_phi_node = dyn_cast<PHINode>(getAssociatedBase(phi_node));
    bound_phi_node = dyn_cast<PHINode>(getAssociatedBound(phi_node));

  
  std::map<Value*, Value*> globals_base;
  std::map<Value*, Value*> globals_bound;
 
  unsigned num_incoming_values = phi_node->getNumIncomingValues();
  for (unsigned m = 0; m < num_incoming_values; m++) {

    Value* incoming_value = phi_node->getIncomingValue(m);
    BasicBlock* bb_incoming = phi_node->getIncomingBlock(m);

    if (isa<ConstantPointerNull>(incoming_value)) {

        base_phi_node->addIncoming(m_void_null_ptr, bb_incoming);
        bound_phi_node->addIncoming(m_void_null_ptr, bb_incoming);

      continue;
    } // ConstantPointerNull ends
   
    // The incoming vlaue can be a UndefValue
    if (isa<UndefValue>(incoming_value)) {        

        base_phi_node->addIncoming(m_void_null_ptr, bb_incoming);
        bound_phi_node->addIncoming(m_void_null_ptr, bb_incoming);

      continue;
    } // UndefValue ends
      
    Value* incoming_value_base = NULL;
    Value* incoming_value_bound = NULL;
    
    // handle global variables      
    GlobalVariable* gv = dyn_cast<GlobalVariable>(incoming_value);
    if (gv) {
        if (!globals_base.count(gv)) {
          Value* tmp_base = NULL;
          Value* tmp_bound = NULL;
          getGlobalVariableBaseBound(incoming_value, tmp_base, tmp_bound);
          assert(tmp_base && "base of a global variable null?");
          assert(tmp_bound && "bound of a global variable null?");
          
          Function * PHI_func = phi_node->getParent()->getParent();
          Instruction* PHI_func_entry = PHI_func->begin()->begin();
          
          incoming_value_base = castToVoidPtr(tmp_base, PHI_func_entry);                                               
          incoming_value_bound = castToVoidPtr(tmp_bound, PHI_func_entry);
            
          globals_base[incoming_value] = incoming_value_base;
          globals_bound[incoming_value] = incoming_value_bound;       
        } else {
          incoming_value_base = globals_base[incoming_value];
          incoming_value_bound = globals_bound[incoming_value];          
        }

      
    } // global variable ends
      
    // handle constant expressions 
    Constant* given_constant = dyn_cast<Constant>(incoming_value);
    if (given_constant) {
        if (!globals_base.count(incoming_value)) {
          Value* tmp_base = NULL;
          Value* tmp_bound = NULL;
          getConstantExprBaseBound(given_constant, tmp_base, tmp_bound);
          assert(tmp_base && tmp_bound  &&
                 "[handlePHIPass2] tmp_base tmp_bound, null?");
          
          Function* PHI_func = phi_node->getParent()->getParent();
          Instruction* PHI_func_entry = PHI_func->begin()->begin();

          incoming_value_base = castToVoidPtr(tmp_base, PHI_func_entry);
          incoming_value_bound = castToVoidPtr(tmp_bound, PHI_func_entry);
          
          globals_base[incoming_value] = incoming_value_base;
          globals_bound[incoming_value] = incoming_value_bound;        
        }
        else{
          incoming_value_base = globals_base[incoming_value];
          incoming_value_bound = globals_bound[incoming_value];          
        }
    }
    
    // handle values having map based pointer base and bounds 
    if(checkBaseBoundMetadataPresent(incoming_value)){
      incoming_value_base = getAssociatedBase(incoming_value);
      incoming_value_bound = getAssociatedBound(incoming_value);
    }
    

    assert(incoming_value_base &&
	   "[handlePHIPass2] incoming_value doesn't have base?");
    assert(incoming_value_bound && 
	   "[handlePHIPass2] incoming_value doesn't have bound?");
      
    base_phi_node->addIncoming(incoming_value_base, bb_incoming);
    bound_phi_node->addIncoming(incoming_value_bound, bb_incoming);
    
  } // Iterating over incoming values ends 

  assert(base_phi_node && "[handlePHIPass2] base_phi_node null?");
  assert(bound_phi_node && "[handlePHIPass2] bound_phi_node null?");
  
  unsigned n_values = phi_node->getNumIncomingValues();

  unsigned n_base_values = base_phi_node->getNumIncomingValues();
  unsigned n_bound_values = bound_phi_node->getNumIncomingValues();    
  assert((n_values == n_base_values)  && 
	 "[handlePHIPass2] number of values different for base");
  assert((n_values == n_bound_values) && 
	 "[handlePHIPass2] number of values different for bound");
  
}

//
// Method: propagateMetadata
//
// Descripton;
//
// This function propagates the metadata from the source to the
// destination in the map for pointer arithmetic operations~(gep) and
// bitcasts. This is the place where we need to shrink bounds.
//

void 
SoftBoundMPXPass:: propagateMetadata(Value* pointer_operand, 
                                      Instruction* inst, 
                                      int instruction_type){

  // Need to just propagate the base and bound here if I am not
  // shrinking bounds
  if(checkBaseBoundMetadataPresent(inst)){
    // Metadata added to the map in the first pass
    return;
  }


  if(isa<ConstantPointerNull>(pointer_operand)) {
      associateBaseBound(inst, m_void_null_ptr, m_void_null_ptr);
      return;
  }


  if (checkBaseBoundMetadataPresent(pointer_operand)) {
    Value* tmp_base = getAssociatedBase(pointer_operand); 
    Value* tmp_bound = getAssociatedBound(pointer_operand);       
    associateBaseBound(inst, tmp_base, tmp_bound);
  } else{
    if(isa<Constant>(pointer_operand)) {
      
      Value* tmp_base = NULL;
      Value* tmp_bound = NULL;
      Constant* given_constant = dyn_cast<Constant>(pointer_operand);
      getConstantExprBaseBound(given_constant, tmp_base, tmp_bound);
      assert(tmp_base && "gep with cexpr and base null?");
      assert(tmp_bound && "gep with cexpr and bound null?");
      tmp_base = castToVoidPtr(tmp_base, inst);
      tmp_bound = castToVoidPtr(tmp_bound, inst);        
      
      associateBaseBound(inst, tmp_base, tmp_bound);
    } // Constant case ends here
      // Could be in the first pass, do nothing here
  }
}

//
// Method: handleBitCast
//
// Description: Propagate metadata from source to destination with
// pointer bitcast operations.

void SoftBoundMPXPass::handleBitCast(BitCastInst* bitcast_inst) {

  Value* pointer_operand = bitcast_inst->getOperand(0);  
  propagateMetadata(pointer_operand, bitcast_inst, SBCETS_BITCAST);
}

//
// Method: getGlobalVariableBaseBound

// Description: This function returns the base and bound for the
// global variables in the input reference arguments. This function
// may now be obsolete. We should try to use getConstantExprBaseBound
// instead in all places.
void 
SoftBoundMPXPass::getGlobalVariableBaseBound(Value* operand, 
                                              Value* & operand_base, 
                                              Value* & operand_bound){

  GlobalVariable* gv = dyn_cast<GlobalVariable>(operand);
  Module* module = gv->getParent();
  assert(gv && "[getGlobalVariableBaseBound] not a global variable?");
    
  std::vector<Constant*> indices_base;
  Constant* index_base = 
    ConstantInt::get(Type::getInt32Ty(module->getContext()), 0);
  indices_base.push_back(index_base);

  Constant* base_exp = ConstantExpr::getGetElementPtr(gv, indices_base);
        
  std::vector<Constant*> indices_bound;
  Constant* index_bound = 
    ConstantInt::get(Type::getInt32Ty(module->getContext()), 1);
  indices_bound.push_back(index_bound);
    
  Constant* bound_exp = ConstantExpr::getGetElementPtr(gv, indices_bound);
    
  operand_base = base_exp;
  operand_bound = bound_exp;    
}

//
// Method: introduceShadowStackAllocation
//
// Description: For every function call that has a pointer argument or
// a return value, shadow stack is used to propagate metadata. This
// function inserts the shadow stack allocation C-handler that
// reserves space in the shadow stack by reserving the requiste amount
// of space based on the input passed to it(number of pointer
// arguments/return).


void SoftBoundMPXPass:: introduceShadowStackAllocation(CallInst* call_inst){
    
  // Count the number of pointer arguments and whether a pointer return     
  int pointer_args_return = getNumPointerArgsAndReturn(call_inst);
  if(pointer_args_return == 0)
    return;
  Value* total_ptr_args;    
  total_ptr_args = 
    ConstantInt::get(Type::getInt32Ty(call_inst->getType()->getContext()), 
                     pointer_args_return, false);

  SmallVector<Value*, 8> args;
  args.push_back(total_ptr_args);
  CallInst::Create(m_shadow_stack_allocate, args, "", call_inst);
}

//
// Method: introduceShadowStackStores
//
// Description: This function inserts a call to the shadow stack store
// C-handler that stores the metadata, before the function call in the
// bitcode for pointer arguments.

void 
SoftBoundMPXPass::introduceShadowStackStores(Value* ptr_value, 
                                              Instruction* insert_at, 
                                              int arg_no){
  if(!isa<PointerType>(ptr_value->getType()))
    return;
  

  Value* argno_value;    
  argno_value = 
    ConstantInt::get(Type::getInt32Ty(ptr_value->getType()->getContext()), 
                     arg_no, false);


    Value* ptr_base = getAssociatedBase(ptr_value);
    Value* ptr_bound = getAssociatedBound(ptr_value);
    
    Value* ptr_base_cast = castToVoidPtr(ptr_base, insert_at);
    Value* ptr_bound_cast = castToVoidPtr(ptr_bound, insert_at);

    SmallVector<Value*, 8> args;
    args.push_back(ptr_base_cast);
    args.push_back(argno_value);
    CallInst::Create(m_shadow_stack_base_store, args, "", insert_at);
    
    args.clear();
    args.push_back(ptr_bound_cast);
    args.push_back(argno_value);
    CallInst::Create(m_shadow_stack_bound_store, args, "", insert_at);    

}

//
// Method: introduceShadowStackDeallocation
//
// Description: This function inserts a call to the C-handler that
// deallocates the shadow stack space on function exit.
  

void 
SoftBoundMPXPass:: introduceShadowStackDeallocation(CallInst* call_inst, 
                                                     Instruction* insert_at){

  int pointer_args_return = getNumPointerArgsAndReturn(call_inst);
  if(pointer_args_return == 0)
    return;
  SmallVector<Value*, 8> args;    
  CallInst::Create(m_shadow_stack_deallocate, args, "", insert_at);
}

//
// Method: getNumPointerArgsAndReturn
//
// Description: Returns the number of pointer arguments and return.
//
int SoftBoundMPXPass:: getNumPointerArgsAndReturn(CallInst* call_inst){

  int total_pointer_count = 0;
  CallSite cs(call_inst);
  for(unsigned i = 0; i < cs.arg_size(); i++){
    Value* arg_value = cs.getArgument(i);
    if(isa<PointerType>(arg_value->getType())){
      total_pointer_count++;
    }
  }

  if (total_pointer_count != 0) {
    // Reserve one for the return address if it has atleast one
    // pointer argument 
    total_pointer_count++;
  } else{
    // Increment the pointer arg return if the call instruction
    // returns a pointer
    if(isa<PointerType>(call_inst->getType())){
      total_pointer_count++;
    }
  }
  return total_pointer_count;
}

// 
// Method: introduceShadowStackLoads
//
// Description: This function introduces calls to the C-handlers that
// performs the loads from the shadow stack to retrieve the metadata.
// This function also associates the loaded metadata with the pointer
// arguments in the SoftBound/CETS maps.

void 
SoftBoundMPXPass::introduceShadowStackLoads(Value* ptr_value, 
                                             Instruction* insert_at, 
                                             int arg_no){
    
  if (!isa<PointerType>(ptr_value->getType()))
    return;
      
  Value* argno_value;    
  argno_value = 
    ConstantInt::get(Type::getInt32Ty(ptr_value->getType()->getContext()), 
                     arg_no, false);
    
  SmallVector<Value*, 8> args;

    args.clear();
    args.push_back(argno_value);
    Value* base = CallInst::Create(m_shadow_stack_base_load, args, "", 
                                   insert_at);    
    args.clear();
    args.push_back(argno_value);
    Value* bound = CallInst::Create(m_shadow_stack_bound_load, args, "", 
                                    insert_at);
    associateBaseBound(ptr_value, base, bound);

}

//
// Method: dissociateBaseBound
//
// Description: This function removes the base/bound metadata
// associated with the pointer operand in the SoftBound/CETS maps.

void SoftBoundMPXPass::dissociateBaseBound(Value* pointer_operand){

  if(m_pointer_base.count(pointer_operand)){
    m_pointer_base.erase(pointer_operand);
  }
  if(m_pointer_bound.count(pointer_operand)){
    m_pointer_bound.erase(pointer_operand);
  }
  assert((m_pointer_base.count(pointer_operand) == 0) && 
         "dissociating base failed\n");
  assert((m_pointer_bound.count(pointer_operand) == 0) && 
         "dissociating bound failed");
}


//
// Method: associateBaseBound
//
// Description: This function associates the base bound with the
// pointer operand in the SoftBound/CETS maps.


void SoftBoundMPXPass::associateBaseBound(Value* pointer_operand, 
                                           Value* pointer_base, 
                                           Value* pointer_bound){

  if(m_pointer_base.count(pointer_operand)){
    dissociateBaseBound(pointer_operand);
  }

  if(pointer_base->getType() != m_void_ptr_type){
    assert(0 && "base does not have a void pointer type ");
  }
  m_pointer_base[pointer_operand] = pointer_base;
  if(m_pointer_bound.count(pointer_operand)){
    assert(0 && "bound map already has an entry in the map");
  }
  if(pointer_bound->getType() != m_void_ptr_type) {
    assert(0 && "bound does not have a void pointer type ");
  }
  m_pointer_bound[pointer_operand] = pointer_bound;

}
//
// Method: handleSelect
//
// This function propagates the metadata with Select IR instruction.
// Select  instruction is also handled in two passes.

void SoftBoundMPXPass::handleSelect(SelectInst* select_ins, int pass) {

  if (!isa<PointerType>(select_ins->getType())) 
    return;
    
  Value* condition = select_ins->getOperand(0);
  Value* operand_base[2];
  Value* operand_bound[2];    

  for(unsigned m = 0; m < 2; m++) {
    Value* operand = select_ins->getOperand(m+1);
    

      operand_base[m] = NULL;
      operand_bound[m] = NULL;
      if (checkBaseBoundMetadataPresent(operand)) {      
        operand_base[m] = getAssociatedBase(operand);
        operand_bound[m] = getAssociatedBound(operand);
      }
      
      if (isa<ConstantPointerNull>(operand) && 
          !checkBaseBoundMetadataPresent(operand)) {            
        operand_base[m] = m_void_null_ptr;
        operand_bound[m] = m_void_null_ptr;
      }        
        
      Constant* given_constant = dyn_cast<Constant>(operand);
      if(given_constant) {
        getConstantExprBaseBound(given_constant, 
                                 operand_base[m], 
                                 operand_bound[m]);     
      }    
      assert(operand_base[m] != NULL && 
             "operand doesn't have base with select?");
      assert(operand_bound[m] != NULL && 
             "operand doesn't have bound with select?");
      
      // Introduce a bit cast if the types don't match 
      if (operand_base[m]->getType() != m_void_ptr_type) {          
        operand_base[m] = new BitCastInst(operand_base[m], m_void_ptr_type,
                                          "select.base", select_ins);          
      }
      
      if (operand_bound[m]->getType() != m_void_ptr_type) {
        operand_bound[m] = new BitCastInst(operand_bound[m], m_void_ptr_type,
                                           "select_bound", select_ins);
      }

      
  } // for loop ends
    

      
    SelectInst* select_base = SelectInst::Create(condition, 
                                                 operand_base[0], 
                                                 operand_base[1], 
                                                 "select.base",
                                                 select_ins);
    
    SelectInst* select_bound = SelectInst::Create(condition, 
                                                  operand_bound[0], 
                                                  operand_bound[1], 
                                                  "select.bound",
                                                  select_ins);
    associateBaseBound(select_ins, select_base, select_bound);

}

//
// Method: checkBaseBoundMetadataPresent()
//
// Description:
// Checks if the metadata is present in the SoftBound/CETS maps.

bool 
SoftBoundMPXPass::checkBaseBoundMetadataPresent(Value* pointer_operand){

  if(m_pointer_base.count(pointer_operand) && 
     m_pointer_bound.count(pointer_operand)){
      return true;
  }
  return false;
}

//
// Method: handleReturnInst
//
// Description: 
// This function inserts C-handler calls to store
// metadata for return values in the shadow stack.

void SoftBoundMPXPass:: handleReturnInst(ReturnInst* ret){

  Value* pointer = ret->getReturnValue();
  if(pointer == NULL){
    return;
  }
  if(isa<PointerType>(pointer->getType())){
    introduceShadowStackStores(pointer, ret, 0);
  }
}

//
// Method: handleGlobalSequentialTypeInitializer
//
// Description: This performs the initialization of the metadata for
// the pointers in the global segments that are initialized with
// non-zero values.
//
// Comments: This function requires review and rewrite

void 
SoftBoundMPXPass::handleGlobalSequentialTypeInitializer(Module& module, 
                                                         GlobalVariable* gv) {

  // Sequential type can be an array type, a pointer type 
  const SequentialType* init_seq_type = 
    dyn_cast<SequentialType>((gv->getInitializer())->getType());
  assert(init_seq_type && 
         "[handleGlobalSequentialTypeInitializer] initializer  null?");

  Instruction* init_function_terminator = getGlobalInitInstruction(module);
  if(gv->getInitializer()->isNullValue())
    return;
    
  if(isa<ArrayType>(init_seq_type)){      
    const ArrayType* init_array_type = dyn_cast<ArrayType>(init_seq_type);     
    if(isa<StructType>(init_array_type->getElementType())){
      // It is an array of structures

      // Check whether the structure has a pointer, if it has a
      // pointer then, we need to store the base and bound of the
      // pointer into the metadata space. However, if the structure
      // does not have any pointer, we can make a quick exit in
      // processing this global
      //

      bool struct_has_pointers = false;
      StructType* init_struct_type = 
        dyn_cast<StructType>(init_array_type->getElementType());
      CompositeType* struct_comp_type = 
        dyn_cast<CompositeType>(init_struct_type);
      
      assert(struct_comp_type && "struct composite type null?");
      assert(init_struct_type && 
             "Array of structures and struct type null?");        
      unsigned num_struct_elements = init_struct_type->getNumElements();        
      for(unsigned i = 0; i < num_struct_elements; i++) {
        Type* element_type = struct_comp_type->getTypeAtIndex(i);
        if(isa<PointerType>(element_type)){
          struct_has_pointers = true;
        }
      }
      if(!struct_has_pointers)
        return;

      // Here implies, global variable is an array of structures with
      // a pointer. Thus for each pointer we need to store the base
      // and bound

      size_t num_array_elements = init_array_type->getNumElements();
      ConstantArray* const_array = 
        dyn_cast<ConstantArray>(gv->getInitializer());
      if(!const_array)
        return;

      for( unsigned i = 0; i < num_array_elements ; i++) {
        Constant* struct_constant = const_array->getOperand(i);
        assert(struct_constant && 
               "Initializer structure type but not a constant?");          
        // Constant has zero initializer 
        if(struct_constant->isNullValue())
          continue;
          
        for( unsigned j = 0 ; j < num_struct_elements; j++) {
          const Type* element_type = init_struct_type->getTypeAtIndex(j);
            
          if(isa<PointerType>(element_type)){
              
            Value* initializer_opd = struct_constant->getOperand(j);
            Value* operand_base = NULL;
            Value* operand_bound = NULL;
            Constant* given_constant = dyn_cast<Constant>(initializer_opd);
            assert(given_constant && 
                   "[handleGlobalStructTypeInitializer] not a constant?");
              
            getConstantExprBaseBound(given_constant, operand_base, operand_bound);            
            // Creating the address of ptr
            Constant* index0 = 
              ConstantInt::get(Type::getInt32Ty(module.getContext()), 0);
            Constant* index1 = 
              ConstantInt::get(Type::getInt32Ty(module.getContext()), i);
            Constant* index2 = 
              ConstantInt::get(Type::getInt32Ty(module.getContext()), j);
              
            std::vector<Constant *> indices_addr_ptr;            
                            
            indices_addr_ptr.push_back(index0);
            indices_addr_ptr.push_back(index1);
            indices_addr_ptr.push_back(index2);

            Constant* Indices[3] = {index0, index1, index2};              
            Constant* addr_of_ptr = ConstantExpr::getGetElementPtr(gv, Indices);
            Type* initializer_type = initializer_opd->getType();
            Value* initializer_size = getSizeOfType(initializer_type);
            

            addStoreBaseBoundFunc(addr_of_ptr, operand_base, operand_bound, 
                                  initializer_opd, 
                                  initializer_size, init_function_terminator);
          }                       
        } // Iterating over struct element ends 
      } // Iterating over array element ends         
    }/// Array of Structures Ends 

    if (isa<PointerType>(init_array_type->getElementType())){
      // It is a array of pointers
    }
  }  // Array type case ends 

  if(isa<PointerType>(init_seq_type)){
    // individual pointer stores 
    Value* initializer_base = NULL;
    Value* initializer_bound = NULL;
    Value* initializer = gv->getInitializer();
    Constant* given_constant = dyn_cast<Constant>(initializer);
    getConstantExprBaseBound(given_constant, 
                             initializer_base, 
                             initializer_bound);
    Type* initializer_type = initializer->getType();
    Value* initializer_size = getSizeOfType(initializer_type);
    
    addStoreBaseBoundFunc(gv, initializer_base, initializer_bound,
                          initializer, initializer_size, 
                          init_function_terminator);        
  }

}

// Method: handleGlobalStructTypeInitializer()
//
// Description: handles the global
// initialization for global variables which are of struct type and
// have a pointer as one of their fields and is globally
// initialized 
//
// Comments: This function requires review and rewrite

void 
SoftBoundMPXPass::
handleGlobalStructTypeInitializer(Module& module, 
                                  StructType* init_struct_type,
                                  Constant* initializer, 
                                  GlobalVariable* gv, 
                                  std::vector<Constant*> indices_addr_ptr, 
                                  int length) {
  
  // TODO:URGENT: Do I handle nesxted structures
  
  // has zero initializer 
  if(initializer->isNullValue())
    return;
    
  Instruction* first = getGlobalInitInstruction(module);
  unsigned num_elements = init_struct_type->getNumElements();
  Constant* constant = dyn_cast<Constant>(initializer);
  assert(constant && 
         "[handleGlobalStructTypeInit] global stype with init but not CA?");

  for(unsigned i = 0; i < num_elements ; i++) {
    
    CompositeType* struct_comp_type = 
      dyn_cast<CompositeType>(init_struct_type);
    assert(struct_comp_type && "not a struct type?");
    
    Type* element_type = struct_comp_type->getTypeAtIndex(i);      
    if(isa<PointerType>(element_type)){        
      Value* initializer_opd = constant->getOperand(i);
      Value* operand_base = NULL;
      Value* operand_bound = NULL;
            
      Constant* addr_of_ptr = NULL;
            

        Constant* given_constant = dyn_cast<Constant>(initializer_opd);
        assert(given_constant && 
               "[handleGlobalStructTypeInitializer] not a constant?");
        
        getConstantExprBaseBound(given_constant, operand_base, operand_bound);   

      // Creating the address of ptr
        //      Constant* index1 = 
        //                ConstantInt::get(Type::getInt32Ty(module.getContext()), 0);
      Constant* index2 = ConstantInt::get(Type::getInt32Ty(module.getContext()), i);
      
      //      indices_addr_ptr.push_back(index1);
      indices_addr_ptr.push_back(index2);
      length++;
      addr_of_ptr = ConstantExpr::getGetElementPtr(gv, indices_addr_ptr);
      
      Type* initializer_type = initializer_opd->getType();
      Value* initializer_size = getSizeOfType(initializer_type);     
      addStoreBaseBoundFunc(addr_of_ptr, operand_base, 
                            operand_bound,  
                            initializer_opd, 
                            initializer_size, first);
      

        indices_addr_ptr.pop_back();
        length--;


      continue;
    }     
    if(isa<StructType>(element_type)){
      StructType* child_element_type = 
        dyn_cast<StructType>(element_type);
      Constant* struct_initializer = 
        dyn_cast<Constant>(constant->getOperand(i));      
      Constant* index2 =
        ConstantInt::get(Type::getInt32Ty(module.getContext()), i);
      indices_addr_ptr.push_back(index2);
      length++;
      handleGlobalStructTypeInitializer(module, child_element_type, 
                                        struct_initializer, gv, 
                                        indices_addr_ptr, length); 
      indices_addr_ptr.pop_back();
      length--;
      continue;
    }
  }
}

//
// Method: getConstantExprBaseBound
//
// Description: This function uniform handles all global constant
// expression and obtains the base and bound for these expressions
// without introducing any extra IR modifications.

void SoftBoundMPXPass::getConstantExprBaseBound(Constant* given_constant, 
                                             Value* & tmp_base,
                                             Value* & tmp_bound){


  if(isa<ConstantPointerNull>(given_constant)){
    tmp_base = m_void_null_ptr;
    tmp_bound = m_void_null_ptr;
    return;
  }
  
  ConstantExpr* cexpr = dyn_cast<ConstantExpr>(given_constant);
  tmp_base = NULL;
  tmp_bound = NULL;
    

  if(cexpr) {

    assert(cexpr && "ConstantExpr and Value* is null??");
    switch(cexpr->getOpcode()) {
        
    case Instruction::GetElementPtr:
      {
        Constant* internal_constant = dyn_cast<Constant>(cexpr->getOperand(0));
        getConstantExprBaseBound(internal_constant, tmp_base, tmp_bound);
        break;
      }
      
    case BitCastInst::BitCast:
      {
        Constant* internal_constant = dyn_cast<Constant>(cexpr->getOperand(0));
        getConstantExprBaseBound(internal_constant, tmp_base, tmp_bound);
        break;
      }
    case Instruction::IntToPtr:
      {
        tmp_base = m_void_null_ptr;
        tmp_bound = m_void_null_ptr;
        return;
        break;
      }
    default:
      {
        break;
      }
    } // Switch ends
    
  } else {
      
    const PointerType* func_ptr_type = 
      dyn_cast<PointerType>(given_constant->getType());
      
    if(isa<FunctionType>(func_ptr_type->getElementType())) {
      tmp_base = m_void_null_ptr;
      tmp_bound = m_infinite_bound_ptr;
      return;
    }
    // Create getElementPtrs to create the base and bound 

    std::vector<Constant*> indices_base;
    std::vector<Constant*> indices_bound;
      
    GlobalVariable* gv = dyn_cast<GlobalVariable>(given_constant);


    // TODO: External globals get zero base and infinite_bound 

    if(gv && !gv->hasInitializer()) {
      tmp_base = m_void_null_ptr;
      tmp_bound = m_infinite_bound_ptr;
      return;
    }

    Constant* index_base0 = 
      Constant::
      getNullValue(Type::getInt32Ty(given_constant->getType()->getContext()));

    Constant* index_bound0 = 
      ConstantInt::
      get(Type::getInt32Ty(given_constant->getType()->getContext()), 1);

    indices_base.push_back(index_base0);
    indices_bound.push_back(index_bound0);

    Constant* gep_base = ConstantExpr::getGetElementPtr(given_constant, 
                                                        indices_base);    
    Constant* gep_bound = ConstantExpr::getGetElementPtr(given_constant, 
                                                         indices_bound);
      
    tmp_base = gep_base;
    tmp_bound = gep_bound;      
  }
}


//
// Methods: getAssociatedBase, getAssociatedBound
//
// Description: Retrieves the metadata from SoftBound/CETS maps 
//

Value* 
SoftBoundMPXPass::getAssociatedBase(Value* pointer_operand) {
    
  if(isa<Constant>(pointer_operand)){
    Value* base = NULL;
    Value* bound = NULL;
    Constant* ptr_constant = dyn_cast<Constant>(pointer_operand);
    getConstantExprBaseBound(ptr_constant, base, bound);

    if(base->getType() != m_void_ptr_type){
      Constant* base_given_const = dyn_cast<Constant>(base);
      assert(base_given_const!=NULL);
      Constant* base_const = ConstantExpr::getBitCast(base_given_const, m_void_ptr_type);
      return base_const;
    }
    return base;
  }

  if(!m_pointer_base.count(pointer_operand)){
    pointer_operand->dump();
  }
  assert(m_pointer_base.count(pointer_operand) && 
         "Base absent. Try compiling with -simplifycfg option?");
    
  Value* pointer_base = m_pointer_base[pointer_operand];
  assert(pointer_base && "base present in the map but null?");

  if(pointer_base->getType() != m_void_ptr_type)
    assert(0 && "base in the map does not have the right type");

  return pointer_base;
}

Value* 
SoftBoundMPXPass::getAssociatedBound(Value* pointer_operand) {

  if(isa<Constant>(pointer_operand)){
    Value* base = NULL;
    Value* bound = NULL;
    Constant* ptr_constant = dyn_cast<Constant>(pointer_operand);
    getConstantExprBaseBound(ptr_constant, base, bound);

    if(bound->getType() != m_void_ptr_type){
      Constant* bound_given_const = dyn_cast<Constant>(bound);
      assert(bound_given_const != NULL);
      Constant* bound_const = ConstantExpr::getBitCast(bound_given_const, m_void_ptr_type);
      return bound_const;
    }

    return bound;
  }

    
  assert(m_pointer_bound.count(pointer_operand) && 
         "Bound absent.");
  Value* pointer_bound = m_pointer_bound[pointer_operand];
  assert(pointer_bound && 
         "bound present in the map but null?");    

  if(pointer_bound->getType() != m_void_ptr_type)
    assert(0 && "bound in the map does not have the right type");

  return pointer_bound;
}

// 
// Method: transformFunctionName
//
// Description:
//
// This function returns the transformed name for the function. This
// function appends softboundmpx_ to the input string.


std::string 
SoftBoundMPXPass::transformFunctionName(const std::string &str) { 

  // If the function name starts with this prefix, don't just
  // concatenate, but instead transform the string
  return "softboundmpx_" + str; 
}


void SoftBoundMPXPass::addMemcopyMemsetCheck(CallInst* call_inst, 
                                              Function* called_func) {

  SmallVector<Value*, 8> args;

  if(called_func->getName().find("llvm.memcpy") == 0 || 
     called_func->getName().find("llvm.memmove") == 0){

    CallSite cs(call_inst);

    Value* dest_ptr = cs.getArgument(0);
    Value* src_ptr  = cs.getArgument(1);
    Value* size_ptr = cs.getArgument(2);
    
    args.push_back(dest_ptr);
    args.push_back(src_ptr);

    Value* cast_size_ptr = size_ptr;
    if(size_ptr->getType() != m_key_type){
      BitCastInst* bitcast = new BitCastInst(size_ptr, m_key_type, 
                                             "", call_inst);
                                             
      cast_size_ptr = bitcast;

    }

    args.push_back(cast_size_ptr);
 
      Value* dest_base = getAssociatedBase(dest_ptr);
      Value* dest_bound =getAssociatedBound(dest_ptr);
      
      Value* src_base = getAssociatedBase(src_ptr);
      Value* src_bound = getAssociatedBound(src_ptr);

      args.push_back(dest_base);
      args.push_back(dest_bound);
      
      args.push_back(src_base);
      args.push_back(src_bound);
      

    
    CallInst::Create(m_memcopy_check, args, "", call_inst);
    return;
  }

  if(called_func->getName().find("llvm.memset") == 0){

    args.clear();
    CallSite cs(call_inst);
    Value* dest_ptr = cs.getArgument(0);
    // Whats cs.getArgrument(1) return? Why am I not using it?
    Value* size_ptr = cs.getArgument(2);

    Value* cast_size_ptr = size_ptr;
    if(size_ptr->getType() != m_key_type){
      BitCastInst* bitcast = new BitCastInst(size_ptr, m_key_type, 
                                             "", call_inst);
                                             
      cast_size_ptr = bitcast;

    }
    args.push_back(dest_ptr);
    args.push_back(cast_size_ptr);
    

      Value* dest_base = getAssociatedBase(dest_ptr);
      Value* dest_bound = getAssociatedBound(dest_ptr);
      args.push_back(dest_base);
      args.push_back(dest_bound);   


    CallInst::Create(m_memset_check, args, "", call_inst);

    return;
  }
}

//
// Method: getSizeOfType 
// 
// Description: This function returns the size of the memory access
// based on the type of the pointer which is being dereferenced.  This
// function is used to pass the size of the access in many checks to
// perform byte granularity checking.
//
// Comments: May we should use TargetData instead of m_is_64_bit
// according Criswell's comments.
 

Value* SoftBoundMPXPass:: getSizeOfType(Type* input_type) {

  // Create a Constant Pointer Null of the input type.  Then get a
  // getElementPtr of it with next element access cast it to unsigned
  // int
   
  const PointerType* ptr_type = dyn_cast<PointerType>(input_type);

  if (isa<FunctionType>(ptr_type->getElementType())) {
    if (m_is_64_bit) {
      return ConstantInt::get(Type::getInt64Ty(ptr_type->getContext()), 0);
    } else{
      return ConstantInt::get(Type::getInt32Ty(ptr_type->getContext()), 0);
    }
  }

  const SequentialType* seq_type = dyn_cast<SequentialType>(input_type);
  Constant* int64_size = NULL;
  assert(seq_type && "pointer dereference and it is not a sequential type\n");
  
  StructType* struct_type = dyn_cast<StructType>(input_type);

  if(struct_type){
    if(struct_type->isOpaque()){
      if(m_is_64_bit) {
        return ConstantInt::get(Type::getInt64Ty(seq_type->getContext()), 0);        
      }
      else {
        return ConstantInt::get(Type::getInt32Ty(seq_type->getContext()), 0);
      }
    }
  }
  
  if(m_is_64_bit) {

    if(!seq_type->getElementType()->isSized()){
      return ConstantInt::get(Type::getInt64Ty(seq_type->getContext()), 0);
    }
    int64_size = ConstantExpr::getSizeOf(seq_type->getElementType());
    return int64_size;
  } else {

    // doing what ConstantExpr::getSizeOf() does 
    Constant* gep_idx = 
      ConstantInt::get(Type::getInt32Ty(seq_type->getContext()), 1);

    PointerType* ptr_type = PointerType::getUnqual(seq_type->getElementType());
    Constant* gep_temp = ConstantExpr::getNullValue(ptr_type);
    Constant* gep = ConstantExpr::getGetElementPtr(gep_temp, gep_idx);
    
    Type* int64Ty = Type::getInt64Ty(seq_type->getContext());
    return ConstantExpr::getPtrToInt(gep, int64Ty);
  }    
  assert(0 && "not handled type?");

  return NULL;
}

// Method: isStructOperand
//
//
//Description: This function elides the checks for the structure
//accesses. This is safe when there are no casts in the program.
//

bool
SoftBoundMPXPass::isStructOperand(Value* pointer_operand){
  
  if(isa<GetElementPtrInst>(pointer_operand)){
    GetElementPtrInst* gep = dyn_cast<GetElementPtrInst>(pointer_operand);
    Value* gep_operand = gep->getOperand(0);
    const PointerType* ptr_type = dyn_cast<PointerType>(gep_operand->getType());
    if(isa<StructType>(ptr_type->getElementType())){
      return true;
    }
  }
  return false;
}


//
// This Code is from SAFECode Project.
// Function: createFaultBlock()
//
// Description:
//  Create a basic block which will cause the program to terminate.
//
// Inputs: 
// F - A pointer to a function to which a faulting basic block
//  will be added.
//
static BasicBlock *
createFaultBlock (Function * F) {
  //
  // Create the basic block.
  //
  BasicBlock * faultBB = BasicBlock::Create (F->getContext(), "fault", F);

  //
  // Terminate the basic block with an unreachable instruction.
  //
  Instruction * UI = new UnreachableInst (F->getContext(), faultBB);

  //
  // Add an instruction that will generate a trap.
  //
  LLVMContext & Context = F->getContext();
  Module * M = F->getParent();

  M->getOrInsertFunction("__softboundmpx_dummy", Type::getVoidTy(Context), NULL);
  CallInst::Create(M->getFunction("__softboundmpx_dummy"), "", UI);
  
  M->getOrInsertFunction ("__softboundmpx_abort", Type::getVoidTy (Context), NULL);
  CallInst::Create (M->getFunction ("__softboundmpx_abort"), "", UI);

  return faultBB;
}



//
//
// Method: addLoadStoreChecks
//
// Description: This function inserts calls to C-handler spatial
// safety check functions and elides the check if the map says it is
// not necessary to check.


void 
SoftBoundMPXPass::addLoadStoreChecks(Instruction* load_store, 
                                      std::map<Value*, int>& FDCE_map) {


  SmallVector<Value*, 8> args;
  Value* pointer_operand = NULL;
    
  if(isa<LoadInst>(load_store)) {

    LoadInst* ldi = dyn_cast<LoadInst>(load_store);
    assert(ldi && "not a load instruction");
    pointer_operand = ldi->getPointerOperand();
  }
    
  if(isa<StoreInst>(load_store)){

    StoreInst* sti = dyn_cast<StoreInst>(load_store);
    assert(sti && "not a store instruction");
    // The pointer where the element is being stored is the second
    // operand
    pointer_operand = sti->getOperand(1);
  }
    
  assert(pointer_operand && "pointer operand null?");

    
    // If it is a null pointer which is being loaded, then it must seg
    // fault, no dereference check here
    
    
    if(isa<ConstantPointerNull>(pointer_operand))
      return;

    // Find all uses of pointer operand, then check if it dominates and
    //if so, make a note in the map
    
    GlobalVariable* gv = dyn_cast<GlobalVariable>(pointer_operand);    
#if 0
    if(gv && GLOBALCONSTANTOPT && !isa<SequentialType>(gv->getType())) {
      return;
    }
#endif    
    if(MPXBOUNDSCHECKOPT) {
      // Enable dominator based dereference check optimization only when
      // suggested
      
      if(FDCE_map.count(load_store)) {
        return;
      }
      
      // FIXME: Add more comments here Iterate over the uses
      
      for(Value::use_iterator ui = pointer_operand->use_begin(), 
            ue = pointer_operand->use_end(); 
          ui != ue; ++ui) {
        
        Instruction* temp_inst = dyn_cast<Instruction>(*ui);       
        if(!temp_inst)
          continue;
        
        if(temp_inst == load_store)
          continue;
        
        if(!isa<LoadInst>(temp_inst) && !isa<StoreInst>(temp_inst))
          continue;
        
        if(isa<StoreInst>(temp_inst)){
          if(temp_inst->getOperand(1) != pointer_operand){
            // When a pointer is a being stored at at a particular
            // address, don't elide the check
            continue;
          }
        }
        
        if(m_dominator_tree->dominates(load_store, temp_inst)) {
          if(!FDCE_map.count(temp_inst)) {
            FDCE_map[temp_inst] = true;
            continue;
          }                  
        }
      } // Iterating over uses ends 
    } // BOUNDSCHECKOPT ends 

    
  Value* tmp_base = NULL;
  Value* tmp_bound = NULL;
    
  Constant* given_constant = dyn_cast<Constant>(pointer_operand);    
  if(given_constant ) {
#if 0
    if(GLOBALCONSTANTOPT)
      return;      
#endif

    getConstantExprBaseBound(given_constant, tmp_base, tmp_bound);
  }
  else {
    tmp_base = getAssociatedBase(pointer_operand);
    tmp_bound = getAssociatedBound(pointer_operand);
  }

  Value* bitcast_base = castToVoidPtr(tmp_base, load_store);
  args.push_back(bitcast_base);
  
  Value* bitcast_bound = castToVoidPtr(tmp_bound, load_store);    
  args.push_back(bitcast_bound);
   
  Value* cast_pointer_operand_value = castToVoidPtr(pointer_operand, 
                                                    load_store);    
  args.push_back(cast_pointer_operand_value);
    
  // Pushing the size of the type 
  Type* pointer_operand_type = pointer_operand->getType();
  Value* size_of_type = getSizeOfType(pointer_operand_type);
  args.push_back(size_of_type);

  if(isa<LoadInst>(load_store)){
            
    CallInst::Create(m_spatial_load_dereference_check, args, "", load_store);
  }
  else{    
    CallInst::Create(m_spatial_store_dereference_check, args, "", load_store);
  }

  return;
}


//
// Method:getPointerLoadStore
//
// Description: This function obtains the pointer operand which is
// being dereferenced in the memory access.

Value* 
SoftBoundMPXPass::getPointerLoadStore(Instruction* load_store) {

  Value* pointer_operand  = NULL;
  if (isa<LoadInst>(load_store)) {
    pointer_operand = load_store->getOperand(0);
  }

  if (isa<StoreInst>(load_store)) {
    pointer_operand = load_store->getOperand(1);
  }
  assert((pointer_operand != NULL) && "pointer_operand null");
  return pointer_operand;
}


void SoftBoundMPXPass::addDereferenceChecks(Function* func) {
  
  if(func->isVarArg())
    return;

  m_dominator_tree = &getAnalysis<DominatorTree>(*func);

  /* intra-procedural load dererference check elimination map */
  std::map<Value*, int> func_deref_check_elim_map;

  /* WorkList Algorithm for adding dereference checks. Each basic
   * block is visited only once. We start by visiting the current
   * basic block, then pushing all the successors of the current
   * basic block on to the queue if it has not been visited
   */
    
  std::set<BasicBlock*> bb_visited;
  std::queue<BasicBlock*> bb_worklist;
  Function:: iterator bb_begin = func->begin();

  BasicBlock* bb = dyn_cast<BasicBlock>(bb_begin);
  assert(bb && "Not a basic block  and I am adding dereference checks?");
  bb_worklist.push(bb);

    
  while(bb_worklist.size() != 0) {
      
    bb = bb_worklist.front();
    assert(bb && "Not a BasicBlock?");
    bb_worklist.pop();

    if(bb_visited.count(bb)) {
      /* Block already visited */
      continue;
    }

    /* If here implies basic block not visited */
    /* Insert the block into the set of visited blocks */
    bb_visited.insert(bb);

    /* Iterating over the successors and adding the successors to
     * the worklist
     */
    for(succ_iterator si = succ_begin(bb), se = succ_end(bb); si != se; ++si) {
        
      BasicBlock* next_bb = *si;
      assert(next_bb && "Not a basic block and I am adding to the base and bound worklist?");
      bb_worklist.push(next_bb);
    }

    std::map<Value*, int> bb_deref_check_map;
    /* structure check optimization */
    std::map<Value*, int> bb_struct_check_opt;

    for(BasicBlock::iterator i = bb->begin(), ie = bb->end(); i != ie; ++i){
      Value* v1 = dyn_cast<Value>(i);
      Instruction* new_inst = dyn_cast<Instruction>(i);
      
      /* Do the dereference check stuff */
      if(!m_present_in_original.count(v1))
        continue;
      
      if(isa<LoadInst>(new_inst)){
	
        addLoadStoreChecks(new_inst, func_deref_check_elim_map);
        continue;
      }

      if(isa<StoreInst>(new_inst)){
        addLoadStoreChecks(new_inst, func_deref_check_elim_map);
        continue;
      }

      /* check call through function pointers */
      if(isa<CallInst>(new_inst)) {
          
        if(!MPXCALLCHECKS) {
          continue;
        }          
	  

        SmallVector<Value*, 8> args;
        CallInst* call_inst = dyn_cast<CallInst>(new_inst);
        Value* tmp_base = NULL;
        Value* tmp_bound = NULL;
        
        assert(call_inst && "call instruction null?");
        
        if(!MPXINDIRECTCALLCHECKS)
          continue;

        /* TODO:URGENT : indirect function call checking commented
         * out for the time being to test other aspect of the code,
         * problem was with spec benchmarks perl and h264. They were
         * primarily complaining that the use of a function did not
         * have base and bound in the map
         */


        /* here implies its an indirect call */
        Value* indirect_func_called = call_inst->getOperand(0);
            
        Constant* func_constant = dyn_cast<Constant>(indirect_func_called);
        if(func_constant) {
          getConstantExprBaseBound(func_constant, tmp_base, tmp_bound);           
        }
        else {
          tmp_base = getAssociatedBase(indirect_func_called);
          tmp_bound = getAssociatedBound(indirect_func_called);
        }
        /* Add BitCast Instruction for the base */
        Value* bitcast_base = castToVoidPtr(tmp_base, new_inst);
        args.push_back(bitcast_base);
            
        /* Add BitCast Instruction for the bound */
        Value* bitcast_bound = castToVoidPtr(tmp_bound, new_inst);
        args.push_back(bitcast_bound);
        Value* pointer_operand_value = castToVoidPtr(indirect_func_called, new_inst);
        args.push_back(pointer_operand_value);            
        CallInst::Create(m_call_dereference_func, args, "", new_inst);
        continue;
      } /* Call check ends */
    }
  }  
}



void SoftBoundMPXPass::renameFunctions(Module& module){
    
  bool change = false;

  do{
    change = false;
    for(Module::iterator ff_begin = module.begin(), ff_end = module.end();
        ff_begin != ff_end; ++ff_begin){
        
      Function* func_ptr = dyn_cast<Function>(ff_begin);

      if(m_func_transformed.count(func_ptr->getName()) || 
         isFuncDefSoftBound(func_ptr->getName())){
        continue;
      }
        
      m_func_transformed[func_ptr->getName()] = true;
      m_func_transformed[transformFunctionName(func_ptr->getName())] = true;
      bool is_external = func_ptr->isDeclaration();
      renameFunctionName(func_ptr, module, is_external);
      change = true;
      break;
    }
  }while(change);
}

  
/* Renames a function by changing the function name to softboundmpx_*
   for only those functions have wrappers
 */
  
void SoftBoundMPXPass:: renameFunctionName(Function* func, 
                                            Module& module, 
                                            bool external) {
    
  Type* ret_type = func->getReturnType();
  const FunctionType* fty = func->getFunctionType();
  std::vector<Type*> params;

  if(!m_func_wrappers_available.count(func->getName()))
    return;

  if(func->getName() == "softboundmpx_pseudo_main")
    return;

  SmallVector<AttributeSet, 8> param_attrs_vec;

#if 0

  const AttrListPtr& pal = func->getAttributes();
  if(Attributes attrs = pal.getRetAttributes())
    param_attrs_vec.push_back(AttributeWithIndex::get(0, attrs));
#endif

  int arg_index = 1;

  for(Function::arg_iterator i = func->arg_begin(), e = func->arg_end();
      i != e; ++i, arg_index++) {

    params.push_back(i->getType());
#if 0
    if(Attributes attrs = pal.getParamAttributes(arg_index))
      param_attrs_vec.push_back(AttributeWithIndex::get(params.size(), attrs));
#endif
  }

  FunctionType* nfty = FunctionType::get(ret_type, params, fty->isVarArg());
  Function* new_func = Function::Create(nfty, func->getLinkage(), transformFunctionName(func->getName()));
  new_func->copyAttributesFrom(func);
  new_func->setAttributes(AttributeSet::get(func->getContext(), param_attrs_vec));
  func->getParent()->getFunctionList().insert(func, new_func);
    
  if(!external) {
    SmallVector<Value*, 16> call_args;      
    new_func->getBasicBlockList().splice(new_func->begin(), func->getBasicBlockList());      
    Function::arg_iterator arg_i2 = new_func->arg_begin();      
    for(Function::arg_iterator arg_i = func->arg_begin(), arg_e = func->arg_end(); 
        arg_i != arg_e; ++arg_i) {
        
      arg_i->replaceAllUsesWith(arg_i2);
      arg_i2->takeName(arg_i);        
      ++arg_i2;
      arg_index++;
    }
  }
  func->replaceAllUsesWith(new_func);                            
  func->eraseFromParent();
}


void SoftBoundMPXPass::handleAlloca (AllocaInst* alloca_inst,
                                            BasicBlock* bb, 
                                            BasicBlock::iterator& i) {

  Value *alloca_inst_value = alloca_inst;


    /* Get the base type of the alloca object For alloca instructions,
     * instructions need to inserted after the alloca instruction LLVM
     * provides interface for inserting before.  So use the iterators
     * and handle the case
     */
    
    BasicBlock::iterator nextInst = i;
    nextInst++;
    Instruction* next = dyn_cast<Instruction>(nextInst);
    assert(next && "Cannot increment the instruction iterator?");
    
    unsigned num_operands = alloca_inst->getNumOperands();
    
    /* For any alloca instruction, base is bitcast of alloca, bound is bitcast of alloca_ptr + 1
     */
    PointerType* ptr_type = PointerType::get(alloca_inst->getAllocatedType(), 0);
    Type* ty1 = ptr_type;
    //    Value* alloca_inst_temp_value = alloca_inst;
    BitCastInst* ptr = new BitCastInst(alloca_inst, ty1, alloca_inst->getName(), next);
    
    Value* ptr_base = castToVoidPtr(alloca_inst_value, next);
    
    Value* intBound;
    
    if(num_operands == 0) {
      if(m_is_64_bit) {      
        intBound = ConstantInt::get(Type::getInt64Ty(alloca_inst->getType()->getContext()), 1, false);
      }
      else{
        intBound = ConstantInt::get(Type::getInt32Ty(alloca_inst->getType()->getContext()), 1, false);
      }
    }
    else {
      // What can be operand of alloca instruction?
      intBound = alloca_inst->getOperand(0);
    }
    GetElementPtrInst* gep = GetElementPtrInst::Create(ptr,
                                                       intBound,
                                                       "mtmp",
                                                       next);
    Value *bound_ptr = gep;
    
    Value* ptr_bound = castToVoidPtr(bound_ptr, next);
    
    associateBaseBound(alloca_inst_value, ptr_base, ptr_bound);

}
   

void SoftBoundMPXPass::handleStore(StoreInst* store_inst) {

  Value* operand = store_inst->getOperand(0);
  Value* pointer_dest = store_inst->getOperand(1);
  Instruction* insert_at = getNextInstruction(store_inst);
    
  /* If a pointer is being stored, then the base and bound
   * corresponding to the pointer must be stored in the shadow space
   */
  if(!isa<PointerType>(operand->getType()))
    return;
      

  if(isa<ConstantPointerNull>(operand)) {
    /* it is a constant pointer null being stored
     * store null to the shadow space
     */
#if 0    
    StructType* ST = dyn_cast<StructType>(operand->getType());

    if(ST){
      if(ST->isOpaque()){
        DEBUG(errs()<<"Opaque type found\n");        
      }

    }
      Value* size_of_type = getSizeOfType(operand->getType());
#endif

      Value* size_of_type = NULL;

      addStoreBaseBoundFunc(pointer_dest, m_void_null_ptr, 
                            m_void_null_ptr, 
                            m_void_null_ptr, 
                            size_of_type, insert_at);

    return;      
  }

      
  /* if it is a global expression being stored, then add add
   * suitable base and bound
   */
    
  Value* tmp_base = NULL;
  Value* tmp_bound = NULL;
    
  Constant* given_constant = dyn_cast<Constant>(operand);
  if(given_constant) {      

      getConstantExprBaseBound(given_constant, tmp_base, tmp_bound);
      assert(tmp_base && "global doesn't have base");
      assert(tmp_bound && "global doesn't have bound");        

  }
  else {      
    /* storing an external function pointer */

      if(!checkBaseBoundMetadataPresent(operand)) {
        return;

    }


      tmp_base = getAssociatedBase(operand);
      tmp_bound = getAssociatedBound(operand);              

  }    
  
  /* Store the metadata into the metadata space
   */
  

  //  Type* stored_pointer_type = operand->getType();
  Value* size_of_type = NULL;
  //    Value* size_of_type  = getSizeOfType(stored_pointer_type);
  addStoreBaseBoundFunc(pointer_dest, tmp_base, tmp_bound, operand,  size_of_type, insert_at);    
  
}

// Currently just a placeholder for functions introduced by us
bool SoftBoundMPXPass::checkIfFunctionOfInterest(Function* func) {

  if(isFuncDefSoftBound(func->getName()))
    return false;

  if(func->isDeclaration())
    return false;


  /* TODO: URGENT: Need to do base and bound propagation in variable
   * argument functions
   */
#if 0
  if(func.isVarArg())
    return false;
#endif

  return true;
}


Instruction* SoftBoundMPXPass:: getGlobalInitInstruction(Module& module){
  Function* global_init_function = module.getFunction("__softboundmpx_global_init");    
  assert(global_init_function && "no __softboundmpx_global_init function??");    
  Instruction *global_init_terminator = NULL;
  bool return_inst_flag = false;
  for(Function::iterator fi = global_init_function->begin(), fe = global_init_function->end(); fi != fe; ++fi) {
      
    BasicBlock* bb = dyn_cast<BasicBlock>(fi);
    assert(bb && "basic block null");
    Instruction* bb_term = dyn_cast<Instruction>(bb->getTerminator());
    assert(bb_term && "terminator null?");
      
    if(isa<ReturnInst>(bb_term)) {
      assert((return_inst_flag == false) && "has multiple returns?");
      return_inst_flag = true;
      global_init_terminator = dyn_cast<ReturnInst>(bb_term);
      assert(global_init_terminator && "return inst null?");
    }
  }
  assert(global_init_terminator && "global init does not have return, strange");
  return global_init_terminator;
}



void SoftBoundMPXPass::handleGEP(GetElementPtrInst* gep_inst) {
  Value* getelementptr_operand = gep_inst->getPointerOperand();
  propagateMetadata(getelementptr_operand, gep_inst, SBCETS_GEP);
}

void SoftBoundMPXPass::handleMemcpy(CallInst* call_inst){
    

  Function* func = call_inst->getCalledFunction();
  if(!func)
    return;

  assert(func && "function is null?");

  CallSite cs(call_inst);
  Value* arg1 = cs.getArgument(0);
  Value* arg2 = cs.getArgument(1);
  Value* arg3 = cs.getArgument(2);

  SmallVector<Value*, 8> args;
  args.push_back(arg1);
  args.push_back(arg2);
  args.push_back(arg3);

  if(arg3->getType() == Type::getInt64Ty(arg3->getContext())){
    CallInst::Create(m_copy_metadata, args, "", call_inst);
  }
  else{
    //    CallInst::Create(m_copy_metadata, args, "", call_inst);
  }
  args.clear();

#if 0

  Value* arg1_base = castToVoidPtr(getAssociatedBase(arg1), call_inst);
  Value* arg1_bound = castToVoidPtr(getAssociatedBound(arg1), call_inst);
  Value* arg2_base = castToVoidPtr(getAssociatedBase(arg2), call_inst);
  Value* arg2_bound = castToVoidPtr(getAssociatedBound(arg2), call_inst);
  args.push_back(arg1);
  args.push_back(arg1_base);
  args.push_back(arg1_bound);
  args.push_back(arg2);
  args.push_back(arg2_base);
  args.push_back(arg2_bound);
  args.push_back(arg3);

  CallInst::Create(m_memcopy_check,args.begin(), args.end(), "", call_inst);

#endif
  return;
    
}

void 
SoftBoundMPXPass:: iterateCallSiteIntroduceShadowStackStores(CallInst* call_inst){
    
  int pointer_args_return = getNumPointerArgsAndReturn(call_inst);

  if(pointer_args_return == 0)
    return;
    
  int pointer_arg_no = 1;

  CallSite cs(call_inst);
  for(unsigned i = 0; i < cs.arg_size(); i++){
    Value* arg_value = cs.getArgument(i);
    if(isa<PointerType>(arg_value->getType())){
      introduceShadowStackStores(arg_value, call_inst, pointer_arg_no);
      pointer_arg_no++;
    }
  }    
}


void SoftBoundMPXPass::handleExtractValue(ExtractValueInst* EVI){

  associateBaseBound(EVI, m_void_null_ptr, m_infinite_bound_ptr);
  return;  
}



void SoftBoundMPXPass::handleCall(CallInst* call_inst) {

  // Function* func = call_inst->getCalledFunction();
  Value* mcall = call_inst;

    
  Function* func = call_inst->getCalledFunction();
  if(func && ((func->getName().find("llvm.memcpy") == 0) || 
              (func->getName().find("llvm.memmove") == 0))){
    addMemcopyMemsetCheck(call_inst, func);
    handleMemcpy(call_inst);
    return;
  }

  

  if(func && func->getName().find("llvm.memset") == 0){
    addMemcopyMemsetCheck(call_inst, func);
  }

  if(func && isFuncDefSoftBound(func->getName())){


      associateBaseBound(call_inst, m_void_null_ptr, m_void_null_ptr);


    return;
  }

  Instruction* insert_at = getNextInstruction(call_inst);
  //  call_inst->setCallingConv(CallingConv::C);

  introduceShadowStackAllocation(call_inst);
  iterateCallSiteIntroduceShadowStackStores(call_inst);
    
  if(isa<PointerType>(mcall->getType())) {

      /* ShadowStack for the return value is 0 */
      introduceShadowStackLoads(call_inst, insert_at, 0);       
  }
  introduceShadowStackDeallocation(call_inst,insert_at);
}

void SoftBoundMPXPass::handleIntToPtr(IntToPtrInst* inttoptrinst) {
    
  Value* inst = inttoptrinst;
    
  associateBaseBound(inst, m_void_null_ptr, m_void_null_ptr);

}


void SoftBoundMPXPass::gatherBaseBoundPass2(Function* func){

  /* WorkList Algorithm for propagating base and bound. Each basic
   * block is visited only once
   */
  std::set<BasicBlock*> bb_visited;
  std::queue<BasicBlock*> bb_worklist;
  Function::iterator bb_begin = func->begin();

  BasicBlock* bb = dyn_cast<BasicBlock>(bb_begin);
  assert(bb && "Not a basic block and gathering base bound in the next pass?");
  bb_worklist.push(bb);
    
  while( bb_worklist.size() != 0) {

    bb = bb_worklist.front();
    assert(bb && "Not a BasicBlock?");

    bb_worklist.pop();
    if( bb_visited.count(bb)) {
      /* Block already visited */

      continue;
    }
    /* If here implies basic block not visited */
      
    /* Insert the block into the set of visited blocks */
    bb_visited.insert(bb);

    /* Iterating over the successors and adding the successors to
     * the work list
     */
    for(succ_iterator si = succ_begin(bb), se = succ_end(bb); si != se; ++si) {

      BasicBlock* next_bb = *si;
      assert(next_bb && "Not a basic block and I am adding to the base and bound worklist?");
      bb_worklist.push(next_bb);
    }

    for(BasicBlock::iterator i = bb->begin(), ie = bb->end(); i != ie; ++i) {
      Value* v1 = dyn_cast<Value>(i);
      Instruction* new_inst = dyn_cast<Instruction>(i);

      // If the instruction is not present in the original, no instrumentaion
      if(!m_present_in_original.count(v1))
        continue;

      switch(new_inst->getOpcode()) {

      case Instruction::GetElementPtr:
        {
          GetElementPtrInst* gep_inst = dyn_cast<GetElementPtrInst>(v1);         
          assert(gep_inst && "Not a GEP instruction?");
          handleGEP(gep_inst);
        }
        break;
          
      case Instruction::Store:
        {
          StoreInst* store_inst = dyn_cast<StoreInst>(v1);
          assert(store_inst && "Not a Store instruction?");
          handleStore(store_inst);
        }
        break;

      case Instruction::PHI:
        {
          PHINode* phi_node = dyn_cast<PHINode>(v1);
          assert(phi_node && "Not a PHINode?");
          handlePHIPass2(phi_node);
        }
        break;
 
      case BitCastInst::BitCast:
        {
          BitCastInst* bitcast_inst = dyn_cast<BitCastInst>(v1);
          assert(bitcast_inst && "Not a bitcast instruction?");
          handleBitCast(bitcast_inst);
        }
        break;

      case SelectInst::Select:
        {
        }
        break;
          
      default:
        break;
      }/* Switch Ends */
    }/* BasicBlock iterator Ends */
  }/* Function iterator Ends */
}

void 
SoftBoundMPXPass::introspectMetadata(Function* func, Value* ptr_value, 
                                      Instruction* insert_at, int arg_no){
  if(func->getName() != "debug_instrument_softboundmpx")
    return;

  Value* ptr_base = getAssociatedBase(ptr_value);
  Value* ptr_bound = getAssociatedBound(ptr_value);

  Value* ptr_value_cast = castToVoidPtr(ptr_value, insert_at);
  Value* ptr_base_cast = castToVoidPtr(ptr_base, insert_at);
  Value* ptr_bound_cast = castToVoidPtr(ptr_bound, insert_at);

  Value* argno_value;

  argno_value = ConstantInt::get(Type::getInt32Ty(ptr_value->getType()->getContext()), 
                                 arg_no, false);
  
  SmallVector<Value*, 8> args;
  
  args.push_back(ptr_value_cast);
  args.push_back(ptr_base_cast);
  args.push_back(ptr_bound_cast);
  args.push_back(argno_value);

  CallInst::Create(m_introspect_metadata, args, "", insert_at);

}

bool SoftBoundMPXPass::checkPtrsInST(StructType* struct_type){
  
  StructType::element_iterator I = struct_type->element_begin();
 

  bool ptr_flag = false;
  for(StructType::element_iterator E = struct_type->element_end(); I != E; ++I){
    
    Type* element_type = *I;

    if(isa<StructType>(element_type)){
      StructType* struct_element_type = dyn_cast<StructType>(element_type);
      bool recursive_flag = checkPtrsInST(struct_element_type);
      ptr_flag = ptr_flag | recursive_flag;
    }
    if(isa<PointerType>(element_type)){
      ptr_flag = true;
    }
    if(isa<ArrayType>(element_type)){
      ptr_flag = true;      
    }
  }
  return ptr_flag;
}


bool SoftBoundMPXPass::checkTypeHasPtrs(Argument* ptr_argument){

  if(!ptr_argument->hasByValAttr())
    return false;

  SequentialType* seq_type = dyn_cast<SequentialType>(ptr_argument->getType());
  assert(seq_type && "byval attribute with non-sequential type pointer, not handled?");

  StructType* struct_type = dyn_cast<StructType>(seq_type->getElementType());

  if(struct_type){
    bool has_ptrs = checkPtrsInST(struct_type);
    return has_ptrs;
  }
  else{
    assert(0 && "non-struct byval parameters?");
  }

  // By default we assume any struct can return pointers 
  return true;                                              

}

void SoftBoundMPXPass::gatherBaseBoundPass1 (Function * func) {

  int arg_count= 0;
    
  //    std::cerr<<"transforming function with name:"<<func->getName()<< "\n";
  /* Scan over the pointer arguments and introduce base and bound */

  for(Function::arg_iterator ib = func->arg_begin(), ie = func->arg_end();
      ib != ie; ++ib) {

    if(!isa<PointerType>(ib->getType())) 
      continue;

    /* it is a pointer, so increment the arg count */
    arg_count++;

    Argument* ptr_argument = dyn_cast<Argument>(ib);
    Value* ptr_argument_value = ptr_argument;
    Instruction* fst_inst = func->begin()->begin();
      
    /* Urgent: Need to think about what we need to do about byval attributes */
    if(ptr_argument->hasByValAttr()){
      
      if(checkTypeHasPtrs(ptr_argument)){
        assert(0 && "Pointer argument has byval attributes and the underlying structure returns pointers");
      }
      
      associateBaseBound(ptr_argument_value, m_void_null_ptr, m_infinite_bound_ptr);

    }
    else{
      introduceShadowStackLoads(ptr_argument_value, fst_inst, arg_count);
      //      introspectMetadata(func, ptr_argument_value, fst_inst, arg_count);
    }
  }

  m_faulting_block[func] =  createFaultBlock(func);


  /* WorkList Algorithm for propagating the base and bound. Each
   * basic block is visited only once. We start by visiting the
   * current basic block, then push all the successors of the
   * current basic block on to the queue if it has not been visited
   */
  std::set<BasicBlock*> bb_visited;
  std::queue<BasicBlock*> bb_worklist;
  Function:: iterator bb_begin = func->begin();

  BasicBlock* bb = dyn_cast<BasicBlock>(bb_begin);
  assert( bb && "Not a basic block and I am gathering base and bound?");
  bb_worklist.push(bb);

  while(bb_worklist.size() != 0) {

    bb = bb_worklist.front();
    assert(bb && "Not a BasicBlock?");

    bb_worklist.pop();
    if( bb_visited.count(bb)) {
      /* Block already visited */
      continue;
    }
    /* If here implies basic block not visited */
      
    /* Insert the block into the set of visited blocks */
    bb_visited.insert(bb);

    /* Iterating over the successors and adding the successors to
     * the work list
     */
    for(succ_iterator si = succ_begin(bb), se = succ_end(bb); si != se; ++si) {

      BasicBlock* next_bb = *si;
      assert(next_bb && "Not a basic block and I am adding to the base and bound worklist?");
      bb_worklist.push(next_bb);
    }
      
    for(BasicBlock::iterator i = bb->begin(), ie = bb->end(); i != ie; ++i){
      Value* v1 = dyn_cast<Value>(i);
      Instruction* new_inst = dyn_cast<Instruction>(i);


      /* If the instruction is not present in the original, no
       * instrumentaion 
       */
      if(!m_present_in_original.count(v1)) {
        continue;
      }

      /* All instructions have been defined here as defining it in
       * switch causes compilation errors. Assertions have been in
       * the inserted in the specific cases
       */

      switch(new_inst->getOpcode()) {
        
      case Instruction::Alloca:
        {
          AllocaInst* alloca_inst = dyn_cast<AllocaInst>(v1);
          assert(alloca_inst && "Not an Alloca inst?");
          handleAlloca(alloca_inst, bb, i);
        }
        break;

      case Instruction::Load:
        {
          LoadInst* load_inst = dyn_cast<LoadInst>(v1);            
          assert(load_inst && "Not a Load inst?");
          handleLoad(load_inst);
        }
        break;

      case Instruction::GetElementPtr:
        {
          GetElementPtrInst* gep_inst = dyn_cast<GetElementPtrInst>(v1);
          assert(gep_inst && "Not a GEP inst?");
          handleGEP(gep_inst);
        }
        break;
	
      case BitCastInst::BitCast:
        {
          BitCastInst* bitcast_inst = dyn_cast<BitCastInst>(v1);
          assert(bitcast_inst && "Not a BitCast inst?");
          handleBitCast(bitcast_inst);
        }
        break;

      case Instruction::PHI:
        {
          PHINode* phi_node = dyn_cast<PHINode>(v1);
          assert(phi_node && "Not a phi node?");
          //printInstructionMap(v1);
          handlePHIPass1(phi_node);
        }
        /* PHINode ends */
        break;
        
      case Instruction::Call:
        {
          CallInst* call_inst = dyn_cast<CallInst>(v1);
          assert(call_inst && "Not a Call inst?");
          handleCall(call_inst);
        }
        break;

      case Instruction::Select:
        {
          SelectInst* select_insn = dyn_cast<SelectInst>(v1);
          assert(select_insn && "Not a select inst?");
          int pass = 1;
          handleSelect(select_insn, pass);
        }
        break;

      case Instruction::Store:
        {
          break;
        }

      case Instruction::IntToPtr:
        {
          IntToPtrInst* inttoptrinst = dyn_cast<IntToPtrInst>(v1);
          assert(inttoptrinst && "Not a IntToPtrInst?");
          handleIntToPtr(inttoptrinst);
          break;
        }

      case Instruction::Ret:
        {
          ReturnInst* ret = dyn_cast<ReturnInst>(v1);
          assert(ret && "not a return inst?");
          handleReturnInst(ret);
        }
        break;

      case Instruction::ExtractValue:
	{
	  ExtractValueInst * EVI = dyn_cast<ExtractValueInst>(v1);
	  assert(EVI && "hanlde extract value inst?");
	  handleExtractValue(EVI);
	}
	break;
        
      default:
        if(isa<PointerType>(v1->getType()))
          assert(!isa<PointerType>(v1->getType())&&
                 " Generating Pointer and not being handled");
      }
    }/* Basic Block iterator Ends */
  } /* Function iterator Ends */
   
}

/* isByValDerived: This function check whether loaded address is
   dervied by a byval argument */

bool SoftBoundMPXPass:: isByValDerived(Value* pointer_operand){

  int count = 0;
  while(true){
    count++;
    if(count > 50){
      assert(0 && "isByValDerived probably looping infinitely");
    }

    if(isa<GetElementPtrInst>(pointer_operand)){
      GetElementPtrInst* gep = dyn_cast<GetElementPtrInst>(pointer_operand);
      pointer_operand = gep->getOperand(0);
      continue;
    }

    if(isa<AllocaInst>(pointer_operand)){
      return false;
    }

    if(isa<Argument>(pointer_operand)){
      Argument* arg = dyn_cast<Argument>(pointer_operand);
      return arg->hasByValAttr();
    }

    if(isa<BitCastInst>(pointer_operand)){
      BitCastInst* bitcast = dyn_cast<BitCastInst>(pointer_operand);
      pointer_operand = bitcast->getOperand(0);
      continue;
    }

    if(isa<PHINode>(pointer_operand)){
      PHINode* phi_node = dyn_cast<PHINode>(pointer_operand);
      unsigned num_values = phi_node->getNumIncomingValues();

      bool arg_flag = false;
      for(unsigned i = 0; i < num_values; i++){
        Value* temp_operand = phi_node->getOperand(i);
        if(isa<PHINode>(temp_operand))
          return false;
        arg_flag = arg_flag | isByValDerived(temp_operand);
      }
      return arg_flag;
    }

    if(isa<LoadInst>(pointer_operand)){
      return false;
    }

    if(isa<Constant>(pointer_operand)){
      return false;
    }
    if(isa<CallInst>(pointer_operand)){
      return false;
    }
  }    
}


/* handleLoad Takes a load_inst If the load is through a pointer
 * which is a global then inserts base and bound for that global
 * Also if the loaded value is a pointer then loads the base and
 * bound for for the pointer from the shadow space
 */

void SoftBoundMPXPass::handleLoad(LoadInst* load_inst) { 

  AllocaInst* base_alloca;
  AllocaInst* bound_alloca;

  SmallVector<Value*, 8> args;

  if(!isa<PointerType>(load_inst->getType()))
    return;

  Value* load_inst_value = load_inst;
  Value* pointer_operand = load_inst->getPointerOperand();
  Instruction* load = load_inst;    

  Instruction* insert_at = getNextInstruction(load);

  /* If the load returns a pointer, then load the base and bound
   * from the shadow space
   */
  Value* pointer_operand_bitcast =  castToVoidPtr(pointer_operand, insert_at);      
  Instruction* first_inst_func = dyn_cast<Instruction>(load_inst->getParent()->getParent()->begin()->begin());
  assert(first_inst_func && "function doesn't have any instruction and there is load???");
  
  /* address of pointer being pushed */
  args.push_back(pointer_operand_bitcast);
    

    
    base_alloca = new AllocaInst(m_void_ptr_type, "base.alloca", first_inst_func);
    bound_alloca = new AllocaInst(m_void_ptr_type, "bound.alloca", first_inst_func);
  
    /* base */
    args.push_back(base_alloca);
    /* bound */
    args.push_back(bound_alloca);

  
  CallInst::Create(m_load_base_bound_func, args, "", insert_at);
      

    Instruction* base_load = new LoadInst(base_alloca, "base.load", insert_at);
    Instruction* bound_load = new LoadInst(bound_alloca, "bound.load", insert_at);
    associateBaseBound(load_inst_value, base_load, bound_load);      


}




/* Identify the initial globals present in the program before we add
 * extra base and bound for all globals
 */
void SoftBoundMPXPass::identifyInitialGlobals(Module& module) {

  for(Module::global_iterator it = module.global_begin(), 
        ite = module.global_end();
      it != ite; ++it) {
      
    GlobalVariable* gv = dyn_cast<GlobalVariable>(it);
    if(gv) {
      m_initial_globals[gv] = true;
    }      
  }
}

void SoftBoundMPXPass::addBaseBoundGlobals(Module& M){
  /* iterate over the globals here */

  for(Module::global_iterator it = M.global_begin(), ite = M.global_end(); it != ite; ++it){
    
    GlobalVariable* gv = dyn_cast<GlobalVariable>(it);
    
    if(!gv){
      continue;
    }

    if(gv->getSection() == "llvm.metadata"){
      continue;
    }
    if(gv->getName() == "llvm.global_ctors"){
      continue;
    }
    
    if(!gv->hasInitializer())
      continue;
    
    /* gv->hasInitializer() is true */
    
    Constant* initializer = dyn_cast<Constant>(it->getInitializer());
    ConstantArray* constant_array = dyn_cast<ConstantArray>(initializer);
    
    if(initializer && isa<CompositeType>(initializer->getType())){

      if(isa<StructType>(initializer->getType())){
        std::vector<Constant*> indices_addr_ptr;
        Constant* index1 = ConstantInt::get(Type::getInt32Ty(M.getContext()), 0);
        indices_addr_ptr.push_back(index1);
        StructType* struct_type = dyn_cast<StructType>(initializer->getType());
        handleGlobalStructTypeInitializer(M, struct_type, initializer, gv, indices_addr_ptr, 1);
        continue;
      }
      
      if(isa<SequentialType>(initializer->getType())){
        handleGlobalSequentialTypeInitializer(M, gv);
      }
    }
    
    if(initializer && !constant_array){
      
      if(isa<PointerType>(initializer->getType())){
        //        std::cerr<<"Pointer type initializer\n";
      }
    }
    
    if(!constant_array)
      continue;
    
    int num_ca_opds = constant_array->getNumOperands();
    
    for(int i = 0; i < num_ca_opds; i++){
      Value* initializer_opd = constant_array->getOperand(i);
      Instruction* first = getGlobalInitInstruction(M);
      Value* operand_base = NULL;
      Value* operand_bound = NULL;
      
      Constant* global_constant_initializer = dyn_cast<Constant>(initializer_opd);
      if(!isa<PointerType>(global_constant_initializer->getType())){
        break;
      }
      getConstantExprBaseBound(global_constant_initializer, operand_base, operand_bound);
      
      SmallVector<Value*, 8> args;
      Constant* index1 = ConstantInt::get(Type::getInt32Ty(M.getContext()), 0);
      Constant* index2 = ConstantInt::get(Type::getInt32Ty(M.getContext()), i);

      std::vector<Constant*> indices_addr_ptr;
      indices_addr_ptr.push_back(index1);
      indices_addr_ptr.push_back(index2);

      Constant* addr_of_ptr = ConstantExpr::getGetElementPtr(gv, indices_addr_ptr);
      Type* initializer_type = initializer_opd->getType();
      Value* initializer_size = getSizeOfType(initializer_type);
      
      addStoreBaseBoundFunc(addr_of_ptr, operand_base, operand_bound, initializer_opd, initializer_size, first);
      
    }
  }

}
void SoftBoundMPXPass::identifyOriginalInst (Function * func) {

  for(Function::iterator bb_begin = func->begin(), bb_end = func->end();
      bb_begin != bb_end; ++bb_begin) {

    for(BasicBlock::iterator i_begin = bb_begin->begin(),
          i_end = bb_begin->end(); i_begin != i_end; ++i_begin){

      Value* insn = dyn_cast<Value>(i_begin);
      if(!m_present_in_original.count(insn)) {
        m_present_in_original[insn] = 1;
      }
      else {
        assert(0 && "present in original map already has the insn?");
      }

      if(isa<PointerType>(insn->getType())) {
        if(!m_is_pointer.count(insn)){
          m_is_pointer[insn] = 1;
        }
      }
    } /* BasicBlock ends */
  }/* Function ends */
}

bool SoftBoundMPXPass::runOnModule(Module& module) {


  TD = &getAnalysis<DataLayout>();
  TLI = &getAnalysis<TargetLibraryInfo>();

  BuilderTy TheBuilder(module.getContext(), TargetFolder(TD));

  Builder = &TheBuilder;

  if (module.getPointerSize() == llvm::Module::Pointer64) {
    m_is_64_bit = true;
  } else {
    m_is_64_bit = false;
  }
  
  initializeSoftBoundVariables(module);
  transformMain(module);

  identifyFuncToTrans(module);

  identifyInitialGlobals(module);
  addBaseBoundGlobals(module);
  
  for(Module::iterator ff_begin = module.begin(), ff_end = module.end(); 
      ff_begin != ff_end; ++ff_begin){
    Function* func_ptr = dyn_cast<Function>(ff_begin);
    assert(func_ptr && "Not a function??");
    
    //
    // No instrumentation for functions introduced by us for updating
    // and retrieving the shadow space
    //
      
    if (!checkIfFunctionOfInterest(func_ptr)) {
      continue;
    }  
    //
    // Iterating over the instructions in the function to identify IR
    // instructions in the original program In this pass, the pointers
    // in the original program are also identified
    //
      
    identifyOriginalInst(func_ptr);
      
    //
    // Iterate over all basic block and then each insn within a basic
    // block We make two passes over the IR for base and bound
    // propagation and one pass for dereference checks
    //

    gatherBaseBoundPass1(func_ptr);
    gatherBaseBoundPass2(func_ptr);
    addDereferenceChecks(func_ptr);            
  }


  renameFunctions(module);
  DEBUG(errs()<<"Done with SoftBoundMPXPass\n");
  
  /* print the external functions not wrapped */

  for(Module::iterator ff_begin = module.begin(), ff_end = module.end();
      ff_begin != ff_end; ++ff_begin){
    Function* func_ptr = dyn_cast<Function>(ff_begin);
    assert(func_ptr && "not a function??");

    if(func_ptr->isDeclaration()){
      if(!isFuncDefSoftBound(func_ptr->getName()) && 
         !(m_func_wrappers_available.count(func_ptr->getName()))){
        DEBUG(errs()<<"External function not wrapped:"<<
              func_ptr->getName()<<"\n");
      }

    }    
  }
  return true;
}
