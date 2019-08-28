//===- CSAMachineFuctionInfo.h - CSA machine function info -------*- C++ -*-==//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares CSA-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSAMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_CSA_CSAMACHINEFUNCTIONINFO_H

#include "CSARegisterInfo.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/Support/ScaledNumber.h"

namespace llvm {
class CSAInstrInfo;
struct CSALicGroup {
  ScaledNumber<uint64_t> executionFrequency;
  unsigned LoopId = 0;
  unsigned LoopDepth = 0;

  CSALicGroup() = default;
};

struct CSAEntryPoint {
  MachineFunction *MF;
  MachineInstr *EntryMI;
  MachineInstr *ReturnMI;
  CSAEntryPoint (
    MachineFunction *MF_t,
    MachineInstr *EntryMI_t,
    MachineInstr *ReturnMI_t
  ) : MF(MF_t), EntryMI(EntryMI_t), ReturnMI(ReturnMI_t) { }
  CSAEntryPoint() : MF(nullptr), EntryMI(nullptr), ReturnMI(nullptr) { }
};

/// CSAMachineFunctionInfo - This class is derived from MachineFunction and
/// contains private CSA target-specific information for each MachineFunction.
///
/// One of the main purposes of this information is to keep track, on the side,
/// of important LIC information, such as depth and initial values.
class CSAMachineFunctionInfo : public MachineFunctionInfo {
  /// A small structure to keep track of LIC information on a per-LIC basis.
  struct LICInfo {
    mutable std::string name;
    short licDepth;
    bool isDeclared;
    bool isGloballyVisible;
    mutable StringMap<std::string> attribs;
    std::shared_ptr<CSALicGroup> licGroup;

    LICInfo() : name(), licDepth(0), isDeclared(true), isGloballyVisible(false), attribs() {}
  };
  DenseMap<unsigned, LICInfo> licInfo;
  void noteNewLIC(unsigned vreg, unsigned licSize, const Twine &name = "", const Twine &fname = "");

  MachineFunction &MF;
  MachineRegisterInfo &MRI;
  const CSAInstrInfo *TII;
  MachineInstr *entryMI = nullptr;
  MachineInstr *returnMI = nullptr;
  int num_call_sites;
  mutable StringSet<> namedLICs;
  mutable unsigned nameCounter;
  unsigned InMemoryLicSXU;
  SmallVector<CSAEntryPoint, 4> CSAEntryPoints;
  bool DoNotEmitAsm;
  virtual void anchor();

  /// Holds for each function where on the stack the Frame Pointer must be
  /// saved. This is used on Prologue and Epilogue to emit FP save/restore
  int FPFrameIndex;

  /// Holds for each function where on the stack the Return Address must be
  /// saved. This is used on Prologue and Epilogue to emit RA save/restore
  int RAFrameIndex;

  /// VarArgsFrameIndex - FrameIndex for start of varargs area.
  int VarArgsFrameIndex;

public:
  explicit CSAMachineFunctionInfo(MachineFunction &MF);
  ~CSAMachineFunctionInfo();

  int getFPFrameIndex() const { return FPFrameIndex; }
  void setFPFrameIndex(int Index) { FPFrameIndex = Index; }

  int getRAFrameIndex() const { return RAFrameIndex; }
  void setRAFrameIndex(int Index) { RAFrameIndex = Index; }

  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
  void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }

  /// Get an entry for extra information for tracking information about LIC
  /// registers.
  const LICInfo &getLICInfo(unsigned regno) const {
    return const_cast<CSAMachineFunctionInfo *>(this)->getLICInfo(regno);
  }
  /// Get an entry for extra information for tracking information about LIC
  /// registers.
  LICInfo &getLICInfo(unsigned regno);

  // This is a potentially temporary approach for handling LIC allocation.
  // LICs are not considered normally allocatable entities for register
  // allocation - all are reserved.  Instead, each case where a LIC is used
  // is explicitly allocated with allocateLIC.  In assembly generation, LIC
  // declarations are emitted for the current routine for each allocated LIC.

  const TargetRegisterClass *licRCFromGenRC(const TargetRegisterClass *RC);

