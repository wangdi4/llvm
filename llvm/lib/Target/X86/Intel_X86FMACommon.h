//====-- Intel_X86FMACommon.h - Fused Multiply Add optimization -----------====
//
//      Copyright (c) 2016 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//
// This file defines the base classes used by two separate components:
//   1) Table-Gen generating efficient FMA patterns;
//   2) GlobalFMA optimization.
//

#ifndef LLVM_LIB_TARGET_X86_X86FMACOMMON_H
#define LLVM_LIB_TARGET_X86_X86FMACOMMON_H

namespace llvm {

// This class defines structures and methods for holding and maintaining
// expression trees (DAGs). Each node of the DAG describes one FMA-like
// operation having 1 MUL and 1 ADD sub-operations, and having '+' or '-' signs
// used for MUL and ADD addends of the FMA operation:
//   ((mul_sign)(op0 * op1) + (add_sign)op2).
//
// The operands of each node may be of 3 different types:
// - Special term: 0.0 or 1.0;
//     With the help of these special terms MUL and ADD operations can be
//     represented as partial cases of FMA operations, e.g.
//       FMA(a, 1.0, c) is really ADD(a, c),
//       FMA(a, b, 0.0) is MUL(a, b).
// - Regular term (or a leaf of the tree);
// - Reference to another node of the same DAG.
//     Only forward directed references are allowed, i.e. the operand of
//     the node with index 'i' can only refer to a DAG node with a greater
//     index.
class FMADagCommon {
  public:
    // This limit defines the maximum number of operations in an FMA DAG.
    static const unsigned MaxNumOfNodesInDAG = 7;

    // An FMA DAG limited to 7 nodes can have at most 15 terms.
    // There may be up to 21 operands totally (3 operands per node * 7 nodes).
    // At least 6 of those 21 operands must be references to FMA nodes to keep
    // the DAG connected. This gives us the limit in (21 - 6) = 15 terms.
    //
    // Also, this limit is caused by the encoding of the terms in this class.
    // Currently only 4 bits are used to encode one regular term.
    static const unsigned MaxNumOfUniqueTermsInDAG = 15;

    // Define two special terms for 0.0 and 1.0 FP values.
    static const uint8_t TermZERO = MaxNumOfUniqueTermsInDAG;
    static const uint8_t TermONE = TermZERO + 1;

  protected:
    // The following 64-bit value encodes a DAG the way it is stored in
    // pre-computed DAGs storage (include file auto-generated at compiler
    // build time). It holds the following information:
    // - The number of nodes in the DAG. Currently the maximum encodable number
    //   of DAG nodes is 7.
    // - The operands of the DAG nodes.
    // The main concept of the bits layout is the following:
    // - The nodes with lower indices are encoded in the lowest bits.
    // - The node encoding contains 3 groups of bits encoding the 3 operands of
    //   the node. The operands with lower indices are encoded in lower bits.
    // - Each operand is encoded using the following approach:
    //      From 2 to 4 bits (depending on the node index) encode the operand
    //      type and/or a reference to another DAG node if the operand type is
    //      not a term. The value formed from the extracted bits are
    //      interpreted this way:
    //        0 - the operand is ZERO;
    //        1 - the operand is ONE;
    //        2 - the operand is a regular term;
    //        Any value greater than 2 - the operand is a reference to one of
    //        the next nodes. In this case 2 must be subtracted from the value.
    //        I.e. the value 3 would mean: the operand is a reference to
    //        a node with the index equal to (current node index + (3 - 2)).
    //
    // The detailed bits layout for a DAG is the following:
    //   [2 : 0]: the number of nodes in the DAG;
    //   // node0 (4 bits per operand):
    //   [6 : 3]: operand0;
    //   [10: 7]: operand1;
    //   [14:11]: operand2;
    //   // node1 (3 bits per operand):
    //   [17:15]: operand0;
    //   [20:18]: operand1;
    //   [23:21]: operand2;
    //   // node2 (3 bits per operand):
    //   [26:24]: operand0;
    //   [29:27]: operand1;
    //   [32:30]: operand2;
    //   // node3 (3 bits per operand):
    //   [35:33]: operand0;
    //   [38:36]: operand1;
    //   [41:39]: operand2;
    //   // node4 (3 bits per operand):
    //   [44:42]: operand0;
    //   [47:45]: operand1;
    //   [50:48]: operand2;
    //   // node5 (2 bits per operand):
    //   [52:51]: operand0;
    //   [54:53]: operand1;
    //   [56:55]: operand2;
    //   // node6 (2 bits per operand):
    //   [58:57]: operand0;
    //   [60:59]: operand1;
    //   [62:61]: operand2;
    //   [63:63]: unused.
    uint64_t EncodedDag;

