//====-- Intel_X86FMA.cpp - Fused Multiply Add optimization ---------------====
//
//      Copyright (c) 2016-2019 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//
// This file defines the pass which finds the best representations of
// the original expression trees consisting of MUL/ADD/SUB/FMA/UnarySub
// operations and performs transformations.
//
//  External interfaces:
//      FunctionPass *llvm::createX86GlobalFMAPass();
//      bool X86GlobalFMA::runOnMachineFunction(MachineFunction &MFunc);
//
// Authors:
// --------
// Vyacheslav Klochkov (vyacheslav.n.klochkov@intel.com)
//

#include "X86.h"
#include "X86InstrBuilder.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/Intel_FMACGCommon.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Target/TargetMachine.h" // INTEL

using namespace llvm;

#define DEBUG_TYPE "global-fma"

/// The internal switch that is used to re-define FMA heuristics.
static cl::opt<unsigned> FMAControl("x86-global-fma-control",
                                    cl::desc("FMA heuristics control."),
                                    cl::init(0), cl::Hidden);

namespace {

class X86FMAImmediateTerm;
class X86FMABasicBlock;
class X86GlobalFMA;

/// This class holds all pre-computed/efficient FMA patterns/DAGs encoded in
/// 64-bit integer values.
class X86FMAPatterns final : public FMAPatterns {
public:
  X86FMAPatterns() : FMAPatterns() {
    // All the code that initializes the patterns storage is in the
    // following included header file.
#include "X86GenMAPatterns.inc"
    init();
  }
};

/// This class describes the known MUL/ADD/SUB/FMA/ZERO/etc operations and
/// utility methods for working with such operations.
///
/// FIXME: This class defines and uses two big static arrays describing known
/// MUL/ADD/SUB/FMA opcodes: AVXOpcodes[] and AVX512Opcodes[]. There are some
/// other existing tables containing the same opcodes (RegularOpcodeGroups[],
/// IntrinOpcodeGroups[], and MemoryFoldTable3[]. See X86InstrInfo.cpp for
/// details.). The information from all tables including the two new tables
/// must be consolidated in one place. Also, if it is possible, then the linear
/// search used now to find an interesting opcode in the table should be
/// replaced with something more efficient.
class FMAOpcodesInfo {
public:
  /// Enum defining the known classes of operations interesting to
  /// FMA optimization.
  typedef enum {
    ADDOpc = 0,
    SUBOpc = 1,
    MULOpc = 2,
    FMA213Opc = 3,
    FMA132Opc = 4,
    FMA231Opc = 5,
    FMS213Opc = 6,
    FMS132Opc = 7,
    FMS231Opc = 8,
    FNMA213Opc = 9,
    FNMA132Opc = 10,
    FNMA231Opc = 11,
    FNMS213Opc = 12,
    FNMS132Opc = 13,
    FNMS231Opc = 14,
    ZEROOpc = 15
  } FMAOpcodeKind;

  /// A structure describing one operation including a register/memory opcode,
  /// a machine value type and an opcode kind.
  struct FMAOpcodeDesc {
    /// Register opcode.
    uint16_t RegOpc;

    /// Memory opcode.
    uint16_t MemOpc;

    /// Machine value type.
    MVT VT;
  };

private:
  /// Known FMA opcodes.
  static const FMAOpcodeDesc VEXOpcodes[15][6];
  static const FMAOpcodeDesc EVEXOpcodes[15][12];

  static const FMAOpcodeDesc *findByOpcode(unsigned Opcode,
                                           FMAOpcodeKind OpcodeKind, bool EVEX);
  static const FMAOpcodeDesc *findByVT(MVT VT, FMAOpcodeKind OpcodeKind,
                                       bool EVEX);

public:
  /// This function returns true iff the given opcode \p Opcode and \p TSFlags
  /// should be recognized by the FMA optimization. Also, if the opcode is
  /// recognized, then machine value type associated with the opcode is returned
  /// in \p VT, the opcode kind is returned in \p OpcodeKind, \p IsMem is set to
  /// true if \p Opcode is a memory opcode.
  /// It is assumed here that all recognized opcodes can be represented as
  /// FMA operations having 3 operands: ((MulSign)(Op1 * Op2) + (AddSign)Op3),
  /// where the MulSign is the sign of the product of the first 2 operands
  /// and AddSign is the sign of the 3rd operand. The MulSign and AddSign signs
  /// are returned in the corresponding parameters \p MulSign and \p AddSign,
  /// and each parameter is set to true iff the corresponding sign is negative.
  /// For example, SUB(a,b) can be represented as (+a*1.0 - c). In this case
  /// \p MulSign must be set to false, and AddSign must be set to true.
  static bool recognizeInstr(const MachineInstr &MI, MVT &VT,
                             FMAOpcodeKind &OpcodeKind, bool &IsMem);

  /// Returns the register form of the opcode of the given opcode kind
  /// \p OpcodeKind and machine value type \p VT. The parameter
  /// \p LookForAVX512 tells if the opcode should be searched among AVX512
  /// opcodes or AVX1/AVX2 opcodes.
  static unsigned getOpcodeOfKind(const X86Subtarget *ST,
                                  FMAOpcodeKind OpcodeKind, MVT VT);

  /// Returns an opcode of FMA213Opc opcode kind with the given signs of
  /// the product of 1st and 2nd FMA operands \p MulSign and the sign of
  /// the 3rd FMA operand \p AddSign.
  static FMAOpcodeKind getFMA213OpcodeKind(bool MulSign, bool AddSign) {
    if (MulSign)
      return AddSign ? FNMS213Opc : FNMA213Opc;
    return AddSign ? FMS213Opc : FMA213Opc;
  }

  /// Returns the sign of the product of the 1st and 2nd  FMA operands for the
  /// given opcode kind \p Kind.
  static bool getMulSign(FMAOpcodeKind Kind) {
    return Kind == FNMA213Opc || Kind == FNMA132Opc || Kind == FNMA231Opc ||
           Kind == FNMS213Opc || Kind == FNMS132Opc || Kind == FNMS231Opc;
  }

