//=== SoftBound/SoftBoundCETSXMMPass.h - Definitions for the SoftBound/CETS --*- C++ -*===// 
// Copyright (c) 2011 Santosh Nagarakatte, Milo M. K. Martin. All rights reserved.

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


#ifndef SOFTBOUNDCETSXMMPASS_H
#define SOFTBOUNDCETSXMMPASS_H

#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/GlobalValue.h"
#include "llvm/Instructions.h"
#include "llvm/Module.h"
#include "llvm/Operator.h"
#include "llvm/ADT/FoldingSet.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/GetElementPtrTypeIterator.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/STLExtras.h"
#include <algorithm>
#include <cstdarg>


#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/CallingConv.h"
#include "llvm/LLVMContext.h"

#include "llvm/Instructions.h"
#include "llvm/Instruction.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Constants.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/InstrTypes.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Support/CFG.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#include<queue>

using namespace llvm;

class SoftBoundCETSXMMPass: public ModulePass {

 private:
  bool spatial_safety_xmm;
  bool temporal_safety_xmm;
  
  Function* m_introspect_metadata;
  
  Function* m_copy_metadata;
  Function* m_shadow_stack_allocate;
  Function* m_shadow_stack_deallocate;
  Function* m_shadow_stack_base_bound_load;
  Function* m_shadow_stack_key_lock_load;
  
  Function* m_shadow_stack_base_bound_store;
  Function* m_shadow_stack_key_lock_store;

  Function* m_convert_v2di_func;
   
  Function* m_spatial_load_dereference_check;
  Function* m_spatial_store_dereference_check;
  
  Function* m_temporal_stack_memory_allocation;
  Function* m_temporal_stack_memory_deallocation;

  Function* m_temporal_load_dereference_check;
  Function* m_temporal_store_dereference_check;
  Function* m_temporal_global_lock_function;
  
  Function* m_call_dereference_func;
  Function* m_memcopy_check;
  Function* m_memset_check;

  Function* m_simple_meta_map;
  Function* m_simple_meta_load_bb;
  Function* m_simple_meta_load_kl;
  
  /* Function Type of the function that loads the base and bound for
   * a given pointer 
   */
  Function* m_load_base_bound_func;

  /* Function Type of the function that stores the base and bound
   * for a given pointer
   */
  Function* m_store_base_bound_func;
  
  /* void pointer type, used many times in the Softboundcets pass */
  Type* m_void_ptr_type;
  VectorType* m_base_bound_ty;
  VectorType* m_key_lock_ty;
  
  /* constant null pointer which is the base and bound for most
   * non-pointers 
   */
  ConstantPointerNull* m_void_null_ptr;
  Value* m_invalid_bounds;
  Value* m_invalid_keylock;
  
  ConstantPointerNull* m_sizet_null_ptr;
  Type* m_key_type;

  Constant* m_constantint_one;
  Constant* m_constantint_zero;

  Constant* m_constantint32ty_one;
  Constant* m_constantint32ty_zero;
  Constant* m_constantint64ty_one;   
  Constant* m_constantint64ty_zero;
    
  /* Infinite bound where bound cannot be inferred in VarArg
   * functions
   */
  Value* m_infinite_bound_ptr;
  Value* m_xmm_infinite_ptr;
    
  
  /* Dominance Tree and Dominance Frontier for avoiding load
   * dereference checks 
   */


  DominatorTree* m_dominator_tree;
  
  /* Book-keeping structures for identifying original instructions
   * in the program, pointers and their corresponding base and bound
   */
  std::map<Value*, int> m_is_pointer;
  std::map<Value*, Value*> m_pointer_base_bound;
  std::map<Value*, Value*> m_pointer_key_lock;

  std::map<Value*, int> m_present_in_original;
  std::map<GlobalVariable*, int> m_initial_globals;
  
  /* Map of all functions for which Softboundcets Transformation must be
   * invoked
   */
  StringMap<bool> m_func_softboundcets_transform;
  
  /* Map of all functions that need to be transformed as they have
   * as they either hava pointer arguments or pointer return type
   * and are defined in the module
   */
  StringMap<bool> m_func_to_transform;
  
  /* Map of all functions defined by Softboundcets */
  StringMap<bool> m_func_def_softbound;

  StringMap<bool> m_func_wrappers_available;
  
