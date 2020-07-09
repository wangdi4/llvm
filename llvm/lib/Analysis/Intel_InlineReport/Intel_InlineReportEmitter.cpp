//===----------- Intel_InlineReportEmitter.cpp - Inlining Report  -----------===//
//
// Copyright (C) 2019-2020 Intel Corporation. All rights reserved.
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
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/Intel_MDInlineReport.h"

using namespace llvm;
using namespace MDInliningReport;

extern cl::opt<unsigned> IntelInlineReportLevel;

#define DEBUG_TYPE "inlinereportemitter"

//
// This is a common helper class to emit the metadata based inlining
// report from either the legacy or new pass manager.
//

class IREmitterInfo {

public:
  // Main constructor
  IREmitterInfo(Module &M, unsigned Level, unsigned OptLevel,
                unsigned SizeLevel, bool PrepareForLTO)
      : M(M), Level(Level), OptLevel(OptLevel), SizeLevel(SizeLevel),
        PrepareForLTO(PrepareForLTO) {}

  // Run the inline report emitter
  bool runImpl();

private:
  Module &M;              // Module of Inline Report being emitted
  unsigned Level;         // Level of Inline Report being emitted
  unsigned OptLevel;      // Opt Level at which Module was compiled
  unsigned SizeLevel;     // Opt For Size Level at which Module was compiled
  bool PrepareForLTO;     // True in the LTO "Compile Step"

  // Print the linkage character for the Function with name 'CalleeName'.
  void printFunctionLinkageChar(StringRef CalleeName);

  // Print the name of the callee, module name, line, and column for the
  // callee. 'MD' is the metadata tuple with callee name, module name, line,
  // and column info.
  void printCalleeNameModuleLineCol(MDTuple *MD);

  // Print a simple message for a callsite. 'Message' is the message,
  // 'IsInlined' is 'true' if the callsite was inlined. 'IndentCount' is
  // the number of spaces the message should be indented.
  void printSimpleMessage(const char *Message, bool IsInlined,
                          unsigned IndentCount);

  // Print the cost and threshold info. 'MD' is a metadata tuple including
  // the info. 'IsInlined' is 'true' if the callsite was inlined.
  void printCostAndThreshold(MDTuple *MD, bool IsInlined);

  // Print the outer cost and threshold info. 'MD' is a metadata tuple
  // including the info.
  void printOuterCostAndThreshold(MDTuple *MD);

  // Print the inline report info for a callsite. 'MD' is a metadata tuple
  // including the info. 'IndentCount' is the number of spaces to
  // indent the info before printing.
  void printCallSiteInlineReport(Metadata *MD, unsigned IndentCount);

  // Print the inline report info for the callsites of a Function. 'MD' is
  // a metadata tuple including the info. 'IndentCount' is the number of
  // spaces to indent the info before printing.
  void printCallSiteInlineReports(Metadata *MD, unsigned IndentCount);

  // Print the inline report info for a Function. 'Node' is a metadata tuple
  // including the info.
  void printFunctionInlineReportFromMetadata(MDNode *Node);
};

void IREmitterInfo::printFunctionLinkageChar(StringRef CalleeName) {
  if (!(Level & InlineReportOptions::Linkage))
    return;
  if (Function *F = M.getFunction(CalleeName)) {
    llvm::errs() << llvm::getLinkageStr(F) << ' ';
    return;
  }
  // If we can't find a function in the module, then it is dead. Which means it
  // should have local linkage.
  llvm::errs() << "L ";
}

void IREmitterInfo::printCalleeNameModuleLineCol(MDTuple *MD) {
  CallSiteInliningReport CSIR(MD);
  StringRef CalleeName = CSIR.getName();
  printFunctionLinkageChar(CalleeName);
  llvm::errs() << CalleeName;
  unsigned LineNum = 0, ColNum = 0;
  CSIR.getLineAndCol(&LineNum, &ColNum);
  if (Level & InlineReportOptions::File)
    llvm::errs() << ' '
                 << getOpStr(MD->getOperand(CSMDIR_ModuleName), "moduleName: ");
  if ((Level & InlineReportOptions::LineCol) && (LineNum != 0 || ColNum != 0))
    llvm::errs() << " (" << LineNum << "," << ColNum << ")";
}

