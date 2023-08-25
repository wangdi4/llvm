//===----------- Intel_InlineReportEmitter.cpp - Inlining Report ----------===//
//
// Copyright (C) 2019-2023 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/Intel_MDInlineReport.h"
#include <iomanip>

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
        PrepareForLTO(PrepareForLTO), OS(OptReportOptions::getOutputStream()) {}

  // Run the inline report emitter
  bool runImpl();

private:
  Module &M;          // Module of Inline Report being emitted
  unsigned Level;     // Level of Inline Report being emitted
  unsigned OptLevel;  // Opt Level at which Module was compiled
  unsigned SizeLevel; // Opt For Size Level at which Module was compiled
  bool PrepareForLTO; // True in the LTO "Compile Step"
  std::set<StringRef> // Names of dead Fortran Functions, used
      DeadFortranFunctionNames;       // to provide language char.
  SmallDenseMap<StringRef, StringRef> // Map from dead function name to linkage
      DeadFunctionLinkage;
  raw_ostream &OS; // Stream to print Inline Report to

  // Print the linkage character for the Function with name 'CalleeName'.
  void printFunctionLinkageChar(StringRef CalleeName);

  // Get the language character for the Function with name 'CalleeName'.
  std::string getFunctionLanguageChar(StringRef CalleeName);

  // Print the language character for the Function with name 'CalleeName'.
  void printFunctionLanguageChar(StringRef CalleeName);

  // Print the indirect call specialization method.
  void printICSMethod(InlICSType ICSMethod);

  // Print the name of the callee, module name, line, and column for the
  // callee. 'MD' is the metadata tuple with callee name, module name, line,
  // and column info.
  void printCalleeNameModuleLineCol(MDTuple *MD);

  // Print the name of the target function called by the broker function.
  void printBrokerTargetName(raw_ostream &OS, unsigned Level, MDTuple *MD);

  // Print a simple message for a callsite. 'Message' is the message,
  // 'IsInlined' is 'true' if the callsite was inlined. 'IndentCount' is
  // the number of spaces the message should be indented.
  void printSimpleMessage(const char *Message, bool IsInlined,
                          unsigned IndentCount);

  // Print the cost and benefit info. 'MD' is a metadata tuple including
  // the info. 'IsInlined' is 'true' if the callsite was inlined.
  void printCostAndBenefit(MDTuple *MD, bool IsInlined);

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

  // Using the metadata inline report 'MIR', find the names of the dead
  // Fortran Functions, in case we need to print the language char for all
  // Functions. Also, find and store the linkage information for dead
  // functions to be used when printing linkage for the callsites.
  void findDeadFunctionInfo(NamedMDNode *MIR);

  // Print the inline report info for a Function. 'Node' is a metadata tuple
  // including the info.
  void printFunctionInlineReportFromMetadata(MDNode *Node);
};

void IREmitterInfo::printFunctionLinkageChar(StringRef CalleeName) {
  if (!(Level & InlineReportOptions::Linkage))
    return;
  if (Function *F = M.getFunction(CalleeName)) {
    OS << llvm::getLinkageStr(F) << ' ';
    return;
  }
  // Otherwise it is dead, so look up the value in the map.
  OS << DeadFunctionLinkage[CalleeName] << ' ';
}

void IREmitterInfo::printFunctionLanguageChar(StringRef CalleeName) {
  if (!(Level & InlineReportOptions::Language))
    return;
  if (Function *F = M.getFunction(CalleeName)) {
    OS << llvm::getLanguageStr(F) << ' ';
    return;
  }
  // If we can't find a function in the module, then it is dead.
  // Use the DeadFortranFunctionNames set to find its language.
  bool IsFortran = DeadFortranFunctionNames.count(CalleeName);
  OS << (IsFortran ? "F" : "C") << ' ';
}

void IREmitterInfo::printICSMethod(InlICSType ICSMethod) {
  if (!(Level & InlineReportOptions::Indirects))
    return;
  switch (ICSMethod) {
  case InlICSNone:
    break;
  case InlICSGPT:
    OS << "(GPT) ";
    break;
  case InlICSSFA:
    OS << "(SFA) ";
    break;
  case InlICSPGO:
    OS << "(PGO) ";
    break;
  }
}

