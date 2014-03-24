// Prototype creator for SoftBoundMPX

#ifndef INITIALIZE_SOFTBOUNDMPX_H
#define INITIALIZE_SOFTBOUNDMPX_H

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm-c/Target.h"
#include "llvm-c/TargetMachine.h"


using namespace llvm;

class InitializeSoftBoundMPX: public ModulePass {

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
  InitializeSoftBoundMPX(): ModulePass(ID){        
    spatial_safety = true;
    temporal_safety= true;
  }
  
  const char* getPassName() const { return "InitializeSoftBoundMPX";}
};

#endif
