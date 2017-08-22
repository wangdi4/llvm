//===-- Nios2InstrFormats.td - Nios2 Instruction Formats -----*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Describe NIOS2 instructions format
//
//
//===----------------------------------------------------------------------===//

// Format specifies the encoding used by the instruction.  This is part of the
// ad-hoc solution used to emit machine instruction encodings by our machine
// code emitter.
//TODO: is this needed? format seems to be encoded currently in instruction definitions
class Format<bits<5> val> { 
  bits<5> Value = val;
}

def Pseudo     : Format<0>;
// Nios2 R1 instr formats:
def FrmI       : Format<1>;
def FrmR       : Format<2>;
def FrmJ       : Format<3>;
def FrmOther   : Format<4>;  // Instruction w/ a custom format
// Nios2 R2 instr 32-bit formats:
def FrmL26     : Format<5>;  // corresponds to J format in R1
def FrmF2I16   : Format<6>;  // corresponds to I format in R1
def FrmF2X4I12 : Format<7>;
def FrmF1X4I12 : Format<8>;
def FrmF1X4L17 : Format<9>;
def FrmF3X6L5  : Format<10>; // corresponds to R format in R1
def FrmF2X6L10 : Format<11>;
def FrmF3X6    : Format<12>; // corresponds to R format in R1
def FrmF3X8    : Format<13>; // corresponds to custom format in R1
// Nios2 R2 instr 16-bit formats:
def FrmI10     : Format<14>;
def FrmT1I7    : Format<15>; 
def FrmT2I4    : Format<16>;
def FrmT1X1I6  : Format<17>;
def FrmX1I7    : Format<18>;
def FrmL5I4X1  : Format<19>;
def FrmT2X1L3  : Format<20>;
def FrmT2X1I3  : Format<21>;
def FrmT3X1    : Format<22>;
def FrmT2X3    : Format<23>;
def FrmF1X1    : Format<24>;
def FrmX2L5    : Format<25>;
def FrmF1I5    : Format<26>;
def FrmF2      : Format<27>;

def isNios2r1 : Predicate<"Subtarget->isNios2r1()">;
def isNios2r2 : Predicate<"Subtarget->isNios2r2()">;

//===----------------------------------------------------------------------===//
// Base classes for 32-bit, 16-bit and pseudo instructions
//===----------------------------------------------------------------------===//

class Nios2Inst32<dag outs, dag ins, string asmstr, list<dag> pattern,
                  InstrItinClass itin, Format f>: Instruction {
  field bits<32> Inst;
  Format Form = f;

  let Namespace = "Nios2";
  let Size = 4;

  bits<6> Opcode = 0;

  // Bottom 6 bits are the 'opcode' field
  let Inst{5-0} = Opcode;

  let OutOperandList = outs;
  let InOperandList  = ins;

  let AsmString = asmstr;
  let Pattern   = pattern;
  let Itinerary = itin;

  // Attributes specific to Nios2 instructions:

  bits<5> FormBits = Form.Value;
  // TSFlags layout should be kept in sync with Nios2InstrInfo.h.
  let TSFlags{4-0}   = FormBits;
  let DecoderNamespace = "Nios2";
  field bits<32> SoftFail = 0;
}

class Nios2Inst16<dag outs, dag ins, string asmstr, list<dag> pattern,
                  InstrItinClass itin, Format f>: Instruction {
  field bits<16> Inst;
  Format Form = f;

  let Namespace = "Nios2";
  let Size = 2;
  // This will make selector choose 16-bit instructions over 32-bit instr
  // with pattern of similiar complexity.
  let AddedComplexity = 10;

  bits<6> Opcode = 0;

  // Bottom 6 bits are the 'opcode' field
  let Inst{5-0} = Opcode;

  let OutOperandList = outs;
  let InOperandList  = ins;

  let AsmString = asmstr;
  let Pattern   = pattern;
  let Itinerary = itin;

  // Attributes specific to Nios2 instructions:
  bits<5> FormBits = Form.Value;
  // TSFlags layout should be kept in sync with Nios2InstrInfo.h.
  let TSFlags{4-0}   = FormBits;
  let DecoderNamespace = "Nios2r2";
  field bits<16> SoftFail = 0;
}

