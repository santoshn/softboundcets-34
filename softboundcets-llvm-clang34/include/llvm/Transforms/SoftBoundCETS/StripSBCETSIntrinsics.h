

#ifndef SOFTBOUNDCETS_STRIPSB_INTRINSICS_H
#define SOFTBOUNDCETS_STRIPSB_INTRINSICS_H

#include "llvm/Intrinsics.h"

#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/LLVMContext.h"
#include "llvm/LLVMContext.h"
#include "llvm/Instructions.h"
#include "llvm/Instruction.h"
#include "llvm/Target/TargetData.h"


using namespace llvm;

class StripSBCETSIntrinsics: public FunctionPass {

 public:
  bool runOnFunction(Function &);
  static char ID;

  bool iterateOverFunctions(Function&);

  StripSBCETSIntrinsics(): FunctionPass(ID){       
  }
  
  const char* getPassName() const { return "StripSBCETSIntrinsics";}
};


#endif

