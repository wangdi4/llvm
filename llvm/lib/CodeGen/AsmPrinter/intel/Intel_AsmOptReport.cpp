//===- Intel_OptReportAsmHandler.cpp - Collect and dump OptReport ---------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file provides support for writing OptReport information into
/// an MCStreamer.
///
//===----------------------------------------------------------------------===//

#include "Intel_AsmOptReport.h"
#include "llvm/Analysis/Intel_OptReport/LoopOptReportBuilder.h"
#include "llvm/Analysis/Intel_LoopAnalysis/OptReport/LoopOptReportSupport.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCSectionCOFF.h"
#include "llvm/MC/MCSectionMachO.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include <algorithm>
#include <iterator>

#define DEBUG_TYPE "intel-debug-optrpt-emit"

using namespace llvm;

OptReportAsmPrinterHandler::OptReportAsmPrinterHandler(AsmPrinter *AP)
  : AP(*AP), OutContext(AP->OutContext) {
  LLVM_DEBUG(dbgs() << "--- OptReport Asm Printer Initialized.\n");
}

void OptReportAsmPrinterHandler::beginInstruction(const MachineInstr *MI) {
  // This function will be called only if debug scopes need to be emitted.
  // As long as optimization reports are embeded into the binary only
  // with -g, this should work.  If -g is not specified, then none of the loops
  // will be registered in endFunction(), so we will not even try to create
  // the optimization reports section(s).
  //
  // If we want to enable binary optimization reports without -g, then
  // we have to force calling beginInstruction() in the AsmPrinter.
  if (!FirstInstructionProcessed) {
    // Find exit blocks for some loops (see details below).
    // This code cannot be run in beginFunction(), because MachineLoopInfo
    // is not guaranteed to be computed there.
    FirstInstructionProcessed = true;

    // If a loop has sibling opt-reports, we need to find its "exit"
    // block, which is where we will attach these opt-reports.
    // There are several questions: what to do, if there are multiple
    // exit blocks, and, what if the ordering of the "live" loop's blocks is not
    // continuous.  For the first implementation we keep these problems
    // aside, and just pick the first exit block.
    auto &MLI = getMLI();
    for (const auto *ML : make_range(MLI.rbegin(), MLI.rend())) {
      auto *LoopID = ML->getLoopID();
      if (!LoopID)
        continue;

      LoopOptReport OptReport = LoopOptReport::findOptReportInLoopID(LoopID);
      if (!OptReport)
        continue;

      if (!OptReport.nextSibling())
        continue;

      // The loop's opt-report has sibling opt-reports attached,
      // so we need to find an anchor for them.
      SmallVector<MachineBasicBlock *, 8> ExitBlocks;
      ML->getExitBlocks(ExitBlocks);
      if (!ExitBlocks.empty()) {
        auto EB = ExitBlocks.front();
        LoopToExit[ML] = EB;
        LoopExitBlocks.insert(EB);
      }
    }
  }
  auto *MBB = MI->getParent();
  if (&MBB->front() != MI)
    return;

  auto &MLI = getMLI();
  // Emit labels for:
  //     - loop header blocks
  //     - loop exit blocks
  //     - function entry blocks
  if (!MLI.isLoopHeader(MBB) &&
      LoopExitBlocks.find(MBB) == LoopExitBlocks.end() &&
      MBB != &*(MBB->getParent()->begin()))
    return;

  // TODO (vzakhari 10/3/2018): we may want to emit optimization reports
  //       into the assembly output.  This is the right place to do it.
  //       Basically, we need to get the innermost loop containing MBB,
  //       and print optimization reports for this loop, and for any
  //       ancestor loop, for which this MBB is a header block.
  //       The printing has to be done from the ancestors to the descendants.
  MCSymbol *InstLabel = OutContext.createTempSymbol("opt_report", true);
  getOS().EmitLabel(InstLabel);

  assert(BlockLabels.find(MBB) == BlockLabels.end() &&
         "OptReport label set more than once for block.");

  BlockLabels[MBB] = InstLabel;

  LLVM_DEBUG(dbgs() << "+++ Setting Label '" << InstLabel->getName() <<
             "' to block '" << MBB->getName() << "'\n");
}