class Nios2Pseudo<dag outs, dag ins, string asmstr, list<dag> pattern,
      InstrItinClass Itin = IIPseudo>:
  Nios2Inst32<outs, ins, asmstr, pattern, Itin, Pseudo> {

  let isCodeGenOnly = 1;
  let isPseudo = 1;
}

//===----------------------------------------------------------------------===//
// Base classes for R1 and R2 instructions
//===----------------------------------------------------------------------===//

class Nios2R1Inst32<dag outs, dag ins, string asmstr, list<dag> pattern,
                    InstrItinClass itin, Format f>: 
      Nios2Inst32<outs, ins, asmstr, pattern, itin, f> {
  let DecoderNamespace = "Nios2";
  let Predicates = [isNios2r1];
}

class Nios2R2Inst32<dag outs, dag ins, string asmstr, list<dag> pattern,
                    InstrItinClass itin, Format f>: 
      Nios2Inst32<outs, ins, asmstr, pattern, itin, f> {
  let DecoderNamespace = "Nios2r2";
  let Predicates = [isNios2r2];
}

class Nios2R2Inst16<dag outs, dag ins, string asmstr, list<dag> pattern,
                    InstrItinClass itin, Format f>: 
      Nios2Inst16<outs, ins, asmstr, pattern, itin, f> {
  let DecoderNamespace = "Nios2r2";
  let Predicates = [isNios2r2];
}

//===----------------------------------------------------------------------===//
// Format I instruction : <|A|B|immediate|opcode|>
//===----------------------------------------------------------------------===//

class FI<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern,
         InstrItinClass itin>: Nios2R1Inst32<outs, ins, asmstr, pattern, itin, FrmI> {

  bits<5>  rA;
  bits<5>  rB;
  bits<16> imm;

  let Opcode = op;

  let Inst{31-27} = rA;
  let Inst{26-22} = rB;
  let Inst{21-6} = imm;
}

//===----------------------------------------------------------------------===//
// Format R instruction : <|A|B|C|opx|imm|opcode|>
//===----------------------------------------------------------------------===//

class FR<bits<6> opx, dag outs, dag ins, string asmstr, list<dag> pattern,
         InstrItinClass itin>: Nios2R1Inst32<outs, ins, asmstr, pattern, itin, FrmR> {
  bits<5> rA;
  bits<5> rB;
  bits<5> rC;
  bits<5> imm = 0;

  let Opcode = 0x3a; /* opcode is always 0x3a for R instr. */

  let Inst{31-27} = rA;
  let Inst{26-22} = rB;
  let Inst{21-17} = rC;
  let Inst{16-11} = opx; /* opx stands for opcode extension */
  let Inst{10-6}  = imm; /* optional 5-bit immediate value */
}

//===----------------------------------------------------------------------===//
// Format J instruction : <|address|opcode|>
//===----------------------------------------------------------------------===//

class FJ<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern,
         InstrItinClass itin>:
      Nios2R1Inst32<outs, ins, asmstr, pattern, itin, FrmJ> {
  bits<26> addr;
  let Opcode = op;
  let Inst{31-6} = addr;
}

//===----------------------------------------------------------------------===//
// Format F3X6L5 (R2) instruction : <|opx|lit5|C|B|A|opcode|>
//===----------------------------------------------------------------------===//

class F3X6L5<bits<6> opx, dag outs, dag ins, string asmstr, list<dag> pattern,
             InstrItinClass itin>: 
      Nios2R2Inst32<outs, ins, asmstr, pattern, itin, FrmF3X6L5> {
  bits<5> lit5; 
  bits<5> rC;
  bits<5> rB;
  bits<5> rA;

  let Opcode = 0x20; /* opcode is always 0x20 (OPX group) for F3X6L5 instr. */

  let Inst{31-26} = opx; /* opx stands for opcode extension */
  let Inst{25-21} = lit5;
  let Inst{20-16} = rC;
  let Inst{15-11} = rB;
  let Inst{10-6}  = rA;
}

