//===-- CSARegisterInfo.td - CSA Register defs -------------*- tablegen -*-===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the CSA register file
//===----------------------------------------------------------------------===//

// The CSA has no "global" registers, though there will likely be registers
// local to individual functional units.
//
// Registers are used to represent LICs as a convenience, because
// that is what the LLVM CG deals in.

class CSAReg<string n> : Register<n> {
  let Namespace = "CSA";
}

// Defining the LIC register set. We use IGN and NA for special registers in the
// register. Beyond that, all LICs are represented as virtual registers. Their
// size is indicated by their actual register class. We use a few extra physical
// registers to keep LLVM happy about register class sizes, but these should
// never be used.
def IGN : CSAReg<"ign">; // Ignored (always writeable and readable)
def NA  : CSAReg<"na">;  // Not available (never writeable or readable)
foreach i = 0-10 in { def CV0_#i : CSAReg<"phy0_"#i>; }
foreach i = 0-129 in { def P64_#i : CSAReg<"p64_"#i>; }
// Define the virtual register classes for LICs. It's important that these
// register classes all share the same register set: if they don't, then
// synthetic register classes will be created from the common intersections,
// and registers might be assigned to those classes.
let CopyCost = -1 in {
  def ANYC : RegisterClass<"CSA", [i1], 64, (add (sequence "CV0_%u", 0, 10), (sequence "P64_%u", 0, 129), NA, IGN)>;
  def CI0  : RegisterClass<"CSA", [i1],      64, (add ANYC)> { let Size=8;  }
  def CI1  : RegisterClass<"CSA", [i1],      8, (add ANYC)> { let Size=8;  }
  def CI8  : RegisterClass<"CSA", [i8],      8, (add ANYC)> { let Size=8;  }
  def CI16 : RegisterClass<"CSA", [i16,f16], 16, (add ANYC)> { let Size=16; }
  def CI32 : RegisterClass<"CSA", [i32,f32], 32, (add ANYC)> { let Size=32; }
  def CI64 : RegisterClass<"CSA", [i64,f64,v4f16,v2f32], 64, (add ANYC)> { let Size=64; }
}

//// Registers in main sequential unit

// CSA SXU calling convention notes
//
// In theory, the registers are mostly temporary, and most code will use LICs.
// However, these are assigned on the chance it does eventually matter.
// Some properties of the assignment:
// - param and results in present in low 4, 8 regs for potentially dense
//   encoding for less frequent code for better i-stream behavior
// - param and return even reg aligned to allow for reg pairs
// - all params contiguous
// - all preserved contiguous
// - preserved are ~1/4 of 1st 32 or 64.  Expectation is most code will be
//   compiled leaf-first, so reg. signatures of co-compiled callees will be
//   known to allow tailored linkages, including awareness of unused regs
//   in callss.  Preserved mostly relevant for unknown callouts from CSA
//   to elsewhere (could be a different calling conv.)
// Notes:
// - might be desirable to have special regs low, particularly SP
//   (e.g. if there is a dense encoding, might be nice if SP could use it,
//   though current estimation is stack use will be rare.)
// - If there is a dense encoding w/3b reg IDs, might be nice to have 4 params
//   in low 8 regs.  (But having regs in current order allows compiler to
//   have only the low "n" for a particular thread context - e.g. if only
//   8 regs were needed, no additional ones need be allocated.)
// - One approach might be to put TP and RA in a special set.

// WARNING: These assignments are "known" somewhat in CSAAsmPrinter.cpp and
// CSAAsmParser.cpp. Unfortunately, changes here may require re-evaluating the
// resulting CSAGenAsmMatcher.inc in order to update the enums used in
// validateTargetOperandClass.

// R0..R1 - return result/scratch
// R2..R17 - param/scratch, up to 16
// R18..R23 - 6 scratch
// R24..R39 - 16 preserved regs (8 on each side of R32 boundary, 8 reg aligned)
// R40..R59 - 24 scratch (if more regs beyond 1st 64, also scratch)
foreach i = 0-59 in {
  def R#i : CSAReg<"r"#i>;
}

// special regs (60-63).  (SP, FP might want to be in low regs)
def FP : CSAReg<"fp">;  // preserved (conventional FP)
def TP : CSAReg<"tp">;  // thread pointer
def SP : CSAReg<"sp">;  // stack pointer
def RA : CSAReg<"ra">;  // return addr


def RI0  : RegisterClass<"CSA", [i1],      64,
    (add (sequence "R%u", 0, 59), FP, TP, SP, RA)> { let isAllocatable = 0; let Size = 8; }
def RI1  : RegisterClass<"CSA", [i1],      8, (add RI0)> { let Size = 8; }
def RI8  : RegisterClass<"CSA", [i8],      8, (add RI0)>;
def RI16 : RegisterClass<"CSA", [i16,f16], 16, (add RI0)>;
def RI32 : RegisterClass<"CSA", [i32,f32], 32, (add RI0)>;
def RI64 : RegisterClass<"CSA", [i64,f64,v4f16,v2f32], 64, (add RI0)>;

// The following classes can be used in the instruction information to refer to
// either register or LICs. These classes necessarily contain CI* register
// classes for verification purposes, but we need to prevent LLVM from trying
// to allocate LICs if we run out of registers, hence the use of alternative
// regalloc orders that omit the LIC physical registers.
let AltOrders=[(add RI0)], AltOrderSelect = [{ return 1; }] in {
  def I0  : RegisterClass<"CSA", [i1],      64, (add RI0 , CI0 )> { let Size = 8; }
  def I1  : RegisterClass<"CSA", [i1],      8,  (add RI1 , CI1 )> { let Size = 8; }
  def I8  : RegisterClass<"CSA", [i8],      8,  (add RI8 , CI8 )>;
  def I16 : RegisterClass<"CSA", [i16,f16], 16, (add RI16, CI16)>;
  def I32 : RegisterClass<"CSA", [i32,f32], 32, (add RI32, CI32)>;
  def I64 : RegisterClass<"CSA", [i64,f64,v4f16,v2f32], 64, (add RI64, CI64)>;
}
