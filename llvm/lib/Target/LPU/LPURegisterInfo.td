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

// Having actual registers used for global is intended as a temporary work-around.
// Registers are used to represent LICs as a convenience, because
// that is what the LLVM CG deals in.
// This approach also allows compiling to registers for the "ordered" target.
// One question is whether LICs can just be VREGs and remove need for most
// (all?) physical "regs".
// Some targets (e.g. NVPTX) simply use VREGs - probably the best solution.

class LPUReg<string n> : Register<n> {
  let Namespace = "LPU";
}

// These are TEMPORARY "Global" registers to get things going.

def FP : LPUReg<"%fp">;  // preserved (conventional FP)
def SP : LPUReg<"%sp">;  // stack pointer
def RA : LPUReg<"%ra">;  // return addr

def GLOBALREG : RegisterClass<"LPU", [i64], 64,
    (add FP, SP, RA)>;


def IGN : LPUReg<"%ign">; // Ignored (always writeable, never readable)

foreach i = 0-4091 in {
  def C#i : LPUReg<"%c"#i>;
}

// generic channel class
def ANYC : RegisterClass<"LPU", [i1,i8,i16,i32,i64,f16,f32,f64], 64,
   (add   
         IGN,   // IGN (ignore) part of all regs, but not allocatable...
         (sequence "C%u", 0, 4091) )>;

def I0  : RegisterClass<"LPU", [i1],  64, (add ANYC)>; // 0 bit LIC...
def I1  : RegisterClass<"LPU", [i1],  64, (add ANYC)>;
def I8  : RegisterClass<"LPU", [i8],  64, (add ANYC)>;
def I16 : RegisterClass<"LPU", [i16], 64, (add ANYC)>;
def I32 : RegisterClass<"LPU", [i32], 64, (add ANYC)>;
def I64 : RegisterClass<"LPU", [i64], 64, (add ANYC)>;
def F16 : RegisterClass<"LPU", [f16], 64, (add ANYC)>;
def F32 : RegisterClass<"LPU", [f32], 64, (add ANYC)>;
def F64 : RegisterClass<"LPU", [f64], 64, (add ANYC)>;