    // These 3 elements encode the regular terms used in operand0, operand1
    // and operand2 of the DAG nodes.
    // The regular terms are the leaves of FMA expression trees. They could be
    // associated with some temporary variables in LLVM IR or with virtual
    // registers or loads from memory in MachineInstr IR, but this class does
    // not depend on the input IR. Thus, the regular terms have only indices.
    // It is user's responsibility to map the unsigned indices to objects in
    // input IR.
    //
    // Using 4 bits per one term gives us the way to encode up to 16 unique
    // terms in up to 8 nodes. The operand with index OI from the node with
    // index NI is encoded in EncodedTerms[OI].bits[NI * 4 + 3 : NI * 4]'.
    uint32_t EncodedTerms[3];

    // This field encodes MUL and ADD sign bits of FMA DAG nodes.
    // It also encodes the number of regular terms used in the DAG.
    // The bits [3:0] give the number of unique regular terms used in the DAG.
    // The bit in [NI*2+4] gives the sign used for ADD operation. It is set
    // to 1 if the 3rd operand of FMA node with index NI is subtracted from
    // the product of the 1st and 2nd operands of the same node and it is unset
    // if the 3rd operand is added.
    // The bit in [NI*2+5] gives the sign used for MUL operation. It is set
    // to 1 if the product of the 1st and 2nd operands of the node with index
    // NI is negated and it is unset if the product is positive.
    // The bits [31:MaxNumOfUniqueTermsInDAG*2+4] are unused.
    uint32_t EncodedNumTermsAndSigns;

    // Reads \p Size bits at \p Offset offset from \p V64 value.
    static unsigned readFromUInt64(uint64_t V64, unsigned Offset,
                                                 unsigned Size) {
      assert((Size > 0 && Size <= 32 && Offset <= 64 - Size) &&
             "Illegal read from UInt64");
      uint64_t Mask = (1ULL << Size) - 1;
      return (V64 >> Offset) & Mask;
    }

    // Writes the lowest \p Size bits of \p Val to \p V64 bits
    // [\p Offset: \p Offset + \p Size - 1).
    static void writeToUInt64(uint64_t &V64, unsigned Offset,
                              unsigned Size, unsigned Val) {
      assert((Size > 0 && Size <= 32 && Offset <= 64 - Size) &&
             "Illegal write to UInt64");
      uint64_t Mask = ((1ULL << Size) - 1) << Offset;
      uint64_t InsertedBits = (((uint64_t)Val) << Offset) & Mask;
      V64 = (V64 & ~Mask) | InsertedBits;
    }

    // Reads \p Size bits at \p Offset offset from \p V32 value.
    // This method does not support reading of the full 32-bit \p V32 value,
    // i.e. \p Size must be less than 32.
    static unsigned readFromUInt32(uint32_t V32, unsigned Offset,
                                                 unsigned Size) {
      assert((Size > 0 && Size < 32 && Offset <= 32 - Size) &&
             "Illegal read from UInt32");
      uint32_t Mask = (1 << Size) - 1;
      return (V32 >> Offset) & Mask;
    }

    // Writes the lowest \p Size bits of \p Val to \p V32 bits
    // [\p Offset: \p Offset + \p Size - 1).
    // This method does not support writing of the full 32-bit value Val
    // to V32[31:0], or in other words, \p Size must be less than 32.
    static void writeToUInt32(uint32_t &V32, unsigned Offset,
                              unsigned Size, unsigned Val) {
      assert((Size > 0 && Size < 32 && Offset <= 32 - Size) &&
             "Illegal write to UInt32");
      uint32_t Mask = ((1ULL << Size) - 1) << Offset;
      uint32_t InsertedBits = (((uint32_t)Val) << Offset ) & Mask;
      V32 = (V32 & ~Mask) | InsertedBits;
    }

    // Returns the number of bits used to encode one operand in the given
    // \p NodeInd node.
    // Currently all operands within one node have the same bit size,
    // thus the index of the operand is not passed to this method.
    static unsigned getBitSizeForOperand(unsigned NodeInd) {
      static const uint8_t
        BitsForNodeOpnd[MaxNumOfNodesInDAG] = {4, 3, 3, 3, 3, 2, 2};

      assert(NodeInd < MaxNumOfNodesInDAG && "NodeInd is incorrect.");
      return BitsForNodeOpnd[NodeInd];
    }

    // Returns the bit-offset for the operand \p OpndInd of the node
    // \p NodeInd.
    static unsigned getBitOffsetForOperand(unsigned NodeInd,
                                           unsigned OpndInd) {
      static const uint8_t OpndOffsets[MaxNumOfNodesInDAG][3] = { 3,  7, 11,
                                                                 15, 18, 21,
                                                                 24, 27, 30,
                                                                 33, 36, 39,
                                                                 42, 45, 48,
                                                                 51, 53, 55,
                                                                 57, 59, 61};
      assert((NodeInd < MaxNumOfNodesInDAG && OpndInd < 3) &&
             "NodeInd is incorrect.");
      return OpndOffsets[NodeInd][OpndInd];
    }