  /* Map of all functions transformed */
  StringMap<bool> m_func_transformed;
  
  StringMap<Value*> m_func_global_key_lock;
  
  /* Boolean indicating whether bitcode generated is for 64bit or 32bit */
  bool m_is_64_bit;
  
  /* Main functions implementing the structure of the Softboundcets pass
   */
  bool runOnModule(Module&);
  void initializeSoftBoundVariables(Module&);
  bool isAllocaPresent(Function*);
  void identifyOriginalInst(Function*);
  void gatherBaseBoundPass1(Function*);
  void gatherBaseBoundPass2(Function*);
  void addDereferenceChecks(Function*);
  bool checkIfFunctionOfInterest(Function*);
  bool isFuncDefSoftBound(const std::string &str);
  std::string transformFunctionName(const std::string &str);
  void runForEachFunctionIndirectCallPass(Function&);
  void indirectCallInstPass(Module&);
  bool checkStructTypeWithGEP(BasicBlock*, std::map<Value*, int> &, 
                              Value*, BasicBlock::iterator);
  
  
  /* Specific LLVM instruction handlers in the bitcode */
  void handleAlloca(AllocaInst*, Value*, 
                    BasicBlock*,  BasicBlock::iterator&);  
  void handleLoad(LoadInst*);
  void handleStore(StoreInst*);
  void handleGEP(GetElementPtrInst*);

  void handleBitCast(BitCastInst*);
  void handlePHIPass1(PHINode*);
  void handlePHIPass2(PHINode*);
  void handleCall(CallInst*);
  void handleExtractValue(ExtractValueInst*);
  void handleMemcpy(CallInst*);
  void handleIndirectCall(CallInst*);
  void handleSelect(SelectInst*, int);
  void handleIntToPtr(IntToPtrInst*);
  void identifyFuncToTrans(Module&);
  
  void transformFunctions(Module&);
  bool transformIndividualFunction(Module&);  
  bool hasPtrArgRetType(Function*);
  void iterateOverSuccessors(Function&);
  void transformExternalFunctions(Module&);
  bool transformIndividualExternalFunctions(Module&);
  void transformMain(Module&);
  void renameFunctions(Module&);
  void renameFunctionName(Function*, Module&, bool);
  bool checkAndShrinkBounds(GetElementPtrInst*, Value* func_global_lock);
  bool checkTypeHasPtrs(Argument*);
  bool checkPtrsInST(StructType*);

  bool checkBitcastShrinksBounds(Instruction* );
  void addLoadStoreChecks(Instruction*, std::map<Value*, int>&);
  void addTemporalChecks(Instruction*, std::map<Value*, int>&, std::map<Value*, int>&);
  bool optimizeTemporalChecks(Instruction*, std::map<Value*, int>&, std::map<Value*,int>&);
  bool bbTemporalCheckElimination(Instruction*, std::map<Value*, int>&);
  bool funcTemporalCheckElimination(Instruction*, std::map<Value*, int>&);
  bool optimizeGlobalAndStackVariableChecks(Instruction*);
  bool checkLoadStoreSourceIsGEP(Instruction*, Value*);
  void addMemcopyMemsetCheck(CallInst*, Function*);


  bool isMemcopyFunction(Function*);
  void addMemoryAllocationCall(Function*, Value* &, Instruction*);
  void getFunctionKeyLock(Function*, Value* &);
  void freeFunctionKeyLock(Function*, Value* &);
  Value* getPointerLoadStore(Instruction*);
  void propagateMetadata(Value*, Instruction*, int);
  
  void getFunctionKeyLock(Function &, Value* &, Value* &, Value* &);
  void addMemoryAllocationCall(Function&, Value* , Value* &, Value* & , Instruction*, bool) ;

  bool isStructOperand(Value*);

  
  enum { SBCETS_BITCAST, SBCETS_GEP};
  /* Auxillary base and propagation functions */

  void handleGlobalSequentialTypeInitializer(Module&, GlobalVariable*);
  void handleGlobalStructTypeInitializer(Module& , StructType* , Constant* , GlobalVariable*, std::vector<Constant*>, int) ;
  void addBaseBoundGlobals(Module&);
  Instruction* getGlobalInitInstruction(Module&);
  void identifyInitialGlobals(Module&);
  void getGlobalVariableBaseBound(Value*, Value* &);
  void dissociateBaseBound(Value*);
  void dissociateKeyLock(Value*);
  
