//===- CSAFortranIntrinsics.cpp - Make intrinsics usable in Fortran -===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------===//
//
// Because we don't want to modify the GCC Fortran frontend that we use
// with DragonEgg, this pass is a workaround to make it possible to put
// CSA intrinsics in Fortran programs. To do that, this pass just scans
// functions for particular (Fortran-style) function calls and replaces
// those with their corresponding intrinsics. The mapping between function
// names and intrinsic names is encoded below in intrinsic_table.
//
//===----------------------------------------------------------------===//

#include "CSAFortranIntrinsics.h"
#include "CSATargetMachine.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/StackProtector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <utility>

#define DEBUG_TYPE "csa-fortran-intrinsics"

STATISTIC(NumReplaces,
          "Number of Fortran \"builtin\" calls replaced with intrinsics");

using namespace llvm;

namespace {

//>>>>>> THE INTRINSIC MAPPING TABLE <<<<<<
// Add entries for any more intrinsics that need to be converted:
//
//  intrinsic_table[i].first:  The generated name of the Fortran function to
//                             convert.
//  intrinsic_table[i].second: The ID of the intrinsic to convert it to.
//
// Note the underscore at the end of the Fortran function names - that seems to
// be added by the compiler, so if this table has an entry for "builtin_thing_"
// it will correspond to a call that looks like this in Fortran:
//
//  call builtin_thing()
//
constexpr std::pair<const char *, Intrinsic::ID> intrinsic_table[] = {
  {"builtin_csa_parallel_loop_", Intrinsic::csa_parallel_loop},
  {"builtin_csa_spmdization_", Intrinsic::csa_spmdization},
  {"builtin_csa_spmd_", Intrinsic::csa_spmd}};

struct CSAFortranIntrinsics : FunctionPass {
  static char ID;

  CSAFortranIntrinsics() : FunctionPass{ID} {
    initializeCSAFortranIntrinsicsPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<StackProtector>();
  }

  bool runOnFunction(Function &F) override;

  StringRef getPassName() const override {
    return "CSA: Convert particular Fortran function calls to intrinsics";
  }
};

char CSAFortranIntrinsics::ID = 0;

bool CSAFortranIntrinsics::runOnFunction(Function &F) {
  using namespace std;

  // look at all of the instructions in each function
  for (BasicBlock &BB : F)
    for (auto II = begin(BB); II != end(BB); ++II) {

      // due to interesting Fortran calling conventions, the instructions that
      // we're  interested in aren't direct calls but calls through a bitcast,
      // like:
      //
      //  call void bitcast (void (...)* @builtin_csa_parallel_loop_ to void
      //  ()*)()
      //

      CallInst *const call_inst = dyn_cast<CallInst>(II);
      if (not call_inst)
        continue;

      ConstantExpr *const call_expr =
        dyn_cast<ConstantExpr>(call_inst->getCalledValue());
      if (not call_expr)
        continue;

      if (not call_expr->isCast())
        continue;
      assert(call_expr->getNumOperands() >= 1);

      const StringRef proc_name = call_expr->getOperand(0)->getName();
      const auto found =
        find_if(begin(intrinsic_table), end(intrinsic_table),
                [proc_name](const pair<const char *, Intrinsic::ID> &p) {
                  return p.first == proc_name;
                });
      if (found == end(intrinsic_table))
        continue;

      // Get the arguments too.
      SmallVector<Value *, 2> args;
      bool bad_args   = false;
      string err_name = proc_name;
      err_name.pop_back();
      for (Value *const arg : call_inst->arg_operands()) {
        GlobalVariable *const glob_arg = dyn_cast<GlobalVariable>(arg);
        if (dyn_cast<ConstantInt>(arg)) {
          // this is the length of the string passed to the intrinsic
          continue;
        } else if (dyn_cast<ConstantExpr>(arg)) {
          args.push_back(arg);
          // the next arg is the length of the string, to be ignored
        } else if (glob_arg and glob_arg->isConstant() and
                   glob_arg->getInitializer()) {
          args.push_back(glob_arg->getInitializer());
        } else {
          errs() << "\n";
          errs().changeColor(raw_ostream::BLUE, true);
          errs() << "!! WARNING: BAD CSA FORTRAN INTRINSIC !!";
          errs().resetColor();
          errs() << "\n\nA call to " << err_name
                 << " was found with non-constant arguments.\n"
                    "This call will be ignored.\n\n";

          bad_args = true;
          break;
        }
      }
      if (bad_args)
        continue;

      // Grab the intrinsic declaration and check the parameters.
      Function *const intrinsic =
        Intrinsic::getDeclaration(F.getParent(), found->second);
      const FunctionType *const intr_sig = intrinsic->getFunctionType();
      if (args.size() != intr_sig->getNumParams()) {
        errs() << "\n";
        errs().changeColor(raw_ostream::BLUE, true);
        errs() << "!! WARNING: BAD CSA FORTRAN INTRINSIC !!";
        errs().resetColor();
        errs() << "\n\nA call to " << err_name
               << " was found with the wrong number of arguments (expected "
               << intr_sig->getNumParams() << ", got " << args.size()
               << ").\n"
                  "This call will be ignored.\n\n";
        continue;
      }

      // replace with the correct intrinsic if there is a match
      LLVM_DEBUG(errs() << "in function " << F.getName() << ":\n"
                 << "replacing:" << *II << "\n"
                 << "with intrinsic: " << intrinsic->getName() << "\n");
      IRBuilder<>{&*II}.CreateCall(intrinsic, args);

      II = II->eraseFromParent();
      --II;
      ++NumReplaces;
    }

  return true;
}

} // namespace

INITIALIZE_PASS(CSAFortranIntrinsics, "csa-fortran-intrinsics",
                "Convert particular Fortran function calls to intrinsics",
                false, false)

FunctionPass *llvm::createFortranIntrinsics() {
  return new CSAFortranIntrinsics();
}
