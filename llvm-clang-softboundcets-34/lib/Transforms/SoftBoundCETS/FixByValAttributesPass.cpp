///=== SoftBoundCETS/FixByValAttributes.cpp --*- C++ -*=====///
// Pointer based Spatial and Temporal Memory Safety Pass
//Copyright (c) 2011 Santosh Nagarakatte. All rights reserved.

// Collaborators: Santosh Nagarakatte, Milo M.K. Martin,
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

#include "llvm/Transforms/SoftBoundCETS/FixByValAttributes.h"

static cl::opt<bool>
fix_all_byval
("fix_all_byval",
 cl::desc("Transform all byval attributes"),
 cl::init(true));

static cl::opt<bool>
fix_specific_byval
("fix_specific_byval",
 cl::desc("Transform only pointer byval attributes"),
 cl::init(false));

char FixByValAttributesPass:: ID = 0;

static RegisterPass<FixByValAttributesPass> P ("FixByValAttributesPass",
                                               "Transform all byval Attributes");

void 
FixByValAttributesPass::createGEPStores(Value* result_alloca, 
                                        Value* call_site_arg, 
                                        StructType* struct_type,
                                        Instruction* insert_at, 
                                        std::vector<Value*> indices){


  int i = 0;
  StructType::element_iterator I = struct_type->element_begin();
  
  for(StructType::element_iterator E = struct_type->element_end(); 
      I!=E ; ++I, i++){

    std::vector<Value*> new_indices;
    Type* element_type = *I;
    
    for(int i = 0; i< indices.size(); i++){
      new_indices.push_back(indices[i]);
    }

    Constant* loop_index = ConstantInt::get(Type::getInt32Ty(result_alloca->getType()->getContext()), i, false);
    new_indices.push_back(loop_index);

    if(isa<StructType>(element_type)){
      StructType* elem_struct_type = dyn_cast<StructType>(element_type);
      createGEPStores(result_alloca, call_site_arg,  elem_struct_type,insert_at, new_indices);
    }
    else{
      GetElementPtrInst* gep_idx_src = GetElementPtrInst::Create(call_site_arg, 
                                                             new_indices, 
                                                             "", insert_at);

      GetElementPtrInst* gep_idx_dest = GetElementPtrInst::Create(result_alloca, 
                                                              new_indices,
                                                              "", insert_at);

      LoadInst* src_load = new LoadInst(gep_idx_src, "", insert_at);
      new StoreInst(src_load, gep_idx_dest, false, insert_at);
    }
  }

}

