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

class InstLPU<dag outs, dag ins, string asmstr, list<dag> pattern,
      list<Predicate> preds, InstrItinClass itin = NoItinerary>
    : Instruction {
  // Actual encoding TBD.  Might be variable length.
  field bits<64> Inst;

  let Namespace = "LPU";
  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString   = asmstr;
  let Pattern = pattern;
  let Predicates = preds;
  let Itinerary = itin;
}

// LPU pseudo instruction format
class PseudoInstLPU<dag outs, dag ins, string asmstr, list<dag> pattern,
      list<Predicate> preds = [], InstrItinClass itin = NoItinerary>
    : InstLPU<outs, ins, asmstr, pattern, preds, itin> {
  let isPseudo = 1;
  let Itinerary = IIPseudo;
}


// Generic format for now
class FMTGEN<dag outs, dag ins, string asmstr, list<dag> pattern,
      list<Predicate> preds = [], InstrItinClass itin = NoItinerary>
    : InstLPU<outs, ins, asmstr, pattern, preds, itin> {
//let DecoderMethod = "DecodeFGENInstruction";
}
