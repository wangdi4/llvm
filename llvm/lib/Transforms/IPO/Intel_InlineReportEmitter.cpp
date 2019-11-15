//===----------- Intel_InlineReportEmitter.cpp - Inlining Report  -----------===//
//
// Copyright (C) 2019-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file describes inlining report emitter process.
//
// Inline report emitter pass takes module metadata and prints out inline report
// in a user-friendly form.
//
//===----------------------------------------------------------------------===//
//
#include "llvm/Transforms/IPO/Intel_InlineReportEmitter.h"

#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/Intel_MDInlineReport.h"

using namespace llvm;
using namespace MDInliningReport;

extern cl::opt<unsigned> IntelInlineReportLevel;

#define DEBUG_TYPE "inlinereportemitter"

///
/// \brief Print function linkage char
///
/// CalleeName: function name
/// Level: The level N from '-inline-report=N'
///
static void printFunctionLinkageChar(StringRef CalleeName, Module &M,
                                     unsigned Level) {
  if (!(Level & InlineReportOptions::Linkage))
    return;
  if (Function *F = M.getFunction(CalleeName)) {
    llvm::errs() << llvm::getLinkageStr(F) << ' ';
    return;
  }
  // If the function is dead we should look for its linkage info in the
  // metadata.
  NamedMDNode *ModuleInlineReport =
      M.getOrInsertNamedMetadata("intel.module.inlining.report");
  for (unsigned I = 0; I < ModuleInlineReport->getNumOperands(); ++I) {
    MDNode *Node = ModuleInlineReport->getOperand(I);
    assert(isa<MDTuple>(Node) && "Bad format of function inlining report");
    MDTuple *FIR = cast<MDTuple>(Node);
    StringRef FuncName = getOpStr(FIR->getOperand(FMDIR_FuncName), "name: ");
    if (FuncName == CalleeName) {
      llvm::errs() << getOpStr(FIR->getOperand(FMDIR_LinkageStr), "linkage: ")
                   << ' ';
      return;
    }
  }
}

///
/// \brief Print a simple message
///
/// MD: Metadata tuple with module and location information
/// M: Current module
/// Level: The level N from '-inline-report=N'
///
static void printCalleeNameModuleLineCol(MDTuple *MD, Module &M,
                                         unsigned Level) {
  CallSiteInliningReport CSIR(MD);
  StringRef CalleeName = CSIR.getName();
  printFunctionLinkageChar(CalleeName, M, Level);
  llvm::errs() << CalleeName;
  unsigned LineNum = 0, ColNum = 0;
  CSIR.getLineAndCol(&LineNum, &ColNum);
  if (Level & InlineReportOptions::File)
    llvm::errs() << ' '
                 << getOpStr(MD->getOperand(CSMDIR_ModuleName), "moduleName: ");
  if ((Level & InlineReportOptions::LineCol) && (LineNum != 0 || ColNum != 0))
    llvm::errs() << " (" << LineNum << "," << ColNum << ")";
}

///
/// \brief Print a simple message
///
/// Message: The message being printed
/// IsInline: The flag saying if the call site was inlined
/// IndentCount: The number of indentations before printing the message
/// Level: The level N from '-inline-report=N'
///
static void printSimpleMessage(const char *Message, bool IsInline,
                               unsigned IndentCount, unsigned Level) {
#if !INTEL_PRODUCT_RELEASE
  if (Level & InlineReportOptions::Reasons) {
    if (Level & InlineReportOptions::SameLine) {
      llvm::errs() << " ";
    } else {
      llvm::errs() << "\n";
      printIndentCount(IndentCount + 1);
    }
    llvm::errs() << (IsInline ? "<<" : "[[");
    llvm::errs() << Message;
    llvm::errs() << (IsInline ? ">>" : "]]");
    llvm::errs() << "\n";
    return;
  }
#endif // !INTEL_PRODUCT_RELEASE
  llvm::errs() << "\n";
}

///
/// \brief Print the inlining cost and threshold values
///
static void printCostAndThreshold(MDTuple *MD, bool IsInlined, unsigned Level) {
  int64_t InlineCost = -1;
  getOpVal(MD->getOperand(CSMDIR_InlineCost), "inlineCost: ", &InlineCost);
  int64_t InlineThreshold = -1;
  getOpVal(MD->getOperand(7), "inlineThreshold: ", &InlineThreshold);

  llvm::errs() << " (" << InlineCost;
  if (IsInlined)
    llvm::errs() << "<=";
  else
    llvm::errs() << ">";
  llvm::errs() << InlineThreshold;

  int64_t EECost = INT_MAX;
  getOpVal(MD->getOperand(CSMDIR_EarlyExitCost), "earlyExitCost: ", &EECost);
  int64_t EEThreshold = INT_MAX;
  getOpVal(MD->getOperand(CSMDIR_EarlyExitThreshold),
           "earlyExitThreshold: ", &EEThreshold);

  if (((Level & InlineReportOptions::RealCost) != 0) && (EECost != INT_MAX) &&
      !IsInlined) {
    // Under RealCost flag we compute both real and "early exit" costs and
    // thresholds of inlining.
    llvm::errs() << " [EE:" << EECost;
    llvm::errs() << ">";
    llvm::errs() << EEThreshold << "]";
  }
  llvm::errs() << ")";
}

