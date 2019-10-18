//===- Intel_AsmOptReport.h - Collect and dump OptReport --------*- C++ -*-===//
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

#ifndef LLVM_LIB_CODEGEN_ASMPRINTER_INTEL_INTEL_ASMOPTREPORT_H
#define LLVM_LIB_CODEGEN_ASMPRINTER_INTEL_INTEL_ASMOPTREPORT_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/Intel_OptReport/LoopOptReport.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/AsmPrinterHandler.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Compiler.h"

namespace llvm {

class LLVM_LIBRARY_VISIBILITY OptReportAsmPrinterHandler
  : public AsmPrinterHandler {
  AsmPrinter &AP;
  MCContext &OutContext;

  static constexpr auto IdentString = ".itt_notify_tab";
  static constexpr auto OptReportVersionAnnotation =
    "optimization_report_version";
  static constexpr auto OptReportAnnotation =
    "optimization_report";

  enum TableFlags {
    PcrelAnchorsFlag = 0x01,      // Unused
    HaveProbeRegionFlag = 0x02,   // Unused
    AnchorAddrIs32BitFlag = 0x04,
    OptReportFlag = 0x08
  };

  bool FirstInstructionProcessed = false;

public:
  OptReportAsmPrinterHandler(AsmPrinter *AP);

  void setSymbolSize(const MCSymbol *, uint64_t) override {}

  void endModule() override;
  void beginFunction (const MachineFunction *MF) override;
  void endFunction (const MachineFunction *MF) override;
  void beginInstruction(const MachineInstr *MI) override;
  void endInstruction () override {}

private:
  MCStreamer &getOS() const {
    assert(AP.OutStreamer && "Null OutStreamer.");
    return *AP.OutStreamer;
  }

  const MachineLoopInfo &getMLI() const {
    assert(AP.MLI && "MachineLoopInfo should have been computed.");

    return *AP.MLI;
  }

  const MCAsmInfo &getMAI() const {
    assert(AP.MAI && "MCAsmInfo should have been initialized.");

    return *AP.MAI;
  }

  // Optimization report descriptor for a single loop, which holds
  // all the information required to emit the optimization report
  // entry for this loop into the optimization reports table.
  class OptReportDesc {
  public:
    // Label of the loop's header block.
    // The label is created and emitted in beginInstruction() for any
    // instruction that is the first one in any loop's header block.
    //
    // Note that multiple loops may have the same header, and so
    // the same label.
    MCSymbol *MBBSym = nullptr;

    // Optimization report for the loop.
    LoopOptReport OptReport;

    // Label of the corresponding entry in the expression table.
    // This is a scratch field initialized and used during the emission
    // in endModule().
    MCSymbol *EntrySym = nullptr;

    OptReportDesc(MCSymbol *MBBSym, LoopOptReport OptReport)
      : MBBSym(MBBSym), OptReport(OptReport) {}
  };

  // Optimization report descriptor for a single function, which holds
  // information about all optimization reports in this function.
  class FunctionDesc {
  public:
    // Section in which the optimization reports table has to be emitted.
    // For non-COMDAT functions we use .debug_opt_report section, for COMDAT
    // functions we create separate sections with .debug_opt_report prefix.
    MCSection *Section = nullptr;

    // A collection of optimization report descriptors for the loops
    // in this function.  It is initialized in endFunction().
    SmallVector<std::unique_ptr<OptReportDesc>, 20> OptReports;

    explicit FunctionDesc(MCSection *Section)
      : Section(Section) {
      assert(Section && "Invalid nullptr section.");
    }
  };

  // Scratch map to communicate labels created for the loops' header
  // blocks between beginInstruction() and endFunction().
  // It is cleared at the end of endFunction().
  DenseMap<const MachineBasicBlock *, MCSymbol *> BlockLabels;

  // Scratch map between MachineLoops with sibling opt-reports attached,
  // and "exit" blocks of these loops.  The map is built on the first call
  // to beginInstruction() and is cleared in endFunction().
  // Finding an "exit" block for a sparsely blocked loop may be tricky,
  // see code in beginInstruction() for the current way of computing
  // the "exit" block.
  DenseMap<const MachineLoop *, MachineBasicBlock *> LoopToExit;

  // A set of MachineBasicBlocks that are "exit" blocks for MachineLoops
  // with sibling opt-reports attached.  We emit labels for blocks
  // in this set in beginInstruction(), so that later we are able to get
  // these labels for MachineLoops via LoopToExit and BlockLabels maps.
  SmallPtrSet<const MachineBasicBlock *, 16> LoopExitBlocks;

  // A collection of optimization report descriptors for the functions
  // in the current module.
  SmallVector<std::unique_ptr<FunctionDesc>, 40> FunctionDescs;

  // Add new optimization report descriptor for a function into
  // FunctionDescs collection.
  template<typename... ArgsT>
  void registerFunction(ArgsT &&... Args) {
    FunctionDescs.push_back(std::make_unique<FunctionDesc>(
                                std::forward<ArgsT>(Args)...));
  }

  // Add new optimization report descriptor for a loop into
  // OptReports collection of the recent registered function's
  // optimization report descriptor.
  template<typename... ArgsT>
  void registerLoop(ArgsT &&... Args) {
    assert(!FunctionDescs.empty() &&
           "Cannot register loop for unregistered function.");

    auto *FunctionDesc = FunctionDescs.back().get();

    FunctionDesc->OptReports.push_back(std::make_unique<OptReportDesc>(
                                           std::forward<ArgsT>(Args)...));
  }

  // Emit optimization report expression into the exprtab portion
  // of the optimization report table.  BeginLabel is a label
  // by which this expression entry is referenced in the table,
  // so it has to be emitted before the expression entry.
  // Data is a stream of bytes encoded by the optimization report
  // library to represent some loop's optimization report remarks.
  void emitOptReportExpression(MCSymbol *BeginLabel, StringRef Data);

  // Combine OptReports vectors for all functions whose optimization
  // reports must be emitted to the same section.  A function which
  // OptReports vector was moved to OptReports vector of another function
  // will have its OptReports vector empty.
  void combineFunctionDescs();
};
} // end namespace llvm

#endif  // LLVM_LIB_CODEGEN_ASMPRINTER_INTEL_INTEL_ASMOPTREPORT_H