  /// Returns the sign of the 3rd FMA operand for the given opcode kind
  /// \p Kind.
  static bool getAddSign(FMAOpcodeKind Kind) {
    return Kind == FNMS213Opc || Kind == FNMS132Opc || Kind == FNMS231Opc ||
           Kind == FMS213Opc  || Kind == FMS132Opc  || Kind == FMS231Opc ||
           Kind == SUBOpc;
  }
};

const FMAOpcodesInfo::FMAOpcodeDesc FMAOpcodesInfo::VEXOpcodes[15][6] = {
  { // ADD
    { X86::VADDSSrr,       X86::VADDSSrm,       MVT::f32   },
    { X86::VADDSDrr,       X86::VADDSDrm,       MVT::f64   },
    { X86::VADDPSrr,       X86::VADDPSrm,       MVT::v4f32 },
    { X86::VADDPDrr,       X86::VADDPDrm,       MVT::v2f64 },
    { X86::VADDPSYrr,      X86::VADDPSYrm,      MVT::v8f32 },
    { X86::VADDPDYrr,      X86::VADDPDYrm,      MVT::v4f64 },
  },
  { // SUB
    { X86::VSUBSSrr,       X86::VSUBSSrm,       MVT::f32   },
    { X86::VSUBSDrr,       X86::VSUBSDrm,       MVT::f64   },
    { X86::VSUBPSrr,       X86::VSUBPSrm,       MVT::v4f32 },
    { X86::VSUBPDrr,       X86::VSUBPDrm,       MVT::v2f64 },
    { X86::VSUBPSYrr,      X86::VSUBPSYrm,      MVT::v8f32 },
    { X86::VSUBPDYrr,      X86::VSUBPDYrm,      MVT::v4f64 },
  },
  { // MUL
    { X86::VMULSSrr,       X86::VMULSSrm,       MVT::f32   },
    { X86::VMULSDrr,       X86::VMULSDrm,       MVT::f64   },
    { X86::VMULPSrr,       X86::VMULPSrm,       MVT::v4f32 },
    { X86::VMULPDrr,       X86::VMULPDrm,       MVT::v2f64 },
    { X86::VMULPSYrr,      X86::VMULPSYrm,      MVT::v8f32 },
    { X86::VMULPDYrr,      X86::VMULPDYrm,      MVT::v4f64 },
  },
  { // FMA213
    { X86::VFMADD213SSr,   X86::VFMADD213SSm,   MVT::f32   },
    { X86::VFMADD213SDr,   X86::VFMADD213SDm,   MVT::f64   },
    { X86::VFMADD213PSr,   X86::VFMADD213PSm,   MVT::v4f32 },
    { X86::VFMADD213PDr,   X86::VFMADD213PDm,   MVT::v2f64 },
    { X86::VFMADD213PSYr,  X86::VFMADD213PSYm,  MVT::v8f32 },
    { X86::VFMADD213PDYr,  X86::VFMADD213PDYm,  MVT::v4f64 },
  },
  { // FMA132
    { X86::VFMADD132SSr,   X86::VFMADD132SSm,   MVT::f32   },
    { X86::VFMADD132SDr,   X86::VFMADD132SDm,   MVT::f64   },
    { X86::VFMADD132PSr,   X86::VFMADD132PSm,   MVT::v4f32 },
    { X86::VFMADD132PDr,   X86::VFMADD132PDm,   MVT::v2f64 },
    { X86::VFMADD132PSYr,  X86::VFMADD132PSYm,  MVT::v8f32 },
    { X86::VFMADD132PDYr,  X86::VFMADD132PDYm,  MVT::v4f64 },
  },
  { // FMA231
    { X86::VFMADD231SSr,   X86::VFMADD231SSm,   MVT::f32   },
    { X86::VFMADD231SDr,   X86::VFMADD231SDm,   MVT::f64   },
    { X86::VFMADD231PSr,   X86::VFMADD231PSm,   MVT::v4f32 },
    { X86::VFMADD231PDr,   X86::VFMADD231PDm,   MVT::v2f64 },
    { X86::VFMADD231PSYr,  X86::VFMADD231PSYm,  MVT::v8f32 },
    { X86::VFMADD231PDYr,  X86::VFMADD231PDYm,  MVT::v4f64 },
  },
  { // FMS213
    { X86::VFMSUB213SSr,   X86::VFMSUB213SSm,   MVT::f32   },
    { X86::VFMSUB213SDr,   X86::VFMSUB213SDm,   MVT::f64   },
    { X86::VFMSUB213PSr,   X86::VFMSUB213PSm,   MVT::v4f32 },
    { X86::VFMSUB213PDr,   X86::VFMSUB213PDm,   MVT::v2f64 },
    { X86::VFMSUB213PSYr,  X86::VFMSUB213PSYm,  MVT::v8f32 },
    { X86::VFMSUB213PDYr,  X86::VFMSUB213PDYm,  MVT::v4f64 },
  },
  { // FMS132
    { X86::VFMSUB132SSr,   X86::VFMSUB132SSm,   MVT::f32   },
    { X86::VFMSUB132SDr,   X86::VFMSUB132SDm,   MVT::f64   },
    { X86::VFMSUB132PSr,   X86::VFMSUB132PSm,   MVT::v4f32 },
    { X86::VFMSUB132PDr,   X86::VFMSUB132PDm,   MVT::v2f64 },
    { X86::VFMSUB132PSYr,  X86::VFMSUB132PSYm,  MVT::v8f32 },
    { X86::VFMSUB132PDYr,  X86::VFMSUB132PDYm,  MVT::v4f64 },
  },
  { // FMS231
    { X86::VFMSUB231SSr,   X86::VFMSUB231SSm,   MVT::f32   },
    { X86::VFMSUB231SDr,   X86::VFMSUB231SDm,   MVT::f64   },
    { X86::VFMSUB231PSr,   X86::VFMSUB231PSm,   MVT::v4f32 },
    { X86::VFMSUB231PDr,   X86::VFMSUB231PDm,   MVT::v2f64 },
    { X86::VFMSUB231PSYr,  X86::VFMSUB231PSYm,  MVT::v8f32 },
    { X86::VFMSUB231PDYr,  X86::VFMSUB231PDYm,  MVT::v4f64 },
  },
  { // FNMA213
    { X86::VFNMADD213SSr,  X86::VFNMADD213SSm,  MVT::f32   },
    { X86::VFNMADD213SDr,  X86::VFNMADD213SDm,  MVT::f64   },
    { X86::VFNMADD213PSr,  X86::VFNMADD213PSm,  MVT::v4f32 },
    { X86::VFNMADD213PDr,  X86::VFNMADD213PDm,  MVT::v2f64 },
    { X86::VFNMADD213PSYr, X86::VFNMADD213PSYm, MVT::v8f32 },
    { X86::VFNMADD213PDYr, X86::VFNMADD213PDYm, MVT::v4f64 },
  },
  { // FNMA132
    { X86::VFNMADD132SSr,  X86::VFNMADD132SSm,  MVT::f32   },
    { X86::VFNMADD132SDr,  X86::VFNMADD132SDm,  MVT::f64   },
    { X86::VFNMADD132PSr,  X86::VFNMADD132PSm,  MVT::v4f32 },
    { X86::VFNMADD132PDr,  X86::VFNMADD132PDm,  MVT::v2f64 },
    { X86::VFNMADD132PSYr, X86::VFNMADD132PSYm, MVT::v8f32 },
    { X86::VFNMADD132PDYr, X86::VFNMADD132PDYm, MVT::v4f64 },
  },
  { // FNMA231
    { X86::VFNMADD231SSr,  X86::VFNMADD231SSm,  MVT::f32   },
    { X86::VFNMADD231SDr,  X86::VFNMADD231SDm,  MVT::f64   },
    { X86::VFNMADD231PSr,  X86::VFNMADD231PSm,  MVT::v4f32 },
    { X86::VFNMADD231PDr,  X86::VFNMADD231PDm,  MVT::v2f64 },
    { X86::VFNMADD231PSYr, X86::VFNMADD231PSYm, MVT::v8f32 },
    { X86::VFNMADD231PDYr, X86::VFNMADD231PDYm, MVT::v4f64 },
  },
  { // FNMS213
    { X86::VFNMSUB213SSr,  X86::VFNMSUB213SSm,  MVT::f32   },
    { X86::VFNMSUB213SDr,  X86::VFNMSUB213SDm,  MVT::f64   },
    { X86::VFNMSUB213PSr,  X86::VFNMSUB213PSm,  MVT::v4f32 },
    { X86::VFNMSUB213PDr,  X86::VFNMSUB213PDm,  MVT::v2f64 },
    { X86::VFNMSUB213PSYr, X86::VFNMSUB213PSYm, MVT::v8f32 },
    { X86::VFNMSUB213PDYr, X86::VFNMSUB213PDYm, MVT::v4f64 },
  },
  { // FNMS132
    { X86::VFNMSUB132SSr,  X86::VFNMSUB132SSm,  MVT::f32   },
    { X86::VFNMSUB132SDr,  X86::VFNMSUB132SDm,  MVT::f64   },
    { X86::VFNMSUB132PSr,  X86::VFNMSUB132PSm,  MVT::v4f32 },
    { X86::VFNMSUB132PDr,  X86::VFNMSUB132PDm,  MVT::v2f64 },
    { X86::VFNMSUB132PSYr, X86::VFNMSUB132PSYm, MVT::v8f32 },
    { X86::VFNMSUB132PDYr, X86::VFNMSUB132PDYm, MVT::v4f64 },
  },
  { // FNMS231
    { X86::VFNMSUB231SSr,  X86::VFNMSUB231SSm,  MVT::f32   },
    { X86::VFNMSUB231SDr,  X86::VFNMSUB231SDm,  MVT::f64   },
    { X86::VFNMSUB231PSr,  X86::VFNMSUB231PSm,  MVT::v4f32 },
    { X86::VFNMSUB231PDr,  X86::VFNMSUB231PDm,  MVT::v2f64 },
    { X86::VFNMSUB231PSYr, X86::VFNMSUB231PSYm, MVT::v8f32 },
    { X86::VFNMSUB231PDYr, X86::VFNMSUB231PDYm, MVT::v4f64 },
  }
};
const FMAOpcodesInfo::FMAOpcodeDesc FMAOpcodesInfo::EVEXOpcodes[15][12] = {
  { // ADD
    { X86::VADDSHZrr,         X86::VADDSHZrm,         MVT::f16    },
    { X86::VADDSSZrr,         X86::VADDSSZrm,         MVT::f32    },
    { X86::VADDSDZrr,         X86::VADDSDZrm,         MVT::f64    },
    { X86::VADDPHZ128rr,      X86::VADDPHZ128rm,      MVT::v8f16  },
    { X86::VADDPSZ128rr,      X86::VADDPSZ128rm,      MVT::v4f32  },
    { X86::VADDPDZ128rr,      X86::VADDPDZ128rm,      MVT::v2f64  },
    { X86::VADDPHZ256rr,      X86::VADDPHZ256rm,      MVT::v16f16 },
    { X86::VADDPSZ256rr,      X86::VADDPSZ256rm,      MVT::v8f32  },
    { X86::VADDPDZ256rr,      X86::VADDPDZ256rm,      MVT::v4f64  },
    { X86::VADDPHZrr,         X86::VADDPHZrm,         MVT::v32f16 },
    { X86::VADDPSZrr,         X86::VADDPSZrm,         MVT::v16f32 },
    { X86::VADDPDZrr,         X86::VADDPDZrm,         MVT::v8f64  },
  },
  { // SUB
    { X86::VSUBSHZrr,         X86::VSUBSHZrm,         MVT::f16    },
    { X86::VSUBSSZrr,         X86::VSUBSSZrm,         MVT::f32    },
    { X86::VSUBSDZrr,         X86::VSUBSDZrm,         MVT::f64    },
    { X86::VSUBPHZ128rr,      X86::VSUBPHZ128rm,      MVT::v8f16  },
    { X86::VSUBPSZ128rr,      X86::VSUBPSZ128rm,      MVT::v4f32  },
    { X86::VSUBPDZ128rr,      X86::VSUBPDZ128rm,      MVT::v2f64  },
    { X86::VSUBPHZ256rr,      X86::VSUBPHZ256rm,      MVT::v16f16 },
    { X86::VSUBPSZ256rr,      X86::VSUBPSZ256rm,      MVT::v8f32  },
    { X86::VSUBPDZ256rr,      X86::VSUBPDZ256rm,      MVT::v4f64  },
    { X86::VSUBPHZrr,         X86::VSUBPHZrm,         MVT::v32f16 },
    { X86::VSUBPSZrr,         X86::VSUBPSZrm,         MVT::v16f32 },
    { X86::VSUBPDZrr,         X86::VSUBPDZrm,         MVT::v8f64  },
  },
  { // MUL
    { X86::VMULSHZrr,         X86::VMULSHZrm,         MVT::f16    },
    { X86::VMULSSZrr,         X86::VMULSSZrm,         MVT::f32    },
    { X86::VMULSDZrr,         X86::VMULSDZrm,         MVT::f64    },
    { X86::VMULPHZ128rr,      X86::VMULPHZ128rm,      MVT::v8f16  },
    { X86::VMULPSZ128rr,      X86::VMULPSZ128rm,      MVT::v4f32  },
    { X86::VMULPDZ128rr,      X86::VMULPDZ128rm,      MVT::v2f64  },
    { X86::VMULPHZ256rr,      X86::VMULPHZ256rm,      MVT::v16f16 },
    { X86::VMULPSZ256rr,      X86::VMULPSZ256rm,      MVT::v8f32  },
    { X86::VMULPDZ256rr,      X86::VMULPDZ256rm,      MVT::v4f64  },
    { X86::VMULPHZrr,         X86::VMULPHZrm,         MVT::v32f16 },
    { X86::VMULPSZrr,         X86::VMULPSZrm,         MVT::v16f32 },
    { X86::VMULPDZrr,         X86::VMULPDZrm,         MVT::v8f64  },
  },
  { // FMA213
    { X86::VFMADD213SHZr,     X86::VFMADD213SHZm,     MVT::f16    },
    { X86::VFMADD213SSZr,     X86::VFMADD213SSZm,     MVT::f32    },
    { X86::VFMADD213SDZr,     X86::VFMADD213SDZm,     MVT::f64    },
    { X86::VFMADD213PHZ128r,  X86::VFMADD213PHZ128m,  MVT::v8f16  },
    { X86::VFMADD213PSZ128r,  X86::VFMADD213PSZ128m,  MVT::v4f32  },
    { X86::VFMADD213PDZ128r,  X86::VFMADD213PDZ128m,  MVT::v2f64  },
    { X86::VFMADD213PHZ256r,  X86::VFMADD213PHZ256m,  MVT::v16f16 },
    { X86::VFMADD213PSZ256r,  X86::VFMADD213PSZ256m,  MVT::v8f32  },
    { X86::VFMADD213PDZ256r,  X86::VFMADD213PDZ256m,  MVT::v4f64  },
    { X86::VFMADD213PHZr,     X86::VFMADD213PHZm,     MVT::v32f16 },
    { X86::VFMADD213PSZr,     X86::VFMADD213PSZm,     MVT::v16f32 },
    { X86::VFMADD213PDZr,     X86::VFMADD213PDZm,     MVT::v8f64  },
  },
  { // FMA132
    { X86::VFMADD132SHZr,     X86::VFMADD132SHZm,     MVT::f16    },
    { X86::VFMADD132SSZr,     X86::VFMADD132SSZm,     MVT::f32    },
    { X86::VFMADD132SDZr,     X86::VFMADD132SDZm,     MVT::f64    },
    { X86::VFMADD132PHZ128r,  X86::VFMADD132PHZ128m,  MVT::v8f16  },
    { X86::VFMADD132PSZ128r,  X86::VFMADD132PSZ128m,  MVT::v4f32  },
    { X86::VFMADD132PDZ128r,  X86::VFMADD132PDZ128m,  MVT::v2f64  },
    { X86::VFMADD132PHZ256r,  X86::VFMADD132PHZ256m,  MVT::v16f16 },
    { X86::VFMADD132PSZ256r,  X86::VFMADD132PSZ256m,  MVT::v8f32  },
    { X86::VFMADD132PDZ256r,  X86::VFMADD132PDZ256m,  MVT::v4f64  },
    { X86::VFMADD132PHZr,     X86::VFMADD132PHZm,     MVT::v32f16 },
    { X86::VFMADD132PSZr,     X86::VFMADD132PSZm,     MVT::v16f32 },
    { X86::VFMADD132PDZr,     X86::VFMADD132PDZm,     MVT::v8f64  },
  },
  { // FMA231
    { X86::VFMADD231SHZr,     X86::VFMADD231SHZm,     MVT::f16    },
    { X86::VFMADD231SSZr,     X86::VFMADD231SSZm,     MVT::f32    },
    { X86::VFMADD231SDZr,     X86::VFMADD231SDZm,     MVT::f64    },
    { X86::VFMADD231PHZ128r,  X86::VFMADD231PHZ128m,  MVT::v8f16  },
    { X86::VFMADD231PSZ128r,  X86::VFMADD231PSZ128m,  MVT::v4f32  },
    { X86::VFMADD231PDZ128r,  X86::VFMADD231PDZ128m,  MVT::v2f64  },
    { X86::VFMADD231PHZ256r,  X86::VFMADD231PHZ256m,  MVT::v16f16 },
    { X86::VFMADD231PSZ256r,  X86::VFMADD231PSZ256m,  MVT::v8f32  },
    { X86::VFMADD231PDZ256r,  X86::VFMADD231PDZ256m,  MVT::v4f64  },
    { X86::VFMADD231PHZr,     X86::VFMADD231PHZm,     MVT::v32f16 },
    { X86::VFMADD231PSZr,     X86::VFMADD231PSZm,     MVT::v16f32 },
    { X86::VFMADD231PDZr,     X86::VFMADD231PDZm,     MVT::v8f64  },
  },
  { // FMS213
    { X86::VFMSUB213SHZr,     X86::VFMSUB213SHZm,     MVT::f16    },
    { X86::VFMSUB213SSZr,     X86::VFMSUB213SSZm,     MVT::f32    },
    { X86::VFMSUB213SDZr,     X86::VFMSUB213SDZm,     MVT::f64    },
    { X86::VFMSUB213PHZ128r,  X86::VFMSUB213PHZ128m,  MVT::v8f16  },
    { X86::VFMSUB213PSZ128r,  X86::VFMSUB213PSZ128m,  MVT::v4f32  },
    { X86::VFMSUB213PDZ128r,  X86::VFMSUB213PDZ128m,  MVT::v2f64  },
    { X86::VFMSUB213PHZ256r,  X86::VFMSUB213PHZ256m,  MVT::v16f16 },
    { X86::VFMSUB213PSZ256r,  X86::VFMSUB213PSZ256m,  MVT::v8f32  },
    { X86::VFMSUB213PDZ256r,  X86::VFMSUB213PDZ256m,  MVT::v4f64  },
    { X86::VFMSUB213PHZr,     X86::VFMSUB213PHZm,     MVT::v32f16 },
    { X86::VFMSUB213PSZr,     X86::VFMSUB213PSZm,     MVT::v16f32 },
    { X86::VFMSUB213PDZr,     X86::VFMSUB213PDZm,     MVT::v8f64  },
  },
  { // FMS132
    { X86::VFMSUB132SHZr,     X86::VFMSUB132SHZm,     MVT::f16    },
    { X86::VFMSUB132SSZr,     X86::VFMSUB132SSZm,     MVT::f32    },
    { X86::VFMSUB132SDZr,     X86::VFMSUB132SDZm,     MVT::f64    },
    { X86::VFMSUB132PHZ128r,  X86::VFMSUB132PHZ128m,  MVT::v8f16  },
    { X86::VFMSUB132PSZ128r,  X86::VFMSUB132PSZ128m,  MVT::v4f32  },
    { X86::VFMSUB132PDZ128r,  X86::VFMSUB132PDZ128m,  MVT::v2f64  },
    { X86::VFMSUB132PHZ256r,  X86::VFMSUB132PHZ256m,  MVT::v16f16 },
    { X86::VFMSUB132PSZ256r,  X86::VFMSUB132PSZ256m,  MVT::v8f32  },
    { X86::VFMSUB132PDZ256r,  X86::VFMSUB132PDZ256m,  MVT::v4f64  },
    { X86::VFMSUB132PHZr,     X86::VFMSUB132PHZm,     MVT::v32f16 },
    { X86::VFMSUB132PSZr,     X86::VFMSUB132PSZm,     MVT::v16f32 },
    { X86::VFMSUB132PDZr,     X86::VFMSUB132PDZm,     MVT::v8f64  },
  },
  { // FMS231
    { X86::VFMSUB231SHZr,     X86::VFMSUB231SHZm,     MVT::f16    },
    { X86::VFMSUB231SSZr,     X86::VFMSUB231SSZm,     MVT::f32    },
    { X86::VFMSUB231SDZr,     X86::VFMSUB231SDZm,     MVT::f64    },
    { X86::VFMSUB231PHZ128r,  X86::VFMSUB231PHZ128m,  MVT::v8f16  },
    { X86::VFMSUB231PSZ128r,  X86::VFMSUB231PSZ128m,  MVT::v4f32  },
    { X86::VFMSUB231PDZ128r,  X86::VFMSUB231PDZ128m,  MVT::v2f64  },
    { X86::VFMSUB231PHZ256r,  X86::VFMSUB231PHZ256m,  MVT::v16f16 },
    { X86::VFMSUB231PSZ256r,  X86::VFMSUB231PSZ256m,  MVT::v8f32  },
    { X86::VFMSUB231PDZ256r,  X86::VFMSUB231PDZ256m,  MVT::v4f64  },
    { X86::VFMSUB231PHZr,     X86::VFMSUB231PHZm,     MVT::v32f16 },
    { X86::VFMSUB231PSZr,     X86::VFMSUB231PSZm,     MVT::v16f32 },
    { X86::VFMSUB231PDZr,     X86::VFMSUB231PDZm,     MVT::v8f64  },
  },
  { // FNMA213
    { X86::VFNMADD213SHZr,    X86::VFNMADD213SHZm,    MVT::f16    },
    { X86::VFNMADD213SSZr,    X86::VFNMADD213SSZm,    MVT::f32    },
    { X86::VFNMADD213SDZr,    X86::VFNMADD213SDZm,    MVT::f64    },
    { X86::VFNMADD213PHZ128r, X86::VFNMADD213PHZ128m, MVT::v8f16  },
    { X86::VFNMADD213PSZ128r, X86::VFNMADD213PSZ128m, MVT::v4f32  },
    { X86::VFNMADD213PDZ128r, X86::VFNMADD213PDZ128m, MVT::v2f64  },
    { X86::VFNMADD213PHZ256r, X86::VFNMADD213PHZ256m, MVT::v16f16 },
    { X86::VFNMADD213PSZ256r, X86::VFNMADD213PSZ256m, MVT::v8f32  },
    { X86::VFNMADD213PDZ256r, X86::VFNMADD213PDZ256m, MVT::v4f64  },
    { X86::VFNMADD213PHZr,    X86::VFNMADD213PHZm,    MVT::v32f16 },
    { X86::VFNMADD213PSZr,    X86::VFNMADD213PSZm,    MVT::v16f32 },
    { X86::VFNMADD213PDZr,    X86::VFNMADD213PDZm,    MVT::v8f64  },
  },
  { // FNMA132
    { X86::VFNMADD132SHZr,    X86::VFNMADD132SHZm,    MVT::f16    },
    { X86::VFNMADD132SSZr,    X86::VFNMADD132SSZm,    MVT::f32    },
    { X86::VFNMADD132SDZr,    X86::VFNMADD132SDZm,    MVT::f64    },
    { X86::VFNMADD132PHZ128r, X86::VFNMADD132PHZ128m, MVT::v8f16  },
    { X86::VFNMADD132PSZ128r, X86::VFNMADD132PSZ128m, MVT::v4f32  },
    { X86::VFNMADD132PDZ128r, X86::VFNMADD132PDZ128m, MVT::v2f64  },
    { X86::VFNMADD132PHZ256r, X86::VFNMADD132PHZ256m, MVT::v16f16 },
    { X86::VFNMADD132PSZ256r, X86::VFNMADD132PSZ256m, MVT::v8f32  },
    { X86::VFNMADD132PDZ256r, X86::VFNMADD132PDZ256m, MVT::v4f64  },
    { X86::VFNMADD132PHZr,    X86::VFNMADD132PHZm,    MVT::v32f16 },
    { X86::VFNMADD132PSZr,    X86::VFNMADD132PSZm,    MVT::v16f32 },
    { X86::VFNMADD132PDZr,    X86::VFNMADD132PDZm,    MVT::v8f64  },
  },
  { // FNMA231
    { X86::VFNMADD231SHZr,    X86::VFNMADD231SHZm,    MVT::f16    },
    { X86::VFNMADD231SSZr,    X86::VFNMADD231SSZm,    MVT::f32    },
    { X86::VFNMADD231SDZr,    X86::VFNMADD231SDZm,    MVT::f64    },
    { X86::VFNMADD231PHZ128r, X86::VFNMADD231PHZ128m, MVT::v8f16  },
    { X86::VFNMADD231PSZ128r, X86::VFNMADD231PSZ128m, MVT::v4f32  },
    { X86::VFNMADD231PDZ128r, X86::VFNMADD231PDZ128m, MVT::v2f64  },
    { X86::VFNMADD231PHZ256r, X86::VFNMADD231PHZ256m, MVT::v16f16 },
    { X86::VFNMADD231PSZ256r, X86::VFNMADD231PSZ256m, MVT::v8f32  },
    { X86::VFNMADD231PDZ256r, X86::VFNMADD231PDZ256m, MVT::v4f64  },
    { X86::VFNMADD231PHZr,    X86::VFNMADD231PHZm,    MVT::v32f16 },
    { X86::VFNMADD231PSZr,    X86::VFNMADD231PSZm,    MVT::v16f32 },
    { X86::VFNMADD231PDZr,    X86::VFNMADD231PDZm,    MVT::v8f64  },
  },
  { // FNMS213
    { X86::VFNMSUB213SHZr,    X86::VFNMSUB213SHZm,    MVT::f16    },
    { X86::VFNMSUB213SSZr,    X86::VFNMSUB213SSZm,    MVT::f32    },
    { X86::VFNMSUB213SDZr,    X86::VFNMSUB213SDZm,    MVT::f64    },
    { X86::VFNMSUB213PHZ128r, X86::VFNMSUB213PHZ128m, MVT::v8f16  },
    { X86::VFNMSUB213PSZ128r, X86::VFNMSUB213PSZ128m, MVT::v4f32  },
    { X86::VFNMSUB213PDZ128r, X86::VFNMSUB213PDZ128m, MVT::v2f64  },
    { X86::VFNMSUB213PHZ256r, X86::VFNMSUB213PHZ256m, MVT::v16f16 },
    { X86::VFNMSUB213PSZ256r, X86::VFNMSUB213PSZ256m, MVT::v8f32  },
    { X86::VFNMSUB213PDZ256r, X86::VFNMSUB213PDZ256m, MVT::v4f64  },
    { X86::VFNMSUB213PHZr,    X86::VFNMSUB213PHZm,    MVT::v32f16 },
    { X86::VFNMSUB213PSZr,    X86::VFNMSUB213PSZm,    MVT::v16f32 },
    { X86::VFNMSUB213PDZr,    X86::VFNMSUB213PDZm,    MVT::v8f64  },
  },
  { // FNMS132
    { X86::VFNMSUB132SHZr,    X86::VFNMSUB132SHZm,    MVT::f16    },
    { X86::VFNMSUB132SSZr,    X86::VFNMSUB132SSZm,    MVT::f32    },
    { X86::VFNMSUB132SDZr,    X86::VFNMSUB132SDZm,    MVT::f64    },
    { X86::VFNMSUB132PHZ128r, X86::VFNMSUB132PHZ128m, MVT::v8f16  },
    { X86::VFNMSUB132PSZ128r, X86::VFNMSUB132PSZ128m, MVT::v4f32  },
    { X86::VFNMSUB132PDZ128r, X86::VFNMSUB132PDZ128m, MVT::v2f64  },
    { X86::VFNMSUB132PHZ256r, X86::VFNMSUB132PHZ256m, MVT::v16f16 },
    { X86::VFNMSUB132PSZ256r, X86::VFNMSUB132PSZ256m, MVT::v8f32  },
    { X86::VFNMSUB132PDZ256r, X86::VFNMSUB132PDZ256m, MVT::v4f64  },
    { X86::VFNMSUB132PHZr,    X86::VFNMSUB132PHZm,    MVT::v32f16 },
    { X86::VFNMSUB132PSZr,    X86::VFNMSUB132PSZm,    MVT::v16f32 },
    { X86::VFNMSUB132PDZr,    X86::VFNMSUB132PDZm,    MVT::v8f64  },
  },
  { // FNMS231
    { X86::VFNMSUB231SHZr,    X86::VFNMSUB231SHZm,    MVT::f16    },
    { X86::VFNMSUB231SSZr,    X86::VFNMSUB231SSZm,    MVT::f32    },
    { X86::VFNMSUB231SDZr,    X86::VFNMSUB231SDZm,    MVT::f64    },
    { X86::VFNMSUB231PHZ128r, X86::VFNMSUB231PHZ128m, MVT::v8f16  },
    { X86::VFNMSUB231PSZ128r, X86::VFNMSUB231PSZ128m, MVT::v4f32  },
    { X86::VFNMSUB231PDZ128r, X86::VFNMSUB231PDZ128m, MVT::v2f64  },
    { X86::VFNMSUB231PHZ256r, X86::VFNMSUB231PHZ256m, MVT::v16f16 },
    { X86::VFNMSUB231PSZ256r, X86::VFNMSUB231PSZ256m, MVT::v8f32  },
    { X86::VFNMSUB231PDZ256r, X86::VFNMSUB231PDZ256m, MVT::v4f64  },
    { X86::VFNMSUB231PHZr,    X86::VFNMSUB231PHZm,    MVT::v32f16 },
    { X86::VFNMSUB231PSZr,    X86::VFNMSUB231PSZm,    MVT::v16f32 },
    { X86::VFNMSUB231PDZr,    X86::VFNMSUB231PDZm,    MVT::v8f64  },
  }
};

const FMAOpcodesInfo::FMAOpcodeDesc *
FMAOpcodesInfo::findByOpcode(unsigned Opcode, FMAOpcodeKind OpcodeKind,
                             bool EVEX) {
  ArrayRef<FMAOpcodeDesc> Table = EVEX ? makeArrayRef(EVEXOpcodes[OpcodeKind])
                                       : makeArrayRef(VEXOpcodes[OpcodeKind]);
  auto I = llvm::find_if(Table, [Opcode](const FMAOpcodeDesc &OD) {
    return OD.RegOpc == Opcode || OD.MemOpc == Opcode;
  });
  if (I != Table.end())
    return &*I;

  return nullptr;
}

const FMAOpcodesInfo::FMAOpcodeDesc *
FMAOpcodesInfo::findByVT(MVT VT, FMAOpcodeKind OpcodeKind, bool EVEX) {
  ArrayRef<FMAOpcodeDesc> Table = EVEX ? makeArrayRef(EVEXOpcodes[OpcodeKind])
                                       : makeArrayRef(VEXOpcodes[OpcodeKind]);
  auto I = llvm::find_if(Table,
                         [VT](const FMAOpcodeDesc &OD) { return OD.VT == VT; });
  if (I != Table.end())
    return &*I;

  return nullptr;
}

// FIXME: It would be great if we could do this directly from TSFlags.
static bool isADDSUBMULIntrinsic(const MCInstrDesc &Desc) {
  // Check if this uses one of the scalar prefixes.
  if ((Desc.TSFlags & X86II::OpPrefixMask) != X86II::XD &&
      (Desc.TSFlags & X86II::OpPrefixMask) != X86II::XS)
    return false;

  // Check the register class of the destination. If it's 128-bit register
  // this is an intrinsic.
  int16_t RegClassID = Desc.OpInfo[0].RegClass;
  if (RegClassID == X86::VR128RegClassID ||
      RegClassID == X86::VR128XRegClassID)
    return true;

  // Otherwise it should be a scalar register class.
  assert((RegClassID == X86::FR16XRegClassID ||
          RegClassID == X86::FR32RegClassID ||
          RegClassID == X86::FR32XRegClassID ||
          RegClassID == X86::FR64RegClassID ||
          RegClassID == X86::FR64XRegClassID) &&
         "Unexpected regclass!");

  return false;
}

bool FMAOpcodesInfo::recognizeInstr(const MachineInstr &MI,
                                    MVT &VT, FMAOpcodeKind &OpcodeKind,
                                    bool &IsMem) {
  unsigned Opcode = MI.getOpcode();
  uint64_t TSFlags = MI.getDesc().TSFlags;
  uint8_t BaseOpcode = X86II::getBaseOpcodeFor(TSFlags);

  // FP MUL/ADD/SUB have well defined encodings. They all lie on the two byte
  // 0x0F two byte opcode map.
  if (((TSFlags & X86II::EncodingMask) == X86II::VEX ||
       (TSFlags & X86II::EncodingMask) == X86II::EVEX) &&
      (TSFlags & X86II::EVEX_B) == 0 && (TSFlags & X86II::EVEX_K) == 0 &&
      ((TSFlags & X86II::OpMapMask) == X86II::TB ||
       // FP16 instructions use TMAP5.
       (TSFlags & X86II::OpMapMask) == X86II::T_MAP5) &&
      (BaseOpcode == 0x59 /*MUL*/ || BaseOpcode == 0x58 /*ADD*/ ||
       BaseOpcode == 0x5c /*SUB*/) &&
      !isADDSUBMULIntrinsic(MI.getDesc())) {
    switch (BaseOpcode) {
    default: llvm_unreachable("Unexpected opcode!");
    case 0x59: OpcodeKind = MULOpc; break;
    case 0x58: OpcodeKind = ADDOpc; break;
    case 0x5c: OpcodeKind = SUBOpc; break;
    }

    bool EVEX = (TSFlags & X86II::EncodingMask) == X86II::EVEX;
    const FMAOpcodeDesc *OD = findByOpcode(Opcode, OpcodeKind, EVEX);
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AVX512_BF16_NE
    if (!OD)
      return false;
#else // INTEL_FEATURE_ISA_AVX512_BF16_NE
    assert(OD != nullptr && "Didn't find in table!");
#endif // INTEL_FEATURE_ISA_AVX512_BF16_NE
#endif // INTEL_CUSTOMIZATION
    IsMem = Opcode == OD->MemOpc;
    VT = OD->VT;
    return true;
  }

  // FMA3 can use X86InstrFMA3 to do initial classification. We still need to
  // reject some cases.
  // FIXME: Put more information into X86InstrFMA3Group?
  const X86InstrFMA3Group *FMA3Group = getFMA3Group(Opcode, TSFlags);
  if (FMA3Group && !FMA3Group->isIntrinsic() &&
      (BaseOpcode & 0x8) == 0x8 && // Reject ADDSUB/SUBADD.
      (TSFlags & X86II::EVEX_B) == 0 && (TSFlags & X86II::EVEX_K) == 0) {
    assert(((TSFlags & X86II::EncodingMask) == X86II::VEX ||
            (TSFlags & X86II::EncodingMask) == X86II::EVEX) &&
           "Unexpected encoding!");
    assert(((TSFlags & X86II::OpMapMask) == X86II::T8 ||
            (TSFlags & X86II::OpMapMask) == X86II::T_MAP6) &&
           "Unexpected opcode map!");
    assert((TSFlags & X86II::OpPrefixMask) == X86II::PD &&
           "Unexpected prefix!");
    assert(((BaseOpcode >= 0x98 && BaseOpcode <= 0x9F) ||
            (BaseOpcode >= 0xA8 && BaseOpcode <= 0xAF) ||
            (BaseOpcode >= 0xB8 && BaseOpcode <= 0xBF)) &&
           "Unexpected opcode!");
    // Form is determined by the first nibble
    bool Form132 = Opcode == FMA3Group->get132Opcode();
    bool Form213 = Opcode == FMA3Group->get213Opcode();
    assert((Form132 || Form213 || Opcode == FMA3Group->get231Opcode()) &&
           "Unexpected FMA form!");

    // Operation is determined by bits 2:1
    switch (BaseOpcode & 0x6) {
    default: llvm_unreachable("Unexpected opcode!");
    case 0x0:
      OpcodeKind = Form132 ? FMA132Opc : Form213 ? FMA213Opc : FMA231Opc;
      break;
    case 0x2:
      OpcodeKind = Form132 ? FMS132Opc : Form213 ? FMS213Opc : FMS231Opc;
      break;
    case 0x4:
      OpcodeKind = Form132 ? FNMA132Opc : Form213 ? FNMA213Opc : FNMA231Opc;
      break;
    case 0x6:
      OpcodeKind = Form132 ? FNMS132Opc : Form213 ? FNMS213Opc : FNMS231Opc;
      break;
    }

    bool EVEX = (TSFlags & X86II::EncodingMask) == X86II::EVEX;
    const FMAOpcodeDesc *OD = findByOpcode(Opcode, OpcodeKind, EVEX);
    assert(OD != nullptr && "Didn't find in table!");
    IsMem = Opcode == OD->MemOpc;
    VT = OD->VT;
    return true;
  }

  // Regular opcodes did not match. Check ZERO opcodes below.
  switch (Opcode) {
  case X86::AVX512_FsFLD0SH:
    VT = MVT::f16;
    break;
  case X86::FsFLD0SS:
  case X86::AVX512_FsFLD0SS:
    VT = MVT::f32;
    break;
  case X86::FsFLD0SD:
  case X86::AVX512_FsFLD0SD:
    VT = MVT::f64;
    break;
  case X86::V_SET0:
  case X86::AVX512_128_SET0:
    // Choose an arbitrary vector type.
    VT = MVT::v2f64;
    break;
  case X86::AVX_SET0:
  case X86::AVX512_256_SET0:
    // Choose an arbitrary vector type.
    VT = MVT::v4f64;
    break;
  case X86::AVX512_512_SET0:
    // Choose an arbitrary vector type.
    VT = MVT::v8f64;
    break;
  default:
    return false;
  }
  IsMem = false;
  OpcodeKind = ZEROOpc;

  return true;
}

unsigned FMAOpcodesInfo::getOpcodeOfKind(const X86Subtarget *ST,
                                         FMAOpcodeKind OpcodeKind, MVT VT) {
  if (OpcodeKind == ZEROOpc) {
    assert((VT.getScalarType() == MVT::f16 || VT.getScalarType() == MVT::f32 ||
            VT.getScalarType() == MVT::f64) &&
           "Only F16, F32 and F64 ZERO vectors/scalars are supported.");
    switch (VT.getSizeInBits()) {
    case 16:
      return X86::AVX512_FsFLD0SH;
    case 32:
      return ST->hasAVX512() ? X86::AVX512_FsFLD0SS : X86::FsFLD0SS;
    case 64:
      return ST->hasAVX512() ? X86::AVX512_FsFLD0SD : X86::FsFLD0SD;
    case 128:
      return ST->hasAVX512() ? X86::AVX512_128_SET0 : X86::V_SET0;
    case 256:
      return ST->hasAVX512() ? X86::AVX512_256_SET0 : X86::AVX_SET0;
    case 512:
      assert(ST->hasAVX512() && "Expected AVX512!");
      return X86::AVX512_512_SET0;
    default:
      break;
    }
    llvm_unreachable("GlobalFMA: Cannot choose appropriate ZERO opcode.");
  }

  bool EVEX = (VT.is128BitVector() || VT.is256BitVector()) ? ST->hasVLX()
                                                           : ST->hasAVX512();
  const FMAOpcodeDesc *OD = findByVT(VT, OpcodeKind, EVEX);
  assert(OD != nullptr && "Didn't find in table!");
  return OD->RegOpc;
}

/// This class does all the optimization work, it goes through the functions,
/// searches for the optimizable expressions and replaces then with more
/// efficient equivalents.
class X86GlobalFMA final : public GlobalFMA {
public:
  /// Pass identification, replacement for typeid.
  static char ID;