///
/// \brief Print the outer inlining cost and threshold values
///
static void printOuterCostAndThreshold(MDTuple *MD, unsigned Level) {
  int64_t OuterCost = -1;
  getOpVal(MD->getOperand(CSMDIR_OuterInlineCost),
           "outerInlineCost: ", &OuterCost);
  int64_t InlineCost = -1;
  getOpVal(MD->getOperand(CSMDIR_InlineCost), "inlineCost: ", &InlineCost);
  int64_t InlineThreshold = -1;
  getOpVal(MD->getOperand(CSMDIR_InlineThreshold),
           "inlineThreshold: ", &InlineThreshold);
  llvm::errs() << " (" << OuterCost << ">" << InlineCost << ">"
               << InlineThreshold << ")";
}

///
/// \brief Print the inline report for dependent call sites
///
static void printCallSiteInlineReports(Metadata *MD, unsigned IndentCount,
                                       Module &M, unsigned Level);

///
/// \brief Print the inline report for call site
///
void printCallSiteInlineReport(Metadata *MD, unsigned IndentCount, Module &M,
                               unsigned Level) {
  MDTuple *CSIR = cast<MDTuple>(MD);
  assert(CSIR && (CSIR->getNumOperands() == CallSiteMDSize) &&
         "Bad call site inlining report format");
  int64_t Reason = 0;
  getOpVal(CSIR->getOperand(CSMDIR_InlineReason), "reason: ", &Reason);
  assert(InlineReasonText[Reason].Type != InlPrtNone);
  printIndentCount(IndentCount);
  int64_t IsInlined = 0;
  getOpVal(CSIR->getOperand(CSMDIR_IsInlined), "isInlined: ", &IsInlined);
  if (IsInlined) {
    llvm::errs() << "-> INLINE: ";
    printCalleeNameModuleLineCol(CSIR, M, Level);
    if (InlineReasonText[Reason].Type == InlPrtCost) {
      printCostAndThreshold(CSIR, true, Level);
    }
    printSimpleMessage(InlineReasonText[Reason].Message, true, IndentCount,
                       Level);
  } else {
    if (InlineReasonText[Reason].Type == InlPrtSpecial) {
      switch (Reason) {
      case NinlrDeleted:
        llvm::errs() << "-> DELETE: ";
        printCalleeNameModuleLineCol(CSIR, M, Level);
        llvm::errs() << "\n";
        break;
      case NinlrExtern:
        llvm::errs() << "-> EXTERN: ";
        printCalleeNameModuleLineCol(CSIR, M, Level);
        llvm::errs() << "\n";
        break;
      case NinlrIndirect:
        llvm::errs() << "-> INDIRECT: ";
        printCalleeNameModuleLineCol(CSIR, M, Level);
        printSimpleMessage(InlineReasonText[Reason].Message, false, IndentCount,
                           Level);
        break;
      case NinlrOuterInlining:
        llvm::errs() << "-> ";
        printCalleeNameModuleLineCol(CSIR, M, Level);
        printOuterCostAndThreshold(CSIR, Level);
        printSimpleMessage(InlineReasonText[Reason].Message, false, IndentCount,
                           Level);
        break;
      default:
        assert(0);
      }
    } else {
      llvm::errs() << "-> ";
      printCalleeNameModuleLineCol(CSIR, M, Level);
      if (InlineReasonText[Reason].Type == InlPrtCost) {
        printCostAndThreshold(CSIR, false, Level);
      }
      printSimpleMessage(InlineReasonText[Reason].Message, false, IndentCount,
                         Level);
    }
  }
  printCallSiteInlineReports(CSIR->getOperand(CSMDIR_CSs), IndentCount + 1, M,
                             Level);
}

///
/// \brief Print call site inline report (for external use)
///
void printCallSiteInlineReport(Instruction *I, unsigned Level) {
  if (!I)
    return;
  if (!isa<CallBase>(I))
    return;
  if (MDNode *Node = I->getMetadata(CallSiteTag))
    printCallSiteInlineReport(Node, /*IndentCount*/ 0,
                              *(I->getParent()->getParent()->getParent()),
                              Level);
  return;
}

static void printCallSiteInlineReports(Metadata *MD, unsigned IndentCount,
                                       Module &M, unsigned Level) {
  if (!MD)
    return;
  MDTuple *MDCSs = cast<MDTuple>(MD);
  if (!MDCSs || (MDCSs->getNumOperands() < 2))
    return;
  const MDString *S = dyn_cast<MDString>(MDCSs->getOperand(0));
  if (!S || (S->getString() != CallSitesTag))
    return;
  for (unsigned I = 1; I < MDCSs->getNumOperands(); ++I)
    printCallSiteInlineReport(MDCSs->getOperand(I), IndentCount, M, Level);
}

