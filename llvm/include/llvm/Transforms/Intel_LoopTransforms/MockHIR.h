
#ifndef LLVM_TRANSFORMS_MOCKHIR_H
#define LLVM_TRANSFORMS_MOCKHIR_H
namespace llvm {

  class AliasAnalysis;
  class Loop;
  class LoopInfo;
  class ScalarEvolution;
  class SCEV;
  class SCEVConstant;
FunctionPass *createMockHIRPass();
}
#endif