  /* Explicit Map manipulation functions */

  /* Single function that adds base/bound/key to the pointer map,
   * first argument - pointer operand
   * second argument - associated base
   * third argument - associated bound 
   * fourth argument - associated key 
   * fifth argument - associated lock 
   */
  void associateBaseBoundKeyLock(Value*, Value*, Value*);
  
  void associateBaseBound(Value*, Value*);
  void associateKeyLock(Value*, Value*);
    

  Value* getAssociatedBaseBound(Value*);
  Value* getAssociatedKeyLock(Value*, Value*);
  Value* getAssociatedFuncKeyLock(Value*);
      
  bool checkBaseBoundMetadataPresent(Value*);
  
  bool checkKeyLockMetadataPresent(Value*);

  /* Function to add a call to m_store_base_bound_func */
  void addStoreBaseBoundFunc(Value*, Value*, Value*, Value*, Value*, Instruction*);
  
  void addStoreXMMBaseBoundFunc(Value*, Value*, Value*, Instruction*);
  
  void setFunctionPtrBaseBound(Value*, Instruction*);
  
  void replaceAllInMap(std::map<Value*, Value*> &, 
                         Value*, Value*);
  
  void castAddToPhiNode(PHINode* , Value*, BasicBlock*, 
                        std::map<Value*, Value*>&, Value*);
  
  void getConstantExprBaseBound(Constant*, Value* &);
  
  Value* castAndReplaceAllUses(Value*, Value*, Instruction*);
  
  bool checkIfNonCallUseOfFunction(Function*);
  
  
  /* Other helper functions */
  
  Value* introduceGEPWithLoad(Value*, int, Instruction*);
  Value* storeShadowStackBaseForFunctionArgs(Instruction*, int);
  Value* storeShadowStackBoundForFunctionArgs(Instruction*, int);
  Value* storeShadowStackKeyForFunctionArgs(Instruction*, int);
  Value* storeShadowStackLockForFunctionArgs(Instruction*, int);
  
  Value* retrieveShadowStackBaseForFunctionArgs(Instruction*, int );
  Value* retrieveShadowStackBoundForFunctionArgs(Instruction*, int);
  Value* retrieveShadowStackKeyForFunctionArgs(Instruction*, int);
  Value* retrieveShadowStackLockForFunctionArgs(Instruction*, int);
    
  Value* introduceGlobalLockFunction(Instruction*);
  void introspectMetadata(Function*, Value*, Instruction*, int);
  void introduceShadowStackLoads(Value*, Instruction*, int);
  void introduceShadowStackAllocation(CallInst*);
  void iterateCallSiteIntroduceShadowStackStores(CallInst*);
  void introduceShadowStackStores(Value*, Instruction*, int);
  void introduceShadowStackDeallocation(CallInst*, Instruction*);
  int getNumPointerArgsAndReturn(CallInst*);

  void checkIfRetTypePtr(Function*, bool &);
  Instruction* getReturnInst(Function*, int);
  
  // 
  // Method: getNextInstruction
  // 
  // Description:
  // This method returns the next instruction after the input instruction.
  //
  
  Instruction* getNextInstruction(Instruction* I){
    
    if (isa<TerminatorInst>(I)) {
      return I;
    } else {
      BasicBlock::iterator i = I;
      return ++i;
    }    
  }
  
  const Type* getStructType(const Type*);
  Value*  getSizeOfType(Type*);
  
  Value* castToVoidPtr(Value*, Instruction*);
  bool checkGEPOfInterestSB(GetElementPtrInst*);
  void handleReturnInst(ReturnInst*);    
  
 public:
  static char ID;

  /* INITIALIZE_PASS(SoftBoundCETSXMMPass, "softboundcetspass", */
  /*               "SoftBound CETS for memory safety", false, false) */
    
    
  SoftBoundCETSXMMPass(): ModulePass(ID){
    
    spatial_safety_xmm = true;
    temporal_safety_xmm = true;
#if 0
    initializeSoftBoundCETSXMMPass(*PassRegistry::getPassRegistry());
#endif
  }
  const char* getPassName() const { return " SoftBoundCETSXMMPass";}


  void getAnalysisUsage(AnalysisUsage& au) const {
    au.addRequired<DominatorTree>();
    au.addRequired<LoopInfo>();
  }


};

#endif
