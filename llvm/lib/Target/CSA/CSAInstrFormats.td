//===-- CSAInstrFormats.td - CSA Instruction Formats -------*- tablegen -*-===//
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

// Generic operations
// For now, this just declares generic opcodes.  In the long run, we may want
// to have things like operands, type signatures
class GenericOp<string opStr, list<Predicate> preds = []> : Instruction {
  string Namespace = "CSA";
  string AsmString = opStr;
  // No description for generic ops currently
  let OutOperandList = (outs);
  let InOperandList = (ins);
  list<Predicate> Preds = preds;
}

class CSAOpInfo;

class InstCSA<dag outs, dag ins, string asmstr, list<dag> pattern = [],
      list<Predicate> preds = [], InstrItinClass itin = NoItinerary>
    : Instruction {
  // Actual encoding TBD.  Might be variable length.
  field bits<64> Inst;

  let Namespace = "CSA";
  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString   = asmstr;
  let Pattern = pattern;
  let Predicates = preds;
  let Itinerary = itin;

  GenericOp GenOp; // The generic operator for this operation (e.g. ADD for ADD64)
  CSAOpInfo OpInfo;
}

// CSA pseudo instruction format
class PseudoInstCSA<dag outs, dag ins, string asmstr, list<dag> pattern,
      list<Predicate> preds = [], InstrItinClass itin = IIPseudo>
    : InstCSA<outs, ins, asmstr, pattern, preds, itin> {
  let isPseudo = 1;
}


// Generic format for now
class FMTGEN<dag outs, dag ins, string asmstr, list<dag> pattern,
      list<Predicate> preds = [], InstrItinClass itin = NoItinerary>
    : InstCSA<outs, ins, asmstr, pattern, preds, itin> {
//let DecoderMethod = "DecodeFGENInstruction";
  let hasSideEffects = 0;
}