//===----------------------------------------------------------------------===//
// Format F2X6L10 (R2) instruction : <|opx|msb|lsb|B|A|opcode|>
//===----------------------------------------------------------------------===//

class F2X6L10<bits<6> opx, dag outs, dag ins, string asmstr, list<dag> pattern,
              InstrItinClass itin>: 
      Nios2R2Inst32<outs, ins, asmstr, pattern, itin, FrmF2X6L10> {
  bits<5> msb; 
  bits<5> lsb;
  bits<5> rB;
  bits<5> rA;

  let Opcode = 0x20; /* opcode is always 0x20 (OPX group) for F2X6L10 instr. */

  let Inst{31-26} = opx; /* opx stands for opcode extension */
  let Inst{25-21} = msb;
  let Inst{20-16} = lsb;
  let Inst{15-11} = rB;
  let Inst{10-6}  = rA;
}

//===----------------------------------------------------------------------===//
// Format F3X6 (R2) instruction : <|opx|RSV|C|B|A|opcode|>
//===----------------------------------------------------------------------===//

class F3X6<bits<6> opx, dag outs, dag ins, string asmstr, list<dag> pattern,
           InstrItinClass itin>: 
      Nios2R2Inst32<outs, ins, asmstr, pattern, itin, FrmF3X6> {
  bits<5> rC;
  bits<5> rB;
  bits<5> rA;
  bits<5> rsv = 0;

  let Opcode = 0x20; /* opcode is always 0x20 (OPX group) for F3X6 instr. */

  let Inst{31-26} = opx; /* opx stands for opcode extension */
  let Inst{25-21} = rsv;
  let Inst{20-16} = rC;
  let Inst{15-11} = rB;
  let Inst{10-6}  = rA;
}

//===----------------------------------------------------------------------===//
// Format F2I16 (R2) instruction : <|imm|B|A|opcode|>
//===----------------------------------------------------------------------===//

class F2I16<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern,
            InstrItinClass itin>:
      Nios2R2Inst32<outs, ins, asmstr, pattern, itin, FrmI> {
  bits<5>  rA;
  bits<5>  rB;
  bits<16> imm;

  let Opcode = op;

  let Inst{31-16} = imm;
  let Inst{15-11} = rB;
  let Inst{10-6}  = rA;
  let DecoderNamespace = "Nios2r2";
}

//===----------------------------------------------------------------------===//
// Format L26 (R2) instruction : <|addr|opcode|>
//===----------------------------------------------------------------------===//

class L26<bits<6> op, dag ins, string asmstr, list<dag> pattern, InstrItinClass itin>:
    Nios2R2Inst32<(outs), ins, asmstr, pattern, itin, FrmL26> {
  bits<26> addr;
  let Opcode = op;
  let Inst{31-6} = addr;
}

//===----------------------------------------------------------------------===//
// Format F2X4I12 (R2) instruction : <|x|imm|rB|rA|opcode|>
//===----------------------------------------------------------------------===//

class F2X4I12<bits<4> opx, dag outs, dag ins, string asmstr, list<dag> pattern,
              InstrItinClass itin>:
      Nios2R2Inst32<outs, ins, asmstr, pattern, itin, FrmF2X4I12> {
  bits<5>  rA;
  bits<5>  rB;
  bits<12> imm;
  bits<4>  x = opx;

  let Opcode = 0x28;

  let Inst{31-28} = x;
  let Inst{27-16} = imm;
  let Inst{15-11} = rB;
  let Inst{10-6}  = rA;
}

//===----------------------------------------------------------------------===//
// Format F1X4I12 (R2) instruction : <|x|imm|y|rA|opcode|>
//===----------------------------------------------------------------------===//