    // Returns the regular term (i.e. not equal to 0.0 or 1.0) used in
    // the operand \p OpndInd of the node \p NodeInd.
    // Note that this routine does not check that the referred operand is
    // really a regual term (i.e. that it is encoded as term in 'EncodedDag').
    unsigned getTerm(unsigned NodeInd, unsigned OpndInd) const {
      assert((NodeInd < getNumNodes() && OpndInd < 3) &&
             "Incorrect operands in getTerm() method.");

      // 4 bits to encode 1 term.
      return readFromUInt32(EncodedTerms[OpndInd], NodeInd * 4, 4);
    }

    // Write the \p Term to 'EncodedTerms'.
    // Note that this routine only writes to EncodedTerms, it does NOT write
    // to 'EncodedDag' (i.e. it does not change the operand kind).
    void setTerm(unsigned NodeInd, unsigned OpndInd, unsigned Term) {
      assert((NodeInd < getNumNodes() && OpndInd < 3) &&
             "Incorrect operands in setTerm() method.");

      // 4 bits to encode 1 term.
      writeToUInt32(EncodedTerms[OpndInd], NodeInd * 4, 4, Term);
    }

    // Initializes the terms number using the given \p NumTerms value.
    void setNumTerms(unsigned NumTerms) {
      writeToUInt32(EncodedNumTermsAndSigns, 0, 4, NumTerms);
    }

    // Assigns unique term indices to the regular terms in DAG and initializes
    // the DAG terms number.
    void renumberTerms() {
      unsigned NumNodes = getNumNodes();
      unsigned NumTerms = 0;

      for (unsigned NodeInd = 0; NodeInd < NumNodes; NodeInd++) {
        for (unsigned OpndInd = 0; OpndInd < 3; OpndInd++) {
          bool IsTerm;
          unsigned Opnd = getOperand(NodeInd, OpndInd, &IsTerm);
          if (IsTerm && Opnd != TermZERO && Opnd != TermONE) {
            setTerm(NodeInd, OpndInd, NumTerms);
            NumTerms++;
          }
        }
      }
      setNumTerms(NumTerms);
    }

  public:
    // This constructor creates an FMA DAG using the pre-computed and packed
    // to unsigned 64-bit int form DAG. It also assigns unique indices to
    // the regular terms in this DAG.
    FMADagCommon(uint64_t Encoded64) {
      EncodedDag = Encoded64;
      for (unsigned &ET : EncodedTerms)
        ET = 0;
      EncodedNumTermsAndSigns = 0;
      renumberTerms();
    }

    // This constructor creates a copy of the given \p Dag.
    FMADagCommon(const FMADagCommon &Dag) {
      EncodedDag = Dag.EncodedDag;
      EncodedTerms[0] = Dag.EncodedTerms[0];
      EncodedTerms[1] = Dag.EncodedTerms[1];
      EncodedTerms[2] = Dag.EncodedTerms[2];
      EncodedNumTermsAndSigns = Dag.EncodedNumTermsAndSigns;
    }

    // Uses the given \m TermsMapping array/map to re-map the regular terms.
    // For example, for the following input array:
    //   TermsMapping[] = {1, 1, 0}; // t0->t1, t1->t1, t2->t0
    // and the initial state of the DAG:
    //   t0 * t1 + t2   // DAG has 3 unique terms
    // it changes the DAG to the following:
    //   t1 * t1 + t0   // DAG has 2 unique terms
    void doTermsMapping(unsigned TermsMapping[]) {
      unsigned NumNodes = getNumNodes();
      unsigned NumTerms = 0;
      for (unsigned NodeInd = 0; NodeInd < NumNodes; NodeInd++) {
        for (unsigned OpndInd = 0; OpndInd < 3; OpndInd++) {
          bool IsTerm;
          unsigned Term = getOperand(NodeInd, OpndInd, &IsTerm);
          if (IsTerm && Term != TermZERO && Term != TermONE) {
            unsigned MappedTerm = TermsMapping[Term];
            NumTerms = std::max(NumTerms, MappedTerm + 1);
            setTerm(NodeInd, OpndInd, MappedTerm);
          }
        }
      }
      // The updated DAG may have less terms than the passed 'Dag'
      // because of the mapping. The example is shown in the method comment
      // section above.
      // So, the NumTerms must be fixed accordingly.
      setNumTerms(NumTerms);
    }

    virtual ~FMADagCommon() {}

    // Returns the number of operations in the DAG.
    unsigned getNumNodes() const {return (EncodedDag & 0x7);}

