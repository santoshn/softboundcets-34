// Prototype creator for SoftBoundPass

#ifndef INITIALIZE_SOFTBOUNDXMM_H
#define INITIALIZE_SOFTBOUNDXMM_H

#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/LLVMContext.h"
#include "llvm/LLVMContext.h"
#include "llvm/Instructions.h"
#include "llvm/Instruction.h"
#include "llvm/Target/TargetData.h"


using namespace llvm;

class InitializeSoftBoundXMM: public ModulePass {

 private:
  bool spatial_safety_xmm;
  bool temporal_safety_xmm;

 public:
  bool runOnModule(Module &);
  static char ID;

  void constructCheckHandlers(Module &);
  void constructMetadataHandlers(Module &);
  void constructShadowStackHandlers(Module &);
  void constructAuxillaryFunctionHandlers(Module &);
  InitializeSoftBoundXMM(): ModulePass(ID){        
    spatial_safety_xmm = true;
    temporal_safety_xmm = false;
  }
  
  const char* getPassName() const { return "InitializeSoftBoundXMM";}
};

#endif
