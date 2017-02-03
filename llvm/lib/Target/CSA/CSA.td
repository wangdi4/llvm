//===--- CSA.td - Describe the CSA Target machine ----------*- tablegen -*-===//
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

include "CSASchedule.td"

//===----------------------------------------------------------------------===//
// CSA subtarget features
//===----------------------------------------------------------------------===//

// Large scale features
def FeatureOrdered : SubtargetFeature<"ordered", "IsOrdered", "true",
                            "All operations have ordered semantics - 1 unit">;

// Operation type features
// Not all of these can reasonably be disabled currently.   The intent
// is to have infrastructure allow selective control for experimentation
// where relevant.
def FeatureI0    : SubtargetFeature<"i0", "HasI0", "true",
                            "Enable 0b integer ALU ops">;
def FeatureI1    : SubtargetFeature<"i1", "HasI1", "true",
                            "Enable 1b integer ALU ops">;
def FeatureI8    : SubtargetFeature<"i8", "HasI8", "true",
                            "Enable 8b integer ALU ops">;
def FeatureI16   : SubtargetFeature<"i16", "HasI16", "true",
                            "Enable 16b integer ALU ops">;
def FeatureI32   : SubtargetFeature<"i32", "HasI32", "true",
                            "Enable 32b integer ALU ops">;
def FeatureI64   : SubtargetFeature<"i64", "HasI64", "true",
                            "Enable 64b integer ALU ops">;
def FeatureF16   : SubtargetFeature<"f16", "HasF16", "true",
                            "Enable 16b float ALU ops">;
def FeatureF32   : SubtargetFeature<"f32", "HasF32", "true",
                            "Enable 32b float ALU ops">;
def FeatureF64   : SubtargetFeature<"f64", "HasF64", "true",
                            "Enable 64b float ALU ops">;

// Memref features
def FeatureSextL : SubtargetFeature<"sextl", "HasSextL", "true",
                            "Enable sign-extending loads">;
def FeatureDispl : SubtargetFeature<"displ", "HasDispl", "true",
                            "Enable displacement mode memrefs">;
def FeatureIndex : SubtargetFeature<"index", "HasIndex", "true",
                            "Enable indexed mode memrefs">;

// Operation features
def FeatureShAdd : SubtargetFeature<"shadd", "HasShAdd", "true",
                            "Enable shift/add">;
def FeatureBitOp : SubtargetFeature<"bitop", "HasBitOp", "true",
                            "Enable ctlz, cttz, popc">;
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
// CSA supported processors
//===----------------------------------------------------------------------===//

// These map to processor layouts in csasim

class Proc<string Name, SchedMachineModel Model,
           list<SubtargetFeature> Features>
  : Processor<Name, NoItineraries, Features>;

def : Proc<"ordered",  CSAGeneric,
                       [FeatureOrdered,
                        FeatureI0,    FeatureI1,    FeatureI8,    FeatureI16,
                        FeatureI32,   FeatureI64,   FeatureF16,   FeatureF32,
                        FeatureF64,
                        FeatureSextL, FeatureDispl, FeatureIndex,
                        FeatureShAdd, FeatureBitOp,
                        FeatureIDiv,  FeatureFDiv,  FeatureFMA,
                        FeatureSqrt,  FeatureMath0]>;

def : Proc<"autounit", CSAGeneric,
                       [FeatureI0,    FeatureI1,    FeatureI8,    FeatureI16,
                        FeatureI32,   FeatureI64,   FeatureF16,   FeatureF32,
                        FeatureF64,
                        FeatureSextL, FeatureDispl, FeatureIndex,
                        FeatureShAdd, FeatureBitOp,
                        FeatureIDiv,  FeatureFDiv,  FeatureFMA,
                        FeatureSqrt,  FeatureMath0]>;
// Like autounit, but almost all more complex ops disabled
def : Proc<"automin",  CSAGeneric,
                       [FeatureI0,    FeatureI1,    FeatureI32,   FeatureI64,
                        FeatureF32,   FeatureF64]>;

def : Proc<"config0",  CSAConfig0,
                       [FeatureI0,    FeatureI1,    FeatureI8,    FeatureI16,
                        FeatureI32,   FeatureI64,   FeatureF16,   FeatureF32,
                        FeatureF64,
                        FeatureSextL, FeatureDispl, FeatureIndex,
                        FeatureShAdd, FeatureBitOp,
                        FeatureIDiv,  FeatureFDiv,  FeatureFMA,
                        FeatureSqrt,  FeatureMath0]>;
def : Proc<"config1",  CSAConfig1,
                       [FeatureI0,    FeatureI1,    FeatureI8,    FeatureI16,
                        FeatureI32,   FeatureI64,   FeatureF16,   FeatureF32,
                        FeatureF64,
                        FeatureSextL, FeatureDispl, FeatureIndex,
                        FeatureShAdd, FeatureBitOp,
                        FeatureIDiv,  FeatureFDiv , FeatureFMA,
                        FeatureSqrt,  FeatureMath0]>;

//===----------------------------------------------------------------------===//
// Register file description
//===----------------------------------------------------------------------===//

include "CSARegisterInfo.td"

//===----------------------------------------------------------------------===//
// Calling convention description
//===----------------------------------------------------------------------===//

include "CSACallingConv.td"

//===----------------------------------------------------------------------===//
// Instruction descriptions
//===----------------------------------------------------------------------===//

include "CSAInstrInfo.td"

def CSAInstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// Assembly parser
//===----------------------------------------------------------------------===//

def CSAAsmParser : AsmParser{
  let ShouldEmitMatchRegisterName = 1;
}

def CSAAsmParserVariant : AsmParserVariant{
  int Variant = 0;
  string RegisterPrefix = "%";
}

//===----------------------------------------------------------------------===//
// Top-level target declaration
//===----------------------------------------------------------------------===//

def CSA : Target {
  let InstructionSet = CSAInstrInfo;
  let AssemblyParsers = [CSAAsmParser];
}