  X86GlobalFMA()
      : GlobalFMA(ID), MF(nullptr), ST(nullptr), TII(nullptr), MRI(nullptr) {
    initializeX86GlobalFMAPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return "X86 GlobalFMA"; }

  bool runOnMachineFunction(MachineFunction &MFunc) override;

private:
  /// A reference to the function being currently compiled.
  MachineFunction *MF;

  /// This field is used to get information about available target instruction
  /// sets.
  const X86Subtarget *ST;

  /// This field is used to get information about available target operations.
  const X86InstrInfo *TII;

  /// Machine register information.
  MachineRegisterInfo *MRI;

  /// The bits that are used to define various FMA heuristics in the
  /// internal switch FMAControl/"-x86-global-fma-control".
  enum FMAControls : const unsigned {
    HaswellFMAs = 0x1,
    BroadwellFMAs = 0x2,
    SkylakeFMAs = 0x4,
    TargetFMAsMask = 0xFF,
    ForceFMAs = 0x100,
    TuneForLatency = 0x200,
    TuneForThroughput = 0x400
  };

  typedef enum {
    IsF16,
    IsF32,
    IsF64
  } FloatKind;

  /// Returns true iff all of the passed features \p F are enabled
  /// by the internal switch FMAControl/"-x86-global-fma-control".
  static bool checkAllFMAFeatures(unsigned F) {
    return (FMAControl & F) == F;
  }