  /// Allocate LIC register of the given register classes (we expect it to be
  /// one of the CI* classes, not I* or RI*). In addition to allocating it, you
  /// may optionally attach a name that will be reflected in the output
  /// assembly (if no name is provided, one is generated based on the LIC size).
  ///
  /// TODO: ensure uniqueness of LIC names.
  unsigned allocateLIC(const TargetRegisterClass *RegClass,
                       const Twine &name = "",
                       const Twine &fname = "",
                       bool isDeclared = true,
                       bool isGloballyVisible = false);
                     

  /// Set the depth for a particular LIC explicitly, rather than the default.
  void setLICDepth(unsigned lic, int amount) {
    getLICInfo(lic).licDepth = amount;
  }

  /// Return the depth of the specified LIC.
  /// A depth of 0 means to use the default value.
  int getLICDepth(unsigned lic) const { return getLICInfo(lic).licDepth; }

  /// Get a user-readable name of the LIC for the virtual register, or return
  /// an empty string if none is known.
  StringRef getLICName(unsigned vreg) const {
    if (Register::isPhysicalRegister(vreg))
      return "";
    return getLICInfo(vreg).name;
  }

  bool getIsDeclared(unsigned vreg) const { return getLICInfo(vreg).isDeclared; }
  bool getIsGloballyVisible(unsigned vreg) const { return getLICInfo(vreg).isGloballyVisible; }
  /// Set the name of the LIC to have the specified name.
  void setLICName(unsigned vreg, const Twine &name, const Twine &fname = "") const;

  /// Get the lic size (e.g., 0, 1, 8, 16, 32, 64) for a register number.
  unsigned getLICSize(unsigned reg) const;

  MachineInstr *getEntryMI() const { return entryMI; }
  MachineInstr *getReturnMI() const { return returnMI; }

  void setEntryMI(MachineInstr *MI) { entryMI = MI; }
  void setReturnMI(MachineInstr *MI) { returnMI = MI; }

  void setNumCallSites(int n) { num_call_sites = n; }
  int getNumCallSites() const { return num_call_sites; }

  void setDoNotEmitAsm(bool t) { DoNotEmitAsm = t; }
  bool getDoNotEmitAsm(void) const { return DoNotEmitAsm; }
  /// Add key+value attribute to this LIC. The value is optional, the absence
  /// of which will generally be interpreted as a "1"/true.
  void addLICAttribute(unsigned reg, const StringRef key, const StringRef value = "") const;

  /// Get an range-based iterator for the attributes currently set. These can
  /// be used to query the singular "getLICAttribute."
  iterator_range<StringMapKeyIterator<std::string>>
    getLICAttributes(unsigned reg) const { return getLICInfo(reg).attribs.keys(); }

  /// Get the value of the given attribute, or "" if not set.
  /// TODO: this interface cannot be used to distinguish between the case where
  /// the attribute is unset and the case where it is set to "".
  StringRef getLICAttribute(unsigned reg, StringRef key) const;

  /// Helper function to determine if a LIC can be deleted
  bool canDeleteLICReg(unsigned reg) const;

  std::shared_ptr<CSALicGroup> getLICGroup(unsigned vreg) const {
    return getLICInfo(vreg).licGroup;
  }

  void setLICGroup(unsigned vreg, std::shared_ptr<CSALicGroup> group) {
    getLICInfo(vreg).licGroup = group;
  }

  /// Return the register that has the input memory edge.
  unsigned getInMemoryLic() const;

  /// Return the register that has the output memory edge.
  unsigned getOutMemoryLic() const;

  void addCSAEntryPoint(
    MachineFunction *MF, MachineInstr *EntryMI, MachineInstr *ReturnMI) {
    CSAEntryPoints.emplace_back(MF,EntryMI,ReturnMI);
  }

  unsigned getNumCSAEntryPoints(void) const {
    return CSAEntryPoints.size();
  }

  const CSAEntryPoint &getCSAEntryPoint(unsigned i) const {
    assert(i < getNumCSAEntryPoints());
    return CSAEntryPoints[i];
  }
};

} // namespace llvm

#endif
