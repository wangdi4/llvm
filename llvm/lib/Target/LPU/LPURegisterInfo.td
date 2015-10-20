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
def TP : LPUReg<"%tp">;  // thread base
def RA : LPUReg<"%ra">;  // return addr

def GLOBALREG : RegisterClass<"LPU", [i64], 64,
    (add FP, SP, TP, RA)>;


def IGN : LPUReg<"%ign">; // Ignored (always writeable, never readable)

// Channels between units

foreach i = 0-4095 in {
  def C#i : LPUReg<"%c"#i>;
}

// generic channel class
def ANYC : RegisterClass<"LPU", [i1,i8,i16,i32,i64,f16,f32,f64], 64,
   (add   
         IGN,   // IGN (ignore) part of all LICs, but not allocatable...
         (sequence "C%u", 0, 4095) )>;

def I0C  : RegisterClass<"LPU", [i1],  64, (add ANYC)>; // 0 bit LIC...
def I1C  : RegisterClass<"LPU", [i1],  64, (add ANYC)>;
def I8C  : RegisterClass<"LPU", [i8],  64, (add ANYC)>;
def I16C : RegisterClass<"LPU", [i16], 64, (add ANYC)>;
def I32C : RegisterClass<"LPU", [i32], 64, (add ANYC)>;
def I64C : RegisterClass<"LPU", [i64], 64, (add ANYC)>;
def F16C : RegisterClass<"LPU", [f16], 64, (add ANYC)>;
def F32C : RegisterClass<"LPU", [f32], 64, (add ANYC)>;
def F64C : RegisterClass<"LPU", [f64], 64, (add ANYC)>;

//// Registers in main sequential unit

foreach i = 0-63 in {
  def R#i : LPUReg<"%r"#i>;
}

// generic register class
def ANYR : RegisterClass<"LPU", [i1,i8,i16,i32,i64,f16,f32,f64], 64,
   (add
         IGN,   // IGN (ignore) part of all regs, but not allocatable...
         (sequence "R%u", 0, 63) )>;

def I0R  : RegisterClass<"LPU", [i1],  64, (add ANYR)>;
def I1R  : RegisterClass<"LPU", [i1],  64, (add ANYR)>;
def I8R  : RegisterClass<"LPU", [i8],  64, (add ANYR)>;
def I16R : RegisterClass<"LPU", [i16], 64, (add ANYR)>;
def I32R : RegisterClass<"LPU", [i32], 64, (add ANYR)>;
def I64R : RegisterClass<"LPU", [i64], 64, (add ANYR)>;
def F16R : RegisterClass<"LPU", [f16], 64, (add ANYR)>;
def F32R : RegisterClass<"LPU", [f32], 64, (add ANYR)>;
def F64R : RegisterClass<"LPU", [f64], 64, (add ANYR)>;

// These classes are used for instruction descriptions.
// They allow either channels or registers.
def I0  : RegisterClass<"LPU", [i1],  64, (add I0R, I0C)>;
def I1  : RegisterClass<"LPU", [i1],  64, (add I1R, I1C)>;
def I8  : RegisterClass<"LPU", [i8],  64, (add I8R, I8C)>;
def I16 : RegisterClass<"LPU", [i16], 64, (add I16R, I16C)>;
def I32 : RegisterClass<"LPU", [i32], 64, (add I32R, I32C)>;
def I64 : RegisterClass<"LPU", [i64], 64, (add I64R, I64C)>;
def F16 : RegisterClass<"LPU", [f16], 64, (add F16R, F16C)>;
def F32 : RegisterClass<"LPU", [f32], 64, (add F32R, F32C)>;
def F64 : RegisterClass<"LPU", [f64], 64, (add F64R, F64C)>;

def ANYS : RegisterClass<"LPU", [i1,i8,i16,i32,i64,f16,f32,f64], 64,
   (add ANYC, ANYR)>;
