#ifndef LLVM_TRANSFORMS_HIRCG_H
#define LLVM_TRANSFORMS_HIRCG_H
namespace llvm {

  class Loop;
  class LoopInfo;
  class ScalarEvolution;
  class SCEV;
  class SCEVConstant;
FunctionPass *createHIRCodeGenPass();
}
#endif