void OptReportAsmPrinterHandler::beginFunction(const MachineFunction *MF) {
  LLVM_DEBUG(
    const Function &F = MF->getFunction();
    dbgs() << "*** Begin OptReport Asm Printer for '" << F.getName() <<
        "' ***\n"
  );
}

void OptReportAsmPrinterHandler::endFunction(const MachineFunction *MF) {
  // TODO (vzakhari 10/1/2018): we'd better put optimization reports for
  //       COMDAT functions into the corresponding COMDAT opt-report sections.
  //       We will need to create new sections for different binary file formats.
  auto *Section = AP.getObjFileLowering().getOptReportSection();
  registerFunction(Section);

  // This function anchors the given child/sibling opt-report
  // (and all its child/sibling opt-reports recursively) to the given symbol.
  std::function<void(MCSymbol *, LoopOptReport)>
      registerMissingOptReports =
          [this, &registerMissingOptReports](
              MCSymbol *HeaderSym, LoopOptReport OptReport) {
            if (!HeaderSym || !OptReport)
              return;

            // First, anchor the child/sibling opt-report itself.
            registerLoop(HeaderSym, OptReport);
            // Recursively anchor child opt-reports.
            registerMissingOptReports(HeaderSym, OptReport.firstChild());
            // Recursively anchor sibling opt-reports.
            registerMissingOptReports(HeaderSym, OptReport.nextSibling());
          };

  // Register loops so that parents precede children.
  std::function<void(const MachineLoop *)> addLoop =
      [this, &addLoop, &registerMissingOptReports](const MachineLoop *ML) {

        auto *Header = ML->getHeader();
        assert(Header && "Loop without header.");
        LLVM_DEBUG(dbgs() << "+++ Found Loop with header '" <<
                   Header->getName() << "'\n");

        auto *HeaderSym = BlockLabels[Header];

        auto *LoopID = ML->getLoopID();
        LoopOptReport OptReport = LoopOptReport::findOptReportInLoopID(LoopID);

        // We register the loop, even if OptReport is null.
        // The opt-report entry will be empty, but it will indicate
        // a loop in the generated code.
        if (HeaderSym)
          registerLoop(HeaderSym, OptReport);

        if (!OptReport)
          LLVM_DEBUG(dbgs() << "!!! Loop does not have OptReport. !!!\n");
        if (!HeaderSym)
          LLVM_DEBUG(dbgs() << "!!! Header does not have a label. !!!\n");

        // Process child opt-reports, if any.
        if (OptReport)
          registerMissingOptReports(HeaderSym, OptReport.firstChild());

        // Process child loops.
        auto EntryNode = GraphTraits<const MachineLoop *>::getEntryNode(ML);
        for (auto I = GraphTraits<const MachineLoop *>::child_begin(EntryNode),
                  E = GraphTraits<const MachineLoop *>::child_end(EntryNode);
             I != E; ++I)
          addLoop(*I);

        // If we labeled the "exit" block for this loop, this means
        // we found that sibling opt-reports are attached to it.
        // Here we need to anchor these sibling opt-reports to the "exit"
        // block.
        if (OptReport) {
          auto EBI = LoopToExit.find(ML);
          if (EBI != LoopToExit.end())
            registerMissingOptReports(
                BlockLabels[EBI->second], OptReport.nextSibling());
        }
  };

  // Loop opt-reports attached to the function will be anchored
  // to the first block.
  //
  // TODO (vzakhari 02/11/2019): we have to find better anchors
  //       for such opt-reports.
  LoopOptReport FunOR =
      LoopOptReportTraits<Function>::getOptReport(MF->getFunction());
  if (FunOR)
    registerMissingOptReports(BlockLabels[&*(MF->begin())], FunOR.firstChild());

  // Traverse loops and emit their opt-reports.
  auto &MLI = getMLI();
  for (const auto *ML : make_range(MLI.rbegin(), MLI.rend())) {
    addLoop(ML);
  }

  BlockLabels.clear();
  LoopToExit.clear();
  LoopExitBlocks.clear();

  LLVM_DEBUG(
    const Function &F = MF->getFunction();

    dbgs() << "*** End OptReport Asm Printer for '" << F.getName() <<
        "' ***\n"
  );
}

