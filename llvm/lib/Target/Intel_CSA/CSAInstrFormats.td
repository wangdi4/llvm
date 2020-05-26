//===-- CSAInstrFormats.td - CSA Instruction Formats -------*- tablegen -*-===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Instruction format superclass
//===----------------------------------------------------------------------===//

include "llvm/TableGen/SearchableTable.td"

// Class specifying the available MUL/ADD/FMA operations,
// used by MAPatternsEmitter table-gen/backend.
class MAOperation<bits<2> val> {
  int Index = val;
}
def MAOperationADD : MAOperation<0>; // ADD: A + B
def MAOperationMUL : MAOperation<1>; // MUL: A * B
def MAOperationFMA : MAOperation<2>; // FMA: A * B + C

// Generic operations
// For now, this just declares generic opcodes.  In the long run, we may want
// to have things like operands, type signatures
class GenericOp<string opStr, list<Predicate> preds = []> {
  string AsmString = opStr;
  list<Predicate> Preds = preds;

  // These values are unused, but are defined to make it easier to define these
  // operations with Instruction.
  bit isMultiTriggered = ?;
  int AddedComplexity = 0;
}

class CSAOpInfo;

class InstCSA<GenericOp genOp, CSAOpInfo opInfo, dag outs, dag ins,
      string asmstr, list<dag> pattern = [],
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

  // The generic operator for this operation (e.g. ADD for ADD64)
  GenericOp GenOp = genOp;
  CSAOpInfo OpInfo = opInfo;

  // Multi-triggered instructions are those instructions which do not always
  // consume all inputs and produce on all outputs.
  bit isMultiTriggered = 0;
  let TSFlags{0} = isMultiTriggered;
}

// CSA pseudo instruction format
class PseudoInstCSA<dag outs, dag ins, string asmstr, list<dag> pattern,
      list<Predicate> preds = [], InstrItinClass itin = IIPseudo>
    : InstCSA<?, ?, outs, ins, asmstr, pattern, preds, itin> {
  let isPseudo = 1;
}


// Generic format for now
class FMTGEN<GenericOp genOp, CSAOpInfo opInfo, dag outs, dag ins,
      string asmstr, list<dag> pattern,
      list<Predicate> preds = [], InstrItinClass itin = NoItinerary>
    : InstCSA<genOp, opInfo, outs, ins, asmstr, pattern, preds, itin> {
//let DecoderMethod = "DecodeFGENInstruction";
  let hasSideEffects = 0;
}