  /// Returns true iff any of the passed features \p F is enabled
  /// by the internal switch FMAControl/"fma-control".
  static bool checkAnyOfFMAFeatures(unsigned F) {
    return (FMAControl & F) != 0;
  }

  /// Extracts the load operands from the given mem operands.
  /// This method is the exact copy of the static function extractLoadMMOs()
  /// defined in X86InstrInfo.cpp. The more suitable way to have this
  /// functionality would be to share the implementation from X86InstrInfo.cpp,
  /// but it would also add more merging points and more problems during
  /// pulldowns from llorg.
  static SmallVector<MachineMemOperand *, 2>
  extractLoadMMOs(ArrayRef<MachineMemOperand *> MMOs, MachineFunction &MF)  {
    SmallVector<MachineMemOperand *, 2> LoadMMOs;

    for (MachineMemOperand *MMO : MMOs) {
      if (!MMO->isLoad())
        continue;

      if (!MMO->isStore()) {
        // Reuse the MMO.
        LoadMMOs.push_back(MMO);
      } else {
        // Clone the MMO and unset the store flag.
        LoadMMOs.push_back(MF.getMachineMemOperand(
            MMO->getPointerInfo(),
            MMO->getFlags() & ~MachineMemOperand::MOStore, MMO->getSize(),
            MMO->getBaseAlign(), MMO->getAAInfo(), nullptr,
            MMO->getSyncScopeID(), MMO->getSuccessOrdering(),
            MMO->getFailureOrdering()));
      }
    }
    return LoadMMOs;
  }