void OptReportAsmPrinterHandler::emitOptReportExpression(
    MCSymbol *BeginLabel, StringRef Data) {
  MCSymbol *EndLabel =
      OutContext.createTempSymbol("optrpt_entry_end", true);
  const MCExpr *SizeExpr =
      MCBinaryExpr::createSub(
          MCBinaryExpr::createSub(MCSymbolRefExpr::create(EndLabel,
                                                          OutContext),
                                  MCSymbolRefExpr::create(BeginLabel,
                                                          OutContext),
                                  OutContext),
          MCConstantExpr::create(1, OutContext),
          OutContext);
  getOS().EmitLabel(BeginLabel);
  getOS().AddComment("DW_FORM_block1 Length");
  getOS().EmitValue(SizeExpr, 1);
  // The target may not even support Dwarf, but the table uses
  // DW_OP_constu, so we have to hard-coded it here.
  getOS().AddComment("DW_OP_constu");
  getOS().EmitIntValue(0x10, 1);
  getOS().AddComment("Data");
  unsigned EncodedLength = getOS().EmitULEB128Buffer(Data);
  (void)EncodedLength;
  assert(EncodedLength <= 254 &&
         "Maximum 255 bytes may be encoded using DW_FORM_block1.");
  getOS().EmitLabel(EndLabel);
}

void OptReportAsmPrinterHandler::combineFunctionDescs() {
  // Keep the original order of function descriptors that must be
  // emitted to the same section by using stable_sort().
  std::stable_sort(FunctionDescs.begin(), FunctionDescs.end(),
                   // Sort the descriptors using the section names
                   // to get the same ordering from run to run.
                   [](const std::unique_ptr<FunctionDesc> &A,
                      const std::unique_ptr<FunctionDesc> &B) {
                     StringRef AName, BName;
                     auto *ASection = A->Section;
                     auto *BSection = B->Section;
                     // TODO (vzakhari 10/8/2018): check if using getSectionName
                     //       is the right thing for all OSes, including the
                     //       COMDAT cases.
                     if (isa<MCSectionCOFF>(ASection)) {
                       AName = cast<MCSectionCOFF>(ASection)->getSectionName();
                       BName = cast<MCSectionCOFF>(BSection)->getSectionName();
                     } else if (isa<MCSectionELF>(ASection)) {
                       AName = cast<MCSectionELF>(ASection)->getSectionName();
                       BName = cast<MCSectionELF>(BSection)->getSectionName();
                     } else if (isa<MCSectionMachO>(ASection)) {
                       AName = cast<MCSectionMachO>(ASection)->getSectionName();
                       BName = cast<MCSectionMachO>(BSection)->getSectionName();
                     }
                     else {
                       llvm_unreachable("Unsupported OS.");
                     }
                     return AName < BName;
                   });

  // We do not actually erase any function optimization report descriptors
  // from the FunctionDescs, instead, we just move all OptReportDesc's,
  // which need to be located in the same section, into one FunctionDesc.
  // FunctionDesc's that have empty list of optimization reports will
  // not trigger any emission.
  for (auto I = FunctionDescs.begin(), IE = FunctionDescs.end(), CurrI = IE;
       I != IE; ++I) {
    if (CurrI == IE || (*CurrI)->Section != (*I)->Section) {
      assert((*I)->Section && "Invalid nullptr section.");
      CurrI = I;
      continue;
    }

    std::move((*I)->OptReports.begin(), (*I)->OptReports.end(),
              std::back_inserter((*CurrI)->OptReports));
    (*I)->OptReports.clear();
  }
}

