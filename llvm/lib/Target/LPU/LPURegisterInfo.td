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

// The LPU has no standard registers.  The RCI* registers are temporary
// to satisfy code that expects there to be registers.
// Eventually, everything will likely be LICs, though it is possible this
// will be used for local register alloc.
// Ideally, LICs would be unbounded / allocated "on demand".


// Registers are identified with 3-bit ID numbers.
class LPUReg<bits<10> num, string n> : Register<n> {
  field bits<10> Num;
  let Namespace = "LPU";
  let Num = num;
}

// These are TEMPORARY "Global" registers to get things going.
// ALU registers
// Note that there would be a copy for each ALU for various ALU kinds

def R0  : LPUReg< 0, "r0">;
def R1  : LPUReg< 1, "r1">;
def R2  : LPUReg< 2, "r2">;
def R3  : LPUReg< 3, "r3">;
def R4  : LPUReg< 4, "r4">;  // preserved
def FP  : LPUReg< 5, "fp">;  // preserved (conventional FP)
def SP  : LPUReg< 6, "sp">;  // stack pointer
def RA  : LPUReg< 7, "ra">;  // return addr

def RCR : RegisterClass<"LPU", [i1,i8,i16,i32,i64,f32,f64], 64,
    (add R0, R1, R2, R3, R4, FP, SP, RA)>;
//def RCRI8  : RegisterClass<"LPU",  [i8], 64, (add RCI1)>;
//def RCRI16 : RegisterClass<"LPU", [i16], 64, (add RCI1)>;
//def RCRI32 : RegisterClass<"LPU", [i32], 64, (add RCI1)>;
//def RCRI64 : RegisterClass<"LPU", [i64], 64, (add RCI1)>;
//def RCRF32 : RegisterClass<"LPU", [f32], 64, (add RCI1)>;
//def RCRF64 : RegisterClass<"LPU", [f64], 64, (add RCI1)>;

// LPU Invocation arguments
def ARGS : LPUReg<8,  "args">;  // control sequencing arg
def ARG0 : LPUReg<9,  "arg0">;  // arg 0
def ARG1 : LPUReg<10, "arg1">;  // arg 1
def ARG2 : LPUReg<11, "arg2">;  // arg 2
def ARG3 : LPUReg<12, "arg3">;  // arg 3
def ARG4 : LPUReg<13, "arg4">;  // arg 4
def ARG5 : LPUReg<14, "arg5">;  // arg 5
def ARG6 : LPUReg<15, "arg6">;  // arg 6
def ARG7 : LPUReg<16, "arg7">;  // arg 7

// Channel Class for arguments
def CCARG : RegisterClass<"LPU", [i1,i8,i16,i32,i64,f32,f64], 64,
    (add ARGS,ARG0,ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7)>;

// LPU Invocation results
def RESS : LPUReg<17, "ress">;  // control sequence result
def RES0 : LPUReg<18, "res0">;  // result 0
def RES1 : LPUReg<19, "res1">;  // result 1
def RES2 : LPUReg<20, "res2">;  // result 2
def RES3 : LPUReg<21, "res3">;  // result 3
def RES4 : LPUReg<22, "res4">;  // result 4
def RES5 : LPUReg<23, "res5">;  // result 5
def RES6 : LPUReg<24, "res6">;  // result 6
def RES7 : LPUReg<25, "res7">;  // result 7

// Channel Class for results
def CCRES : RegisterClass<"LPU", [i1,i8,i16,i32,i64,f32,f64], 64,
    (add RESS,RES0,RES1,RES2,RES3,RES4,RES5,RES6,RES7)>;

// This is a TEMPORARY work-around.  Ideally, LICs of the appropriate size
// should be allocated on demand, even if they still "look like" registers
// in some way.

def C0 : LPUReg<64, "c0">;
def C1 : LPUReg<65, "c1">;
def C2 : LPUReg<66, "c2">;
def C3 : LPUReg<67, "c3">;
def C4 : LPUReg<68, "c4">;
def C5 : LPUReg<69, "c5">;
def C6 : LPUReg<70, "c6">;
def C7 : LPUReg<71, "c7">;