    // Returns the number of regular terms used in the DAG.
    unsigned getNumTerms() const {return (EncodedNumTermsAndSigns & 0xf);}

    // Returns the operand having the index \p OpndInd and used in the DAG node
    // with the index \p NodeInd.
    // The requested operand may be either a term or a reference to some
    // DAG node. If it is a term, then the parameter \p IsTerm is set to true.
    // Otherwise, \p IsTerm is set to false.
    unsigned getOperand(unsigned NodeInd,
                        unsigned OpndInd,
                        bool *IsTerm) const {
      assert((NodeInd < getNumNodes() && OpndInd < 3) &&
             "Illegal parameters in the method getOperand()");

      unsigned BitsPerOpnd = getBitSizeForOperand(NodeInd);
      unsigned Offset = getBitOffsetForOperand(NodeInd, OpndInd);

      unsigned Opnd = readFromUInt64(EncodedDag, Offset, BitsPerOpnd);
      *IsTerm = true;
      if (Opnd == 0)
        Opnd = TermZERO;
      else if (Opnd == 1)
        Opnd = TermONE;
      else if (Opnd == 2)
        Opnd = getTerm(NodeInd, OpndInd);
      else {
        *IsTerm = false;
        Opnd = NodeInd + (Opnd - 2);
      }
      return Opnd;
    }

    // Returns the sign used for the 3rd operand of FMA DAG node referenced by
    // \p NodeInd. The returned value is true if the 3rd operand is subtracted.
    // Otherwise, the returned value is false.
    bool getAddSign(unsigned NodeInd) const {
      return readFromUInt32(EncodedNumTermsAndSigns, 4 + NodeInd * 2, 1);
    }

    // Returns the sign used for the product of the 1st and 2nd operands of
    // the FMA DAG node referenced by \p NodeInd.
    // The returned value is true if the product is subtracted.
    // Otherwise, the returned value is false.
    bool getMulSign(unsigned NodeInd) const {
      return readFromUInt32(EncodedNumTermsAndSigns, 5 + NodeInd * 2, 1);
    }

    // Sets the sign used for the 3rd operand of FMA DAG node referenced by
    // \p NodeInd. The passed value 'true' of \p Sign is used when the 3rd
    // operand is subtracted, and the value 'false' is used when the 3rd
    // operand is added.
    void setAddSign(unsigned NodeInd, bool Sign) {
      writeToUInt32(EncodedNumTermsAndSigns, 4 + NodeInd * 2, 1, Sign ? 1 : 0);
    }

    // Sets the sign used for the product of the 1st and 2nd operand of FMA DAG
    // node referenced by \p NodeInd. The passed value 'true' of \p Sign
    // is used when the product is subtracted, and the value 'false' is used
    // when the product is added.
    void setMulSign(unsigned NodeInd, bool Sign) {
      writeToUInt32(EncodedNumTermsAndSigns, 5 + NodeInd * 2, 1, Sign ? 1 : 0);
    }

    // Returns the depth of the DAG/tree.
    // The optional parameter \p NodeInd may be used to get the depth of
    // the subtree.
    unsigned getDepth(unsigned NodeInd = 0) const {
      unsigned OpndDepth = 0;
      for (unsigned OpndInd = 0; OpndInd < 3; OpndInd++) {
        bool IsTerm;
        unsigned Opnd = getOperand(NodeInd, OpndInd, &IsTerm);
        if (!IsTerm)
          OpndDepth = std::max(OpndDepth, getDepth(Opnd));
      }
      return OpndDepth + 1;
    }

    // Prints the DAG to the given output stream \p OS.
    void print(raw_ostream &OS) const {
      unsigned NumNodes = getNumNodes();
      for (unsigned NodeInd = 0; NodeInd < NumNodes; NodeInd++) {
        if (NodeInd != 0)
          OS << " ";
        OS << "F" << NodeInd << "=" << (getMulSign(NodeInd) ? '-' : '+');
        for (unsigned OpndInd = 0; OpndInd < 3; OpndInd++) {
          bool IsTerm;
          unsigned Opnd = getOperand(NodeInd, OpndInd, &IsTerm);
          if (IsTerm) {
            if (Opnd == TermZERO)
              OS << '0';
            else if (Opnd == TermONE)
              OS << '1';
            else
              OS << char ('a' + Opnd);
          } else {
            OS << 'F' << Opnd;
          }

          if (OpndInd == 0)
            OS << '*';
          else if (OpndInd == 1)
            OS << (getAddSign(NodeInd) ? '-' : '+');
          else if (OpndInd == 2)
            OS << ';';
        }
      }
      OS << "\n";
    }
};

} // End llvm namespace

#endif // LLVM_LIB_TARGET_X86_X86FMACOMMON_H