void OptReportAsmPrinterHandler::endModule() {
  // TODO (vzakhari 10/2/2018): combine all descriptors for non-COMDAT
  //       functions into one to minimize the overhead of the header
  //       emission.
  combineFunctionDescs();

  unsigned PtrSize = getMAI().getCodePointerSize();
  assert(PtrSize <= 8 && "Unsupported pointer size.");

  for (auto &&FD : FunctionDescs) {
    auto &OptReports = FD->OptReports;
    auto *Section = FD->Section;

    if (OptReports.empty())
      continue;

    //   Opt-report section structure:
    //   --------------- Beginning of notify table ---------------------
    //   Notify table header.
    //      char      ident[];         // ".itt_notify_tab\0"
    //      uint16_t  version;         // Major version 1 in the upper byte,
    //                                 // minor version 2 in the lower byte
    //      uint16_t  header_size;     // byte size of this header structure
    //      uint32_t  num_entries;     // number of entries in the table
    //      uint32_t  strtab_offset;   // string table offset
    //      uint32_t  strtab_size;     // byte size of string table
    //      uint32_t  exprtab_offset;  // expression table offset
    //      uint32_t  exprtab_size;    // byte size of expression table
    //      uint64_t  flags;           // various flags bit set
    //
    //      Offsets are in bytes from the beginning of the section.
    //   ------------------------------------
    //   Array of table entries.
    //   Each entry has the following structure:
    //
    //      uint64_t    anchor_address;    // notify anchor PC address.
    //      uint32_t    probe_region_size; // unused for opt-reports
    //      uint32_t    annotation; // offset of annotation string relative
    //                              // to the beginning of the string table
    //      uint32_t    dw_expr;    // offset of dwarf expression relative
    //                              // the beginning of the expression table
    //   ------------------------------------
    //   Annotation string table
    //     String table is a sequence of null-terminated strings followed
    //     each other. Two different table entries may refer to the same
    //     string in the table.
    //   ------------------------------------
    //   DWARF location expression table
    //     Expression is block of form DW_FORM_block1 - a 1 byte length
    //     followed by 0 to 255 information bytes.
    //
    //   ------------------ End of notify table ---------------------
    getOS().SwitchSection(Section);
    MCSymbol *HeaderStartLabel =
      OutContext.createTempSymbol("optrpt_header_start", true);
    MCSymbol *HeaderEndLabel =
      OutContext.createTempSymbol("optrpt_header_end", true);
    MCSymbol *StrtabStartLabel =
      OutContext.createTempSymbol("optrpt_strtab_start", true);
    MCSymbol *StrtabEndLabel =
      OutContext.createTempSymbol("optrpt_strtab_end", true);
    MCSymbol *ExprtabStartLabel =
      OutContext.createTempSymbol("optrpt_exprtab_start", true);
    MCSymbol *ExprtabEndLabel =
      OutContext.createTempSymbol("optrpt_exprtab_end", true);

    // * Start of table header.
    getOS().AddComment("Optimization Report Table's Header Begin");
    getOS().EmitLabel(HeaderStartLabel);
    // Emit null-terminated identity string.
    SmallString<32> NullTerminatedIdentString(IdentString);
    NullTerminatedIdentString.push_back('\0');
    getOS().emitBytes(NullTerminatedIdentString);

    getOS().AddComment("Table Version 1.2");
    getOS().EmitIntValue(0x0102, 2);
    getOS().AddComment("Header Size");
    getOS().emitAbsoluteSymbolDiff(HeaderEndLabel, HeaderStartLabel, 2);
    // TODO (vzakhari 10/2/2018): right now we only have one entry
    //       that specifies the optimization report version.
    getOS().AddComment("Number Of Entries");
    // Add extra entry for optimization_report_version.
    getOS().EmitIntValue(OptReports.size() + 1, 4);
    getOS().AddComment("Strtab Offset");
    getOS().emitAbsoluteSymbolDiff(StrtabStartLabel, HeaderStartLabel, 4);
    getOS().AddComment("Strtab Size");
    getOS().emitAbsoluteSymbolDiff(StrtabEndLabel, StrtabStartLabel, 4);
    getOS().AddComment("Exprtab Offset");
    getOS().emitAbsoluteSymbolDiff(ExprtabStartLabel, HeaderStartLabel, 4);
    getOS().AddComment("Exprtab Size");
    getOS().emitAbsoluteSymbolDiff(ExprtabEndLabel, ExprtabStartLabel, 4);
    getOS().AddComment("Flags");
    getOS().EmitIntValue(TableFlags::OptReportFlag |
                         (PtrSize <= 4 ? AnchorAddrIs32BitFlag : 0), 8);
    getOS().EmitLabel(HeaderEndLabel);
    // * End of table header.

    // * Start of table entries.
    // Define labels of annotation strings, which are referenced
    // by the entries.
    MCSymbol *OptReportVersionAnnLabel =
      OutContext.createTempSymbol("optrpt_version_ann", true);
    MCSymbol *OptReportAnnLabel =
      OutContext.createTempSymbol("optrpt_ann", true);

    // Emit default entry specifying opt-report version.

    // Defined a label that will mark the optimization_report_version
    // expression entry in the exprtab.
    MCSymbol *OptReportVersionEntryLabel =
      OutContext.createTempSymbol("optrpt_entry_begin", true);

    getOS().AddComment("List Of Table Entries");
    // Emit a redundant label just to attach the above comment to it.
    getOS().EmitLabel(OutContext.createTempSymbol("table_entries_begin", true));
    getOS().AddComment("Anchor");
    getOS().EmitZeros(8);
    getOS().AddComment("Annotation Offset");
    getOS().emitAbsoluteSymbolDiff(OptReportVersionAnnLabel,
                                   StrtabStartLabel, 4);
    getOS().AddComment("Expression Index");
    // This index is unused for optimization_report_version.
    getOS().EmitIntValue(0, 4);

    // Emit opt-report entries.
    for (auto &&OR : OptReports) {
      getOS().AddComment("Anchor");
      getOS().EmitSymbolValue(OR->MBBSym, PtrSize);
      // Anchor value is always 8 bytes, so pad it with zeroes
      // if needed.
      if (PtrSize < 8)
        getOS().EmitZeros(8 - PtrSize);
      getOS().AddComment("Annotation Index");
      getOS().emitAbsoluteSymbolDiff(OptReportAnnLabel,
                                     StrtabStartLabel, 4);
      getOS().AddComment("Expression Index");
      // Define a label that will mark the optimization_report
      // expression entry corresponding to this opt-report
      // in the exprtab.
      OR->EntrySym =
        OutContext.createTempSymbol("optrpt_entry_begin", true);
      getOS().emitAbsoluteSymbolDiff(OR->EntrySym,
                                     ExprtabStartLabel, 4);
    }
    // * End of table entries.

    // * Start of strtab.
    getOS().AddComment("String Table Begin");
    getOS().EmitLabel(StrtabStartLabel);
    // Emit default optimization_report_version annotation string.
    SmallString<32> NullTermVersionAnnotation(OptReportVersionAnnotation);
    NullTermVersionAnnotation.push_back('\0');
    getOS().EmitLabel(OptReportVersionAnnLabel);
    getOS().AddComment(OptReportVersionAnnotation);
    getOS().emitBytes(NullTermVersionAnnotation);

    // Emit optimization_report annotation string.
    SmallString<32> NullTermOptRptAnnotation(OptReportAnnotation);
    NullTermOptRptAnnotation.push_back('\0');
    getOS().EmitLabel(OptReportAnnLabel);
    getOS().AddComment(OptReportAnnotation);
    getOS().emitBytes(NullTermOptRptAnnotation);
    getOS().EmitLabel(StrtabEndLabel);
    // * End of strtab.

    // * Start of exprtab.
    getOS().AddComment("Expressions Table Begin");
    getOS().EmitLabel(ExprtabStartLabel);
    // optimization_report_version expression.
    // TODO (vzakhari 10/2/2018): replace the hard-coded opt-report
    //       version with a query of opt-report library.
    //       Right now, we use version 1.5 (the lower byte is 5,
    //       the upper byte is 1).
    emitOptReportExpression(OptReportVersionEntryLabel, "\x05\x01");

    // Loop opt-report entries.
    for (auto &&OR : OptReports) {
      emitOptReportExpression(
          OR->EntrySym,
          llvm::LoopOptReportSupport::formatBinaryStream(OR->OptReport));
    }

    getOS().EmitLabel(ExprtabEndLabel);
    // * End of exprtab.
  }

  FunctionDescs.clear();
}