bool FixByValAttributesPass::checkPtrsInST(StructType* struct_type){
  
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


bool FixByValAttributesPass::checkTypeHasPtrs(Argument* ptr_argument){

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


bool FixByValAttributesPass:: transformFunction(Function* func){
  
  bool byval_arg = false;
  for(Function::arg_iterator ib = func->arg_begin(), ie = func->arg_end(); 
      ib != ie; ++ib){
    Argument* ptr_argument = dyn_cast<Argument>(ib);

    if(ptr_argument->hasByValAttr()){
      if(checkTypeHasPtrs(ptr_argument)){
        byval_arg = true;
      }      
    }    
  }
  if(!byval_arg)
    return false;

  Type* ret_type = func->getReturnType();
  const FunctionType* fty = func->getFunctionType();
  std::vector<Type*> params;
  
  SmallVector<AttributeSet, 8> param_attrs_vec;

  const AttributeSet& pal = func->getAttributes();

  if(pal.hasAttributes(AttributeSet::ReturnIndex))
    param_attrs_vec.push_back(AttributeSet::get(func->getContext(), pal.getRetAttributes()));

  int arg_index = 1;
  for(Function::arg_iterator i = func->arg_begin(), e = func->arg_end();
      i != e; ++i, ++arg_index){
    Argument* arg = dyn_cast<Argument>(i);
 
    params.push_back(i->getType());
    AttributeSet attrs = pal.getParamAttributes(arg_index);
    if(attrs.hasAttributes(arg_index)){
      if(arg->hasByValAttr()){

      }
      else{
	AttrBuilder B(attrs, arg_index);
	param_attrs_vec.push_back(AttributeSet::get(func->getContext(), params.size(), B));
      }
      
    }

#if 0
    if((Attributes attrs = pal.getParamAttributes(arg_index)) && !(arg->hasByValAttr())){
      param_attrs_vec.push_back(AttributeWithIndex::get(params.size(), attrs));
      //param_attrs_vec.push_back(AttributeWithIndex::get(arg_index, attrs));
    }
#endif
  }
  
  FunctionType* nfty = FunctionType::get(ret_type, params, fty->isVarArg());
  Function* new_func = Function::Create(nfty, func->getLinkage(), func->getName()+ ".sb");
  //  new_func->copyAttributesFrom(func);
  new_func->setAttributes(AttributeSet::get(func->getContext(), param_attrs_vec));
                          
  SmallVector<Value*, 16> call_args;      
  new_func->getBasicBlockList().splice(new_func->begin(), func->getBasicBlockList());  
  
  func->getParent()->getFunctionList().insert(func, new_func);

  Function::arg_iterator arg_i2 = new_func->arg_begin();      
  for(Function::arg_iterator arg_i = func->arg_begin(), arg_e = func->arg_end(); 
      arg_i != arg_e; ++arg_i) {
    
    arg_i->replaceAllUsesWith(arg_i2);
    arg_i2->takeName(arg_i);        
    ++arg_i2;
    arg_index++;
  }

  bool change_call = true;
  while(change_call) {
    change_call = false;
    
    for(Value::use_iterator ui = func->use_begin(), ue = func->use_end(); 
        ui != ue;) {
      User* user_call = ui.getUse().getUser();
      assert(user_call && "user null?");

      CallSite cs(user_call);
      Instruction* call = cs.getInstruction();
      if(!call){
        assert(0 && "Byval attribute and indirect call?");
      }

      CallInst* call_inst = dyn_cast<CallInst>(call);
      assert(call_inst && "call inst null?");

      SmallVector<Value*, 16> call_args;
      CallSite::arg_iterator arg_i = cs.arg_begin();

      for(Function::arg_iterator ib = func->arg_begin(), ie = func->arg_end(); 
          ib != ie; ++ib) {

        Value* call_site_arg = dyn_cast<Value>(arg_i);
        ++arg_i;
                
        Argument* ptr_argument = dyn_cast<Argument>(ib);
        if(!(ptr_argument->hasByValAttr() && 
             checkTypeHasPtrs(ptr_argument))){
          call_args.push_back(call_site_arg);
        }
        else{
          /* byval pointer */

          SequentialType* seq_type = dyn_cast<SequentialType>(call_site_arg->getType());
          assert(seq_type && "byval attribute with non-seq type?");

          StructType* struct_type = dyn_cast<StructType>(seq_type->getElementType());
          assert(struct_type && "non-struct byval parameters?");

          
          AllocaInst* byval_alloca = new AllocaInst(struct_type, "", call);
          /* introduce stores, call_site_arg to byval_alloca */
          
          // introduce an alloca of the pointer type of byval
          // introduce stores
                   
          std::vector<Value*> indices;
          Constant* start_index = ConstantInt::get(Type::getInt64Ty(byval_alloca->getType()->getContext()),
                                         0, false);
          indices.push_back(start_index);

          createGEPStores(byval_alloca, call_site_arg, struct_type, call, indices);
          call_args.push_back(byval_alloca);

        }      
      }

      CallInst* result_call = CallInst::Create(new_func,
                                               call_args, "", call);
      call->replaceAllUsesWith(result_call);
      call->eraseFromParent();
      change_call = true;
      break;
    }
  }
  
  func->eraseFromParent();
  return true;
    
}



bool FixByValAttributesPass::runOnModule(Module & module) {
  
  if (module.getPointerSize() == llvm::Module::Pointer64) {
    m_is_64bit = true;
  } else {
    m_is_64bit = false;
  }

  bool change = true;

  while (change){
    change = false;
    for(Module::iterator ff_begin = module.begin(), ff_end = module.end();
        ff_begin != ff_end; ++ff_begin){
      Function* func_ptr = dyn_cast<Function>(ff_begin);
      assert(func_ptr && "Not a function?");
      change = transformFunction(func_ptr);
      if(change)
        break;
    }
  }
  
  return true;
}


