//===---------------------- Nios2LowerAllocaPass.cpp ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is Intel-private, and has not been upstreamed to LLVM.org
//
//===----------------------------------------------------------------------===//
//
// \file
// This file implements a pass specific for the Nios2 target, which determines
// which local variable goes to which memory hierarchy - L1, L2 or L3.
// The pass makes a notion of a 'master' function - this is a function executed
// only on a Nios2 master core - such as omp_offloading outlined function.
// See details in Nios2LowerAlloca::runOnFunction.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringRef.h"

#include "llvm/CodeGen/Passes.h"
#include "llvm/PassRegistry.h"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/OptBisect.h"
#include "llvm/IR/Value.h"

#include "llvm/Target/TargetMachine.h"

#include "Nios2.h"

#define DEBUG_TYPE "nios2-alloca-lower"
STATISTIC(NumPhase2AllocaL2, "number of locals went to L2");
STATISTIC(SizePhase2AllocaL2, "memory size for L2 locals");
STATISTIC(NumPhase2AllocaL3, "number of locals went to L3");
STATISTIC(SizePhase2AllocaL3, "memory size for L3 locals");

using namespace llvm;

static cl::opt<Nios2MemLevel> MasterLocMemLevel(
  "mem-for-master-locals", cl::init(Nios2_L2_Mem), cl::NotHidden,
  cl::desc("default memory level for locals in a master function"),
  cl::values(
    clEnumValN(Nios2_L1_Mem, "L1", "allocate in L1"),
    clEnumValN(Nios2_L2_Mem, "L2", "allocate in L2"),
    clEnumValN(Nios2_L3_Mem, "L3", "allocate in L3")
  )
);

namespace {

class Nios2LowerAlloca : public FunctionPass {
public:
  static char ID;
  const TargetMachine *tm;

  Nios2LowerAlloca(const TargetMachine *_tm = nullptr)
    : FunctionPass(ID), tm(_tm)
  {}

  StringRef getPassName() const override { 
    return "Nios2 alloca lowering";
  }

  // Converts alloca instructions to nios2_alloca_l2 or nios2_alloca_l3
  // where necessary.
  // The decision tree about necessity is generally as follows:
  // * if a local is allocated with alloca_l* intrinsic - nothing to do
  //   (otherwise, the local is allocated with usual alloca and the decision
  //    process branches below)
  // * if a local belongs to a non-master function - nothing to do
  //   (this local will be allocated on traditional Nios2 stack, which is in L1.)
  //   Otherwise the local belongs to a master function
  // * Locals go to the memory level specified by the MasterLocMemLevel option
  bool runOnFunction(Function &f) override;

private:
  bool skipFunction(const Function &f);
  bool lowerAlloca(AllocaInst &ai);
};

} // end anonymous namespace

// Checks if given function 
static bool isOffloadEntry(const Function &f) {
	// TODO(nios) quick hack implementation - should be based on information provided by FE 
	return f.getName().find("__omp_offloading", 0) != StringRef::npos;
}

bool Nios2LowerAlloca::skipFunction(const Function &f) {
  return
    // alloca allocates in L1 - no need to convert
    MasterLocMemLevel == Nios2_L1_Mem ||
    // alloca's get converted to something else only in master functions
    !isOffloadEntry(f) ||
    !f.getContext().getOptBisect().shouldRunPass(this, f);
}

bool Nios2LowerAlloca::runOnFunction(Function &f) {
  if (skipFunction(f)) {
    return false;
  }
  bool action_taken = false;
  // now convert all alloca's to nios2 intrinsics according to
  // MasterLocMemLevel setting;
  SmallVector<AllocaInst*,32> allocas;
  auto bb_i = f.begin();

#ifndef NDEBUG // in product build traverse the entry block only
  for (; bb_i != f.end(); bb_i++) {
#endif
    BasicBlock &bb = *bb_i;

    for (auto i = bb.begin(), e = bb.end(); i != e;) {
      AllocaInst *ai = dyn_cast<AllocaInst>(i);
      ++i;

      if (ai) {
        assert(bb_i == f.begin() && "alloca outside entry block");
        bool lowered = lowerAlloca(*ai);
        action_taken = lowered || action_taken;

        // Mark instruction for deletion
        if (lowered) {
          allocas.push_back(ai);
        }
      }
    }
#ifndef NDEBUG
  }
#endif
  // delete the replaced alloca instructions
  for (auto ai : allocas) {
    ai->dropAllReferences();
    ai->eraseFromParent();
  }
  return action_taken;
}

// replace alloca with an intrinsic call
bool Nios2LowerAlloca::lowerAlloca(AllocaInst &ai) {
  Module *m = ai.getModule();
  LLVMContext &c = m->getContext();
  const DataLayout &dl = m->getDataLayout();
  assert(ai.isStaticAlloca() && "Dynamic alloca not supported");
  bool is_l2_default = MasterLocMemLevel == Nios2_L2_Mem;

  // find approapriate builtin and generate a call to it
  Value* builtin = nios2::Util::getUnsafeMallocBuiltin(*m, MasterLocMemLevel);
  Type* int32_ty = Type::getInt32Ty(c);
  Type* ty = ai.getAllocatedType();
  uint64_t ty_size = dl.getTypeAllocSize(ty);
  Value* p1 = ConstantInt::get(int32_ty, ty_size);
  
  if (ai.isArrayAllocation()) {
    p1 = BinaryOperator::CreateMul(p1, ai.getArraySize(), "", &ai);
  }
  p1 = CastInst::CreateZExtOrBitCast(p1, nios2::Util::getSizetTy(*m), "", &ai);
  Value* p2 = ConstantInt::get(int32_ty, ai.getAlignment());

  auto* nios_alloca = CallInst::Create(builtin, { p1, p2, }, "", &ai);
  auto* cast = CastInst::CreatePointerCast(nios_alloca, ai.getType(), "", &ai);

  // replace alloca uses with the new call
  ai.replaceAllUsesWith(cast);

  // rcord some statistics
  auto& num_cnt = is_l2_default ? NumPhase2AllocaL2 : NumPhase2AllocaL3;
  auto& mem_cnt = is_l2_default ? SizePhase2AllocaL2 : SizePhase2AllocaL3;
  num_cnt++;
  
  if (ConstantInt* c = dyn_cast<ConstantInt>(ai.getArraySize())) {
    const APInt& n_elems = c->getValue();
    // TODO(nios) move this to code generator, as only CG can calculate
    // precisely the actual memory used 
    mem_cnt += ty_size * static_cast<int>(n_elems.getSExtValue());
  }
  return true;
}

char Nios2LowerAlloca::ID = 0;

INITIALIZE_TM_PASS(
  Nios2LowerAlloca, DEBUG_TYPE,
  "Nios2 master local variable allocation - last phase", false, false)

FunctionPass* llvm::createNios2LowerAllocaPass(const TargetMachine *tm) {
  return new Nios2LowerAlloca(tm);
}