void IREmitterInfo::printSimpleMessage(const char *Message, bool IsInlined,
                                       unsigned IndentCount) {
#if !INTEL_PRODUCT_RELEASE
  if (Level & InlineReportOptions::Reasons) {
    if (Level & InlineReportOptions::SameLine) {
      llvm::errs() << " ";
    } else {
      llvm::errs() << "\n";
      printIndentCount(IndentCount + 1);
    }
    llvm::errs() << (IsInlined ? "<<" : "[[");
    llvm::errs() << Message;
    llvm::errs() << (IsInlined ? ">>" : "]]");
    llvm::errs() << "\n";
    return;
  }
#endif // !INTEL_PRODUCT_RELEASE
  llvm::errs() << "\n";
}

void IREmitterInfo::printCostAndThreshold(MDTuple *MD, bool IsInlined) {
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

void IREmitterInfo::printOuterCostAndThreshold(MDTuple *MD) {
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

void IREmitterInfo::printCallSiteInlineReport(Metadata *MD,
                                              unsigned IndentCount) {
  MDTuple *CSIR = cast<MDTuple>(MD);
  assert((CSIR->getNumOperands() == CallSiteMDSize) &&
      "Bad call site inlining report format");
  int64_t SuppressPrint = 0;
  getOpVal(CSIR->getOperand(CSMDIR_SuppressPrintReport),
           "isSuppressPrint: ", &SuppressPrint);
  if (SuppressPrint) {
    LLVM_DEBUG(
        dbgs() << "SuppressPrint flag is ON on CallSite, suppress print\n";);
    return;
  }

  int64_t Reason = 0;
  getOpVal(CSIR->getOperand(CSMDIR_InlineReason), "reason: ", &Reason);
  assert(InlineReasonText[Reason].Type != InlPrtNone);
  printIndentCount(IndentCount);
  int64_t IsInlined = 0;
  getOpVal(CSIR->getOperand(CSMDIR_IsInlined), "isInlined: ", &IsInlined);
  if (IsInlined) {
    llvm::errs() << "-> INLINE: ";
    printCalleeNameModuleLineCol(CSIR);
    if (InlineReasonText[Reason].Type == InlPrtCost) {
      printCostAndThreshold(CSIR, true);
    }
    printSimpleMessage(InlineReasonText[Reason].Message, true, IndentCount);
  } else {
    if (InlineReasonText[Reason].Type == InlPrtSpecial) {
      switch (Reason) {
      case NinlrDeleted:
        llvm::errs() << "-> DELETE: ";
        printCalleeNameModuleLineCol(CSIR);
        llvm::errs() << "\n";
        break;
      case NinlrExtern:
        llvm::errs() << "-> EXTERN: ";
        printCalleeNameModuleLineCol(CSIR);
        llvm::errs() << "\n";
        break;
      case NinlrIndirect:
        llvm::errs() << "-> INDIRECT: ";
        printCalleeNameModuleLineCol(CSIR);
        printSimpleMessage(InlineReasonText[Reason].Message, false,
                           IndentCount);
        break;
      case NinlrOuterInlining:
        llvm::errs() << "-> ";
        printCalleeNameModuleLineCol(CSIR);
        printOuterCostAndThreshold(CSIR);
        printSimpleMessage(InlineReasonText[Reason].Message, false,
                           IndentCount);
        break;
      default:
        assert(0);
      }
    } else {
      llvm::errs() << "-> ";
      printCalleeNameModuleLineCol(CSIR);
      if (InlineReasonText[Reason].Type == InlPrtCost)
        printCostAndThreshold(CSIR, false);
      printSimpleMessage(InlineReasonText[Reason].Message, false, IndentCount);
    }
  }
  printCallSiteInlineReports(CSIR->getOperand(CSMDIR_CSs), IndentCount + 1);
}

void IREmitterInfo::printCallSiteInlineReports(Metadata *MD,
                                               unsigned IndentCount) {
  if (!MD)
    return;
  MDTuple *MDCSs = cast<MDTuple>(MD);
  if (!MDCSs || (MDCSs->getNumOperands() < 2))
    return;
  const MDString *S = dyn_cast<MDString>(MDCSs->getOperand(0));
  if (!S || (S->getString() != CallSitesTag))
    return;

  for (unsigned I = 1, E = MDCSs->getNumOperands(); I < E; ++I) {
    printCallSiteInlineReport(MDCSs->getOperand(I), IndentCount);
  }
}

void IREmitterInfo::printFunctionInlineReportFromMetadata(MDNode *Node) {
  assert(isa<MDTuple>(Node) && "Bad format of function inlining report");
  MDTuple *FuncReport = cast<MDTuple>(Node);
  if (!FuncReport) {
    LLVM_DEBUG(dbgs() << "Fail to obtain FuncReport\n";);
    return;
  }
  assert((FuncReport->getNumOperands() == FunctionMDSize) &&
      "Bad format of function inlining report");
  int64_t SuppressPrint = 0;
  getOpVal(FuncReport->getOperand(FMDIR_SuppressPrintReport),
           "isSuppressPrint: ", &SuppressPrint);
  if (SuppressPrint) {
    LLVM_DEBUG(dbgs() << "Hit SuppressPrint flag ON, suppress print\n";);
    return;
  }

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
    llvm::errs() << getOpStr(FuncReport->getOperand(FMDIR_FuncName), "name: ");
    // Module name
    if (Level & InlineReportOptions::File)
      llvm::errs() << ' '
                   << getOpStr(FuncReport->getOperand(FMDIR_ModuleName),
                               "moduleName: ");
    llvm::errs() << "\n\n";
    return;
  }

  // if function is declaration
  int64_t IsDecl = 0;
  getOpVal(FuncReport->getOperand(FMDIR_IsDeclaration),
           "isDeclaration: ", &IsDecl);
  if (IsDecl)
    return;
  llvm::errs() << "COMPILE FUNC: ";
  std::string Name =
      std::string(getOpStr(FuncReport->getOperand(FMDIR_FuncName), "name: "));
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
  printCallSiteInlineReports(FuncReport->getOperand(FMDIR_CSs), 1);
  llvm::errs() << '\n';

  return;
}

bool IREmitterInfo::runImpl() {
  if (!(Level & InlineReportOptions::BasedOnMetadata))
    return false;
  if (PrepareForLTO && (Level & InlineReportOptions::CompositeReport))
    return false;
  llvm::errs() << "---- Begin Inlining Report ---- (via metadata)\n";

  llvm::printOptionValues(OptLevel, SizeLevel);
  NamedMDNode *ModuleInlineReport =
      M.getOrInsertNamedMetadata("intel.module.inlining.report");
  if (!ModuleInlineReport) {
    LLVM_DEBUG(dbgs() << "Failed to obtain ModuleInlineReport\n";);
    return false;
  }

  for (unsigned I = 0, E = ModuleInlineReport->getNumOperands(); I < E; ++I) {
    MDNode *Node = ModuleInlineReport->getOperand(I);
    printFunctionInlineReportFromMetadata(Node);
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
    unsigned Level = IntelInlineReportLevel;
    return IREmitterInfo(M, Level, OptLevel, SizeLevel,
        PrepareForLTO).runImpl();
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
  unsigned Level = IntelInlineReportLevel;
  IREmitterInfo(M, Level, OptLevel, SizeLevel, PrepareForLTO).runImpl();
  return PreservedAnalyses::all();
}