  /// Walks through all instructions in the machine basic block, finds
  /// MUL/ADD/FMA operations and creates FMA expressions (FMAExpr) for them.
  /// Returns the number of optimizable expressions found in the block.
  /// The parameter \p MRI is passed to this method to make it possible
  /// to find virtual registers associated with FMARegisterTerms and
  /// having uses that are not recognized as FMAExpr operations.
  std::unique_ptr<FMABasicBlock>
  parseBasicBlock(MachineBasicBlock &MBB) override;

  /// Make sure the virtual register class is legal. If the register class
  /// of operand is the sub-class of the legal register class, insert a
  /// COPY. For example:
  /// Before:
  ///  %1331:vr128 = nofpexcept VFMADD213PDr %1204:vr128(tied-def 0),
  ///                %71:fr64, killed %1534:vr128, implicit $mxcsr
  /// After:
  ///    %1529:vr128 = COPY %71:fr64
  ///    %1331:vr128 = nofpexcept VFMADD213PDr %1204:vr128(tied-def 0),
  ///                  %1529:vr128, killed %1526:vr128, implicit $mxcsr
  Register getRegister(MachineBasicBlock &MBB, MachineInstr *InsertPos,
                       const DebugLoc &DL, const MCInstrDesc &MCID,
                       const TargetRegisterInfo *TRI, const MachineOperand &MO,
                       unsigned MoPos);

  /// Generates a machine instruction and returns a reference to it.
  /// The parameters:
  /// \p Opcode - specifies the opcode of the new instruction.
  /// \p DstReg - the destination virtual register.
  /// \p MOs - Source operands of the new machine instruction.
  /// \p DL - Debug location that should be used for the new instruction.
  MachineInstr *genInstruction(unsigned Opcode, unsigned DstReg,
                               const SmallVectorImpl<MachineOperand> &MOs,
                               const DebugLoc &DL, MachineBasicBlock &MBB,
                               MachineInstr* MI);

  MachineInstr *genInstruction(unsigned Opcode,
                               const SmallVectorImpl<MachineOperand> &MOs,
                               const DebugLoc &DL, MachineBasicBlock &MBB,
                               MachineInstr* MI);

  /// Generates a machine operand for the given FMA term \p Term.
  /// The parameter \p InsertPointMI gives the insertion point for any
  /// additional machine instructions that may need to be generated.
  /// For example, a new instruction performing a load from memory may be
  /// generated if the passed term is a memory term.
  MachineOperand
  generateMachineOperandForFMATerm(FMATerm *Term, MachineInstr *InsertPointMI);

  /// Generates output IR for the FMA expression \p Expr. The given DAG \p Dag
  /// gives the efficient version of the generated expression tree.
  /// The new instruction is inserted into the machine basic block \p MBB.
  /// The exact insertion point for the new instructions is taken from
  /// the field MI of the given FMA expression \p Expr.
  ///
  /// Note that the parameter \p Expr does not have 'const' attribute because
  /// it may be changed during the code-gen phase. For example, virtual
  /// registers created and assigned to operands of \p Expr may be written
  /// to those operands.
  void generateOutputIR(FMAExpr &Expr, const FMADag &Dag) override;

  /// Creates code loading the FP const 1.0 into a new virtual register and
  /// returns that register. The parameter \p VT specifies the data type.
  /// The parameter \p InsertPointMI is the insertion point.
  unsigned createConstOne(MVT VT, MachineInstr *InsertPointMI);

  /// Creates code loading the FP const 1.0 into a new virtual register and
  /// returns that register. The generated code does not use instructions
  /// performing loads from memory, it uses immediate values in GPRs instead,
  /// which then are loaded and broadcasted to the returned vector register.
  /// TODO: This method is added here temporarily as a quick solution
  /// in situations when createConstOne() cannot load a constant from
  /// the pool of constants. It also must be removed when the more general
  /// and complete solution for constants materialization is implemented.
  unsigned createConstOneFromImm(MVT VT, MachineInstr *InsertPointMI);

  /// Selects and returns a broadcast opcode having the input element in GPR.
  /// The register class is also returned in \p RC.
  /// The parameters \p VecBitSize and \p ElemBitSize specify accordingly
  /// the size of the broadcasted vector and the size of the element to be
  /// broadcasted.
  unsigned selectBroadcastFromGPR(unsigned VecBitSize, unsigned ElemBitSize,
                                  const TargetRegisterClass **RC,
                                  FloatKind FK) const;
};

/// X86 specific variant of the immediate term.
class X86FMAImmediateTerm final : public FMAImmediateTerm {
public:
  X86FMAImmediateTerm(MVT VT, FMABasicBlock *BB, int64_t Imm)
      : FMAImmediateTerm(VT, BB, Imm) {
    assert((Imm == 0 || Imm == 1) && "Unexpected special value");
  }

  bool isZero() const override { return Imm == 0; }
  bool isOne() const override { return Imm == 1; }

  /// Prints the FMA expression or term to the given stream \p OS.
  /// The parameter \p PrintAttributes specifies if the caller wants to see
  /// more information and some of FMA node attributes should be printed out.
  void print(raw_ostream &OS, bool PrintAttributes) const override {
    OS << Imm;
    if (PrintAttributes)
      OS << " // Type: " << EVT(VT).getEVTString();
  }
};

/// This class represents one optimizable basic block. It holds all FMAExpr
/// objects created for operations in one MachineBasicBlock.
/// It also keeps references to special terms 0.0 and 1.0 created only once and
/// returned when they are used again.
class X86FMABasicBlock final : public FMABasicBlock {
private:
  /// Register terms used or defined by FMA expressions in the basic block
  /// are stored into map to avoid creation of duplicated terms and
  /// to have quick search through already existing terms using virtual
  /// registers as keys.
  SmallDenseMap<unsigned, std::unique_ptr<FMARegisterTerm>>
      RegisterToFMARegisterTerm;

  /// Zero terms available in the original IR and used in the basic block
  /// are stored into map to recognize such terms by their virtual registers.
  SmallDenseMap<unsigned, FMAImmediateTerm *> RegisterToZeroTerm;

  /// This field maps terms to expressions defining those terms.
  /// For example, for any expression T1 = FMA1(...) there should be a pair
  /// <T1, FMA1> in this map.
  SmallDenseMap<FMARegisterTerm *, FMAExpr *> TermToDefFMA;

  /// Memory terms used by FMA expressions in the basic block are stored into
  /// std::map to avoid creation of duplicated terms and to have quick search
  /// through already existing terms using machine instructions as keys.
  SmallDenseMap<const MachineInstr *, std::unique_ptr<FMAMemoryTerm>>
      MIToFMAMemoryTerm;

  /// Special terms 0.0 and 1.0 created for the basic block.
  SmallDenseMap<unsigned, std::unique_ptr<FMAImmediateTerm>> ZeroTerms;
  std::map<MVT, std::unique_ptr<FMAImmediateTerm>> OneTerms;

public:
  /// Creates an FMA basic block for the given MachineBasicBlock \p MBB.
  X86FMABasicBlock(MachineBasicBlock &MBB) : FMABasicBlock(MBB) {}

  /// Creates an FMA term for a special/const value of the given type \p VT.
  FMAImmediateTerm *createZeroTerm(MVT VT);
  FMAImmediateTerm *createOneTerm(MVT VT);

  /// Creates an FMA term associated with the virtual register used in
  /// the passed machine operand \p MO. The parameter \p VT specifies
  /// the type of the created term.
  FMARegisterTerm *createRegisterTerm(MVT VT, const MachineOperand &MO);

  /// Creates an FMA term associated with a load from memory performed in
  /// the passed machine instruction \p MI. The parameter \p VT specifies
  /// the type of the created term.
  FMAMemoryTerm *createMemoryTerm(MVT VT, MachineInstr *MI);

  /// Creates an FMA term associated with the virtual register used in
  /// the passed machine operand \p MO. The parameter \p VT specifies
  /// the type of the created term.
  FMATerm *createRegisterOrSpecialTerm(MVT VT, const MachineOperand &MO);

  /// Walks through all instructions in the machine basic block, finds
  /// MUL/ADD/FMA operations and creates FMA expressions (FMAExpr) for them.
  /// Returns the number of optimizable expressions found in the block.
  /// The parameter \p MRI is passed to this method to make it possible
  /// to find virtual registers associated with FMARegisterTerms and
  /// having uses that are not recognized as FMAExpr operations.
  unsigned parseBasicBlock(MachineRegisterInfo *MRI);

  /// Updates the <isKill> attribute to machine operands associated with the
  /// last uses of terms.
  void updateIsKilledAttributeForTerms(MachineRegisterInfo *MRI) override {
    for (auto &T : RegisterToFMARegisterTerm)
      T.second->updateIsKilledAttribute(MRI);
    for (auto &T : MIToFMAMemoryTerm)
      T.second->updateIsKilledAttribute(MRI);
    for (auto &T : ZeroTerms)
      T.second->updateIsKilledAttribute(MRI);
    for (auto &T : OneTerms)
      T.second->updateIsKilledAttribute(MRI);
  }