std::string IREmitterInfo::getFunctionLanguageChar(StringRef CalleeName) {
  if (Function *F = M.getFunction(CalleeName))
    return llvm::getLanguageStr(F);
  // If we can't find a function in the module, then it is dead.
  // Use the DeadFortranFunctionNames set to find its language.
  bool IsFortran = DeadFortranFunctionNames.count(CalleeName);
  return IsFortran ? std::string("F") : std::string("C");
}

void IREmitterInfo::printCalleeNameModuleLineCol(MDTuple *MD) {
  CallSiteInliningReport CSIR(MD);
  StringRef CalleeName = CSIR.getName();
  int64_t ICSMethod = 0;
  getOpVal(MD->getOperand(CSMDIR_ICSMethod), "icsMethod: ", &ICSMethod);
  printICSMethod(static_cast<InlICSType>(ICSMethod));
  printFunctionLinkageChar(CalleeName);
  printFunctionLanguageChar(CalleeName);
  if ((Level & InlineReportOptions::Demangle) &&
      getFunctionLanguageChar(CalleeName) == "C")
    OS << demangle(CalleeName.str());
  else
    OS << CalleeName;
  unsigned LineNum = 0, ColNum = 0;
  CSIR.getLineAndCol(&LineNum, &ColNum);
  if (Level & InlineReportOptions::File)
    OS << ' ' << getOpStr(MD->getOperand(CSMDIR_ModuleName), "moduleName: ");
  if ((Level & InlineReportOptions::LineCol) && (LineNum != 0 || ColNum != 0))
    OS << " (" << LineNum << "," << ColNum << ")";
}

void IREmitterInfo::printSimpleMessage(const char *Message, bool IsInlined,
                                       unsigned IndentCount) {
#if !INTEL_PRODUCT_RELEASE
  if (Level & InlineReportOptions::Reasons) {
    if (Level & InlineReportOptions::SameLine) {
      OS << " ";
    } else {
      OS << "\n";
      printIndentCount(OS, IndentCount + 1);
    }
    OS << (IsInlined ? "<<" : "[[");
    OS << Message;
    OS << (IsInlined ? ">>" : "]]");
    OS << "\n";
    return;
  }
#endif // !INTEL_PRODUCT_RELEASE
  OS << "\n";
}

void IREmitterInfo::printCostAndBenefit(MDTuple *MD, bool IsInlined) {
  if (!(Level & InlineReportOptions::EarlyExitCost))
    return;
  int64_t CBPairCost = -1;
  getOpVal(MD->getOperand(CSMDIR_CBPairCost), "CBPairCost: ", &CBPairCost);
  int64_t CBPairBenefit = -1;
  getOpVal(MD->getOperand(CSMDIR_CBPairBenefit),
           "CBPairBenefit: ", &CBPairBenefit);
  OS << " (" << CBPairCost;
  if (IsInlined)
    OS << "<=";
  else
    OS << ">";
  OS << CBPairBenefit;
  OS << ")";
}

void IREmitterInfo::printCostAndThreshold(MDTuple *MD, bool IsInlined) {
  if (!(Level & InlineReportOptions::EarlyExitCost))
    return;
  int64_t InlineCost = -1;
  getOpVal(MD->getOperand(CSMDIR_InlineCost), "inlineCost: ", &InlineCost);
  int64_t InlineThreshold = -1;
  getOpVal(MD->getOperand(7), "inlineThreshold: ", &InlineThreshold);

  OS << " (" << InlineCost;
  if (IsInlined)
    OS << "<=";
  else
    OS << ">";
  OS << InlineThreshold;

  int64_t EECost = INT_MAX;
  getOpVal(MD->getOperand(CSMDIR_EarlyExitCost), "earlyExitCost: ", &EECost);
  int64_t EEThreshold = INT_MAX;
  getOpVal(MD->getOperand(CSMDIR_EarlyExitThreshold),
           "earlyExitThreshold: ", &EEThreshold);

  if (((Level & InlineReportOptions::RealCost) != 0) && (EECost != INT_MAX) &&
      !IsInlined) {
    // Under RealCost flag we compute both real and "early exit" costs and
    // thresholds of inlining.
    OS << " [EE:" << EECost;
    OS << ">";
    OS << EEThreshold << "]";
  }
  OS << ")";
}