class F1X4I12<bits<5> opy, dag outs, dag ins, string asmstr, list<dag> pattern,
              InstrItinClass itin>:
      Nios2R2Inst32<outs, ins, asmstr, pattern, itin, FrmF2X4I12> {
  bits<5>  rA;
  bits<5>  y = opy;
  bits<12> imm;
  bits<4>  x = 0x03;

  let Opcode = 0x28;

  let Inst{31-28} = x;
  let Inst{27-16} = imm;
  let Inst{15-11} = y;
  let Inst{10-6}  = rA;
}

//===----------------------------------------------------------------------===//
// Format I10 (R2, 16-bit) instruction : <|imm|opcode|>
//===----------------------------------------------------------------------===//

class I10<bits<6> op, dag ins, string asmstr, list<dag> pattern,
          InstrItinClass itin>:
      Nios2R2Inst16<(outs), ins, asmstr, pattern, itin, FrmI10> {
  bits<10> imm;
  let Opcode = op;
  let Inst{15-6} = imm;
}

//===----------------------------------------------------------------------===//
// Format T1I7 (R2, 16-bit) instruction : <|imm|a3|opcode|>
//===----------------------------------------------------------------------===//

class T1I7<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern,
           InstrItinClass itin>:
      Nios2R2Inst16<outs, ins, asmstr, pattern, itin, FrmT1I7> {
  bits<7> imm;
  bits<3> a3;

  let Opcode = op;

  let Inst{15-9} = imm;
  let Inst{8-6}  = a3;
}

//===----------------------------------------------------------------------===//
// Format T2I4 (R2, 16-bit) instruction : <|imm|b3|a3|opcode|>
//===----------------------------------------------------------------------===//

class T2I4<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern,
           InstrItinClass itin>:
      Nios2R2Inst16<outs, ins, asmstr, pattern, itin, FrmT2I4> {
  bits<4> imm;
  bits<3> b3;
  bits<3> a3;

  let Opcode = op;

  let Inst{15-12} = imm;
  let Inst{11-9}  = b3;
  let Inst{8-6}   = a3;
}

//===----------------------------------------------------------------------===//
// Format T1X1I6 (R2, 16-bit) instruction : <|x|imm|a3|opcode|>
//===----------------------------------------------------------------------===//

class T1X1I6<bits<1> opx, dag outs, dag ins, string asmstr, list<dag> pattern,
             InstrItinClass itin>:
      Nios2R2Inst16<outs, ins, asmstr, pattern, itin, FrmT1X1I6> {
  bits<1> x = opx;
  bits<6> imm;
  bits<3> a3;

  let Opcode = 0x1d; /* STZ.N */

  let Inst{15}   = x;
  let Inst{14-9} = imm;
  let Inst{8-6}  = a3;
}

//===----------------------------------------------------------------------===//
// Format X1I7 (R2, 16-bit) instruction : <|x|rsv|imm|opcode|>
//===----------------------------------------------------------------------===//

class X1I7<bits<1> opx, dag outs, dag ins, string asmstr, list<dag> pattern,
             InstrItinClass itin>:
      Nios2R2Inst16<outs, ins, asmstr, pattern, itin, FrmX1I7> {
  bits<1> x = opx;
  bits<2> rsv;
  bits<7> imm;

  let Opcode = 0x29; /* SPI.N */

  let Inst{15}    = x;
  let Inst{14-13} = rsv;
  let Inst{12-6}  = imm;
}

//===----------------------------------------------------------------------===//
// Format L5I4X1 (R2, 16-bit) instruction : <|x|cs|fp|reg|imm|opcode|>
//===----------------------------------------------------------------------===//

class L5I4X1<bits<1> opx, dag outs, dag ins, string asmstr, list<dag> pattern,
             InstrItinClass itin>:
      Nios2R2Inst16<outs, ins, asmstr, pattern, itin, FrmL5I4X1> {
  bits<1> x = opx;
  bits<1> cs;
  bits<1> fp;
  bits<3> reg;
  bits<4> imm;

  let Opcode = 0x21; /* PP.N */

  let Inst{15}    = x;
  let Inst{14}    = cs;
  let Inst{13}    = fp;
  let Inst{12-10} = reg;
  let Inst{9-6}  = imm;
}

