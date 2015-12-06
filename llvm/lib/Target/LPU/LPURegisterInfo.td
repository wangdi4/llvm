//===-- LPURegisterInfo.td - LPU Register defs -------------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the LPU register file 
//===----------------------------------------------------------------------===//

// The LPU has no "global" registers, though there will likely be registers
// local to individual functional units.
//
// Registers are used to represent LICs as a convenience, because
// that is what the LLVM CG deals in.

class LPUReg<string n> : Register<n> {
  let Namespace = "LPU";
}

// Channels between units.  Note that these are only for transient use
// as needed during code gen.
// The intent is that channels are vregs and effectively unbounded.
foreach i = 0-7 in { def CI0_#i : LPUReg<"%ci0_"#i>; }
foreach i = 0-7 in { def CI1_#i : LPUReg<"%ci1_"#i>; }
foreach i = 0-7 in { def CI8_#i : LPUReg<"%ci8_"#i>; }
foreach i = 0-7 in { def CI16_#i : LPUReg<"%ci16_"#i>; }
foreach i = 0-7 in { def CI32_#i : LPUReg<"%ci32_"#i>; }
foreach i = 0-7 in { def CI64_#i : LPUReg<"%ci64_"#i>; }
foreach i = 0-7 in { def CF16_#i : LPUReg<"%cf16_"#i>; }
foreach i = 0-7 in { def CF32_#i : LPUReg<"%cf32_"#i>; }
foreach i = 0-7 in { def CF64_#i : LPUReg<"%cf64_"#i>; }

def CI0  : RegisterClass<"LPU", [i1],  64, (add (sequence "CI0_%u", 0, 7))>;
def CI1  : RegisterClass<"LPU", [i1],  64, (add (sequence "CI1_%u", 0, 7))>;
def CI8  : RegisterClass<"LPU", [i8],  64, (add (sequence "CI8_%u", 0, 7))>;
def CI16 : RegisterClass<"LPU", [i16], 64, (add (sequence "CI16_%u", 0, 7))>;
def CI32 : RegisterClass<"LPU", [i32], 64, (add (sequence "CI32_%u", 0, 7))>;
def CI64 : RegisterClass<"LPU", [i64], 64, (add (sequence "CI64_%u", 0, 7))>;
def CF16 : RegisterClass<"LPU", [f16], 64, (add (sequence "CF16_%u", 0, 7))>;
def CF32 : RegisterClass<"LPU", [f32], 64, (add (sequence "CF32_%u", 0, 7))>;
def CF64 : RegisterClass<"LPU", [f64], 64, (add (sequence "CF64_%u", 0, 7))>;

//// Registers in main sequential unit

// LPU SXU calling convention notes
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
//   in callss.  Preserved mostly relevant for unknown callouts from LPU
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

// WARNING: These assignments are "known" somewhat in LPUAsmPrinter.cpp

// R0..R1 - return result/scratch
// R2..R17 - param/scratch, up to 16
// R18..R23 - 6 scratch
// R24..R39 - 16 preserved regs (8 on each side of R32 boundary, 8 reg aligned)
// R40..R59 - 24 scratch (if more regs beyond 1st 64, also scratch)
foreach i = 0-59 in {
  def R#i : LPUReg<"%r"#i>;
}

def IGN : LPUReg<"%ign">; // Ignored (always writeable, never readable)

// special regs (60-63).  (SP, FP might want to be in low regs)
def FP : LPUReg<"%fp">;  // preserved (conventional FP)
def TP : LPUReg<"%tp">;  // thread pointer
def SP : LPUReg<"%sp">;  // stack pointer
def RA : LPUReg<"%ra">;  // return addr


def RI0  : RegisterClass<"LPU", [i1],  64,
    (add (sequence "R%u", 0, 59), FP, TP, SP, RA)>;
def RI1  : RegisterClass<"LPU", [i1],  64, (add RI0)>;
def RI8  : RegisterClass<"LPU", [i8],  64, (add RI0)>;
def RI16 : RegisterClass<"LPU", [i16], 64, (add RI0)>;
def RI32 : RegisterClass<"LPU", [i32], 64, (add RI0)>;
def RI64 : RegisterClass<"LPU", [i64], 64, (add RI0)>;
def RF16 : RegisterClass<"LPU", [f16], 64, (add RI0)>;
def RF32 : RegisterClass<"LPU", [f32], 64, (add RI0)>;
def RF64 : RegisterClass<"LPU", [f64], 64, (add RI0)>;

// These classes are used for instruction descriptions.
// They allow either channels or registers.
def I0  : RegisterClass<"LPU", [i1],  64, (add RI0 /*, CI0 */ )>;
def I1  : RegisterClass<"LPU", [i1],  64, (add RI1 /*, CI1 */ )>;
def I8  : RegisterClass<"LPU", [i8],  64, (add RI8 /*, CI8 */ )>;
def I16 : RegisterClass<"LPU", [i16], 64, (add RI16 /*, CI16 */ )>;
def I32 : RegisterClass<"LPU", [i32], 64, (add RI32 /*, CI32 */ )>;
def I64 : RegisterClass<"LPU", [i64], 64, (add RI64 /*, CI64 */ )>;
def F16 : RegisterClass<"LPU", [f16], 64, (add RF16 /*, CF16 */ )>;
def F32 : RegisterClass<"LPU", [f32], 64, (add RF32 /*, CF32 */ )>;
def F64 : RegisterClass<"LPU", [f64], 64, (add RF64 /*, CF64 */ )>;
