// Prototype creator for SoftBoundPass

#ifndef INITIALIZE_SOFTBOUNDCETS_H
#define INITIALIZE_SOFTBOUNDCETS_H

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm-c/Target.h"
#include "llvm-c/TargetMachine.h"


using namespace llvm;

class InitializeSoftBoundCETS: public ModulePass {

 private:
  bool spatial_safety;
  bool temporal_safety;
  
 public:
  bool runOnModule(Module &);
  static char ID;

  void constructCheckHandlers(Module &);
  void constructMetadataHandlers(Module &);
  void constructShadowStackHandlers(Module &);
  void constructAuxillaryFunctionHandlers(Module &);
  InitializeSoftBoundCETS(): ModulePass(ID){        
    spatial_safety = true;
    temporal_safety= true;
  }
  
  const char* getPassName() const { return "InitializeSoftBoundCETS";}
};

#endif
