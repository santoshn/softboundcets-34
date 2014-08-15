//=== SoftBoundCETS/SoftBoundCETS.cpp --*- C++ -*=====///
// Pointer based Spatial and Temporal Memory Safety Pass
// Copyright (c) 2014 Santosh Nagarakatte, Milo M. K. Martin. All rights reserved.
//
// Developed by: Santosh Nagarakatte, 
//               Department of Computer Science,
//               Rutgers University
//               http://www.cs.rutgers.edu/~santosh.nagarakatte/softbound/
//               
//               in collaboration with
//               Milo Martin, Jianzhou Zhao, Steve Zdancewic
//               University of Pennsylvania
//
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal with the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
//   1. Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimers.

//   2. Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimers in the
//      documentation and/or other materials provided with the distribution.

//   3. Neither the names of Santosh Nagarakatte, Milo M. K. Martin,
//      Jianzhou Zhao, Steve Zdancewic, University of Pennsylvania,
//      Rutgers University, nor the names of its contributors may be
//      used to endorse or promote products derived from this Software
//      without specific prior written permission.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// WITH THE SOFTWARE.
//===---------------------------------------------------------------------===//

#include "llvm/Transforms/SoftBoundCETS/SoftBoundCETSPass.h"


cl::opt<bool>
eliminate_struct_checks
("eliminate_struct_checks",
 cl::desc("don't perform any spatial checking for structure accesses"),
 cl::init(false));

cl::opt<bool>
simple_metadata_mode
("simple_metadata_mode",
 cl::desc("use wrapper functions for metadata loads instead of allocas"),
 cl::init(false));

cl::opt<bool>
disable_spatial_check_opt
("disable_spatial_check_opt",
 cl::desc("disable spatial check optimizations"),
 cl::init(false));

cl::opt<bool>
disable_temporal_check_opt
("disable_temporal_check_opt",
 cl::desc("disable temporal check optimizations"),
 cl::init(false));

cl::opt<bool>
disable_spatial_safety
("softboundcets_disable_spatial_safety",
 cl::desc("disable transformation for spatial safety"),
 cl::init(false));

cl::opt<bool>
disable_temporal_safety
("softboundcets_disable_temporal_safety",
 cl::desc("disable transformation for temporal safety"),
 cl::init(false));

static cl::opt<bool>
store_only
("softboundcets_store_only",
 cl::desc("perform store only checking"),
 cl::init(false));

static cl::opt<bool>
metadata_prop_only
("softboundcets_mdprop_only",
 cl::desc("perform only metadata propagation"),
 cl::init(false));

static cl::opt<bool>
chk_intrinsic
("softboundcets_chk_intrinsic",
 cl::desc("insert intrinsics for spatial and temporal safety"),
 cl::init(false));


static cl::opt<bool>
LOADCHECKS
("softboundcets_spatial_safety_load_checks",
 cl::desc("introduce load dereference checks for spatial safety"),
 cl::init(true));

static cl::opt<bool>
STORECHECKS
("softboundcets_spatial_safety_store_checks",
 cl::desc("introduce store dereference checks for spatial safety"),
 cl::init(true));

static cl::opt<bool>
TEMPORALLOADCHECKS
("softboundcets_temporal_load_checks",
 cl::desc("introduce temporal load dereference checks"),
 cl::init(true));

static cl::opt<bool>
TEMPORALSTORECHECKS
("softboundcets_temporal_store_checks",
 cl::desc("introduce temporal store dereference checks"),
 cl::init(true));




static cl::opt<bool>
FUNCDOMTEMPORALCHECKOPT
("softboundcets_func_dom_temporal_check_opt",
 cl::desc("eliminate function based redundant checks with dominator analysis"),
 cl::init(true));

static cl::opt<bool>
STRUCTOPT
("softboundcets_struct_opt", 
 cl::desc("enable or disable structure optimization"),
 cl::init(true));

static cl::opt<bool>
BOUNDSCHECKOPT 
("softboundcets_bounds_check_opt",
 cl::desc("enable dominator based load dereference check elimination"),
 cl::init(true));

static cl::opt<bool>
SHRINKBOUNDS 
("softboundcets_shrink_bounds",
 cl::desc("enable shrinking bounds for the softboundboundcetswithss pass"),
 cl::init(false)); 

static cl::opt<bool>
DISABLE_MEMCOPYCHECK
("softboundcets_disable_memcopy_check",
 cl::desc("disable check memcopy calls"),
 cl::init(false));

static cl::opt<bool>
GLOBALCONSTANTOPT
("softboundcets_global_const_opt",
 cl::desc("global constant expressions are not checked"),
 cl::init(false));

static cl::opt<bool>
CALLCHECKS
("softboundcets_call_checks",
 cl::desc("introduce call checks"),
 cl::init(true));

static cl::opt<bool>
INDIRECTCALLCHECKS
("softboundcets_indirect_call_checks",
 cl::desc("introduce indirect call checks"),
 cl::init(false));

static cl::opt<bool>
OPAQUECALLS
("softboundcets_opaque_calls",
 cl::desc("consider all calls as opaque for func_dom_check_elimination"),
 cl::init(true));

static cl::opt<bool>
TEMPORALBOUNDSCHECKOPT
("softboundcets_temporal_bounds_check_opt",
 cl::desc("enable or disable temporal dominator based check elimination"),
 cl::init(true));

static cl::opt<bool>
STACKTEMPORALCHECKOPT
("softboundcets_stack_temporal_check_opt",
 cl::desc("eliminate temporal checks for stack variables"),
 cl::init(true));

static cl::opt<bool>
GLOBALTEMPORALCHECKOPT
("softboundcets_global_temporal_check_opt",
 cl::desc("eliminate temporal checks for global variables"),
 cl::init(true));

static cl::opt<bool>
BBDOMTEMPORALCHECKOPT
("softboundcets_bb_dom_temporal_check_opt",
 cl::desc("eliminate redundant checks in the basic block"),
 cl::init(true));

static cl::opt<bool>
DISABLE_MEMCOPY_METADATA_COPIES
("softboundcets_disable_memcpy_metadata_copies",
 cl::desc("disable metadata copies with memcopy"),
 cl::init(false));

#if 0
static cl::opt<bool>
unsafe_byval_opt
("unsafe_byval_opt",
 cl::desc("Unbound byval attributed pointers so that check always succeeds"),
 cl::init(false));
#endif

// #define SOFTBOUNDCETS_CHK_INTRINSIC 1

char SoftBoundCETSPass:: ID = 0;

static RegisterPass<SoftBoundCETSPass> P ("SoftBoundCETSPass",
                                          "SoftBound Pass for Spatial Safety");


//
// Method: getAssociateFuncLock()
//
// Description: 
//
// This method looks up the "lock" for global variables associated
// with the function. Every will have a getGlobalLockAddr function
// inserted at the beginning which will serve as the lock for all the
// global variables used in the function.
//
//
// Inputs:
//
// Pointer_inst: An instruction that is manipulating a global pointer
// value.
//
// Return value:
// 
// Returns the "lock associated with the function. Should never return
// NULL.
//

Value* 
SoftBoundCETSPass:: getAssociatedFuncLock(Value* PointerInst){

  Instruction* inst = dyn_cast<Instruction>(PointerInst);

  Value* tmp_lock = NULL;
  if (!inst) {
    assert(0 && "Function does not have global lock?");
    return NULL;
  }
  
  if (m_func_global_lock.count(inst->getParent()->getParent()->getName())) {
    tmp_lock = m_func_global_lock[inst->getParent()->getParent()->getName()];
  }
  
  return tmp_lock;
}

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

