//===-- LPUInstrFormats.td - LPU Instruction Formats -------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Instruction format superclass
//===----------------------------------------------------------------------===//

class InstLPU<dag outs, dag ins, string asmstr, list<dag> pattern>
    : Instruction {
  // Actual encoding TBD.  Might be variable length.
  field bits<64> Inst;

  let Namespace = "LPU";
  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString   = asmstr;
  let Pattern = pattern;
}

// LPU pseudo instruction format
class PseudoInstLPU<dag outs, dag ins, string asmstr, list<dag> pattern>
   : InstLPU<outs, ins, asmstr, pattern> {
  let isPseudo = 1;
  let Itinerary = IIPseudo;
}


// Generic format for now
class FMTGEN<dag outs, dag ins, string asmstr, list<dag> pattern>
    : InstLPU<outs,ins,asmstr,pattern> {
//let DecoderMethod = "DecodeFGENInstruction";
}
