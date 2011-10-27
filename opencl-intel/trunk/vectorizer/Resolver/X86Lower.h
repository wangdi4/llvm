#ifndef __X86LOWER_H_
#define __X86LOWER_H_
#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Module.h"

#include <vector>

using namespace llvm;

namespace intel {
/// @brief Lower predicates to i32 words on x86
///
///   Warning: This code is a big hack!
///
///   Currently we do no have bool-vector support in the backend,
///   so this pass converts the vector of bools to vector of 32bits
///   and places intrinsics in place of the llvm instructions.
///   This hack needs to be removed once we actually add this support
///   to the llvm backend.
///
/// Places intrinsics instead of unsupported instruction
/// @Author Nadav Rotem
class X86Lower : public FunctionPass {
  public:
    enum Arch {
      SSE2 = 0,  // Only SSE2
      SSE4 = 1,  // Only SSE4
      AVX  = 2,  // AVX1
      AVX2 = 3   // AVX2
    };

    static char ID; // Pass identification, replacement for typeid
    X86Lower(Arch arch = SSE4): FunctionPass(ID), m_arch(arch) {}

    /// @brief LLVM Function pass entry
    /// @param F Function to transform
    /// @return True if changed
    virtual bool runOnFunction(Function &F);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {  }

    virtual bool doInitialization(Module &M);

  private:

    /// @brief Templated function for collecting all instructions of a certain
    //type.
    /// @param instrs Instruction container
    /// @param F Function to scan
    template<typename T>
      void findAllInstructions(std::vector<T*>& instrs, Function& F) {
        for (Function::iterator bb = F.begin(), bbe = F.end(); bb != bbe; ++bb) {
          BasicBlock *block = bb;
          for (BasicBlock::iterator it = block->begin(), e = block->end();
               it != e ; ++it) {
            if (T *v = dyn_cast<T>(it)) {
              instrs.push_back(v);
            }
          }
        }
        return;
      }

    /// @brief Find 32bit equivalent version of A
    ///  Use previously stored calculated value or constant conversion.
    /// @param A Value to translate.
    /// @param loc Location of value.
    /// @return Translated value, a 32-bit value if tranlation is needed or
    ///  original value
    Value* convertToI32(Value* A, Instruction* loc);
    /// @brief Checks if this value is a 1-bit predicate and needs translation
    /// @param val Value to translate.
    /// @return True if 1-bit
    bool needTranslate(Value* val);

    /// @brief Translation router.  Handles predicate consumers.
    ///  Translate values to 32-bit version if needed.
    ///  Translates all values which are potentially predicate consumers or
    ///  producers.
    /// @param val Value to translte. Saves the translated value to alloca
    /// variable.
    void Translate(Value* val);
    void Translate(BinaryOperator* bi);
    void Translate(SelectInst* si);
    void Translate(CallInst* ci);
    void Translate(PHINode* phi);
    void Translate(ShuffleVectorInst* sv);
    void Translate(InsertElementInst* ie);
    void Translate(ExtractElementInst* ee);
    void Translate(CmpInst* cmp);
    void TranslateVector(ICmpInst* cmp);
    void TranslateVector(FCmpInst* cmp);
    void TranslateFallback(CmpInst* cmp);

    /// @brief Translate instructions which are strictly consumers of predicates
    /// @param bi
    void LowerInst(BranchInst* bi);
    void LowerInst(SelectInst* si);
    void LowerInst(ZExtInst* ex);
    void LowerInst(SExtInst* ex);

    /// @brief Generate a 32bit constant or vector of 32-bit from input constant
    /// @param val Constant to translate.
    /// @return 32bit predicate constants.
    Constant* TranslateConst(Value* val);

    /// @brief Translate predicate type to 32bit predicate
    /// @param tp Type to translate
    /// @return Translated type
    const Type* TranslateType(const Type* tp);

    /// @brief Scalarize instruction if all useers and producers
    /// are scalar.
    //  Potential optimization. Currently unused.
    /// @param inst Instruction to translate, if needed
    void scalarizeSingleVectorInstruction(Instruction* inst);

    /// @brief Return intrinsic name for vector CMP instruction based on LLVM
    ///   predicate
    /// @param predicate LLVM predicate encoding
    /// @param vec Vector Type
    /// @return intrinsic name or null
    const char* getIntrinsicNameForCMPType(int predicate, const Type* vec);

    /// Type
    const Type* m_i1;
    const Type* m_i8;
    const Type* m_i32;

    /// Numeric Constant
    Constant* m_i32_0;   // the number 0
    Constant* m_i32_31;  // the number 31
    Constant* m_i32_111; // the number 111111...

    ///The working function
    Function* m_func;
    /// The working context
    LLVMContext* m_context;
    /// Translation table
    std::map<Value*, Value*> m_trans;
    /// ReplaceAlUsesWith table. Only after function translation is finished.
    std::map<Instruction*, Instruction*> m_rauw;

    /// What is the CPU architecture we are targetting
    Arch m_arch;
};

}
#endif //define __X86LOWER_H_