  /// Prints the basic block to the given stream \p OS.
  void print(raw_ostream &OS) const override;
};

FMAImmediateTerm *X86FMABasicBlock::createZeroTerm(MVT VT) {
  // For 0.0 values we only check the vector size, e.g. (v2f64)0.0 can
  // be re-used as (v4f32)0.0.
  auto &Term = ZeroTerms[VT.getSizeInBits()];
  if (!Term)
    Term = std::make_unique<X86FMAImmediateTerm>(VT, this, 0u);
  return Term.get();
}

FMAImmediateTerm *X86FMABasicBlock::createOneTerm(MVT VT) {
  auto &Term = OneTerms[VT];
  if (!Term)
    Term = std::make_unique<X86FMAImmediateTerm>(VT, this, 1u);
  return Term.get();
}

FMARegisterTerm *X86FMABasicBlock::createRegisterTerm(MVT VT,
                                                   const MachineOperand &MO) {
  assert(MO.isReg() && "Cannot create an FMA term for MachineOperand.");
  unsigned Reg = MO.getReg();

  // If there is a term created for this machine operand (or identical to it)
  // then just return the existing term. Otherwise, create a new term.
  auto &Term = RegisterToFMARegisterTerm[Reg];
  if (!Term)
    Term = std::make_unique<FMARegisterTerm>(VT, this, Reg,
                                        RegisterToFMARegisterTerm.size() +
                                        MIToFMAMemoryTerm.size());
  if (MO.isKill())
    Term->setIsEverKilled();
  return Term.get();
}

FMATerm *X86FMABasicBlock::createRegisterOrSpecialTerm(MVT VT,
                                                    const MachineOperand &MO) {
  assert(MO.isReg() && "Cannot create an FMA term for MachineOperand.");
  unsigned Reg = MO.getReg();

  // Try to find a special term associated with the virtual register Reg.
  if (auto *Term = RegisterToZeroTerm.lookup(Reg))
    return Term;

  return createRegisterTerm(VT, MO);
}

FMAMemoryTerm *X86FMABasicBlock::createMemoryTerm(MVT VT, MachineInstr *MI) {
  // If there is an FMA term created for this memory reference then just
  // return the existing term. Otherwise, create a new term.
  auto &Term = MIToFMAMemoryTerm[MI];
  if (!Term)
    // FIXME: If there are two different machine instructions having loads
    // from exactly the same memory and there are no stores to that memory
    // between those loads, then the memory term created for the first machine
    // instruction could be re-used in the next machine instruction.
    // Currently, we just create a new memory term.
    Term = std::make_unique<FMAMemoryTerm>(VT, this, MI,
                                      RegisterToFMARegisterTerm.size() +
                                      MIToFMAMemoryTerm.size());
  return Term.get();
}

unsigned X86FMABasicBlock::parseBasicBlock(MachineRegisterInfo *MRI) {
  LLVM_DEBUG(FMADbg::dbgs() << "FMA-STEP1: FIND FMA OPERATIONS:\n");

  for (auto &MI : getMBB()) {
    MVT VT;
    FMAOpcodesInfo::FMAOpcodeKind OpcodeKind;
    bool IsMem;
    if (!FMAOpcodesInfo::recognizeInstr(MI, VT, OpcodeKind, IsMem))
      continue;
    // Sometimes the fast flags lost during the instruction lowering.
    // Sometimes non fast fp instruction is inlined, so there is mixed
    // instruction that have fast flags and non fast flags. We only
    // optimize thoes that have fast flags declared.
    if (!MI.isFast())
      continue;

    std::array<FMANode *, 3u> Ops;
    FMATerm *MemTerm = IsMem ? createMemoryTerm(VT, &MI) : nullptr;
    switch (OpcodeKind) {
    case FMAOpcodesInfo::MULOpc: // op1 * op2 + 0
      Ops[0] = createRegisterOrSpecialTerm(VT, MI.getOperand(1));
      Ops[1] =
          IsMem ? MemTerm : createRegisterOrSpecialTerm(VT, MI.getOperand(2));
      Ops[2] = createZeroTerm(VT);
      break;
    case FMAOpcodesInfo::ADDOpc: // op1 * 1 + op3
    case FMAOpcodesInfo::SUBOpc: // op1 * 1 - op3
      Ops[0] = createRegisterOrSpecialTerm(VT, MI.getOperand(1));
      Ops[1] = createOneTerm(VT);
      Ops[2] =
          IsMem ? MemTerm : createRegisterOrSpecialTerm(VT, MI.getOperand(2));
      break;
    case FMAOpcodesInfo::FMA132Opc:  // op1 * op3 + op2
    case FMAOpcodesInfo::FMS132Opc:  // op1 * op3 - op2
    case FMAOpcodesInfo::FNMA132Opc: // -op1 * op3 + op2
    case FMAOpcodesInfo::FNMS132Opc: // -op1 * op3 - op2
      Ops[0] = createRegisterOrSpecialTerm(VT, MI.getOperand(1));
      Ops[1] =
          IsMem ? MemTerm : createRegisterOrSpecialTerm(VT, MI.getOperand(3));
      Ops[2] = createRegisterOrSpecialTerm(VT, MI.getOperand(2));
      break;
    case FMAOpcodesInfo::FMA213Opc:  // op2 * op1 + op3
    case FMAOpcodesInfo::FMS213Opc:  // op2 * op1 - op3
    case FMAOpcodesInfo::FNMA213Opc: // -op2 * op1 + op3
    case FMAOpcodesInfo::FNMS213Opc: // -op2 * op1 - op3
      Ops[0] = createRegisterOrSpecialTerm(VT, MI.getOperand(2));
      Ops[1] = createRegisterOrSpecialTerm(VT, MI.getOperand(1));
      Ops[2] =
          IsMem ? MemTerm : createRegisterOrSpecialTerm(VT, MI.getOperand(3));
      break;
    case FMAOpcodesInfo::FMA231Opc:  // op2 * op3 + op1
    case FMAOpcodesInfo::FMS231Opc:  // op2 * op3 - op1
    case FMAOpcodesInfo::FNMA231Opc: // -op2 * op3 + op1
    case FMAOpcodesInfo::FNMS231Opc: // -op2 * op3 - op1
      Ops[0] = createRegisterOrSpecialTerm(VT, MI.getOperand(2));
      Ops[1] =
          IsMem ? MemTerm : createRegisterOrSpecialTerm(VT, MI.getOperand(3));
      Ops[2] = createRegisterOrSpecialTerm(VT, MI.getOperand(1));
      break;
    case FMAOpcodesInfo::ZEROOpc:
      // Handled below.
      break;
    }

    if (OpcodeKind == FMAOpcodesInfo::ZEROOpc) {
      // Put the term 0.0 to the SpecialTerms container and to the map
      // RegisterToZeroTerm, so we can recognize this special term
      // by a virtual register.
      FMAImmediateTerm *ZeroTerm = createZeroTerm(VT);
      RegisterToZeroTerm[MI.getOperand(0).getReg()] = ZeroTerm;
    } else {
      bool MulSign = FMAOpcodesInfo::getMulSign(OpcodeKind);
      bool AddSign = FMAOpcodesInfo::getAddSign(OpcodeKind);
      // Create a new register term for the result of the FMA operation and
      // the FMAExpr node for this operation.
      FMARegisterTerm *ResTerm = createRegisterTerm(VT, MI.getOperand(0));
      createFMA(VT, &MI, ResTerm, Ops, MulSign, AddSign);
    }
  }
  setDefHasUnknownUsersForRegisterTerms(MRI);

  LLVM_DEBUG(FMADbg::dbgs() << *this << "FMA-STEP1 DONE.\n");
  return getFMAs().size();
}

void X86FMABasicBlock::print(raw_ostream &OS) const {
  OS << "\nFMA MEMORY TERMs:\n";
  for (const auto &T : MIToFMAMemoryTerm) {
    OS << "  ";
    T.second->print(OS, true /* PrintAttributes */);
  }

  OS << "\nFMA REGISTER TERMs:\n  ";
  for (const auto &T : RegisterToFMARegisterTerm) {
    T.second->print(OS, true /* PrintAttributes */);
    OS << "\n  ";
  }

  OS << "\nFMA EXPRESSIONs:\n";
  unsigned Index = 0;
  for (const auto &E : getFMAs())
    if (!E->isFullyConsumed()) {
      OS << "  " << Index++ << ": ";
      E->print(OS, true /* PrintAttributes */);
      OS << "\n";
    }
}

/// Loop over all of the basic blocks, performing the FMA optimization for
/// each block separately.
bool X86GlobalFMA::runOnMachineFunction(MachineFunction &MFunc) {
  if (!MFunc.getTarget().Options.DoFMAOpt || skipFunction(MFunc.getFunction()))
    return false;

  MF = &MFunc;
  ST = &MF->getSubtarget<X86Subtarget>();
  TII = ST->getInstrInfo();
  MRI = &MF->getRegInfo();

  // SubTarget must support FMA ISA.
  if (!ST->hasFMA())
    return false;

  // Don't optimize StrictFP functions.
  if (MF->getFunction().hasFnAttribute(Attribute::StrictFP))
    return false;

  // The patterns storage initialization code is not cheap, so it is better
  // to call it only when FMA instructions have a chance to be generated.
  // Also, if the patterns storage is already created/initialized once, it
  // does not make sense to re-initialize it again.
  // This place may need to be updated if/when the patterns storage
  // initialization gets dependent on the target CPU settings. For example,
  // if the patterns are initialized one way for AVX, another way for AVX2,
  // and there are functions with different target CPU settings.
  if (!Patterns)
    Patterns = std::make_unique<X86FMAPatterns>();

  // TODO: CMPLRLLVM-9046: Need to fix the following block of code to
  // have correct latency values for SKL-client and Broadwell without
  // using FMA internal switches. The latency must be already set/written
  // properly to the opcode information, just need to extract/use it properly.
  if ((ST->hasAVX512() &&
       !checkAnyOfFMAFeatures(FMAControls::TargetFMAsMask)) ||
      checkAllFMAFeatures(FMAControls::SkylakeFMAs)) {
    AddSubLatency = 4;
    MulLatency = 4;
    FMALatency = 4;
  } else if (checkAllFMAFeatures(FMAControls::BroadwellFMAs)) {
    AddSubLatency = 3;
    MulLatency = 3;
    FMALatency = 5;
  } else {
    // Haswell is the last available option.
    AddSubLatency = 3;
    MulLatency = 5;
    FMALatency = 5;
  }

  Control.ForceFMAs = checkAllFMAFeatures(FMAControls::ForceFMAs);
  Control.TuneForLatency = checkAllFMAFeatures(FMAControls::TuneForLatency);
  Control.TuneForThroughput =
      checkAllFMAFeatures(FMAControls::TuneForThroughput);

  // And finally do the transformation.
  return GlobalFMA::runOnMachineFunction(*MF);
}

std::unique_ptr<FMABasicBlock>
X86GlobalFMA::parseBasicBlock(MachineBasicBlock &MBB) {
  // Find MUL/ADD/SUB/FMA/etc operations in the input machine instructions
  // and create internal FMA structures for them.
  // Exit if there are not enough optimizable expressions.
  auto FMABB = std::make_unique<X86FMABasicBlock>(MBB);
  if (FMABB->parseBasicBlock(MRI) < 2)
    return nullptr;
  return FMABB;
}

Register X86GlobalFMA::getRegister(MachineBasicBlock &MBB,
                                   MachineInstr *InsertPos, const DebugLoc &DL,
                                   const MCInstrDesc &MCID,
                                   const TargetRegisterInfo *TRI,
                                   const MachineOperand &MO, unsigned MoPos) {
  if (MO.isReg() && Register::isVirtualRegister(MO.getReg())) {
    const TargetRegisterClass *RC = MRI->getRegClassOrNull(MO.getReg());
    if (RC) {
      const TargetRegisterClass *ORC = TII->getRegClass(MCID, MoPos, TRI, *MF);
      if (RC->hasSubClass(ORC)) {
        Register NewReg = MRI->createVirtualRegister(ORC);
        BuildMI(MBB, *InsertPos, DL, TII->get(TargetOpcode::COPY), NewReg)
            .addReg(MO.getReg());
        return NewReg;
      }
    }
  }
  return Register();
}

MachineInstr *
X86GlobalFMA::genInstruction(unsigned Opcode, unsigned DstReg,
                             const SmallVectorImpl<MachineOperand> &MOs,
                             const DebugLoc &DL,
                             MachineBasicBlock &MBB,
                             MachineInstr* MI) {
  const MCInstrDesc &MCID = TII->get(Opcode);
  MachineInstr *NewMI = MF->CreateMachineInstr(MCID, DL, false);
  MachineInstrBuilder MIB(*MF, NewMI);

  MIB.add(MachineOperand::CreateReg(DstReg, true));
  const TargetRegisterInfo *TRI = ST->getRegisterInfo();
  for (unsigned I = 0, E = MOs.size(); I != E; ++I) {
    auto &MO = MOs[I];
    Register NewReg = getRegister(MBB, MI, DL, MCID, TRI, MO, I);
    if (NewReg.isValid())
      MIB.addReg(NewReg);
    else
      MIB.add(MO);
  }
  return NewMI;
}

MachineInstr *
X86GlobalFMA::genInstruction(unsigned Opcode,
                             const SmallVectorImpl<MachineOperand> &MOs,
                             const DebugLoc &DL,
                             MachineBasicBlock &MBB,
                             MachineInstr* MI) {
  const MCInstrDesc &MCID = TII->get(Opcode);
  MachineInstr *NewMI = MF->CreateMachineInstr(MCID, DL, false);
  MachineInstrBuilder MIB(*MF, NewMI);

  const TargetRegisterInfo *TRI = ST->getRegisterInfo();
  for (unsigned I = 0, E = MOs.size(); I != E; ++I) {
    auto &MO = MOs[I];
    Register NewReg = getRegister(MBB, MI, DL, MCID, TRI, MO, I);
    if (NewReg.isValid())
      MIB.addReg(NewReg);
    else
      MIB.add(MO);
  }

  return NewMI;
}

MachineOperand
X86GlobalFMA::generateMachineOperandForFMATerm(FMATerm *Term,
                                               MachineInstr *InsertPointMI) {
  unsigned Reg = Term->getReg();
  if (Reg != 0) {
    // Return a machine operand using virtual register created before.
    // For FMARegisterTerm terms it must be available.
    // For FMAMemoryTerm and FMAImmediateTerm terms it could be created by some
    // previous call of this method.
    return MachineOperand::CreateReg(Reg, false);
  }

  MVT VT = Term->getVT();

  MachineBasicBlock *MBB = InsertPointMI->getParent();
  if (Term->isZero()) {
    unsigned ZeroOpcode =
        FMAOpcodesInfo::getOpcodeOfKind(ST, FMAOpcodesInfo::ZEROOpc, VT);
    SmallVector<MachineOperand, 0> MOs;
    const DebugLoc &DL = InsertPointMI->getDebugLoc();

    Reg = InsertPointMI->getOperand(0).getReg();
    const TargetRegisterClass *RC = MRI->getRegClass(Reg);
    Reg = MRI->createVirtualRegister(RC);

    MachineInstr *NewMI = genInstruction(ZeroOpcode, Reg, MOs, DL, *MBB,
                                         InsertPointMI);
    MBB->insert(InsertPointMI, NewMI);
    // Store the register to the term, so the instruction generated for this
    // special term can be re-used the next time.
    Term->setReg(Reg);
    return MachineOperand::CreateReg(Reg, false);
  }

  if (Term->isOne()) {
    Reg = createConstOne(VT, InsertPointMI);
    // Store the register to the term, so the instruction(s) generated for this
    // special term can be re-used the next time.
    Term->setReg(Reg);
    return MachineOperand::CreateReg(Reg, false);
  }

  assert(isa<FMAMemoryTerm>(Term) && "Unexpected FMA term kind.");

  // 1. Create a new register that would hold the result of the load.
  // BTW, we could re-use the dst operand of MI here; did not do it for
  // additional safety.
  auto *MI = cast<FMAMemoryTerm>(Term)->getMI();
  const TargetRegisterClass *RC = MRI->getRegClass(MI->getOperand(0).getReg());
  Reg = MRI->createVirtualRegister(RC);

  // 2. Generate a load instruction.
  SmallVector<MachineOperand, X86::AddrNumOperands> AddrOps;
  unsigned NumOperands = MI->getNumExplicitOperands();
  for (unsigned I = NumOperands - X86::AddrNumOperands; I != NumOperands; I++) {
    MachineOperand Op = MI->getOperand(I);
    AddrOps.push_back(Op);
  }
  auto MMOs = extractLoadMMOs(MI->memoperands(), *MF);
  SmallVector<MachineInstr *, 1> NewMIs;
  TII->loadRegFromAddr(*MF, Reg, AddrOps, RC, MMOs, NewMIs);

  // In case of FMAMemoryTerm terms the new instruction must be inserted before
  // the original machine instruction performing the load from memory.
  MBB->insert(MI, NewMIs[0]);
  LLVM_DEBUG(FMADbg::dbgs() << "  GENERATE NEW LOAD FROM MEM for MemTerm: "
                            << Term << "\n    " << *NewMIs[0]);

  // 3. This is the first time when the load to a virtual register was
  // generated. Save the register to avoid the load duplication when the same
  // term is used again.
  Term->setReg(Reg);

  // 4. Create a copy of the dst operand of the load and return it.
  return MachineOperand::CreateReg(Reg, false);
}

void X86GlobalFMA::generateOutputIR(FMAExpr &Expr, const FMADag &Dag) {
  auto *FMABB = static_cast<X86FMABasicBlock*>(Expr.getFMABB());
  auto &MBB = FMABB->getMBB();
  auto *MI = Expr.getMI();
  const DebugLoc &DL = MI->getDebugLoc();
  const TargetRegisterClass *RC = MRI->getRegClass(MI->getOperand(0).getReg());

  std::array<unsigned, FMADagCommon::MaxNumOfNodesInDAG> ResultRegs;

  for (unsigned NodeInd = Dag.getNumNodes() - 1; NodeInd != ~0U; NodeInd--) {
    bool AIsTerm, BIsTerm, CIsTerm;
    unsigned A = Dag.getOperand(NodeInd, 0, &AIsTerm);
    unsigned B = Dag.getOperand(NodeInd, 1, &BIsTerm);
    unsigned C = Dag.getOperand(NodeInd, 2, &CIsTerm);

    SmallVector<MachineOperand, 3> MOs;
    SmallVector<FMATerm *, 3> FMAOpnds;
    SmallVector<unsigned, 3> FMAOpndIndices;

    MVT VT = Expr.getVT();
    bool IsAddOrSub = false, IsMul = false;
    bool NegateResult = false, SwapAC = false;

    if (AIsTerm) {
      // If A is 0.0, then the DAG is inefficient, emit error.
      // If A is 1.0, then the DAG is either inefficient or non-standard:
      //   (1.0 * B + C) is non-standard; (B * 1.0 + C) would be Ok.
      //   (1.0 * 0.0 + C) is inefficient;
      //   (1.0 * 1.0 + C) is non-standard; (C * 1.0 + 1.0) would be Ok.
      assert(A != FMADagCommon::TermZERO && A != FMADagCommon::TermONE &&
             "Bad FMA DAG: the operand A cannot be equal to 0.0 or 1.0.");
      FMATerm *Term = Expr.getUsedTermByIndex(A);
      MOs.push_back(generateMachineOperandForFMATerm(Term, MI));
      FMAOpnds.push_back(Term);
    } else {
      MOs.push_back(MachineOperand::CreateReg(ResultRegs[A], false));
      FMAOpnds.push_back(nullptr);
    }
    FMAOpndIndices.push_back(0);

    if (BIsTerm) {
      assert(B != FMADagCommon::TermZERO &&
             "Bad FMA DAG: the operand B cannot be equal to 0.0.");
      if (B == FMADagCommon::TermONE)
        IsAddOrSub = true;
      else {
        FMATerm *Term = Expr.getUsedTermByIndex(B);
        MOs.push_back(generateMachineOperandForFMATerm(Term, MI));
        FMAOpnds.push_back(Term);
        FMAOpndIndices.push_back(1);
      }
    } else {
      MOs.push_back(MachineOperand::CreateReg(ResultRegs[B], false));
      FMAOpnds.push_back(nullptr);
      FMAOpndIndices.push_back(1);
    }

    if (CIsTerm) {
      if (C == FMADagCommon::TermZERO)
        IsMul = true;
      else {
        FMATerm *Term = C == FMADagCommon::TermONE
                            ? FMABB->createOneTerm(VT)
                            : Expr.getUsedTermByIndex(C);
        MOs.push_back(generateMachineOperandForFMATerm(Term, MI));
        FMAOpnds.push_back(Term);
        FMAOpndIndices.push_back(2);
      }
    } else {
      MOs.push_back(MachineOperand::CreateReg(ResultRegs[C], false));
      FMAOpnds.push_back(nullptr);
      FMAOpndIndices.push_back(2);
    }

    unsigned Opcode = 0;
    if (IsAddOrSub) {
      // Generate ADD(A,C) or SUB(A,C).
      // The ADD(-A,C) case is replaced with SUB(C,A), i.e. just swap A and C.
      // The ADD(-A,-C) case is replaced with ADD(A,C), but we also remember
      // that the result of this FMA node must be negated.
      bool AddSign = Dag.getAddSign(NodeInd);
      if (Dag.getMulSign(NodeInd)) {
        if (AddSign) {
          AddSign = false;
          NegateResult = true;
        } else {
          AddSign = true;
          SwapAC = true;
        }
      }
      Opcode = FMAOpcodesInfo::getOpcodeOfKind(
          ST, AddSign ? FMAOpcodesInfo::SUBOpc : FMAOpcodesInfo::ADDOpc, VT);
    } else if (IsMul) {
      // Generate MUL(A,B).
      if (!Dag.getMulSign(NodeInd))
        Opcode =
            FMAOpcodesInfo::getOpcodeOfKind(ST, FMAOpcodesInfo::MULOpc, VT);
      else {
        // Instead of (0 - MUL(A,B)) it is better to have FNMA(A,B,0).
        Opcode =
            FMAOpcodesInfo::getOpcodeOfKind(ST, FMAOpcodesInfo::FNMA213Opc, VT);
        FMATerm *Term = FMABB->createZeroTerm(VT);
        MOs.push_back(generateMachineOperandForFMATerm(Term, MI));
        FMAOpnds.push_back(Term);
        FMAOpndIndices.push_back(0); // dummy value, not used for special terms.
      }
    } else {
      FMAOpcodesInfo::FMAOpcodeKind Kind = FMAOpcodesInfo::getFMA213OpcodeKind(
          Dag.getMulSign(NodeInd), Dag.getAddSign(NodeInd));
      Opcode = FMAOpcodesInfo::getOpcodeOfKind(ST, Kind, VT);
    }

    for (unsigned OpndInd = 0; OpndInd < MOs.size(); OpndInd++) {
      unsigned Index = FMAOpndIndices[OpndInd];
      FMATerm *Term = FMAOpnds[OpndInd];
      if (!Term && Dag.isLastUse(NodeInd, Index))
        MOs[OpndInd].setIsKill(true);
    }

    if (SwapAC)
      std::swap(MOs[0], MOs[1]);

    // Create the new instruction.
    unsigned DstReg = (NodeInd == 0 && !NegateResult)
        ? MI->getOperand(0).getReg()
        : MRI->createVirtualRegister(RC);
    MachineInstr *NewMI = genInstruction(Opcode, DstReg, MOs, DL, MBB, MI);
    NewMI->setFlag(MachineInstr::MIFlag::NoFPExcept);

    for (auto *T : FMAOpnds)
      if (T)
        T->setLastUseMI(NewMI);

    if (NodeInd > 0)
      ResultRegs[NodeInd] = NewMI->getOperand(0).getReg();

    MBB.insert(MI, NewMI);
    LLVM_DEBUG(FMADbg::dbgs() << "  GENERATE NEW INSTRUCTION:\n    " << *NewMI);

    if (NegateResult) {
      // Currently, for the expression -R we generate SUB(0, R).
      unsigned SubOpcode =
          FMAOpcodesInfo::getOpcodeOfKind(ST, FMAOpcodesInfo::SUBOpc, VT);
      FMAImmediateTerm *Term = FMABB->createZeroTerm(VT);
      MOs[0] = generateMachineOperandForFMATerm(Term, MI);
      MOs[1] = MachineOperand::CreateReg(NewMI->getOperand(0).getReg(), false);

      NewMI = genInstruction(SubOpcode, MI->getOperand(0).getReg(), MOs, DL,
                             MBB, MI);
      Term->setLastUseMI(NewMI);
      MBB.insert(MI, NewMI);
      LLVM_DEBUG(FMADbg::dbgs() << "  GENERATE NEW INSTRUCTION:\n    "
                                << *NewMI);
    }
  }
  auto DeleteMI = [&MBB](MachineInstr *MI) {
    LLVM_DEBUG(FMADbg::dbgs() << "  DELETE the MI (it is replaced): \n    "
                              << *MI);
    MBB.erase(MI);
  };
  for (auto *MI : Expr.getConsumedMIs()) {
    // Set DBG_VALUE MI's first operand to noreg.
    if (MI->isDebugValue()) {
      SmallVector<MachineOperand, 3> MOs;
      const DebugLoc &DL = MI->getDebugLoc();
      MachineOperand Noreg = MachineOperand::CreateReg(Register(), false);
      MOs.push_back(Noreg);
      for (unsigned i = 1; i < MI->getNumOperands(); i++)
        MOs.push_back(MI->getOperand(i));
      MachineInstr *NewMI = genInstruction(MI->getOpcode(), MOs, DL, MBB, MI);
      // FIXME: This DBG_VALUE should actually be set to noreg in the pass
      // which do the optimization.
      MI->getParent()->insert(MI, NewMI);
    }
    DeleteMI(MI);
  }
  DeleteMI(MI);
}

unsigned X86GlobalFMA::selectBroadcastFromGPR(unsigned VecBitSize,
                                              unsigned ElemBitSize,
                                              const TargetRegisterClass **RC,
                                              FloatKind FK) const {
  assert(FK == IsF16 || FK == IsF32 ||
         FK == IsF64 && "Unsupported element size in selectBroadcastFromGPR()");
  switch (VecBitSize) {
  case 128:
    assert(ST->hasVLX() && "Expected AVX512VL");
    *RC = &X86::VR128XRegClass;
    return FK == IsF16   ? X86::VPBROADCASTWrZ128rr
           : FK == IsF32 ? X86::VPBROADCASTDrZ128rr
                         : X86::VPBROADCASTQrZ128rr;
  case 256:
    assert(ST->hasVLX() && "Expected AVX512VL");
    *RC = &X86::VR256XRegClass;
    return FK == IsF16   ? X86::VPBROADCASTWrZ256rr
           : FK == IsF32 ? X86::VPBROADCASTDrZ256rr
                         : X86::VPBROADCASTQrZ256rr;
  case 512:
    *RC = &X86::VR512RegClass;
    return FK == IsF16   ? X86::VPBROADCASTWrZrr
           : FK == IsF32 ? X86::VPBROADCASTDrZrr
                         : X86::VPBROADCASTQrZrr;
  default:
    break;
  }
  llvm_unreachable("Unsupported vector size in selectBroadcastFromGPR()");
}

unsigned X86GlobalFMA::createConstOneFromImm(MVT VT,
                                             MachineInstr *InsertPointMI) {
  assert(ST->is64Bit() && "32-bit should always use constant pool!");
  const DebugLoc &DbgLoc = InsertPointMI->getDebugLoc();
  MachineBasicBlock *MBB = InsertPointMI->getParent();
  unsigned Opc;

  MVT ElementType = VT.getScalarType();
  FloatKind FK =
      ElementType == MVT::f16 ? IsF16 :
      ElementType == MVT::f32 ? IsF32
                              : IsF64;

  assert(FK == IsF16 || FK == IsF32 || FK == IsF64 &&
         "Unexpected element type!");
  unsigned GPReg;
  unsigned R;
  uint64_t Imm;
  unsigned VTBitSize = VT.getSizeInBits();

  // Load the FP 1.0 value to GPR register.
  // If the target is 32-bit and the loaded value is 64-bit 1.0 then only
  // the upper 32-bits of fp64 1.0 are loaded to 32-bit GPR register. Such
  // solution would require additional shift left or permute operation
  // on the destination vector register.
  if (FK == IsF16 || FK == IsF32) {
    GPReg = MRI->createVirtualRegister(&X86::GR32RegClass);
    Opc = X86::MOV32ri;
    Imm = FK == IsF32 ? 0x3f800000 : 0x3c00;
  } else {
    GPReg = MRI->createVirtualRegister(&X86::GR64RegClass);
    Opc = X86::MOV64ri;
    Imm = 0x3ff0000000000000LL;
  }
  BuildMI(*MBB, InsertPointMI, DbgLoc, TII->get(Opc), GPReg).addImm(Imm);

  // For scalars, use the instructions that use a scalar register class.
  if (!VT.isVector()) {
    if (FK == IsF16) {
      R = MRI->createVirtualRegister(&X86::VR128XRegClass);
      Opc = X86::VMOVW2SHrr;
    } else if (FK == IsF32) {
      R = MRI->createVirtualRegister(
          ST->getTargetLowering()->getRegClassFor(VT));
      Opc = ST->hasAVX512() ? X86::VMOVDI2SSZrr : X86::VMOVDI2SSrr;
    } else {
      R = MRI->createVirtualRegister(
          ST->getTargetLowering()->getRegClassFor(VT));
      Opc = ST->hasAVX512() ? X86::VMOV64toSDZrr : X86::VMOV64toSDrr;
    }

    BuildMI(*MBB, InsertPointMI, DbgLoc, TII->get(Opc), R)
        .addReg(GPReg, RegState::Kill);

    return R;
  }

  // Use a broadcast from GPR if such is available in the target ISA.
  if (ST->hasAVX512()) {
    bool UseVLX = ST->hasVLX() && VTBitSize <= 256;
    unsigned VecBitSize = UseVLX ? VTBitSize : 512;
    unsigned ElemSize =
        FK == IsF16 ? 16 :
        FK == IsF32 ? 32
                    : 64;

    const TargetRegisterClass *RC;
    Opc = selectBroadcastFromGPR(VecBitSize, ElemSize, &RC, FK);
    unsigned VReg = MRI->createVirtualRegister(RC);
    BuildMI(*MBB, InsertPointMI, DbgLoc, TII->get(Opc), VReg)
        .addReg(GPReg, RegState::Kill);
    R = VReg;

    if (!UseVLX && VTBitSize <= 256) {
      const TargetRegisterClass *ExtractRC;
      unsigned SubIdx;
      if (VTBitSize == 256) {
        ExtractRC = &X86::VR256RegClass;
        SubIdx = X86::sub_ymm;
      } else {
        ExtractRC = &X86::VR128RegClass;
        SubIdx = X86::sub_xmm;
      }
      unsigned ExtractReg = MRI->createVirtualRegister(ExtractRC);
      BuildMI(*MBB, InsertPointMI, DbgLoc, TII->get(TargetOpcode::COPY),
              ExtractReg)
        .addReg(R, RegState::Kill, SubIdx);
      R = ExtractReg;
    }
    return R;
  }

  // For vectors we need to insert into 128 bits and then broadcast.
  unsigned R128 = MRI->createVirtualRegister(&X86::VR128RegClass);
  assert(FK != IsF16 && "FP16 should be handled upper");
  if (FK == IsF32)
    Opc = ST->hasAVX512() ? X86::VMOVDI2PDIZrr : X86::VMOVDI2PDIrr;
  else
    Opc = ST->hasAVX512() ? X86::VMOV64toPQIZrr : X86::VMOV64toPQIrr;
  BuildMI(*MBB, InsertPointMI, DbgLoc, TII->get(Opc), R128)
      .addReg(GPReg, RegState::Kill);

  // Broadcast the lowest element of the XMM to the bigger vector register.
  if (VT.getSizeInBits() == 128) {
    R = MRI->createVirtualRegister(&X86::VR128RegClass);
    Opc = FK == IsF32 ? X86::VBROADCASTSSrr : X86::VMOVDDUPrr;
  } else {
    assert(VT.getSizeInBits() == 256 && "Unexpected size!");
    R = MRI->createVirtualRegister(&X86::VR256RegClass);
    Opc = FK == IsF32 ? X86::VBROADCASTSSYrr : X86::VBROADCASTSDYrr;
  }

  auto MIB = BuildMI(*MBB, InsertPointMI, DbgLoc, TII->get(Opc), R);
  MIB.addReg(R128, RegState::Kill);
  return R;
}

unsigned X86GlobalFMA::createConstOne(MVT VT, MachineInstr *InsertPointMI) {
  // FIXME: Loads of consts from memory are supported only for small and large
  // code models. Use alternative approach for other code models.
  const TargetMachine &TM = MF->getTarget();
  CodeModel::Model CM = TM.getCodeModel();
  if (ST->is64Bit() && CM != CodeModel::Small && CM != CodeModel::Large)
    return createConstOneFromImm(VT, InsertPointMI);

  LLVMContext &Context = MF->getFunction().getContext();

  // Get opcode and type for the created const.
  unsigned Opc;
  Type *Ty;
  switch (VT.SimpleTy) {
  case MVT::f16:
    Opc = X86::VMOVSHZrm_alt;
    Ty = Type::getHalfTy(Context);
    break;
  case MVT::f32:
    Opc = ST->hasAVX512() ? X86::VMOVSSZrm_alt : X86::VMOVSSrm_alt;
    Ty = Type::getFloatTy(Context);
    break;
  case MVT::f64:
    Opc = ST->hasAVX512() ? X86::VMOVSDZrm_alt : X86::VMOVSDrm_alt;
    Ty = Type::getDoubleTy(Context);
    break;
  case MVT::v8f16:
    Opc = ST->hasVLX() ? X86::VMOVAPSZ128rm : X86::VMOVAPSrm;
    Ty = FixedVectorType::get(Type::getHalfTy(Context), 8);
    break;
  case MVT::v4f32:
    Opc = ST->hasVLX() ? X86::VMOVAPSZ128rm : X86::VMOVAPSrm;
    Ty = FixedVectorType::get(Type::getFloatTy(Context), 4);
    break;
  case MVT::v2f64:
    Opc = ST->hasVLX() ? X86::VMOVAPDZ128rm : X86::VMOVAPDrm;
    Ty = FixedVectorType::get(Type::getDoubleTy(Context), 2);
    break;
  case MVT::v16f16:
    Opc = ST->hasVLX() ? X86::VMOVAPSZ256rm : X86::VMOVAPSYrm;
    Ty = FixedVectorType::get(Type::getHalfTy(Context), 16);
    break;
  case MVT::v8f32:
    Opc = ST->hasVLX() ? X86::VMOVAPSZ256rm : X86::VMOVAPSYrm;
    Ty = FixedVectorType::get(Type::getFloatTy(Context), 8);
    break;
  case MVT::v4f64:
    Opc = ST->hasVLX() ? X86::VMOVAPDZ256rm : X86::VMOVAPDYrm;
    Ty = FixedVectorType::get(Type::getDoubleTy(Context), 4);
    break;
  case MVT::v32f16:
    assert(ST->hasAVX512() && "Expected AVX512 enabled!");
    Opc = X86::VMOVAPSZrm;
    Ty = FixedVectorType::get(Type::getHalfTy(Context), 32);
    break;
  case MVT::v16f32:
    assert(ST->hasAVX512() && "Expected AVX512 enabled!");
    Opc = X86::VMOVAPSZrm;
    Ty = FixedVectorType::get(Type::getFloatTy(Context), 16);
    break;
  case MVT::v8f64:
    assert(ST->hasAVX512() && "Expected AVX512 enabled!");
    Opc = X86::VMOVAPDZrm;
    Ty = FixedVectorType::get(Type::getDoubleTy(Context), 8);
    break;
  default:
    llvm_unreachable("Unsupported type of 1.0 const.");
  }

  // TODO: We need some more general solution for constants materialization.
  // For example, that could be a method of MachineInstrBuilder class,
  // which would be available from any MIR optimization.
  // When such solution is available, all the code below must be simplified to
  // a new const creation (i.e. ConstantFP::get(Ty, 1.0)) and a call to some
  // method that would create a const in the pool of constants and generate
  // a load from the pool.

  unsigned BaseReg = 0;
  unsigned char LoadFlags = 0;
  if (ST->isPICStyleStubPIC()) { // Not dynamic-no-pic
    LoadFlags = X86II::MO_PIC_BASE_OFFSET;
    BaseReg = TII->getGlobalBaseReg(MF);
  } else if (ST->isPICStyleGOT()) {
    LoadFlags = X86II::MO_GOTOFF;
    BaseReg = TII->getGlobalBaseReg(MF);
  } else if (ST->isPICStyleRIPRel() && CM == CodeModel::Small)
    BaseReg = X86::RIP;
  else if (ST->is64Bit() && CM == CodeModel::Large)
    BaseReg = MRI->createVirtualRegister(&X86::GR64RegClass);

  // Create the load from the constant pool.
  MachineConstantPool *MCP = MF->getConstantPool();
  unsigned AlignVal = VT.getSizeInBits() / 8;
  Align Alignment(AlignVal);
  Constant *FPOne = ConstantFP::get(Ty, 1.0);
  unsigned CPI = MCP->getConstantPoolIndex(FPOne, Alignment);
  const TargetRegisterClass *RC = ST->getTargetLowering()->getRegClassFor(VT);
  unsigned ResultReg = MRI->createVirtualRegister(RC);
  MachineInstrBuilder MIB;
  const DebugLoc &DbgLoc = InsertPointMI->getDebugLoc();
  MachineBasicBlock *MBB = InsertPointMI->getParent();
  if (ST->is64Bit() && CM == CodeModel::Large) {
    BuildMI(*MBB, InsertPointMI, DbgLoc, TII->get(X86::MOV64ri), BaseReg)
        .addConstantPoolIndex(CPI, 0, LoadFlags);

    MIB = BuildMI(*MBB, InsertPointMI, DbgLoc, TII->get(Opc), ResultReg);
    MIB.addReg(BaseReg).addImm(1).addReg(0).addImm(0).addReg(0);
  } else {
    MIB = BuildMI(*MBB, InsertPointMI, DbgLoc, TII->get(Opc), ResultReg);
    MIB.addReg(BaseReg).addImm(1).addReg(0)
      .addConstantPoolIndex(CPI, 0, LoadFlags).addReg(0);
  }
  MachineMemOperand *MMO = MF->getMachineMemOperand(
      MachinePointerInfo::getConstantPool(*MF),
      MachineMemOperand::MOLoad | MachineMemOperand::MOInvariant, AlignVal,
      Alignment);
  ArrayRef<MachineMemOperand *> ARMMOs(MMO);
  auto MMOs = extractLoadMMOs(ARMMOs, *MF);
  MIB.setMemRefs(MMOs);
  return ResultReg;
}

} // End anonymous namespace.

char X86GlobalFMA::ID = 0;

INITIALIZE_PASS(X86GlobalFMA, DEBUG_TYPE, "Global FMA",
                false, false)

FunctionPass *llvm::createX86GlobalFMAPass() { return new X86GlobalFMA(); }
