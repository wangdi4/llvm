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

// The LPU has no standard registers.  This is mainly to satisfy code that
// expects there to be registers...


// Registers are identified with 3-bit ID numbers.
class LPUReg<bits<3> num, string n> : Register<n> {
  field bits<3> Num;
  let Namespace = "LPU";
  let Num = num;
}

// These are TEMPORARY to get things going.
// ALU registers
// Note that there would be a copy for each ALU for various ALU kinds
def R0 : LPUReg< 0, "r0">;
def R1 : LPUReg< 1, "r1">;
def R2 : LPUReg< 2, "r2">;
def R3 : LPUReg< 3, "r3">;
def R4 : LPUReg< 4, "r4">;
def R5 : LPUReg< 5, "r5">;
def R6 : LPUReg< 6, "r6">;
def R7 : LPUReg< 7, "r7">;

// The generic "REG" class can take any value
def REG : RegisterClass<"LPU",
    [i64,i32,i16,i8,f64,f32,f16,v2i32,v2f32,v4i16,v4f16,v8i8], 64,
    (add R0, R1, R2, R3, R4, R5, R6, R7)>;

// Specific sized/type register classes for operand constraints
def RCI32 : RegisterClass<"LPU", [i32], 64, (add REG)>;
def RCI64 : RegisterClass<"LPU", [i64], 64, (add REG)>;

def RCF32 : RegisterClass<"LPU", [f32], 64, (add REG)>;
def RCF64 : RegisterClass<"LPU", [f64], 64, (add REG)>;
