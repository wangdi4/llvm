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

// The LPU has no standard registers.  This is temporary to satisfy code that
// expects there to be registers.  Eventually, everything will likely be
// LICs, though it is possible this will be used for local register alloc.


// Registers are identified with 3-bit ID numbers.
class LPUReg<bits<4> num, string n> : Register<n> {
  field bits<4> Num;
  let Namespace = "LPU";
  let Num = num;
}

// These are TEMPORARY "Global" registers to get things going.
// ALU registers
// Note that there would be a copy for each ALU for various ALU kinds
def R0  : LPUReg< 0, "r0">;  // arg 0
def R1  : LPUReg< 1, "r1">;  // arg 1
def R2  : LPUReg< 2, "r2">;  // arg 2
def R3  : LPUReg< 3, "r3">;  // arg 3
def R4  : LPUReg< 4, "r4">;  // return 0
def R5  : LPUReg< 5, "r5">;  // return 1
def R6  : LPUReg< 6, "r6">;  // return 2
def R7  : LPUReg< 7, "r7">;  // return 3
def R8  : LPUReg< 8, "r8">;
def R9  : LPUReg< 9, "r9">;
def R10 : LPUReg<10, "r10">;
def R11 : LPUReg<11, "r11">;
def R12 : LPUReg<12, "r12">; // preserved
def FP  : LPUReg<13, "fp">;  // preserved (conventional FP)
def SP  : LPUReg<14, "sp">;  // stack pointer
def RA  : LPUReg<15, "ra">;  // return addr


def RCI32 : RegisterClass<"LPU", [i32], 64,
    (add R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, FP, SP, RA)>;

def RCI64 : RegisterClass<"LPU", [i64], 64, (add RCI32)>;

def RCF32 : RegisterClass<"LPU", [f32], 64, (add RCI32)>;

def RCF64 : RegisterClass<"LPU", [f64], 64, (add RCI32)>;
