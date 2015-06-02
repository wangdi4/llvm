//===--- LPU.td - Describe the LPU Target machine ----------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces which we are implementing
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// LPU subtarget features
//===----------------------------------------------------------------------===//

// Currently no features

//===----------------------------------------------------------------------===//
// LPU supported processors
//===----------------------------------------------------------------------===//

// These map to processor layouts in lpusim

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"autounit",          []>;
//def : Proc<"typedcluster1",     []>;
//def : Proc<"mixedcluster1",     []>;

//===----------------------------------------------------------------------===//
// Register file description
//===----------------------------------------------------------------------===//

include "LPURegisterInfo.td"

//===----------------------------------------------------------------------===//
// Calling convention description
//===----------------------------------------------------------------------===//

include "LPUCallingConv.td"

//===----------------------------------------------------------------------===//
// Instruction descriptions
//===----------------------------------------------------------------------===//

include "LPUInstrInfo.td"

def LPUInstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// Assembly parser
//===----------------------------------------------------------------------===//

// N/A

//===----------------------------------------------------------------------===//
// Top-level target declaration
//===----------------------------------------------------------------------===//

def LPU : Target {
  let InstructionSet = LPUInstrInfo;
}