// generic channel class
def CC : RegisterClass<"LPU", [i1,i8,i16,i32,i64,f32,f64], 64,
   (add C0, C1, C2, C3, C4, C5, C6, C7)>;
def CCI1  : RegisterClass<"LPU",  [i1], 64, (add CC)>;
def CCI8  : RegisterClass<"LPU",  [i8], 64, (add CC)>;
def CCI16 : RegisterClass<"LPU", [i16], 64, (add CC)>;
def CCI32 : RegisterClass<"LPU", [i32], 64, (add CC)>;
def CCI64 : RegisterClass<"LPU", [i64], 64, (add CC)>;
def CCF32 : RegisterClass<"LPU", [f32], 64, (add CC)>;
def CCF64 : RegisterClass<"LPU", [f64], 64, (add CC)>;


/*

// This is a TEMPORARY work-around.  Ideally, LICs of the appropriate size
// should be allocated on demand, even if they still "look like" registers
// in some way.

def C1_0 : LPUReg<64, "c1_0">;
def C1_1 : LPUReg<65, "c1_1">;
def C1_2 : LPUReg<66, "c1_2">;
def C1_3 : LPUReg<67, "c1_3">;
def C1_4 : LPUReg<68, "c1_4">;
def C1_5 : LPUReg<69, "c1_5">;
def C1_6 : LPUReg<70, "c1_6">;
def C1_7 : LPUReg<71, "c1_7">;

def C1 : RegisterClass<"LPU", [i1], 1,
   (add C1_0, C1_1, C1_2, C1_3, C1_4, C1_5, C1_6, C1_7)>;

def C8_0 : LPUReg<128, "c8_0">;
def C8_1 : LPUReg<129, "c8_1">;
def C8_2 : LPUReg<130, "c8_2">;
def C8_3 : LPUReg<131, "c8_3">;
def C8_4 : LPUReg<132, "c8_4">;
def C8_5 : LPUReg<133, "c8_5">;
def C8_6 : LPUReg<134, "c8_6">;
def C8_7 : LPUReg<135, "c8_7">;

def C8 : RegisterClass<"LPU", [i1], 1,
   (add C8_0, C8_1, C8_2, C8_3, C8_4, C8_5, C8_6, C8_7)>;

def C16_0 : LPUReg<192, "c16_0">;
def C16_1 : LPUReg<193, "c16_1">;
def C16_2 : LPUReg<194, "c16_2">;
def C16_3 : LPUReg<195, "c16_3">;
def C16_4 : LPUReg<196, "c16_4">;
def C16_5 : LPUReg<197, "c16_5">;
def C16_6 : LPUReg<198, "c16_6">;
def C16_7 : LPUReg<199, "c16_7">;

def C16 : RegisterClass<"LPU", [i1], 1,
   (add C16_0, C16_1, C16_2, C16_3, C16_4, C16_5, C16_6, C16_7)>;

def C32_0 : LPUReg<256, "c32_0">;
def C32_1 : LPUReg<257, "c32_1">;
def C32_2 : LPUReg<258, "c32_2">;
def C32_3 : LPUReg<259, "c32_3">;
def C32_4 : LPUReg<260, "c32_4">;
def C32_5 : LPUReg<261, "c32_5">;
def C32_6 : LPUReg<262, "c32_6">;
def C32_7 : LPUReg<263, "c32_7">;

def C32 : RegisterClass<"LPU", [i1], 1,
   (add C32_0, C32_1, C32_2, C32_3, C32_4, C32_5, C32_6, C32_7)>;

def C64_0 : LPUReg<320, "c64_0">;
def C64_1 : LPUReg<321, "c64_1">;
def C64_2 : LPUReg<322, "c64_2">;
def C64_3 : LPUReg<323, "c64_3">;
def C64_4 : LPUReg<324, "c64_4">;
def C64_5 : LPUReg<325, "c64_5">;
def C64_6 : LPUReg<326, "c64_6">;
def C64_7 : LPUReg<327, "c64_7">;

def C64 : RegisterClass<"LPU", [i1], 1,
   (add C64_0, C64_1, C64_2, C64_3, C64_4, C64_5, C64_6, C64_7)>;
*/