void IREmitterInfo::printBrokerTargetName(raw_ostream &OS, unsigned Level,
                                          MDTuple *MD) {
  StringRef Name =
      llvm::getOpStr(MD->getOperand(CSMDIR_BrokerTargetName), "name: ");
  OS << "(" << Name << ")\n";
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
  OS << " (" << OuterCost << ">" << InlineCost << ">" << InlineThreshold << ")";
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
  int64_t IsInlined = 0;
  getOpVal(CSIR->getOperand(CSMDIR_IsInlined), "isInlined: ", &IsInlined);
  if (IsInlined) {
    printIndentCount(OS, IndentCount);
    int64_t IsCompact = 0;
    getOpVal(CSIR->getOperand(CSMDIR_IsCompact), "isCompact: ", &IsCompact);
    if (IsCompact)
      OS << "-> <C> INLINE: ";
    else
      OS << "-> INLINE: ";
    printCalleeNameModuleLineCol(CSIR);
    if (InlineReasonText[Reason].Type == InlPrtCost) {
      int64_t IsCostBenefit = 0;
      getOpVal(CSIR->getOperand(CSMDIR_IsCostBenefit),
               "isCostBenefit: ", &IsCostBenefit);
      if (IsCostBenefit)
        printCostAndBenefit(CSIR, true);
      else
        printCostAndThreshold(CSIR, true);
    }
    printSimpleMessage(InlineReasonText[Reason].Message, true, IndentCount);
  } else {
    if (InlineReasonText[Reason].Type == InlPrtSpecial) {
      switch (Reason) {
      case NinlrExtern:
        if (Level & InlineReportOptions::Externs) {
          printIndentCount(OS, IndentCount);
          OS << "-> EXTERN: ";
          printCalleeNameModuleLineCol(CSIR);
          OS << "\n";
        }
        break;
      case NinlrIndirect:
        if (Level & InlineReportOptions::Indirects) {
          printIndentCount(OS, IndentCount);
          OS << "-> INDIRECT:";
          printCalleeNameModuleLineCol(CSIR);
          printSimpleMessage(InlineReasonText[Reason].Message, false,
                             IndentCount);
        }
        break;
      case NinlrDeletedIndCallConv:
        if (Level & InlineReportOptions::Indirects) {
          printIndentCount(OS, IndentCount);
          OS << "-> INDIRECT: DELETE:";
          printCalleeNameModuleLineCol(CSIR);
          printSimpleMessage(InlineReasonText[Reason].Message, false,
                             IndentCount);
        }
        break;
      case NinlrOuterInlining:
        printIndentCount(OS, IndentCount);
        OS << "-> ";
        printCalleeNameModuleLineCol(CSIR);
        printOuterCostAndThreshold(CSIR);
        printSimpleMessage(InlineReasonText[Reason].Message, false,
                           IndentCount);
        break;
      case NinlrBrokerFunction:
        printIndentCount(OS, IndentCount);
        OS << "-> BROKER: ";
        printCalleeNameModuleLineCol(CSIR);
        printBrokerTargetName(OS, Level, CSIR);
        break;
      default:
        assert(0);
      }
    } else if (InlineReasonText[Reason].Type == InlPrtDeleted) {
      printIndentCount(OS, IndentCount);
      OS << "-> DELETE: ";
      printCalleeNameModuleLineCol(CSIR);
      if (InlineReasonText[Reason].Message)
        printSimpleMessage(InlineReasonText[Reason].Message, false,
                           IndentCount);
      else
        OS << "\n";
    } else {
      printIndentCount(OS, IndentCount);
      OS << "-> ";
      printCalleeNameModuleLineCol(CSIR);
      if (InlineReasonText[Reason].Type == InlPrtCost) {
        int64_t IsCostBenefit = 0;
        getOpVal(CSIR->getOperand(CSMDIR_IsCostBenefit),
                 "isCostBenefit: ", &IsCostBenefit);
        if (IsCostBenefit)
          printCostAndBenefit(CSIR, false);
        else
          printCostAndThreshold(CSIR, false);
      }
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

void IREmitterInfo::findDeadFunctionInfo(NamedMDNode *MIR) {
  if (!(Level & InlineReportOptions::Language) &&
      !(Level & InlineReportOptions::Linkage))
    return;
  for (unsigned I = 0, E = MIR->getNumOperands(); I < E; ++I) {
    MDNode *Node = MIR->getOperand(I);
    MDTuple *FuncReport = cast<MDTuple>(Node);
    if (!FuncReport)
      continue;
    int64_t IsDead = 0;
    getOpVal(FuncReport->getOperand(FMDIR_IsDead), "isDead: ", &IsDead);
    if (!IsDead)
      continue;
    StringRef LangStr =
        getOpStr(FuncReport->getOperand(FMDIR_LanguageStr), "language: ");
    StringRef SR = getOpStr(FuncReport->getOperand(FMDIR_FuncName), "name: ");
    if ((Level & InlineReportOptions::Demangle) && LangStr == "C")
      SR = StringRef(demangle(SR.str()));
    StringRef LinkageStr =
        getOpStr(FuncReport->getOperand(FMDIR_LinkageStr), "linkage: ");
    DeadFunctionLinkage[SR] = LinkageStr;
    bool IsFortran = LangStr == "F";
    if (!IsFortran)
      continue;
    DeadFortranFunctionNames.insert(SR);
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
  if (IsDead && (Level & InlineReportOptions::DeadStatics)) {
    OS << "DEAD STATIC FUNC: ";
    if (Level & InlineReportOptions::Linkage) {
      // Linkage letter
      OS << getOpStr(FuncReport->getOperand(FMDIR_LinkageStr), "linkage: ")
         << ' ';
    }
    StringRef LangStr =
        getOpStr(FuncReport->getOperand(FMDIR_LanguageStr), "language: ");
    if (Level & InlineReportOptions::Language)
      // Language letter
      OS << LangStr << ' ';
    // Function name
    StringRef NameStr =
        getOpStr(FuncReport->getOperand(FMDIR_FuncName), "name: ");
    if ((Level & InlineReportOptions::Demangle) && LangStr == "C")
      OS << demangle(NameStr.str());
    else
      OS << NameStr;
    // Module name
    if (Level & InlineReportOptions::File)
      OS << ' '
         << getOpStr(FuncReport->getOperand(FMDIR_ModuleName), "moduleName: ");
    OS << "\n\n";
    return;
  }

  // if function is declaration
  int64_t IsDecl = 0;
  getOpVal(FuncReport->getOperand(FMDIR_IsDeclaration),
           "isDeclaration: ", &IsDecl);
  if (IsDecl)
    return;
  OS << "COMPILE FUNC: ";
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
      OS << Linkage << ' ';
  } else {
    if (Level & InlineReportOptions::Linkage)
      OS << getOpStr(FuncReport->getOperand(FMDIR_LinkageStr), "linkage: ")
         << ' ';
  }
  // Update language last time before printing.
  if (Function *F = M.getFunction(Name)) {
    std::string Language = llvm::getLanguageStr(F);
    std::string LanguageStr = "language: " + Language;
    auto LanguageMD = MDNode::get(
        M.getContext(), llvm::MDString::get(M.getContext(), LanguageStr));
    FuncReport->replaceOperandWith(FMDIR_LanguageStr, LanguageMD);
    if (Level & InlineReportOptions::Language)
      OS << Language << ' ';
  } else {
    if (Level & InlineReportOptions::Language)
      OS << getOpStr(FuncReport->getOperand(FMDIR_LanguageStr), "language: ")
         << ' ';
  }
  StringRef LangStr =
      getOpStr(FuncReport->getOperand(FMDIR_LanguageStr), "language: ");
  if ((Level & InlineReportOptions::Demangle) && LangStr == "C")
    OS << demangle(Name) << '\n';
  else
    OS << Name << '\n';
  printCallSiteInlineReports(FuncReport->getOperand(FMDIR_CSs), 1);
  // Print out the summarized inlines for this function.
  if (Metadata *MDCIs = FuncReport->getOperand(FMDIR_CompactIndexes).get()) {
     OS << "  SUMMARIZED INLINED CALL SITE COUNTS\n";
     auto CIs = cast<MDTuple>(MDCIs);
     Metadata *MDCCs = FuncReport->getOperand(FMDIR_CompactCounts).get();
     auto CCs = cast<MDTuple>(MDCCs);
     assert(CIs->getNumOperands() == CCs->getNumOperands() &&
            "Expecting op match");
     NamedMDNode *ModuleInlineReport =
       M.getOrInsertNamedMetadata("intel.module.inlining.report");
     for (unsigned I = 0, E = CIs->getNumOperands(); I < E; ++I) {
       std::stringstream SS;
       unsigned CompactIndex = 0;
       StringRef CI = getOpStr(CIs->getOperand(I).get(), "Index: ");
       CI.getAsInteger<unsigned>(0, CompactIndex);
       auto FR = cast<MDTuple>(ModuleInlineReport->getOperand(CompactIndex));
       std::string Name =
           std::string(getOpStr(FR->getOperand(FMDIR_FuncName), "name: "));
       unsigned CompactCount = 0;
       StringRef CC = getOpStr(CCs->getOperand(I).get(), "Count: ");
       CC.getAsInteger<unsigned>(0, CompactCount);
       OS << "    ";
       SS << std::setw(5) << " " << CompactCount << " ";
       OS << SS.str() << Name << "\n";
     }
  }
  OS << '\n';
  return;
}

bool IREmitterInfo::runImpl() {
  if (!(Level & InlineReportOptions::BasedOnMetadata))
    return false;
  if (PrepareForLTO && (Level & InlineReportOptions::CompositeReport))
    return false;
  OS << "---- Begin Inlining Report ---- (via metadata)\n";

  if (Level & InlineReportOptions::Options)
    llvm::printOptionValues(OS, OptLevel, SizeLevel);
  NamedMDNode *ModuleInlineReport =
      M.getOrInsertNamedMetadata("intel.module.inlining.report");
  if (!ModuleInlineReport) {
    LLVM_DEBUG(dbgs() << "Failed to obtain ModuleInlineReport\n";);
    return false;
  }

  findDeadFunctionInfo(ModuleInlineReport);
  SmallPtrSet<Function *, 16> FS;
  for (unsigned I = 0, E = ModuleInlineReport->getNumOperands(); I < E; ++I) {
    MDNode *Node = ModuleInlineReport->getOperand(I);
    printFunctionInlineReportFromMetadata(Node);
    MDTuple *FR = cast<MDTuple>(Node);
    std::string Name =
        std::string(getOpStr(FR->getOperand(FMDIR_FuncName), "name: "));
    if (Function *F = M.getFunction(Name))
       FS.insert(F);
  }
  // Make one last pass through the functions looking for "rogue"
  // functions that have inlining report metadata, but were not inserted
  // into the module inlining report table.
  for (auto &F : M.functions()) {
    if (FS.count(&F) || F.isDeclaration())
       continue;
    if (auto MDN = dyn_cast_or_null<MDTuple>(F.getMetadata(FunctionTag)))
       printFunctionInlineReportFromMetadata(MDN);
  }
  OS << "---- End Inlining Report ------ (via metadata)\n";
  delete getMDInlineReport();
  return true;
}

PreservedAnalyses InlineReportEmitterPass::run(Module &M,
                                               ModuleAnalysisManager &AM) {
  unsigned Level = IntelInlineReportLevel;
  IREmitterInfo(M, Level, OptLevel, SizeLevel, PrepareForLTO).runImpl();
  return PreservedAnalyses::all();
}