//===----------------------------------------------------------------------===//
// Format T2X1L3 (R2, 16-bit) instruction : <|x|shamt|b3|a3|opcode|>
//===----------------------------------------------------------------------===//

class T2X1L3<bits<1> opx, dag outs, dag ins, string asmstr, list<dag> pattern,
             InstrItinClass itin>:
      Nios2R2Inst16<outs, ins, asmstr, pattern, itin, FrmT2X1L3> {
  bits<1> x = opx;
  bits<3> shamt;
  bits<3> b3;
  bits<3> a3;

  let Opcode = 0x19; /* SHI.N */

  let Inst{15}    = x;
  let Inst{14-12} = shamt;
  let Inst{11-9}  = b3;
  let Inst{8-6}   = a3;
}

//===----------------------------------------------------------------------===//
// Format T2X1I3 (R2, 16-bit) instruction : <|x|imm|b3|a3|opcode|>
//===----------------------------------------------------------------------===//

class T2X1I3<bits<1> opx, dag outs, dag ins, string asmstr, list<dag> pattern,
             InstrItinClass itin>:
      Nios2R2Inst16<outs, ins, asmstr, pattern, itin, FrmT2X1I3> {
  bits<1> x = opx;
  bits<3> imm;
  bits<3> b3;
  bits<3> a3;

  let Opcode = 0x11; /* ASI.N */

  let Inst{15}    = x;
  let Inst{14-12} = imm;
  let Inst{11-9}  = b3;
  let Inst{8-6}   = a3;
}

//===----------------------------------------------------------------------===//
// Format T3X1 (R2, 16-bit) instruction : <|x|c3|b3|a3|opcode|>
//===----------------------------------------------------------------------===//

class T3X1<bits<1> opx, dag outs, dag ins, string asmstr, list<dag> pattern,
             InstrItinClass itin>:
      Nios2R2Inst16<outs, ins, asmstr, pattern, itin, FrmT3X1> {
  bits<1> x = opx;
  bits<3> c3;
  bits<3> b3;
  bits<3> a3;

  let Opcode = 0x01; /* AS.N */

  let Inst{15}    = x;
  let Inst{14-12} = c3;
  let Inst{11-9}  = b3;
  let Inst{8-6}   = a3;
}

//===----------------------------------------------------------------------===//
// Format T2X3 (R2, 16-bit) instruction : <|0|x|b3|a3|opcode|>
//===----------------------------------------------------------------------===//

class T2X3<bits<3> opx, dag outs, dag ins, string asmstr, list<dag> pattern,
             InstrItinClass itin>:
      Nios2R2Inst16<outs, ins, asmstr, pattern, itin, FrmT2X3> {
  bits<3> x = opx;
  bits<3> b3;
  bits<3> a3;

  let Opcode = 0x09; /* R.N */

  let Inst{15}    = 0;
  let Inst{14-12} = x;
  let Inst{11-9}  = b3;
  let Inst{8-6}   = a3;
}

//===----------------------------------------------------------------------===//
// Format F1X1 (R2, 16-bit) instruction : <|1|0|x|rsv|A|opcode|>
//===----------------------------------------------------------------------===//

class F1X1<bits<1> opx, dag outs, dag ins, string asmstr, list<dag> pattern,
             InstrItinClass itin>:
      Nios2R2Inst16<outs, ins, asmstr, pattern, itin, FrmF1X1> {
  bits<1> x = opx;
  bits<2> rsv;
  bits<5> rA;

  let Opcode = 0x09; /* R.N */

  let Inst{15}    = 1;
  let Inst{14}    = 0;
  let Inst{13}    = x;
  let Inst{12-11} = rsv;
  let Inst{10-6}  = rA;
}

//===----------------------------------------------------------------------===//
// Format X2L5 (R2, 16-bit) instruction : <|1|1|x|rsv|lit|opcode|>
//===----------------------------------------------------------------------===//

