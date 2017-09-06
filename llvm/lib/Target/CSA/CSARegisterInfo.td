//===-- CSARegisterInfo.td - CSA Register defs -------------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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

// Channels between units.  Note that these are only for transient use
// as needed during code gen.
// The intent is that channels are vregs and effectively unbounded.
foreach i = 0-127 in { def CI0_#i  : CSAReg<"ci0_"#i>;  }
foreach i = 0-8191 in { def CI1_#i  : CSAReg<"ci1_"#i>;  }
foreach i = 0-512 in { def CI8_#i  : CSAReg<"ci8_"#i>;  }
foreach i = 0-127 in { def CI16_#i : CSAReg<"ci16_"#i>; }
foreach i = 0-8191 in { def CI32_#i : CSAReg<"ci32_"#i>; }
foreach i = 0-8191 in { def CI64_#i : CSAReg<"ci64_"#i>; }
// Special channels.
def IGN : CSAReg<"ign">; // Ignored (always writeable and readable)
def NA  : CSAReg<"na">;  // Not available (never writeable or readable)

// Define LICs.  It is less than ideal to have a full enumeration, but it
// works for now.
// Do not allow standard allocation of LICs, or copying.  Allocation is
// handled specially in conversion from control flow to dataflow.
let isAllocatable = 0, CopyCost = -1 in {
  def CI0  : RegisterClass<"CSA", [i1],       0, (add (sequence "CI0_%u",  0, 127), NA, IGN)>;
  def CI1  : RegisterClass<"CSA", [i1],       1, (add (sequence "CI1_%u",  0, 8191), NA, IGN)> { let Size=1;  }
  def CI8  : RegisterClass<"CSA", [i8],       8, (add (sequence "CI8_%u",  0, 512), NA, IGN)> { let Size=8;  }
  def CI16 : RegisterClass<"CSA", [i16,f16], 16, (add (sequence "CI16_%u", 0, 127), NA, IGN)> { let Size=16; }
  def CI32 : RegisterClass<"CSA", [i32,f32], 32, (add (sequence "CI32_%u", 0, 8191), NA, IGN)> { let Size=32; }
  def CI64 : RegisterClass<"CSA", [i64,f64], 64, (add (sequence "CI64_%u", 0, 8191), NA, IGN)> { let Size=64; }
  def ANYC : RegisterClass<"CSA", [i1],       0, (add CI0, CI1, CI8, CI16, CI32, CI64)>;
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
    (add (sequence "R%u", 0, 59), FP, TP, SP, RA)> { let isAllocatable = 0; }
def RI1  : RegisterClass<"CSA", [i1],      64, (add RI0)>;
def RI8  : RegisterClass<"CSA", [i8],      64, (add RI0)>;
def RI16 : RegisterClass<"CSA", [i16,f16], 64, (add RI0)>;
def RI32 : RegisterClass<"CSA", [i32,f32], 64, (add RI0)>;
def RI64 : RegisterClass<"CSA", [i64,f64], 64, (add RI0)>;

// These classes are used for instruction descriptions.
// They allow either channels or registers.
// When it comes to final register assignment, they can only be assigned
// to registers.  (The assumption is that things that could be channels
// were explicitly identified before register allocation.)
def I0  : RegisterClass<"CSA", [i1],      64, (add RI0  /*, CI0 */  )>;
def I1  : RegisterClass<"CSA", [i1],      64, (add RI1  /*, CI1 */  )>;
def I8  : RegisterClass<"CSA", [i8],      64, (add RI8  /*, CI8 */  )>;
def I16 : RegisterClass<"CSA", [i16,f16], 64, (add RI16 /*, CI16 */ )>;
def I32 : RegisterClass<"CSA", [i32,f32], 64, (add RI32 /*, CI32 */ )>;
def I64 : RegisterClass<"CSA", [i64,f64], 64, (add RI64 /*, CI64 */ )>;
