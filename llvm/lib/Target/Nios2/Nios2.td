//===-- Nios2.td - Describe the Nios2 Target Machine ---------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Calling Conv, Instruction Descriptions
//===----------------------------------------------------------------------===//

include "Nios2Schedule.td"
include "Nios2InstrInfo.td"
include "Nios2RegisterBanks.td"
include "Nios2CallingConv.td"

def Nios2InstrInfo : InstrInfo;

// Without this will have error: 'nios232I' is not a recognized processor for 
//  this target (ignoring processor)
//===----------------------------------------------------------------------===//
// Nios2 Subtarget features                                                    //
//===----------------------------------------------------------------------===//

def FeatureNios2r2BMX : SubtargetFeature<"nios2r2bmx", "HasBMX", "true", "">;
def FeatureNios2r2CDX : SubtargetFeature<"nios2r2cdx", "HasCDX", "true", "">;
def FeatureNios2r2MPX : SubtargetFeature<"nios2r2mpx", "HasMPX", "true", "">;
def FeatureNios2r2Mandatory :
  SubtargetFeature<"nios2r2mandatory", "HasR2Mandatory", "true", "">;

def FeatureNios2r1     : SubtargetFeature<"nios2r1", "Nios2ArchVersion", 
                                "Nios2r1", "Nios2 R1 ISA Support">;
def FeatureNios2r2     : SubtargetFeature<"nios2r2", "Nios2ArchVersion",                      
                               "Nios2r2", "Nios2 R2 ISA Support">;
def FeatureNios2r2bmx    : SubtargetFeature<"nios2r2bmx", "Nios2ArchVersion",                      
                               "Nios2r2", "dgdlgkjdglkd">;

//===----------------------------------------------------------------------===//
// Nios2 processors supported.
//===----------------------------------------------------------------------===//

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, Nios2GenericItineraries, Features>;

def : Proc<"nios2r1", [FeatureNios2r1]>;
def : Proc<"nios2r2", [FeatureNios2r2]>;

def Nios2AsmParser : AsmParser {
  let ShouldEmitMatchRegisterName = 0;
}

def Nios2AsmParserVariant : AsmParserVariant {
  int Variant = 0;

  // Recognize hard coded registers.
  string RegisterPrefix = "$";
}

// Will generate Nios2GenAsmWrite.inc included by Nios2InstPrinter.cpp, contents 
//  as follows,
// void Nios2InstPrinter::printInstruction(const MCInst *MI, raw_ostream &O) {...}
// const char *Nios2InstPrinter::getRegisterName(unsigned RegNo) {...}
def Nios2 : Target {
// def Nios2InstrInfo : InstrInfo as before.
  let InstructionSet = Nios2InstrInfo;
  let AssemblyParsers = [Nios2AsmParser];
  let AssemblyParserVariants = [Nios2AsmParserVariant];
}
