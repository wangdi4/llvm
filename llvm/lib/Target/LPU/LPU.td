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

// Large scale features
def FeatureOrdered : SubtargetFeature<"ordered", "IsOrdered", "true",
                            "All operations have ordered semantics - 1 unit">;

// Type features
def FeatureS8    : SubtargetFeature<"s8", "HasS8", "true",
                            "Enable 8b small integer ALU ops">;
def FeatureS16   : SubtargetFeature<"s16", "HasS16", "true",
                            "Enable 16b small integer ALU ops">;
def FeatureF16   : SubtargetFeature<"f16", "HasF16", "true",
                            "Enable 16b float ALU ops">;

// Operation features
def FeatureIDiv  : SubtargetFeature<"idiv", "HasIDiv", "true",
                            "Enable integer div">;
def FeatureFDiv  : SubtargetFeature<"fdiv", "HasFDiv", "true",
                            "Enable floating div">;
def FeatureFMA   : SubtargetFeature<"fma", "HasFMA", "true",
                            "Enable fused mul/add">;
def FeatureSqrt  : SubtargetFeature<"sqrt", "HasSqrt", "true",
                            "Enable sqrt">;
def FeatureMath0 : SubtargetFeature<"math0", "HasMath0", "true",
                            "Enable exp[2]/log[2]/sin/cos">;

//===----------------------------------------------------------------------===//
// LPU supported processors
//===----------------------------------------------------------------------===//

// These map to processor layouts in lpusim

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"ordered",  [FeatureOrdered,
                        FeatureS8,   FeatureS16,  FeatureF16,
                        FeatureIDiv, FeatureFDiv, FeatureFMA,
                        FeatureSqrt, FeatureMath0]>;

def : Proc<"autounit", [FeatureS8,   FeatureS16,  FeatureF16,
                        FeatureFMA,  FeatureIDiv, FeatureFDiv,
                        FeatureSqrt, FeatureMath0]>;
// Like autounit, but almost all more complex ops disabled
def : Proc<"automin",  []>;

def : Proc<"config0",  [FeatureS8,   FeatureS16,  FeatureF16,
                        FeatureIDiv, FeatureFDiv, FeatureFMA,
                        FeatureSqrt, FeatureMath0]>;
def : Proc<"config1",  [FeatureS8,   FeatureS16,  FeatureF16,
                        FeatureIDiv, FeatureFDiv, FeatureFMA,
                        FeatureSqrt, FeatureMath0]>;

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