///
/// \brief Print function inline report from metadata
///
static void printFunctionInlineReportFromMetadata(MDNode *Node, Module &M,
                                                  unsigned Level) {
  assert(isa<MDTuple>(Node) && "Bad format of function inlining report");
  MDTuple *FuncReport = cast<MDTuple>(Node);
  assert(FuncReport->getNumOperands() == FunctionMDSize &&
         "Bad format of function inlining report");
  int64_t IsDead = 0;
  getOpVal(FuncReport->getOperand(FMDIR_IsDead), "isDead: ", &IsDead);
  // Special output for dead static functions.
  if (IsDead) {
    llvm::errs() << "DEAD STATIC FUNC: ";
    if (Level & InlineReportOptions::Linkage) {
      // Linkage letter
      llvm::errs() << getOpStr(FuncReport->getOperand(FMDIR_LinkageStr),
                               "linkage: ")
                   << ' ';
    }
    // Function name
    llvm::errs() << getOpStr(FuncReport->getOperand(FMDIR_FuncName), "name: ")
                 << "\n\n";
    return;
  }
  // if function is declaration
  int64_t IsDecl = 0;
  getOpVal(FuncReport->getOperand(FMDIR_IsDeclaration),
           "isDeclaration: ", &IsDecl);
  if (IsDecl)
    return;
  llvm::errs() << "COMPILE FUNC: ";
  std::string Name = getOpStr(FuncReport->getOperand(FMDIR_FuncName), "name: ");
  // Update linkage last time before printing.
  if (Function *F = M.getFunction(Name)) {
    std::string Linkage = llvm::getLinkageStr(F);
    std::string LinkageStr = "linkage: " + Linkage;
    auto LinkageMD = MDNode::get(
        M.getContext(), llvm::MDString::get(M.getContext(), LinkageStr));
    FuncReport->replaceOperandWith(FMDIR_LinkageStr, LinkageMD);
    if (Level & InlineReportOptions::Linkage)
      llvm::errs() << Linkage << ' ';
  } else {
    if (Level & InlineReportOptions::Linkage)
      llvm::errs() << getOpStr(FuncReport->getOperand(FMDIR_LinkageStr),
                               "linkage: ")
                   << ' ';
  }
  llvm::errs() << Name << '\n';
  printCallSiteInlineReports(FuncReport->getOperand(FMDIR_CSs), 1, M, Level);
  llvm::errs() << '\n';
  return;
}

///
/// \brief Print function inline report (for external use)
///
void printFunctionInlineReport(Function *F, unsigned Level) {
  if (!F)
    return;
  if (MDNode *Node = F->getMetadata(FunctionTag))
    printFunctionInlineReportFromMetadata(Node, *(F->getParent()), Level);
  return;
}

///
/// \brief Print module inline report
///
static bool emitInlineReport(Module &M, unsigned Level, unsigned OptLevel,
                             unsigned SizeLevel, bool PrepareForLTO) {
  if (!(Level & InlineReportOptions::BasedOnMetadata))
    return false;
  if (PrepareForLTO && (Level & InlineReportOptions::CompositeReport))
    return false;
  llvm::errs() << "---- Begin Inlining Report ---- (via metadata)\n";

  llvm::printOptionValues(OptLevel, SizeLevel);
  NamedMDNode *ModuleInlineReport =
      M.getOrInsertNamedMetadata("intel.module.inlining.report");
  for (unsigned I = 0; I < ModuleInlineReport->getNumOperands(); ++I) {
    MDNode *Node = ModuleInlineReport->getOperand(I);
    printFunctionInlineReportFromMetadata(Node, M, Level);
  }
  llvm::errs() << "---- End Inlining Report ------ (via metadata)\n";
  return true;
}

namespace {
struct InlineReportEmitter : public ModulePass {
  static char ID;
  unsigned OptLevel;
  unsigned SizeLevel;
  bool PrepareForLTO;
  InlineReportEmitter(unsigned OL = 0, unsigned SL = 0, bool PrepForLTO = false)
      : ModulePass(ID), OptLevel(OL), SizeLevel(SL), PrepareForLTO(PrepForLTO) {
    initializeInlineReportEmitterPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    return emitInlineReport(M, IntelInlineReportLevel, OptLevel, SizeLevel,
                            PrepareForLTO);
  }
};
} // namespace

char InlineReportEmitter::ID = 0;
INITIALIZE_PASS(InlineReportEmitter, "inlinereportemitter",
                "Emit inlining report", false, false)

ModulePass *llvm::createInlineReportEmitterPass(unsigned OptLevel,
                                                unsigned SizeLevel,
                                                bool PrepareForLTO) {
  return new InlineReportEmitter(OptLevel, SizeLevel, PrepareForLTO);
}

PreservedAnalyses InlineReportEmitterPass::run(Module &M,
                                               ModuleAnalysisManager &AM) {
  emitInlineReport(M, IntelInlineReportLevel, OptLevel, SizeLevel,
                   PrepareForLTO);
  return PreservedAnalyses::all();
}