class X2L5<bits<2> opx, dag outs, dag ins, string asmstr, list<dag> pattern,
             InstrItinClass itin>:
      Nios2R2Inst16<outs, ins, asmstr, pattern, itin, FrmX2L5> {
  bits<2> x = opx;
  bits<1> rsv = 0;
  bits<5> lit = 0;

  let Opcode = 0x09; /* R.N */

  let Inst{15}    = 1;
  let Inst{14}    = 1;
  let Inst{13-12} = x;
  let Inst{11}    = rsv;
  let Inst{10-6}  = lit;
}

//===----------------------------------------------------------------------===//
// Format F1I5 (R2, 16-bit) instruction : <|B|imm|opcode|>
//===----------------------------------------------------------------------===//

class F1I5<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern,
             InstrItinClass itin>:
      Nios2R2Inst16<outs, ins, asmstr, pattern, itin, FrmF1I5> {
  bits<5> rB;
  bits<5> imm;

  let Opcode = op;

  let Inst{15-11} = rB;
  let Inst{10-6}  = imm;
}

//===----------------------------------------------------------------------===//
// Format F2 (R2, 16-bit) instruction : <|B|A|opcode|>
//===----------------------------------------------------------------------===//

class F2<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern,
             InstrItinClass itin>:
      Nios2R2Inst16<outs, ins, asmstr, pattern, itin, FrmF2> {
  bits<5> rB;
  bits<5> rA;

  let Opcode = op;

  let Inst{15-11} = rB;
  let Inst{10-6}  = rA;
}

//===----------------------------------------------------------------------===//
// Multiclasses for common instructions of both R1 and R2:
//===----------------------------------------------------------------------===//

// Multiclass for instructions that have R format in R1 and F3X6 format in R2
// and their opx values differ between R1 and R2
multiclass CommonInstr_R_F3X6_opx<bits<6> opxR1, bits<6> opxR2, dag outs, dag ins,
                                  string asmstr, list<dag> pattern, InstrItinClass itin> {
  def NAME#_R1 : FR<opxR1, outs, ins, asmstr, pattern, itin>;
  def NAME#_R2 : F3X6<opxR2, outs, ins, asmstr, pattern, itin>;
}

// Multiclass for instructions that have R format in R1 and F3X6 format in R2
// and their opx values are the same in R1 and R2
multiclass CommonInstr_R_F3X6<bits<6> opx, dag outs, dag ins, string asmstr,
                              list<dag> pattern, InstrItinClass itin> :
  CommonInstr_R_F3X6_opx<opx, opx, outs, ins, asmstr, pattern, itin>;

// Multiclass for instructions that have R format in R1 and F3X6L5 format in R2
multiclass CommonInstr_R_F2X6L5<bits<6> opx, dag outs, dag ins, string asmstr,
                              list<dag> pattern, InstrItinClass itin> {
  def NAME#_R1 : FR<opx, outs, ins, asmstr, pattern, itin>;
  def NAME#_R2 : F3X6L5<opx, outs, ins, asmstr, pattern, itin>;
}

// Multiclass for instructions that have I format in R1 and F2I16 format in R2
// and their op code values differ between R1 and R2
multiclass CommonInstr_I_F2I16_op<bits<6> opR1, bits<6> opR2, dag outs, dag ins,
                                  string asmstr, list<dag> pattern, InstrItinClass itin> {
  def NAME#_R1 : FI<opR1, outs, ins, asmstr, pattern, itin>;
  def NAME#_R2 : F2I16<opR2, outs, ins, asmstr, pattern, itin>;
}

// Multiclass for instructions that have I format in R1 and F2I16 format in R2
// and their op code values are the same in R1 and R2
multiclass CommonInstr_I_F2I16<bits<6> op, dag outs, dag ins, string asmstr,
                               list<dag> pattern, InstrItinClass itin> :
  CommonInstr_I_F2I16_op<op, op, outs, ins, asmstr, pattern, itin>;