void SoftBoundCETSPass::initializeSoftBoundVariables(Module& module) {

  if(spatial_safety){
    m_spatial_load_dereference_check = 
      module.getFunction("__softboundcets_spatial_load_dereference_check");
    assert(m_spatial_load_dereference_check && 
           "__softboundcets_spatial_load_dereference_check function type null?");
    
    m_spatial_store_dereference_check = 
      module.getFunction("__softboundcets_spatial_store_dereference_check");
    assert(m_spatial_store_dereference_check && 
           "__softboundcets_spatial_store_dereference_check function type null?");
    
  }

  if(temporal_safety){
    m_temporal_load_dereference_check = 
      module.getFunction("__softboundcets_temporal_load_dereference_check");
    assert(m_temporal_load_dereference_check && 
           "__softboundcets_temporal_load_dereference_check function type null?");
    
    m_temporal_global_lock_function = 
      module.getFunction("__softboundcets_get_global_lock");
    assert(m_temporal_global_lock_function && 
           "__softboundcets_get_global_lock function type null?");
    
    m_temporal_store_dereference_check = 
      module.getFunction("__softboundcets_temporal_store_dereference_check");
    assert(m_temporal_store_dereference_check && 
           " __softboundcets_temporal_store_dereference_check function type null?");
  }
  m_introspect_metadata = 
    module.getFunction("__softboundcets_introspect_metadata");
  assert(m_introspect_metadata && 
         "__softboundcets_introspect_metadata null?");
    
  m_copy_metadata = module.getFunction("__softboundcets_copy_metadata");
  assert(m_copy_metadata && "__softboundcets_copy_metadata NULL?");
    
  m_shadow_stack_allocate = 
    module.getFunction("__softboundcets_allocate_shadow_stack_space");
  assert(m_shadow_stack_allocate && 
         "__softboundcets_allocate_shadow_stack_space NULL?");

  m_shadow_stack_deallocate = 
    module.getFunction("__softboundcets_deallocate_shadow_stack_space");
  assert(m_shadow_stack_deallocate && 
         "__softboundcets_deallocate_shadow_stack_space NULL?");

  if(spatial_safety){
    m_shadow_stack_base_store = 
      module.getFunction("__softboundcets_store_base_shadow_stack");
    assert(m_shadow_stack_base_store && 
           "__softboundcets_store_base_shadow_stack NULL?");
    
    m_shadow_stack_bound_store = 
      module.getFunction("__softboundcets_store_bound_shadow_stack");
    assert(m_shadow_stack_bound_store && 
           "__softboundcets_store_bound_shadow_stack NULL?");
  
    
    m_shadow_stack_base_load = 
      module.getFunction("__softboundcets_load_base_shadow_stack");
    assert(m_shadow_stack_base_load && 
           "__softboundcets_load_base_shadow_stack NULL?");
    
    m_shadow_stack_bound_load = 
      module.getFunction("__softboundcets_load_bound_shadow_stack");
    assert(m_shadow_stack_bound_load && 
           "__softboundcets_load_bound_shadow_stack NULL?");
  }
  if(temporal_safety){
    m_shadow_stack_key_load = 
      module.getFunction("__softboundcets_load_key_shadow_stack");
    assert(m_shadow_stack_key_load && 
           "__softboundcets_load_key_shadow_stack NULL?");
    
    m_shadow_stack_lock_load = 
      module.getFunction("__softboundcets_load_lock_shadow_stack");
    assert(m_shadow_stack_lock_load && 
           "__softboundcets_load_lock_shadow_stack NULL?");
 
    m_shadow_stack_key_store = 
      module.getFunction("__softboundcets_store_key_shadow_stack");
    assert(m_shadow_stack_key_store && 
           "__softboundcets_store_key_shadow_stack NULL?");
    
    m_shadow_stack_lock_store = 
      module.getFunction("__softboundcets_store_lock_shadow_stack");
    assert(m_shadow_stack_lock_store && 
           "__softboundcets_store_lock_shadow_stack NULL?");
    
    
    m_temporal_stack_memory_allocation = 
      module.getFunction("__softboundcets_stack_memory_allocation");
    assert(m_temporal_stack_memory_allocation && 
           "__softboundcets_stack_memory_allocation");

    m_temporal_stack_memory_deallocation = 
      module.getFunction("__softboundcets_stack_memory_deallocation");
    assert(m_temporal_stack_memory_deallocation && 
           "__softboundcets_stack_memory_deallocation not defined?");
  }
    
  if(spatial_safety && temporal_safety){
    m_metadata_map_func = module.getFunction("__softboundcets_metadata_map");
    assert(m_metadata_map_func && "__softboundcets_metadata_map null?");
    
    if(spatial_safety){
      m_metadata_load_base_func = module.getFunction("__softboundcets_metadata_load_base");
      assert(m_metadata_load_base_func && "__softboundcets_metadata_load_base null?");
      
      m_metadata_load_bound_func = module.getFunction("__softboundcets_metadata_load_bound");
      assert(m_metadata_load_bound_func && "__softboundcets_metadata_load_bound null?");
    }
    
    if(temporal_safety){
      m_metadata_load_key_func = module.getFunction("__softboundcets_metadata_load_key");
      assert(m_metadata_load_key_func && "__softboundcets_metadata_load_key null");
      
      m_metadata_load_lock_func = module.getFunction("__softboundcets_metadata_load_lock");
      assert(m_metadata_load_lock_func && "__softboundcets_metadata_load_lock null?");

    }
  }

  m_metadata_load_vector_func = module.getFunction("__softboundcets_metadata_load_vector");
  assert(m_metadata_load_vector_func && "__softboundcets_metadata_load_vector null?");

  m_metadata_store_vector_func = module.getFunction("__softboundcets_metadata_store_vector");
  assert(m_metadata_store_vector_func && "__softboundcets_metadata_store_vector null?");
  
  m_load_base_bound_func = module.getFunction("__softboundcets_metadata_load");
  assert(m_load_base_bound_func && "__softboundcets_metadata_load null?");
  
  m_store_base_bound_func = module.getFunction("__softboundcets_metadata_store");
  assert(m_store_base_bound_func && "__softboundcets_metadata_store null?");

  m_call_dereference_func = 
    module.getFunction("__softboundcets_spatial_call_dereference_check");
  assert(m_call_dereference_func && 
         "__softboundcets_spatial_call_dereference_check function null??");

  m_memcopy_check = 
    module.getFunction("__softboundcets_memcopy_check");
  assert(m_memcopy_check && 
         "__softboundcets_memcopy_check function null?");

  m_memset_check = 
    module.getFunction("__softboundcets_memset_check");
  assert(m_memcopy_check && 
         "__softboundcets_memset_check function null?");


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
bool SoftBoundCETSPass::isAllocaPresent(Function* func){

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
// Method: getFunctionKeyLock()
//
// Description: 
//
// This function introduces a memory allocation call for allocating a
// new "key" and "lock" for the stack frames on function entry.  This
// function also stores the key and lock in the reference Value*
// arguments provided to the function.  Further, key and lock is
// allocated only when temporal checking is performed.
//
// Inputs:
//
// func: Function* of the function performing the allocation
// func_key: Value* & is the reference argument to return the key
// func_lock: Value* & is the reference_argument to return the lock
// func_xmm_lock: Value* & is the reference argument that will be
// eventually used to return the key and lock as wide parameters.
//

void 
SoftBoundCETSPass::getFunctionKeyLock(Function* func, 
                                      Value* & func_key, 
                                      Value* & func_lock, 
                                      Value* & func_xmm_key_lock) {

  Instruction* func_alloca_inst = NULL;
  func_key = NULL;
  func_lock = NULL;
  func_xmm_key_lock = NULL;    
  if (!temporal_safety) 
    return; 

  if(!isAllocaPresent(func))
    return;
  
  func_alloca_inst = dyn_cast<Instruction>(func->begin()->begin());  
  assert(func_alloca_inst && "func begin null?");
  addMemoryAllocationCall(func, func_key, 
			  func_lock, func_alloca_inst);
  
  return;
}

//
// Method: addMemoryAllocationCall()
//
// This function introduces a call to the C-handler function for
// allocating key and lock for stack frames. After the handler call,
// it performs the load of the key and the lock to use it as the
// metadata for pointers pointing to stack allocations in the
// function.
//
// Inputs: 
//
// func: Function for which the key and the lock is being allocated
// 
// ptr_key: Reference argument to return the key after the key and lock
// allocation 
//
// ptr_lock: Reference argument to return the lock after
// the key and lock allocation 
//
// insert_at: Instruction* before which the C-handler is introduced
// 
// Outputs:
//
// A new key and lock is allocated by the C-handler and then returned
// via reference arguments that is used as key and lock for pointers
// pointing to stack allocations in the function.
//


void 
SoftBoundCETSPass::addMemoryAllocationCall(Function* func, 
                                           Value* & ptr_key, 
                                           Value* & ptr_lock, 
                                           Instruction* insert_at) {

  SmallVector<Value*, 8> args;
  Instruction* first_inst_func = cast<Instruction>(func->begin()->begin());
  AllocaInst* lock_alloca = new AllocaInst(m_void_ptr_type, 
                                           "lock_alloca", 
                                           first_inst_func);
  AllocaInst* key_alloca = new AllocaInst(Type::getInt64Ty(func->getContext()), 
                                          "key_alloca", first_inst_func);
  args.push_back(lock_alloca);
  args.push_back(key_alloca);
  
  Instruction* 
    flc_call = CallInst::Create(m_temporal_stack_memory_allocation, 
				args, "", first_inst_func);
  
  //
  // Load the key and lock from the reference arguments passed to the
  // C-handler
  //

  Instruction* next_inst = getNextInstruction(flc_call);
  Instruction* alloca_lock = new LoadInst(lock_alloca, 
					  "lock.load", next_inst);
  Instruction* alloca_key = new LoadInst(key_alloca, 
					 "key.load", next_inst);
 
  ptr_key = alloca_key;
  ptr_lock = alloca_lock;
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
// Function renaming to append the function name with softboundcets_
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
// softboundcets_pseudo_main which is subsequently renamed to
// softboundcets_pseudo_main in the first case by renameFunction
//

void SoftBoundCETSPass::transformMain(Module& module) {
    
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
                              "softboundcets_pseudo_main");

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

bool SoftBoundCETSPass::isFuncDefSoftBound(const std::string &str) {
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
    //m_func_wrappers_available["puts"] = true;
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
    m_func_wrappers_available["qsort"] = true;

    m_func_def_softbound["puts"] = true;
    m_func_def_softbound["__softboundcets_intermediate"]= true;
    m_func_def_softbound["__softboundcets_dummy"] = true;
    m_func_def_softbound["__softboundcets_print_metadata"] = true;
    m_func_def_softbound["__softboundcets_introspect_metadata"] = true;
    m_func_def_softbound["__softboundcets_copy_metadata"] = true;
    m_func_def_softbound["__softboundcets_allocate_shadow_stack_space"] = true;
    m_func_def_softbound["__softboundcets_load_base_shadow_stack"] = true;
    m_func_def_softbound["__softboundcets_load_bound_shadow_stack"] = true;
    m_func_def_softbound["__softboundcets_load_key_shadow_stack"] = true;
    m_func_def_softbound["__softboundcets_load_lock_shadow_stack"] = true;
    m_func_def_softbound["__softboundcets_store_base_shadow_stack"] = true;      
    m_func_def_softbound["__softboundcets_store_bound_shadow_stack"] = true;      
    m_func_def_softbound["__softboundcets_store_key_shadow_stack"] = true;      
    m_func_def_softbound["__softboundcets_store_lock_shadow_stack"] = true;      
    m_func_def_softbound["__softboundcets_deallocate_shadow_stack_space"] = true;

    m_func_def_softbound["__softboundcets_trie_allocate"] = true;
    m_func_def_softbound["__shrinkBounds"] = true;
    m_func_def_softbound["__softboundcets_memcopy_check"] = true;

    m_func_def_softbound["__softboundcets_spatial_load_dereference_check"] = true;

    m_func_def_softbound["__softboundcets_spatial_store_dereference_check"] = true;
    m_func_def_softbound["__softboundcets_spatial_call_dereference_check"] = true;
    m_func_def_softbound["__softboundcets_temporal_load_dereference_check"] = true;
    m_func_def_softbound["__softboundcets_temporal_store_dereference_check"] = true;
    m_func_def_softbound["__softboundcets_stack_memory_allocation"] = true;
    m_func_def_softbound["__softboundcets_memory_allocation"] = true;
    m_func_def_softbound["__softboundcets_get_global_lock"] = true;
    m_func_def_softbound["__softboundcets_add_to_free_map"] = true;
    m_func_def_softbound["__softboundcets_check_remove_from_free_map"] = true;
    m_func_def_softbound["__softboundcets_allocation_secondary_trie_allocate"] = true;
    m_func_def_softbound["__softboundcets_allocation_secondary_trie_allocate_range"] = true;
    m_func_def_softbound["__softboundcets_allocate_lock_location"] = true;
    m_func_def_softbound["__softboundcets_memory_deallocation"] = true;
    m_func_def_softbound["__softboundcets_stack_memory_deallocation"] = true;

    m_func_def_softbound["__softboundcets_metadata_load_vector"] = true;
    m_func_def_softbound["__softboundcets_metadata_store_vector"] = true;
    
    m_func_def_softbound["__softboundcets_metadata_load"] = true;
    m_func_def_softbound["__softboundcets_metadata_store"] = true;
    m_func_def_softbound["__hashProbeAddrOfPtr"] = true;
    m_func_def_softbound["__memcopyCheck"] = true;
    m_func_def_softbound["__memcopyCheck_i64"] = true;

    m_func_def_softbound["__softboundcets_global_init"] = true;      
    m_func_def_softbound["__softboundcets_init"] = true;      
    m_func_def_softbound["__softboundcets_abort"] = true;      
    m_func_def_softbound["__softboundcets_printf"] = true;
    
    m_func_def_softbound["__softboundcets_stub"] = true;
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

void SoftBoundCETSPass::identifyFuncToTrans(Module& module) {
    
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
// Method: introduceGlobalLockFunction()
//
// Description:
//
// This function introduces the function to retrieve the lock for the
// global variables. This function should be introduced only once for
// every function in the entry block of the function.
//

Value* SoftBoundCETSPass::introduceGlobalLockFunction(Instruction* insert_at){

  SmallVector<Value*, 8> args;
  Value* call_inst = CallInst::Create(m_temporal_global_lock_function, 
                                      args, "", insert_at);
  return call_inst;
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
SoftBoundCETSPass:: castToVoidPtr(Value* operand, Instruction* insert_at) {

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

bool SoftBoundCETSPass::hasPtrArgRetType(Function* func) {
   
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
void SoftBoundCETSPass::addStoreBaseBoundFunc(Value* pointer_dest, 
                                              Value* pointer_base, 
                                              Value* pointer_bound, 
                                              Value* pointer_key,
                                              Value* pointer_lock,
                                              Value* pointer,
                                              Value* size_of_type, 
                                              Instruction* insert_at) {

  Value* pointer_base_cast = NULL;
  Value* pointer_bound_cast = NULL;

  
  Value* pointer_dest_cast = castToVoidPtr(pointer_dest, insert_at);

  if (spatial_safety) {
    pointer_base_cast = castToVoidPtr(pointer_base, insert_at);
    pointer_bound_cast = castToVoidPtr(pointer_bound, insert_at);
  }
  //  Value* pointer_cast = castToVoidPtr(pointer, insert_at);
    
  SmallVector<Value*, 8> args;

  args.push_back(pointer_dest_cast);

  if (spatial_safety) {
    args.push_back(pointer_base_cast);
    args.push_back(pointer_bound_cast);
  }

  if (temporal_safety) {
    args.push_back(pointer_key);
    args.push_back(pointer_lock);
  }
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

void SoftBoundCETSPass::handlePHIPass1(PHINode* phi_node) {

  // Not a Pointer PHINode, then just return
  if (!isa<PointerType>(phi_node->getType()))
    return;

  unsigned num_incoming_values = phi_node->getNumIncomingValues();

  if (spatial_safety) {
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

  if (temporal_safety) {
    PHINode* key_phi_node = 
      PHINode::Create(Type::getInt64Ty(phi_node->getType()->getContext()),
                      num_incoming_values,
                      "phi.key", phi_node);

    PHINode* lock_phi_node = PHINode::Create(m_void_ptr_type, 
                                             num_incoming_values,
                                             "phi.lock", phi_node);
    
    associateKeyLock(phi_node, key_phi_node, lock_phi_node);
  }

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

void SoftBoundCETSPass::handlePHIPass2(PHINode* phi_node) {

  // Work to be done only for pointer PHINodes.
  if (!isa<PointerType>(phi_node->getType())) 
    return;

  PHINode* base_phi_node = NULL;
  PHINode* bound_phi_node  = NULL;
  PHINode* key_phi_node = NULL;
  PHINode* lock_phi_node = NULL;

  // Obtain the metada PHINodes 
  if (spatial_safety) {
    base_phi_node = dyn_cast<PHINode>(getAssociatedBase(phi_node));
    bound_phi_node = dyn_cast<PHINode>(getAssociatedBound(phi_node));
  }

  if (temporal_safety) {
    key_phi_node = dyn_cast<PHINode>(getAssociatedKey(phi_node));
    Value* func_lock = getAssociatedFuncLock(phi_node);
    lock_phi_node= dyn_cast<PHINode>(getAssociatedLock(phi_node, func_lock));
  }
  
  std::map<Value*, Value*> globals_base;
  std::map<Value*, Value*> globals_bound;
  std::map<Value*, Value*> globals_key;
  std::map<Value*, Value*> globals_lock;
 
  unsigned num_incoming_values = phi_node->getNumIncomingValues();
  for (unsigned m = 0; m < num_incoming_values; m++) {

    Value* incoming_value = phi_node->getIncomingValue(m);
    BasicBlock* bb_incoming = phi_node->getIncomingBlock(m);

    if (isa<ConstantPointerNull>(incoming_value)) {
      if (spatial_safety) {
        base_phi_node->addIncoming(m_void_null_ptr, bb_incoming);
        bound_phi_node->addIncoming(m_void_null_ptr, bb_incoming);
      }
      if (temporal_safety) {
        key_phi_node->addIncoming(m_constantint64ty_zero, bb_incoming);
        lock_phi_node->addIncoming(m_void_null_ptr, bb_incoming);
      }
      continue;
    } // ConstantPointerNull ends
   
    // The incoming vlaue can be a UndefValue
    if (isa<UndefValue>(incoming_value)) {        
      if (spatial_safety) {
        base_phi_node->addIncoming(m_void_null_ptr, bb_incoming);
        bound_phi_node->addIncoming(m_void_null_ptr, bb_incoming);
      }
      if (temporal_safety) {
        key_phi_node->addIncoming(m_constantint64ty_zero, bb_incoming);
        lock_phi_node->addIncoming(m_void_null_ptr, bb_incoming);
      }      
      continue;
    } // UndefValue ends
      
    Value* incoming_value_base = NULL;
    Value* incoming_value_bound = NULL;
    Value* incoming_value_key  = NULL;
    Value* incoming_value_lock = NULL;
    
    // handle global variables      
    GlobalVariable* gv = dyn_cast<GlobalVariable>(incoming_value);
    if (gv) {
      if (spatial_safety) {
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
      } // spatial safety ends
      
      if (temporal_safety) {
        incoming_value_key = m_constantint64ty_one;
        Value* tmp_lock = 
          m_func_global_lock[phi_node->getParent()->getParent()->getName()];
        incoming_value_lock = tmp_lock;
      }
    } // global variable ends
      
    // handle constant expressions 
    Constant* given_constant = dyn_cast<Constant>(incoming_value);
    if (given_constant) {
      if (spatial_safety) {
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
      } // spatial safety ends

      if (temporal_safety) {        
        incoming_value_key = m_constantint64ty_one;
        Value* tmp_lock = 
          m_func_global_lock[phi_node->getParent()->getParent()->getName()];
        incoming_value_lock = tmp_lock;
      }
    }
    
    // handle values having map based pointer base and bounds 
    if(spatial_safety && checkBaseBoundMetadataPresent(incoming_value)){
      incoming_value_base = getAssociatedBase(incoming_value);
      incoming_value_bound = getAssociatedBound(incoming_value);
    }

    if(temporal_safety && checkKeyLockMetadataPresent(incoming_value)){
      incoming_value_key = getAssociatedKey(incoming_value);
      Value* func_lock = getAssociatedFuncLock(phi_node);
      incoming_value_lock = getAssociatedLock(incoming_value, func_lock);
    }
    
    if(spatial_safety){
      assert(incoming_value_base &&
             "[handlePHIPass2] incoming_value doesn't have base?");
      assert(incoming_value_bound && 
             "[handlePHIPass2] incoming_value doesn't have bound?");
      
      base_phi_node->addIncoming(incoming_value_base, bb_incoming);
      bound_phi_node->addIncoming(incoming_value_bound, bb_incoming);
    }

    if(temporal_safety){
      assert(incoming_value_key && 
             "[handlePHIPass2] incoming_value doesn't have key?");
      assert(incoming_value_lock && 
             "[handlePHIPass2] incoming_value doesn't have lock?");

      key_phi_node->addIncoming(incoming_value_key, bb_incoming);
      lock_phi_node->addIncoming(incoming_value_lock, bb_incoming);
    }      
  } // Iterating over incoming values ends 

  if(spatial_safety){
    assert(base_phi_node && "[handlePHIPass2] base_phi_node null?");
    assert(bound_phi_node && "[handlePHIPass2] bound_phi_node null?");
  }  
  if(temporal_safety){
    assert(key_phi_node && "[handlePHIPass2] key_phi_node null?");
    assert(lock_phi_node && "[handlePHIPass2] lock_phi_node null?");
  }  
  unsigned n_values = phi_node->getNumIncomingValues();
  if(spatial_safety){
    unsigned n_base_values = base_phi_node->getNumIncomingValues();
    unsigned n_bound_values = bound_phi_node->getNumIncomingValues();    
    assert((n_values == n_base_values)  && 
           "[handlePHIPass2] number of values different for base");
    assert((n_values == n_bound_values) && 
           "[handlePHIPass2] number of values different for bound");
  }
  
  if(temporal_safety){
    unsigned n_key_values = key_phi_node->getNumIncomingValues();
    unsigned n_lock_values = lock_phi_node->getNumIncomingValues();
    assert((n_values == n_key_values)  && 
           "[handlePHIPass2] number of values different for key");
    assert((n_values == n_lock_values) &&
           "[handlePHIPass2] number of values different for lock");
  }  
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
SoftBoundCETSPass:: propagateMetadata(Value* pointer_operand, 
                                      Instruction* inst, 
                                      int instruction_type){

  // Need to just propagate the base and bound here if I am not
  // shrinking bounds
  if (spatial_safety) {
    if(checkBaseBoundMetadataPresent(inst)){
      // Metadata added to the map in the first pass
      return;
    }
  }
  if (temporal_safety) {
    if (checkKeyLockMetadataPresent(inst)){
      // Metadata added to the map in the first pass
      return;
    }    
  }

  if(isa<ConstantPointerNull>(pointer_operand)) {
    if(spatial_safety){
      associateBaseBound(inst, m_void_null_ptr, m_void_null_ptr);
    }
    if(temporal_safety){
      associateKeyLock(inst, m_constantint64ty_zero, m_void_null_ptr);
    }
    return;
  }

  if (spatial_safety) {
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
  }// Spatial safety ends here

  if(temporal_safety){
    if(checkKeyLockMetadataPresent(pointer_operand)){      
      Value* tmp_key = getAssociatedKey(pointer_operand);
      Value* func_lock = getAssociatedFuncLock(inst);
      Value* tmp_lock = getAssociatedLock(pointer_operand, func_lock);
      associateKeyLock(inst, tmp_key, tmp_lock);
    }
    else{      
      if(isa<Constant>(pointer_operand)){
        Value* func_lock = 
          m_func_global_lock[inst->getParent()->getParent()->getName()];
        associateKeyLock(inst, m_constantint64ty_one, func_lock);
      }
    }
  } // Temporal safety ends here
}

//
// Method: handleBitCast
//
// Description: Propagate metadata from source to destination with
// pointer bitcast operations.

void SoftBoundCETSPass::handleBitCast(BitCastInst* bitcast_inst) {

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
SoftBoundCETSPass::getGlobalVariableBaseBound(Value* operand, 
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


void SoftBoundCETSPass:: introduceShadowStackAllocation(CallInst* call_inst){
    
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
SoftBoundCETSPass::introduceShadowStackStores(Value* ptr_value, 
                                              Instruction* insert_at, 
                                              int arg_no){
  if(!isa<PointerType>(ptr_value->getType()))
    return;
  

  Value* argno_value;    
  argno_value = 
    ConstantInt::get(Type::getInt32Ty(ptr_value->getType()->getContext()), 
                     arg_no, false);

  if(spatial_safety){
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

  if(temporal_safety){
    Value* ptr_key = getAssociatedKey(ptr_value);    
    Value* func_lock = getAssociatedFuncLock(insert_at);
    Value* ptr_lock = getAssociatedLock(ptr_value, func_lock);
 
    SmallVector<Value*, 8> args;
    args.clear();
    args.push_back(ptr_key);
    args.push_back(argno_value);
    CallInst::Create(m_shadow_stack_key_store, args, "", insert_at);

    args.clear();
    args.push_back(ptr_lock);
    args.push_back(argno_value);
    CallInst::Create(m_shadow_stack_lock_store, args, "", insert_at);
  }    
}

//
// Method: introduceShadowStackDeallocation
//
// Description: This function inserts a call to the C-handler that
// deallocates the shadow stack space on function exit.
  

void 
SoftBoundCETSPass:: introduceShadowStackDeallocation(CallInst* call_inst, 
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
int SoftBoundCETSPass:: getNumPointerArgsAndReturn(CallInst* call_inst){

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
SoftBoundCETSPass::introduceShadowStackLoads(Value* ptr_value, 
                                             Instruction* insert_at, 
                                             int arg_no){
    
  if (!isa<PointerType>(ptr_value->getType()))
    return;
      
  Value* argno_value;    
  argno_value = 
    ConstantInt::get(Type::getInt32Ty(ptr_value->getType()->getContext()), 
                     arg_no, false);
    
  SmallVector<Value*, 8> args;
  if(spatial_safety){
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
  
  if(temporal_safety){
    args.clear();
    args.push_back(argno_value);
    Value* key = CallInst::Create(m_shadow_stack_key_load, args, "", insert_at);

    args.clear();
    args.push_back(argno_value);
    Value* lock = CallInst::Create(m_shadow_stack_lock_load, args, "", 
                                   insert_at);
    associateKeyLock(ptr_value, key, lock);
  }    
}
//
// Method: dissociateKeyLock
//
// Description: This function removes the key lock metadata associated
// with the pointer operand in the SoftBound/CETS maps.
void SoftBoundCETSPass:: dissociateKeyLock(Value* pointer_operand){

    if(m_pointer_key.count(pointer_operand)){
      m_pointer_key.erase(pointer_operand);
    }
    if(m_pointer_lock.count(pointer_operand)){
      m_pointer_lock.erase(pointer_operand);
    }
    assert((m_pointer_key.count(pointer_operand) == 0) && 
           "dissociating key failed");    
    assert((m_pointer_lock.count(pointer_operand) == 0) && 
           "dissociating lock failed");
}
//
// Method: dissociateBaseBound
//
// Description: This function removes the base/bound metadata
// associated with the pointer operand in the SoftBound/CETS maps.

void SoftBoundCETSPass::dissociateBaseBound(Value* pointer_operand){

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
// Method: associateKeyLock
//
// Description: This function associates the key lock with the pointer
// operand in the SoftBound/CETS maps.

void SoftBoundCETSPass::associateKeyLock(Value* pointer_operand, 
                                         Value* pointer_key, 
                                         Value* pointer_lock){
  
  if(m_pointer_key.count(pointer_operand)){
    dissociateKeyLock(pointer_operand);
  }
  
  if(pointer_key->getType() != m_key_type)
    assert(0 && "key does not the right type ");

  if(pointer_lock->getType() != m_void_ptr_type)
    assert(0 && "lock does not have the right type");

  m_pointer_key[pointer_operand] = pointer_key;
  if (m_pointer_lock.count(pointer_operand))
    assert(0 && "lock already has an entry in the map");
  
  m_pointer_lock[pointer_operand] = pointer_lock; 
}

//
// Method: associateBaseBound
//
// Description: This function associates the base bound with the
// pointer operand in the SoftBound/CETS maps.


void SoftBoundCETSPass::associateBaseBound(Value* pointer_operand, 
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

void SoftBoundCETSPass::handleSelect(SelectInst* select_ins, int pass) {

  if (!isa<PointerType>(select_ins->getType())) 
    return;
    
  Value* condition = select_ins->getOperand(0);
  Value* operand_base[2];
  Value* operand_bound[2];    
  Value* operand_key[2];
  Value* operand_lock[2];

  for(unsigned m = 0; m < 2; m++) {
    Value* operand = select_ins->getOperand(m+1);
    
    if (spatial_safety) {
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
    } //Spatial safety ends
    
    if (temporal_safety){
      operand_key[m] = NULL;
      operand_lock[m] = NULL;
      if (checkKeyLockMetadataPresent(operand)){
        operand_key[m] = getAssociatedKey(operand);
        Value* func_lock = getAssociatedFuncLock(select_ins);
        operand_lock[m] = getAssociatedLock(operand, func_lock);
      }

      if (isa<ConstantPointerNull>(operand) && 
          !checkKeyLockMetadataPresent(operand)){
        operand_key[m] = m_constantint64ty_zero;
        operand_lock[m] = m_void_null_ptr;
      }

      Constant* given_constant = dyn_cast<Constant>(operand);
      if(given_constant){
        operand_key[m] = m_constantint64ty_one;
        operand_lock[m] = 
          m_func_global_lock[select_ins->getParent()->getParent()->getName()];
      }

      assert(operand_key[m] != NULL && 
             "operand doesn't have key with select?");
      assert(operand_lock[m] != NULL && 
             "operand doesn't have lock with select?");
    } // Temporal safety ends
  
  } // for loop ends
    
  if (spatial_safety) {
      
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

  if(temporal_safety){

    SelectInst* select_key = SelectInst::Create(condition, 
                                                operand_key[0], 
                                                operand_key[1], 
                                                "select.key",
                                                select_ins);
    
    SelectInst* select_lock = SelectInst::Create(condition, 
                                                 operand_lock[0], 
                                                 operand_lock[1], 
                                                 "select.lock",
                                                 select_ins);
    associateKeyLock(select_ins, select_key, select_lock);
  }
}

//
// Method: checkBaseBoundMetadataPresent()
//
// Description:
// Checks if the metadata is present in the SoftBound/CETS maps.

bool 
SoftBoundCETSPass::checkBaseBoundMetadataPresent(Value* pointer_operand){

  if(m_pointer_base.count(pointer_operand) && 
     m_pointer_bound.count(pointer_operand)){
      return true;
  }
  return false;
}

//
// Method: checkKeyLockMetadataPresent()
//
// Description:
// Checks if the metadata is present in the SoftBound/CETS maps.


bool 
SoftBoundCETSPass::checkKeyLockMetadataPresent(Value* pointer_operand){

  if(m_pointer_key.count(pointer_operand) && 
     m_pointer_lock.count(pointer_operand)){
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

void SoftBoundCETSPass:: handleReturnInst(ReturnInst* ret){

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
SoftBoundCETSPass::handleGlobalSequentialTypeInitializer(Module& module, 
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
            
            Value* operand_key = NULL;
            Value* operand_lock = NULL;
            if(temporal_safety){
              operand_key = m_constantint_one;
              operand_lock = 
                introduceGlobalLockFunction(init_function_terminator);
            }
            addStoreBaseBoundFunc(addr_of_ptr, operand_base, operand_bound, 
                                  operand_key, operand_lock, initializer_opd, 
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
    
    Value* operand_key = NULL;
    Value* operand_lock = NULL;
    if(temporal_safety){
      operand_key = m_constantint_one;
      operand_lock = 
        introduceGlobalLockFunction(init_function_terminator);
    }
    
    addStoreBaseBoundFunc(gv, initializer_base, initializer_bound, operand_key,
                          operand_lock, initializer, initializer_size, 
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
SoftBoundCETSPass::
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
      
      Value* operand_key = NULL;
      Value* operand_lock = NULL;
      
      Constant* addr_of_ptr = NULL;
      
      if(temporal_safety){
        operand_key = m_constantint_one;
        operand_lock = introduceGlobalLockFunction(first);  
      }
      
      if(spatial_safety){
        Constant* given_constant = dyn_cast<Constant>(initializer_opd);
        assert(given_constant && 
               "[handleGlobalStructTypeInitializer] not a constant?");
        
        getConstantExprBaseBound(given_constant, operand_base, operand_bound);   
      }
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
                            operand_bound, operand_key, 
                            operand_lock, initializer_opd, 
                            initializer_size, first);
      
      //    if(spatial_safety){
        indices_addr_ptr.pop_back();
        length--;
        //      }

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

void SoftBoundCETSPass::getConstantExprBaseBound(Constant* given_constant, 
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
// Methods: getAssociatedBase, getAssociatedBound, getAssociatedKey,
// getAssociatedLock
//
// Description: Retrieves the metadata from SoftBound/CETS maps 
//

Value* 
SoftBoundCETSPass::getAssociatedBase(Value* pointer_operand) {
    
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
SoftBoundCETSPass::getAssociatedBound(Value* pointer_operand) {

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


Value* 
SoftBoundCETSPass::getAssociatedKey(Value* pointer_operand) {
    
  if(!temporal_safety){
    return NULL;
  }

  if(isa<Constant>(pointer_operand)){
    return m_constantint_one;
  }

  if(!m_pointer_key.count(pointer_operand)){
    pointer_operand->dump();
  }
  assert(m_pointer_key.count(pointer_operand) && 
         "Key absent. Try compiling with -simplifycfg option?");
    
  Value* pointer_key = m_pointer_key[pointer_operand];
  assert(pointer_key && "key present in the map but null?");

  if(pointer_key->getType() != m_key_type)
    assert(0 && "key in the map does not have the right type");

  return pointer_key;
}

Value* 
SoftBoundCETSPass::getAssociatedLock(Value* pointer_operand, Value* func_lock){
    
  if(!temporal_safety){
    return NULL;
  }

  if(isa<GlobalVariable>(pointer_operand)){
    return func_lock;
  }

  if(isa<Constant>(pointer_operand)){
    return func_lock;
  }

  if(!m_pointer_lock.count(pointer_operand)){
    pointer_operand->dump();
  }
  assert(m_pointer_lock.count(pointer_operand) && 
         "Lock absent. Try compiling with -simplifycfg option?");
    
  Value* pointer_lock = m_pointer_lock[pointer_operand];
  assert(pointer_lock && "lock present in the map but null?");

  if(pointer_lock->getType() != m_void_ptr_type)
    assert(0 && "lock in the map does not have the right type");

  return pointer_lock;
}

// 
// Method: transformFunctionName
//
// Description:
//
// This function returns the transformed name for the function. This
// function appends softboundcets_ to the input string.


std::string 
SoftBoundCETSPass::transformFunctionName(const std::string &str) { 

  // If the function name starts with this prefix, don't just
  // concatenate, but instead transform the string
  return "softboundcets_" + str; 
}


void SoftBoundCETSPass::addMemcopyMemsetCheck(CallInst* call_inst, 
                                              Function* called_func) {

  if(DISABLE_MEMCOPYCHECK) 
    return;

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
    if(spatial_safety){
      Value* dest_base = getAssociatedBase(dest_ptr);
      Value* dest_bound =getAssociatedBound(dest_ptr);
      
      Value* src_base = getAssociatedBase(src_ptr);
      Value* src_bound = getAssociatedBound(src_ptr);

      args.push_back(dest_base);
      args.push_back(dest_bound);
      
      args.push_back(src_base);
      args.push_back(src_bound);
      
    }

    if(temporal_safety){
      Value* dest_key = getAssociatedKey(dest_ptr);
      Value* func_lock = getAssociatedFuncLock(call_inst);
      Value* dest_lock = getAssociatedLock(dest_ptr, func_lock);
      
      Value* src_key = getAssociatedKey(src_ptr);
      Value* src_lock = getAssociatedLock(src_ptr, func_lock);

      args.push_back(dest_key);
      args.push_back(dest_lock);
      args.push_back(src_key);
      args.push_back(src_lock);

    }
    
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
    
    if(spatial_safety){
      Value* dest_base = getAssociatedBase(dest_ptr);
      Value* dest_bound = getAssociatedBound(dest_ptr);
      args.push_back(dest_base);
      args.push_back(dest_bound);   
    }

    if(temporal_safety){
      Value* dest_key = getAssociatedKey(dest_ptr);
      Value* func_lock = getAssociatedFuncLock(call_inst);
      Value* dest_lock = getAssociatedLock(dest_ptr, func_lock);
      
      args.push_back(dest_key);
      args.push_back(dest_lock);
    }    
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
 

Value* SoftBoundCETSPass:: getSizeOfType(Type* input_type) {

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
SoftBoundCETSPass::isStructOperand(Value* pointer_operand){
  
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

  M->getOrInsertFunction("__softboundcets_dummy", Type::getVoidTy(Context), NULL);
  CallInst::Create(M->getFunction("__softboundcets_dummy"), "", UI);
  
  M->getOrInsertFunction ("__softboundcets_abort", Type::getVoidTy (Context), NULL);
  CallInst::Create (M->getFunction ("__softboundcets_abort"), "", UI);

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
SoftBoundCETSPass::addLoadStoreChecks(Instruction* load_store, 
                                      std::map<Value*, int>& FDCE_map) {

  if(!spatial_safety)
    return;

  SmallVector<Value*, 8> args;
  Value* pointer_operand = NULL;
    
  if(isa<LoadInst>(load_store)) {
    if(!LOADCHECKS)
      return;

    LoadInst* ldi = dyn_cast<LoadInst>(load_store);
    assert(ldi && "not a load instruction");
    pointer_operand = ldi->getPointerOperand();
  }
    
  if(isa<StoreInst>(load_store)){
    if(!STORECHECKS)
      return;
      
    StoreInst* sti = dyn_cast<StoreInst>(load_store);
    assert(sti && "not a store instruction");
    // The pointer where the element is being stored is the second
    // operand
    pointer_operand = sti->getOperand(1);
  }
    
  assert(pointer_operand && "pointer operand null?");

  if(!disable_spatial_check_opt){
    if(eliminate_struct_checks){
      if(isStructOperand(pointer_operand)){
        return;
      }    
    }
    
    // If it is a null pointer which is being loaded, then it must seg
    // fault, no dereference check here
    
    
    if(isa<ConstantPointerNull>(pointer_operand))
      return;

    // Find all uses of pointer operand, then check if it dominates and
    //if so, make a note in the map
    
    GlobalVariable* gv = dyn_cast<GlobalVariable>(pointer_operand);    
    if(gv && GLOBALCONSTANTOPT && !isa<SequentialType>(gv->getType())) {
      return;
    }
    
    if(BOUNDSCHECKOPT) {
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
  }
    
  Value* tmp_base = NULL;
  Value* tmp_bound = NULL;
    
  Constant* given_constant = dyn_cast<Constant>(pointer_operand);    
  if(given_constant ) {
    if(GLOBALCONSTANTOPT)
      return;      

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
// Method: optimizeGlobalAndStackVariables
//
// Description: This function elides temporal safety checks for stack
// and global variables.


bool 
SoftBoundCETSPass::
optimizeGlobalAndStackVariableChecks(Instruction* load_store) {
    
  Value* pointer_operand = NULL;
  if(isa<LoadInst>(load_store)){
    pointer_operand = load_store->getOperand(0);
  } else{
    pointer_operand = load_store->getOperand(1);
  }

  while(true) {      
    if(isa<AllocaInst>(pointer_operand)){        
      if(STACKTEMPORALCHECKOPT){
        return true;
      } else{
        return false;
      }
    }

    if(isa<GlobalVariable>(pointer_operand)){        
      if(GLOBALTEMPORALCHECKOPT){
        return true;
      } else{
        return false;
      }
    }
      
    if(isa<BitCastInst>(pointer_operand)){
      BitCastInst* bitcast_inst = dyn_cast<BitCastInst>(pointer_operand);
      pointer_operand = bitcast_inst->getOperand(0);        
      continue;
    }

    if(isa<GetElementPtrInst>(pointer_operand)){
      GetElementPtrInst* gep = dyn_cast<GetElementPtrInst>(pointer_operand);
      pointer_operand = gep->getOperand(0); 
      continue;
    } else{
      return false;
    }
  }
}

//
// Method: bbTemporalCheckElimination
//
// Description: This function eliminates the redundant temporal safety
// checks in the basic block
//
// Comments: Describe the algorithm here

bool 
SoftBoundCETSPass::bbTemporalCheckElimination(Instruction* load_store, 
                                              std::map<Value*, int>& BBTCE_map){
    
  if(!BBDOMTEMPORALCHECKOPT)
    return false;

  if(BBTCE_map.count(load_store))
    return true;

  // Check if the operand is a getelementptr, then get the first
  // operand and check for all other load/store instructions in the
  // current basic block and check if they are pointer operands are
  // getelementptrs. If so, check if it is same the pointer being
  // checked now
    
  Value* pointer_operand = getPointerLoadStore(load_store);

  Value* gep_source = NULL;
  if (isa<GetElementPtrInst>(pointer_operand)) {
    GetElementPtrInst* ptr_gep = cast<GetElementPtrInst>(pointer_operand);
    gep_source = ptr_gep->getOperand(0);
  } else {
    gep_source = pointer_operand;
  }
    
  // Iterate over all other instructions in this basic block and look
  // for gep_instructions with the same source 
  BasicBlock* bb_curr = load_store->getParent();
  assert(bb_curr && "bb null?");

  Instruction* next_inst = getNextInstruction(load_store);
  BasicBlock* next_inst_bb = next_inst->getParent();
  while((next_inst_bb == bb_curr) && 
        (next_inst != bb_curr->getTerminator())) {

    if(isa<CallInst>(next_inst) && OPAQUECALLS)
      break;
      
    if(checkLoadStoreSourceIsGEP(next_inst, gep_source)){
      BBTCE_map[next_inst] = 1;
    }

    next_inst = getNextInstruction(next_inst);
    next_inst_bb = next_inst->getParent();
  }
  return false;
}
//
// Method:getPointerLoadStore
//
// Description: This function obtains the pointer operand which is
// being dereferenced in the memory access.

Value* 
SoftBoundCETSPass::getPointerLoadStore(Instruction* load_store) {

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

// 
// Method : checkLoadSourceIsGEP
//
// Description: This function is used to optimize temporal checks by
// identifying the root object of the pointer being dereferenced.  If
// the pointer being deferenced is a bitcast or a GEP instruction then
// the source of GEP/bitcast is noted and checked to ascertain whether
// any check to the root object has been performed and not killed.
// 
// Comments:
//
// TODO: A detailed algorithm here

bool 
SoftBoundCETSPass::checkLoadStoreSourceIsGEP(Instruction* load_store, 
                                             Value* gep_source){

  Value* pointer_operand = NULL;

  if(!isa<LoadInst>(load_store) && !isa<StoreInst>(load_store))
    return false;

  if(isa<LoadInst>(load_store)){
    pointer_operand = load_store->getOperand(0);
  }

  if(isa<StoreInst>(load_store)){
    pointer_operand = load_store->getOperand(1);
  }

  assert(pointer_operand && "pointer_operand null?");

  if(!isa<GetElementPtrInst>(pointer_operand))
    return false;

  GetElementPtrInst* gep_ptr = dyn_cast<GetElementPtrInst>(pointer_operand);
  assert(gep_ptr && "gep_ptr null?"); 

  Value* gep_ptr_operand = gep_ptr->getOperand(0);

  if(gep_ptr_operand == gep_source)    
    return true;

  return false;
}

// 
// Method: funcTemporalCheckElimination
//
// Description: This function elides temporal checks for by performing
// root object identification at the function level.



bool 
SoftBoundCETSPass::funcTemporalCheckElimination(Instruction* load_store, 
                                                std::map<Value*, int>& FTCE_map) {

  if(!FUNCDOMTEMPORALCHECKOPT)
    return false;

  if(FTCE_map.count(load_store))
    return true;

  Value* pointer_operand = getPointerLoadStore(load_store);

  Value* gep_source = NULL;
  if(isa<GetElementPtrInst>(pointer_operand)){

    GetElementPtrInst* ptr_gep = dyn_cast<GetElementPtrInst>(pointer_operand);
    assert(ptr_gep && "[bbTemporalCheckElimination] gep_inst null?");
    gep_source = ptr_gep->getOperand(0);
  }
  else {
    gep_source = pointer_operand;
  }

  BasicBlock* bb_curr = load_store->getParent();
  assert(bb_curr && "bb null?");
          
  std::set<BasicBlock*> bb_visited;
  std::queue<BasicBlock*> bb_worklist;
      
  bb_worklist.push(bb_curr);
  BasicBlock* bb = NULL;
  while(bb_worklist.size() != 0){
      
    bb = bb_worklist.front();
    assert(bb && "Not a BasicBlock?");
      
    bb_worklist.pop();
    if(bb_visited.count(bb)){
      continue;
    }
    bb_visited.insert(bb);

    bool break_flag = false;

    // Iterating over the successors and adding the successors to the
    // work list

    // if this is the current basic block under question 
    if(bb == bb_curr) {
      // bbTemporalCheckElimination should handle this 
      Instruction* next_inst = getNextInstruction(load_store);
      BasicBlock* next_inst_bb = next_inst->getParent();
      while((next_inst_bb == bb_curr) && 
            (next_inst != bb_curr->getTerminator())) {

        if(isa<CallInst>(next_inst) && OPAQUECALLS){
          break_flag = true;
          break;
        }
          
        if(checkLoadStoreSourceIsGEP(next_inst, gep_source)){
          if(m_dominator_tree->dominates(load_store, next_inst)){              
            FTCE_map[next_inst] = 1;
          }
        }
          
        next_inst = getNextInstruction(next_inst);
        next_inst_bb = next_inst->getParent();
      }
    } else {
      for(BasicBlock::iterator i = bb->begin(), ie = bb->end(); i != ie; ++i){
        Instruction* new_inst = dyn_cast<Instruction>(i);
        if(isa<CallInst>(new_inst) && OPAQUECALLS){
          break_flag = true;
          break;
        }
          
        if(checkLoadStoreSourceIsGEP(new_inst, gep_source)){

          if(m_dominator_tree->dominates(load_store, new_inst)){
            FTCE_map[new_inst] = 1;
          }
        }          
      } // Iterating over the instructions in the basic block ends
    }

    for(succ_iterator si = succ_begin(bb), se = succ_end(bb); si != se; ++si) {
        
      if(break_flag)
        break;
        
      BasicBlock* next_bb = cast<BasicBlock>(*si);
      bb_worklist.push(next_bb);
    }      
  } // Worklist algorithm ends
  return false;
}


bool 
SoftBoundCETSPass::optimizeTemporalChecks(Instruction* load_store, 
                                          std::map<Value*, int>& BBTCE_map, 
                                          std::map<Value*, int>& FTCE_map) {
  
  if(optimizeGlobalAndStackVariableChecks(load_store))
    return true;

  if(bbTemporalCheckElimination(load_store, BBTCE_map))
    return true;

  if(funcTemporalCheckElimination(load_store, FTCE_map))
    return true;

  return false;

}


void 
SoftBoundCETSPass::addTemporalChecks(Instruction* load_store, 
                                     std::map<Value*,int>& BBTCE_map, 
                                     std::map<Value*,int>& FTCE_map) {
  
  SmallVector<Value*, 8> args;
  Value* pointer_operand = NULL;

  if(!temporal_safety)
    return;
  
  
  if(!disable_temporal_check_opt){
    if(optimizeTemporalChecks(load_store, BBTCE_map, FTCE_map))
      return;
  }

  if(isa<LoadInst>(load_store)) {
    if(!TEMPORALLOADCHECKS)
      return;
      
    LoadInst* ldi = dyn_cast<LoadInst>(load_store);
    assert(ldi && "not a load instruction");
    pointer_operand = ldi->getPointerOperand();
  }
  
  if(isa<StoreInst>(load_store)){
    if(!TEMPORALSTORECHECKS)
      return;
    
    StoreInst* sti = dyn_cast<StoreInst>(load_store);
    assert(sti && "not a store instruction");
    // The pointer where the element is being stored is the second
    // operand
    pointer_operand = sti->getOperand(1);
  }
  
  assert(pointer_operand && "pointer_operand null?");

  if(!disable_temporal_check_opt){
    if(isa<ConstantPointerNull>(pointer_operand))
      return;
    
    // Do not insert checks for globals and constant expressions
    GlobalVariable* gv = dyn_cast<GlobalVariable>(pointer_operand);    
    if(gv) {
      return;
    }
    Constant* given_constant = dyn_cast<Constant>(pointer_operand);
    if(given_constant)
      return;
  }
  
  if(!disable_temporal_check_opt){
    /* Find all uses of pointer operand, then check if it
     * dominates and if so, make a note in the map
     */
    
    if(TEMPORALBOUNDSCHECKOPT) {
      /* Enable dominator based dereference check optimization only
       * when suggested 
       */
      
      if(FTCE_map.count(load_store)) {
        return;
      }
      
      /* iterate over the uses */            
      for(Value::use_iterator ui = pointer_operand->use_begin(), 
            ue = pointer_operand->use_end(); ui != ue; ++ui) {
        
        Instruction* temp_inst = cast<Instruction>(*ui);       
        if(!temp_inst)
          continue;
        
        if(temp_inst == load_store)
          continue;
        
        if(!isa<LoadInst>(temp_inst) && !isa<StoreInst>(temp_inst))
          continue;
        
        if(isa<StoreInst>(temp_inst)){
          if(temp_inst->getOperand(1) != pointer_operand){
            /* when a pointer is a being stored at at a particular
             * address, don't elide the check
             */
            continue;
          }
        }
        
        if(m_dominator_tree->dominates(load_store, temp_inst)) {
          if(!FTCE_map.count(temp_inst)) {
            FTCE_map[temp_inst] = true;
            continue;
          }                  
        }
      } /* Iterating over uses ends */
    } /* TEMPORALBOUNDSCHECKOPT ends */
  }

  Value* tmp_key = NULL;
  Value* tmp_lock = NULL;
  Value* tmp_base = NULL;
  Value* tmp_bound = NULL;
  
  tmp_key = getAssociatedKey(pointer_operand);
  Value* func_tmp_lock = getAssociatedFuncLock(load_store);
  tmp_lock = getAssociatedLock(pointer_operand, func_tmp_lock);
  
  if(spatial_safety){
    tmp_base = getAssociatedBase(pointer_operand);
    tmp_bound = getAssociatedBound(pointer_operand);
  }
  
  assert(tmp_key && "[addTemporalChecks] pointer does not have key?");
  assert(tmp_lock && "[addTemporalChecks] pointer does not have lock?");
  
  Value* bitcast_lock = castToVoidPtr(tmp_lock, load_store);
  args.push_back(bitcast_lock);
  
  args.push_back(tmp_key);
  
#ifdef SOFTBOUNDCETS_CHK_INTRINSIC

    if(chk_intrinsic){
      Module* M = load_store->getParent()->getParent()->getParent();
      Type* Tys[] = { m_void_ptr_type, m_key_type, m_void_ptr_type, m_void_ptr_type};
      Function* temporal_chk_function =  Intrinsic::getDeclaration(M, Intrinsic::sbcets_temporalchk, Tys);

      CallInst::Create(temporal_chk_function, args, "", load_store);

      return;
    }
#endif

    if(spatial_safety){
      args.push_back(tmp_base);
      args.push_back(tmp_bound);
    }
    
    if(isa<LoadInst>(load_store)){
      CallInst::Create(m_temporal_load_dereference_check, args, "", load_store);
    }
    else {
      CallInst::Create(m_temporal_store_dereference_check, args, "", load_store);
    }    
    return;
}



void SoftBoundCETSPass::addDereferenceChecks(Function* func) {

  Function &F = *func;
  
  if(func->isVarArg())
    return;

  if(metadata_prop_only)
    return;

  if(Blacklist->isIn(F))
    return;

  std::vector<Instruction*> CheckWorkList;
  std::map<Value*, bool> ElideSpatialCheck;
  std::map<Value*, bool> ElideTemporalCheck;
  


  // identify all the instructions where we need to insert the spatial checks
  for(inst_iterator i = inst_begin(F), e = inst_end(F); i != e; ++i){

    Instruction* I = &*i;

    if(!m_present_in_original.count(I)){
      continue;
    }
    // add check optimizations here
    // add checks for memory fences and atomic exchanges
    if(isa<LoadInst>(I) || isa<StoreInst>(I)){
      CheckWorkList.push_back(I);
    }     
    if(isa<AtomicCmpXchgInst>(I) || isa<AtomicRMWInst>(I)){
      assert(0 && "Atomic Instructions not handled");
    }    
  }

#if 0
  // spatial check optimizations here 

  for(std::vector<Instruction*>::iterator i = CheckWorkList.begin(), 
	e = CheckWorkList.end(); i!= e; ++i){

    Instruction* inst = *i;
    Value* pointer_operand = NULL;
    
    if(ElideSpatialCheck.count(inst))
      continue;
    
    if(isa<LoadInst>(inst)){
      LoadInst* ldi = dyn_cast<LoadInst>(inst);
      pointer_operand = ldi->getPointerOperand();
    }
    if(isa<StoreInst>(inst)){
      StoreInst* st = dyn_cast<StoreInst>(inst);
      pointer_operand = st->getOperand(1);      
    }

    for(Value::use_iterator ui = pointer_operand->use_begin(),  
	  ue = pointer_operand->use_end();
	ui != ue; ++ui){

      Instruction* use_inst = dyn_cast<Instruction>(*ui);
      if(!use_inst || (use_inst == inst))
	continue;

      if(!isa<LoadInst>(use_inst)  && !isa<StoreInst>(use_inst))
	continue;

      if(isa<StoreInst>(use_inst)){
	if(use_inst->getOperand(1) != pointer_operand)
	  continue;
      }

      if(m_dominator_tree->dominates(inst, use_inst)){
	if(!ElideSpatialCheck.count(use_inst))
	  ElideSpatialCheck[use_inst] = true;		
      }
    }  
  }

#endif

  //Temporal Check Optimizations

#ifdef INSERT_CHECKS_DIRECTLY
  // the following code adds checks directly into the program. It bypasses the
  // *_check functions in the runtime library. This can be helpful if LTO does
  // not work properly
  for(std::vector<Instruction*>::iterator i = CheckWorkList.begin(), 
	e = CheckWorkList.end(); i != e ; ++i){

    Instruction* Inst = *i;
    
    Value* pointer_operand  = NULL;

    if(isa<LoadInst>(Inst)){
      LoadInst* ldi = dyn_cast<LoadInst>(Inst);
      pointer_operand = ldi->getPointerOperand();
    }
    
    if(isa<StoreInst>(Inst)){
      StoreInst* st = dyn_cast<StoreInst>(Inst);
      pointer_operand = st->getOperand(1);
    }

    if(isa<ConstantExpr>(pointer_operand))
     continue;
    
    Value* pointer_base = getAssociatedBase(pointer_operand);
    Value* pointer_bound = getAssociatedBound(pointer_operand);

    Value* pointer_key = getAssociatedKey(pointer_operand);
    Value* func_temp_lock = getAssociatedFuncLock(Inst);
    Value* pointer_lock = getAssociatedLock(pointer_operand, func_temp_lock);
    
    Builder->SetInsertPoint(Inst);

    Value* cast_pointer_lock = Builder->CreateBitCast(pointer_lock, m_sizet_ptr_type);
    Value* lock_load = Builder->CreateLoad(cast_pointer_lock, "");
    Value* cmp_eq = Builder->CreateICmpNE(pointer_key, lock_load);


    Value* cast_pointer = Builder->CreateBitCast(pointer_operand, m_void_ptr_type);

    Value* sizeofType = getSizeOfType(pointer_operand->getType());
    
    Value* Cmp1 = Builder->CreateICmpULT(cast_pointer, pointer_base);
    Value* sub = Builder->CreatePtrDiff(pointer_bound, cast_pointer);
    
    Value* Cmp2 = Builder->CreateICmpUGT(sizeofType, sub);
    
    Value* Or = Builder->CreateOr(Cmp1, Cmp2);

    Value* Or2 = Builder->CreateOr(Or, cmp_eq);

#if 0
    Function* call_print_metadata = Inst->getParent()->getParent()->getParent()->getFunction("__softboundcets_print_metadata");

    assert(call_print_metadata && "__softboundcets_print_metadata null?");
    Builder->CreateCall5(call_print_metadata, pointer_base, pointer_bound, cast_pointer, pointer_key, cast_pointer_lock);

    Function* intermediate = Inst->getParent()->getParent()->getParent()->getFunction("__softboundcets_intermediate");

    Builder->CreateCall4(intermediate, Cmp1, Cmp2, cmp_eq, lock_load);
#endif

    BasicBlock *OldBB = Inst->getParent();
    BasicBlock *Cont = OldBB->splitBasicBlock(Inst);
    OldBB->getTerminator()->eraseFromParent();

    assert(m_faulting_block.count(Inst->getParent()->getParent()));
    BasicBlock* faultBB = m_faulting_block[Inst->getParent()->getParent()];

    BranchInst::Create(faultBB, Cont, Or2, OldBB);
    
    continue;
  }

  return;
#endif
  
  
  m_dominator_tree = &getAnalysis<DominatorTree>(*func);

  /* intra-procedural load dererference check elimination map */
  std::map<Value*, int> func_deref_check_elim_map;
  std::map<Value*, int> func_temporal_check_elim_map;

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

    /* basic block load deref check optimization */
    std::map<Value*, int> bb_deref_check_map;
    std::map<Value*, int> bb_temporal_check_elim_map;
    /* structure check optimization */
    std::map<Value*, int> bb_struct_check_opt;

    for(BasicBlock::iterator i = bb->begin(), ie = bb->end(); i != ie; ++i){
      Value* v1 = dyn_cast<Value>(i);
      Instruction* new_inst = dyn_cast<Instruction>(i);
      
      /* Do the dereference check stuff */
      if(!m_present_in_original.count(v1))
        continue;
      
      if(isa<LoadInst>(new_inst)){
	
        if(store_only)
          continue;

        addLoadStoreChecks(new_inst, func_deref_check_elim_map);
        addTemporalChecks(new_inst, bb_temporal_check_elim_map, func_temporal_check_elim_map);
        continue;
      }

      if(isa<StoreInst>(new_inst)){
        addLoadStoreChecks(new_inst, func_deref_check_elim_map);
        addTemporalChecks(new_inst, bb_temporal_check_elim_map, func_temporal_check_elim_map);
        continue;
      }

      /* check call through function pointers */
      if(isa<CallInst>(new_inst)) {
          
        if(!CALLCHECKS) {
          continue;
        }          
	  

        SmallVector<Value*, 8> args;
        CallInst* call_inst = dyn_cast<CallInst>(new_inst);
        Value* tmp_base = NULL;
        Value* tmp_bound = NULL;
        
        assert(call_inst && "call instruction null?");
        
        if(!INDIRECTCALLCHECKS)
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



void SoftBoundCETSPass::renameFunctions(Module& module){
    
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

  
/* Renames a function by changing the function name to softboundcets_*
   for only those functions have wrappers
 */
  
void SoftBoundCETSPass:: renameFunctionName(Function* func, 
                                            Module& module, 
                                            bool external) {
    
  Type* ret_type = func->getReturnType();
  const FunctionType* fty = func->getFunctionType();
  std::vector<Type*> params;

  if(!m_func_wrappers_available.count(func->getName()))
    return;

  if(func->getName() == "softboundcets_pseudo_main")
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


void SoftBoundCETSPass::handleAlloca (AllocaInst* alloca_inst,
                                            Value* alloca_key,
                                            Value* alloca_lock,
                                            Value* func_xmm_key_lock,
                                            BasicBlock* bb, 
                                            BasicBlock::iterator& i) {

  Value *alloca_inst_value = alloca_inst;

  if(spatial_safety){
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
  
  if(temporal_safety){    
    associateKeyLock(alloca_inst_value, alloca_key, alloca_lock);
  }
}


void SoftBoundCETSPass::handleVectorStore(StoreInst* store_inst){

  Value* operand = store_inst->getOperand(0);
  Value* pointer_dest = store_inst->getOperand(1);
  Instruction* insert_at = getNextInstruction(store_inst);

  if(!m_vector_pointer_base.count(operand)){
    assert(0 && "vector base not found");
  }
  if(!m_vector_pointer_bound.count(operand)){
    assert(0 && "vector bound not found");
  }
  if(!m_vector_pointer_key.count(operand)){
    assert(0 && "vector key not found");
  }
  
  if(!m_vector_pointer_lock.count(operand)){
    assert(0 && "vector lock not found");
  }

  Value* vector_base = m_vector_pointer_base[operand];
  Value* vector_bound = m_vector_pointer_bound[operand];
  Value* vector_key = m_vector_pointer_key[operand];
  Value* vector_lock = m_vector_pointer_lock[operand];

  const VectorType* vector_ty = dyn_cast<VectorType>(operand->getType());
  uint64_t num_elements = vector_ty->getNumElements();
  if (num_elements > 2){
    assert(0 && "more than 2 element vectors not handled");
  }

  Value* pointer_operand_bitcast = castToVoidPtr(pointer_dest, insert_at);
  for (uint64_t i = 0; i < num_elements; i++){

    Constant* index = ConstantInt::get(Type::getInt32Ty(store_inst->getContext()), i);

    Value* ptr_base = ExtractElementInst::Create(vector_base, index,"", insert_at);
    Value* ptr_bound = ExtractElementInst::Create(vector_bound, index, "", insert_at);
    Value* ptr_key = ExtractElementInst::Create(vector_key, index, "", insert_at);
    Value* ptr_lock = ExtractElementInst::Create(vector_lock, index, "", insert_at);
    
    SmallVector<Value*, 8> args;
    args.clear();

    args.push_back(pointer_operand_bitcast);
    args.push_back(ptr_base);
    args.push_back(ptr_bound);
    args.push_back(ptr_key);
    args.push_back(ptr_lock);
    args.push_back(index);

    CallInst::Create(m_metadata_store_vector_func, args, "", insert_at);    
  }

}   

void SoftBoundCETSPass::handleStore(StoreInst* store_inst) {

  Value* operand = store_inst->getOperand(0);
  Value* pointer_dest = store_inst->getOperand(1);
  Instruction* insert_at = getNextInstruction(store_inst);
    
  if(isa<VectorType>(operand->getType())){
    const VectorType* vector_ty = dyn_cast<VectorType>(operand->getType());
    if(isa<PointerType>(vector_ty->getElementType())){
      handleVectorStore(store_inst);
      return;
    }    
  }

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
                            m_void_null_ptr, m_constantint64ty_zero, 
                            m_void_null_ptr, m_void_null_ptr, 
                            size_of_type, insert_at);

    return;      
  }

      
  /* if it is a global expression being stored, then add add
   * suitable base and bound
   */
    
  Value* tmp_base = NULL;
  Value* tmp_bound = NULL;
  Value* tmp_key = NULL;
  Value* tmp_lock = NULL;

  //  Value* xmm_base_bound = NULL;
  //  Value* xmm_key_lock = NULL;
    
  Constant* given_constant = dyn_cast<Constant>(operand);
  if(given_constant) {      
    if(spatial_safety){
      getConstantExprBaseBound(given_constant, tmp_base, tmp_bound);
      assert(tmp_base && "global doesn't have base");
      assert(tmp_bound && "global doesn't have bound");        
    }

    if(temporal_safety){
      tmp_key = m_constantint_one;
      Value* func_lock = m_func_global_lock[store_inst->getParent()->getParent()->getName()];
      tmp_lock = func_lock;
    } 
  }
  else {      
    /* storing an external function pointer */
    if(spatial_safety){
      if(!checkBaseBoundMetadataPresent(operand)) {
        return;
      }
    }

    if(temporal_safety){
      if(!checkKeyLockMetadataPresent(operand)){
        return;
      }
    }

    if(spatial_safety){
      tmp_base = getAssociatedBase(operand);
      tmp_bound = getAssociatedBound(operand);              
    }

    if(temporal_safety){
      tmp_key = getAssociatedKey(operand);
      Value* func_lock = getAssociatedFuncLock(store_inst);
      tmp_lock = getAssociatedLock(operand, func_lock);
    }
  }    
  
  /* Store the metadata into the metadata space
   */
  

  //  Type* stored_pointer_type = operand->getType();
  Value* size_of_type = NULL;
  //    Value* size_of_type  = getSizeOfType(stored_pointer_type);
  addStoreBaseBoundFunc(pointer_dest, tmp_base, tmp_bound, tmp_key, tmp_lock, operand,  size_of_type, insert_at);    
  
}

// Currently just a placeholder for functions introduced by us
bool SoftBoundCETSPass::checkIfFunctionOfInterest(Function* func) {

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


Instruction* SoftBoundCETSPass:: getGlobalInitInstruction(Module& module){
  Function* global_init_function = module.getFunction("__softboundcets_global_init");    
  assert(global_init_function && "no __softboundcets_global_init function??");    
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



void SoftBoundCETSPass::handleGEP(GetElementPtrInst* gep_inst) {
  Value* getelementptr_operand = gep_inst->getPointerOperand();
  propagateMetadata(getelementptr_operand, gep_inst, SBCETS_GEP);
}

void SoftBoundCETSPass::handleMemcpy(CallInst* call_inst){
    

  if(DISABLE_MEMCOPY_METADATA_COPIES)
    return;


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
SoftBoundCETSPass:: iterateCallSiteIntroduceShadowStackStores(CallInst* call_inst){
    
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

void SoftBoundCETSPass::handleExtractElement(ExtractElementInst* EEI){
  
  if(!isa<PointerType>(EEI->getType()))
     return;
  
  Value* EEIOperand = EEI->getOperand(0);
  
  if(isa<VectorType>(EEIOperand->getType())){
    
    if(!m_vector_pointer_lock.count(EEIOperand) ||
       !m_vector_pointer_base.count(EEIOperand) ||
       !m_vector_pointer_bound.count(EEIOperand) || 
       !m_vector_pointer_key.count(EEIOperand)){
      assert(0 && "Extract element does not have vector metadata");
    }

    Constant* index = dyn_cast<Constant>(EEI->getOperand(1));
    
    Value* vector_base = m_vector_pointer_base[EEIOperand];
    Value* vector_bound = m_vector_pointer_bound[EEIOperand];
    Value* vector_key = m_vector_pointer_key[EEIOperand];
    Value* vector_lock = m_vector_pointer_lock[EEIOperand];
    
    Value* ptr_base = ExtractElementInst::Create(vector_base, index, "", EEI);
    Value* ptr_bound = ExtractElementInst::Create(vector_bound, index, "", EEI);
    Value* ptr_key = ExtractElementInst::Create(vector_key, index, "", EEI);
    Value* ptr_lock = ExtractElementInst::Create(vector_lock, index, "", EEI);
    
    associateBaseBound(EEI, ptr_base, ptr_bound);
    associateKeyLock(EEI, ptr_key, ptr_lock);
    return;
  }
     
  assert (0 && "ExtractElement is returning a pointer, possibly some vectorization going on, not handled, try running with O0 or O1 or O2");    
     
}


void SoftBoundCETSPass::handleExtractValue(ExtractValueInst* EVI){

  if(isa<PointerType>(EVI->getType())){
    assert(0 && "ExtractValue is returning a pointer, possibly some vectorization going on, not handled, try running with O0 or O1 or O2");
  }
  
  if(spatial_safety){
    associateBaseBound(EVI, m_void_null_ptr, m_infinite_bound_ptr);
  }

  if(temporal_safety){
    Value* func_temp_lock = getAssociatedFuncLock(EVI);
    associateKeyLock(EVI, m_constantint64ty_one, func_temp_lock);
  }  
  return;  
}



void SoftBoundCETSPass::handleCall(CallInst* call_inst) {

  // Function* func = call_inst->getCalledFunction();
  Value* mcall = call_inst;

#if 0
  CallingConv::ID id = call_inst->getCallingConv();


  if(id == CallingConv::Fast){
    printf("fast calling convention not handled\n");
    exit(1);
  }
#endif 
    
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

    if(!isa<PointerType>(call_inst->getType())){
      return;
    }
    
    if(spatial_safety){
      associateBaseBound(call_inst, m_void_null_ptr, m_void_null_ptr);
    }
    if(temporal_safety){
      associateKeyLock(call_inst, m_constantint64ty_zero, m_void_null_ptr);
    }
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

void SoftBoundCETSPass::handleIntToPtr(IntToPtrInst* inttoptrinst) {
    
  Value* inst = inttoptrinst;
    
  if(spatial_safety){
    associateBaseBound(inst, m_void_null_ptr, m_void_null_ptr);
  }
  
  if(temporal_safety){
    associateKeyLock(inst, m_constantint64ty_zero, m_void_null_ptr);
  }
}


void SoftBoundCETSPass::gatherBaseBoundPass2(Function* func){

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
SoftBoundCETSPass::introspectMetadata(Function* func, Value* ptr_value, 
                                      Instruction* insert_at, int arg_no){
  if(func->getName() != "debug_instrument_softboundcets")
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


void 
SoftBoundCETSPass::freeFunctionKeyLock(Function* func, Value* & func_key, 
                                       Value* & func_lock, 
                                       Value* & func_xmm_key_lock) {


  if(func_key == NULL && func_lock == NULL){
    return;
  }

  if((func_key == NULL && func_lock != NULL) && (func_key != NULL && func_lock == NULL)){
    assert(0 && "inconsistent key lock");
  }

  Function::iterator  bb_begin = func->begin();
  Instruction* next_inst = NULL;

  for(Function::iterator b = func->begin(), be = func->end(); b != be ; ++b) {

    BasicBlock* bb = dyn_cast<BasicBlock>(b);
    assert(bb && "basic block does not exist?");
      
    for(BasicBlock::iterator i = bb->begin(), ie = bb->end(); i != ie; ++i) {
        
      next_inst = dyn_cast<Instruction>(i);

      if(!isa<ReturnInst>(next_inst))
        continue;
   
      ReturnInst* ret = dyn_cast<ReturnInst>(next_inst);
      /* Insert a call to deallocate key and lock*/
      SmallVector<Value*, 8> args;
      Instruction* first_inst_func = dyn_cast<Instruction>(func->begin()->begin());
      assert(first_inst_func && "function doesn't have any instruction ??");
      args.push_back(func_key);
      CallInst::Create(m_temporal_stack_memory_deallocation, args, "", ret);
    }
  }
}

bool SoftBoundCETSPass::checkPtrsInST(StructType* struct_type){
  
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


bool SoftBoundCETSPass::checkTypeHasPtrs(Argument* ptr_argument){

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



void SoftBoundCETSPass::gatherBaseBoundPass1 (Function * func) {

  Value* func_key = NULL;
  Value* func_lock = NULL;
  Value* func_xmm_key_lock = NULL;
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
      
      if(spatial_safety){
        associateBaseBound(ptr_argument_value, m_void_null_ptr, m_infinite_bound_ptr);
      }
      if(temporal_safety){
        Value* func_temp_lock = getAssociatedFuncLock(func->begin()->begin());      
        associateKeyLock(ptr_argument_value, m_constantint64ty_one, func_temp_lock);
      }
    }
    else{
      introduceShadowStackLoads(ptr_argument_value, fst_inst, arg_count);
      //      introspectMetadata(func, ptr_argument_value, fst_inst, arg_count);
    }
  }

  getFunctionKeyLock(func, func_key, func_lock, func_xmm_key_lock);
  m_faulting_block[func] =  createFaultBlock(func);

#if 0
  if(temporal_safety){
    if(func_key == NULL || func_lock == NULL){
      assert(0 && "function key lock null for the function");
    }
  }
#endif
  

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
          handleAlloca(alloca_inst, func_key, func_lock, func_xmm_key_lock, bb, i);
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
	
      case Instruction::ExtractElement:
	{
	  ExtractElementInst * EEI = dyn_cast<ExtractElementInst>(v1);
	  assert(EEI && "ExtractElementInst inst?");
	  handleExtractElement(EEI);
	}
	break;

      case Instruction::ExtractValue:
	{
	  ExtractValueInst * EVI = dyn_cast<ExtractValueInst>(v1);
	  assert(EVI && "handle extract value inst?");
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

  if(temporal_safety){
    freeFunctionKeyLock(func, func_key, func_lock, func_xmm_key_lock);
  }
   
}

/* isByValDerived: This function check whether loaded address is
   dervied by a byval argument */

bool SoftBoundCETSPass:: isByValDerived(Value* pointer_operand){

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


void SoftBoundCETSPass::insertMetadataLoad(LoadInst* load_inst){

  AllocaInst* base_alloca;
  AllocaInst* bound_alloca;
  AllocaInst* key_alloca;
  AllocaInst* lock_alloca;

  SmallVector<Value*, 8> args;


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
    

  if(spatial_safety){
    
    base_alloca = new AllocaInst(m_void_ptr_type, "base.alloca", first_inst_func);
    bound_alloca = new AllocaInst(m_void_ptr_type, "bound.alloca", first_inst_func);
  
    /* base */
    args.push_back(base_alloca);
    /* bound */
    args.push_back(bound_alloca);
  }

  if(temporal_safety){
    
    key_alloca = new AllocaInst(Type::getInt64Ty(load_inst->getType()->getContext()), "key.alloca", first_inst_func);
    lock_alloca = new AllocaInst(m_void_ptr_type, "lock.alloca", first_inst_func);

    args.push_back(key_alloca);
    args.push_back(lock_alloca);
  }
  
  CallInst::Create(m_load_base_bound_func, args, "", insert_at);
      
  if(spatial_safety){
    Instruction* base_load = new LoadInst(base_alloca, "base.load", insert_at);
    Instruction* bound_load = new LoadInst(bound_alloca, "bound.load", insert_at);
    associateBaseBound(load_inst_value, base_load, bound_load);      
  }

  if(temporal_safety){
    Instruction* key_load = new LoadInst(key_alloca, "key.load", insert_at);
    Instruction* lock_load = new LoadInst(lock_alloca, "lock.load", insert_at);    
    associateKeyLock(load_inst_value, key_load, lock_load);
  }


}


/* handleLoad Takes a load_inst If the load is through a pointer
 * which is a global then inserts base and bound for that global
 * Also if the loaded value is a pointer then loads the base and
 * bound for for the pointer from the shadow space
 */

void SoftBoundCETSPass::handleLoad(LoadInst* load_inst) { 


  if(!isa<VectorType>(load_inst->getType()) && !isa<PointerType>(load_inst->getType())){
    return;
  }
  
  if(isa<PointerType>(load_inst->getType())){
    insertMetadataLoad(load_inst);
    return;
  }
 
  if(isa<VectorType>(load_inst->getType())){
    
    if(!spatial_safety || !temporal_safety){
      assert(0 && "Loading and Storing Pointers as a first-class types");            
      return;
    }

    
    // It should be a vector if here
    const VectorType* vector_ty = dyn_cast<VectorType>(load_inst->getType());
    // Introduce a series of metadata loads and associated it pointers
    if(!isa<PointerType>(vector_ty->getElementType()))
       return;
    
    Value* load_inst_value = load_inst;
    Instruction* load = load_inst;    

    Value* pointer_operand = load_inst->getPointerOperand();
    Instruction* insert_at = getNextInstruction(load_inst);
        
    Value* pointer_operand_bitcast =  castToVoidPtr(pointer_operand, insert_at);      
    Instruction* first_inst_func = dyn_cast<Instruction>(load_inst->getParent()->getParent()->begin()->begin());
    assert(first_inst_func && "function doesn't have any instruction and there is load???");
   
    uint64_t num_elements = vector_ty->getNumElements();

    
    SmallVector<Value*, 8> vector_base;
    SmallVector<Value*, 8> vector_bound;
    SmallVector<Value*, 8> vector_key;
    SmallVector<Value*, 8> vector_lock;

    for(uint64_t i = 0; i < num_elements; i++){

      
      AllocaInst* base_alloca;
      AllocaInst* bound_alloca;
      AllocaInst* key_alloca;
      AllocaInst* lock_alloca;
      
      SmallVector<Value*, 8> args;
      
      args.push_back(pointer_operand_bitcast);
      
      base_alloca = new AllocaInst(m_void_ptr_type, "base.alloca", first_inst_func);
      bound_alloca = new AllocaInst(m_void_ptr_type, "bound.alloca", first_inst_func);
	 
      /* base */
      args.push_back(base_alloca);
      /* bound */
      args.push_back(bound_alloca);

      key_alloca = new AllocaInst(Type::getInt64Ty(load_inst->getType()->getContext()), "key.alloca", first_inst_func);
      lock_alloca = new AllocaInst(m_void_ptr_type, "lock.alloca", first_inst_func);
      
      args.push_back(key_alloca);
      args.push_back(lock_alloca);
  
      Constant* index = ConstantInt::get(Type::getInt32Ty(load_inst->getContext()), i);

      args.push_back(index);
          
      CallInst::Create(m_metadata_load_vector_func, args, "", insert_at);
      
      Instruction* base_load = new LoadInst(base_alloca, "base.load", insert_at);
      Instruction* bound_load = new LoadInst(bound_alloca, "bound.load", insert_at);
      Instruction* key_load = new LoadInst(key_alloca, "key.load", insert_at);
      Instruction* lock_load = new LoadInst(lock_alloca, "lock.load", insert_at);    
      
      vector_base.push_back(base_load);
      vector_bound.push_back(bound_load);
      vector_key.push_back(key_load);
      vector_lock.push_back(lock_load);      
    }
    
    if (num_elements > 2){
      assert(0 && "Loading and Storing Pointers as a first-class types with more than 2 elements");      
    }
    
    VectorType* metadata_ptr_type = VectorType::get(m_void_ptr_type, num_elements);
    VectorType* key_vector_type = VectorType::get(m_key_type, num_elements);
    
    Value *CV0 = ConstantInt::get(Type::getInt32Ty(load_inst->getContext()), 0);
    Value *CV1 = ConstantInt::get(Type::getInt32Ty(load_inst->getContext()), 1);

    Value* base_vector = InsertElementInst::Create(UndefValue::get(metadata_ptr_type),     vector_base[0],  CV0, "", insert_at);
    Value* base_vector_final = InsertElementInst::Create(base_vector, vector_base[1], CV1, "", insert_at);
  
    m_vector_pointer_base[load_inst] = base_vector_final;

    Value* bound_vector = InsertElementInst::Create(UndefValue::get(metadata_ptr_type),     vector_bound[0],  CV0, "", insert_at);
    Value* bound_vector_final = InsertElementInst::Create(bound_vector, vector_bound[1], CV1, "", insert_at);    
    m_vector_pointer_bound[load_inst] = bound_vector_final;


    Value* key_vector = InsertElementInst::Create(UndefValue::get(key_vector_type), vector_key[0], CV0, "", insert_at);
    Value* key_vector_final = InsertElementInst::Create(key_vector, vector_key[1], CV1, "", insert_at);
    m_vector_pointer_key[load_inst] = key_vector_final;


    Value* lock_vector = InsertElementInst::Create(UndefValue::get(metadata_ptr_type),     vector_lock[0],  CV0, "", insert_at);
    Value* lock_vector_final = InsertElementInst::Create(lock_vector, vector_lock[1], CV1, "", insert_at);    

    m_vector_pointer_lock[load_inst] = lock_vector_final;
    
    return;
  }

#if 0
  if(unsafe_byval_opt && isByValDerived(load_inst->getOperand(0))) {

    if(spatial_safety){
      associateBaseBound(load_inst, m_void_null_ptr, m_infinite_bound_ptr);
    }
    if(temporal_safety){
      Value* func_lock = getAssociatedFuncLock(load_inst);
      associateKeyLock(load_inst, m_constantint64ty_one, func_lock);
    }
    return;
  }
#endif

}




/* Identify the initial globals present in the program before we add
 * extra base and bound for all globals
 */
void SoftBoundCETSPass::identifyInitialGlobals(Module& module) {

  for(Module::global_iterator it = module.global_begin(), 
        ite = module.global_end();
      it != ite; ++it) {
      
    GlobalVariable* gv = dyn_cast<GlobalVariable>(it);
    if(gv) {
      m_initial_globals[gv] = true;
    }      
  }
}

void SoftBoundCETSPass::addBaseBoundGlobals(Module& M){
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
      
      Value* operand_key = NULL;
      Value* operand_lock = NULL;

      if(temporal_safety){
        operand_key = m_constantint_one;
        operand_lock = introduceGlobalLockFunction(first);
      }
      
      addStoreBaseBoundFunc(addr_of_ptr, operand_base, operand_bound, operand_key, operand_lock, initializer_opd, initializer_size, first);
      
    }
  }

}
void SoftBoundCETSPass::identifyOriginalInst (Function * func) {

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

bool SoftBoundCETSPass::runOnModule(Module& module) {

  spatial_safety = true;
  temporal_safety = true;

  TD = &getAnalysis<DataLayout>();
  TLI = &getAnalysis<TargetLibraryInfo>();

  BuilderTy TheBuilder(module.getContext(), TargetFolder(TD));

  Builder = &TheBuilder;

  Blacklist.reset(SpecialCaseList::createOrDie(BlacklistFile));


  if(disable_spatial_safety){
    spatial_safety = false;
  }

  if(disable_temporal_safety){
    temporal_safety = false;
  }

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

    if (temporal_safety) {
      Value* func_global_lock = 
        introduceGlobalLockFunction(func_ptr->begin()->begin());
      m_func_global_lock[func_ptr->getName()] = func_global_lock;      
    }
      
    gatherBaseBoundPass1(func_ptr);
    gatherBaseBoundPass2(func_ptr);
    addDereferenceChecks(func_ptr);            
  }


  renameFunctions(module);
  DEBUG(errs()<<"Done with SoftBoundCETSPass\n");
  
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
