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

    // Define the maximum possible index that can be used for a regular or
    // a special term;
    static const uint8_t MaxTermIndex = TermONE;

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
    void doTermsMapping(const unsigned TermsMapping[]) {
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

    // Unsets the Mul and Add sign bits in all nodes.
    void unsetAllSignBits() {
      writeToUInt32(EncodedNumTermsAndSigns, 4, getNumNodes() * 2, 0);
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

    // Returns the DAG in unsigned 64-bit form that does not include
    // information about regular terms, i.e. all regular terms are anonymous.
    uint64_t getEncodedDag() { return EncodedDag; }

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

// This class represents an FMA expression in canonized form which is called
// here Sum Of Products. Each of the products has a sign applied to it.
// Here is an example of Sum Of Products having 3 products:
//   -t0*t1*t2 + t0*t1 + t2
//
// This representation makes it possible to transform various DAGs to this
// form, check if those DAGs are equivalent, and replace one DAG with another.
class FMAExprSPCommon {
  protected:
    // This const defines the maxumim number of terms that can be used in one
    // product. This limit is set to some "reasonable" value such a way that
    // sum of products does not use too much memory. It always can be easily
    // fixed if necessary.
    // It is easy to reach the maximum in 16 terms per one product even with
    // a 4 nodes DAG: F0=F1*F1+0;F1=F2*F2+0;F2=F3+F3+0;F3=a*b+0.
    // For 7 nodes DAG it is possible to have maximally 128 terms.
    // Well, we still define it to 16 as due to some heuristics TableGen does
    // not generate more than 16 terms in 1 product.
    static const unsigned MaxNumOfTermsInProduct = 16;

  public:
    // The number of unique regular terms and the special terms 0.0 and 1.0
    // must be unfied with the class FMADagCommon as the classes FMADagCommon
    // and FMAExprSPCommon are often just different representations of the
    // same expression and after conversion from/to each other they use the
    // exactly the same indices for regular and special terms used in them.
    // So, the sum of products representation should be able to use only
    // MaxNumOfUniqueTermsInDAG different regular terms.
    static const unsigned MaxNumOfUniqueTermsInSP =
                            FMADagCommon::MaxNumOfUniqueTermsInDAG;

    // Define two special terms for 0.0 and 1.0 FP values the same way as
    // they are defined in the class FMADagCommon.
    static const uint8_t TermZERO = FMADagCommon::TermZERO;
    static const uint8_t TermONE = FMADagCommon::TermONE;

  protected:
    // Represent one product of terms. For example, (-t0*t1*t2*...*tN).
    // This is used as a building block for Sum Of Products representation.
    struct FMAExprProduct {
      bool     Sign;
      uint8_t  NumTerms;
      uint8_t  Terms[MaxNumOfTermsInProduct];

      // Returns true iff the product consists of only one term TermZERO.
      bool isZero() const { return NumTerms == 1 && Terms[0] == TermZERO; }

      // Returns true iff the product consists of only one term TermONE.
      bool isOne() const { return NumTerms == 1 && Terms[0] == TermONE; }

      // Initializes a product as a singleton, i.e. a product having only one
      // term \p Term. The parameter \p ProductSign specifies the sign of
      // the product.
      void setSingleton(bool ProductSign, unsigned Term) {
        NumTerms = 1;
        Sign = ProductSign;
        Terms[0] = Term;
      }

      // Prints the product to the given output stream \p OS.
      void print(raw_ostream &OS) const {
        OS << (Sign ? '-' : '+');
        for (unsigned i = 0; i < NumTerms; i++) {
          uint8_t Term = Terms[i];
          if (Term == TermZERO)
            OS << "0";
          else if (Term == TermONE)
            OS << "1";
          else
            OS << (char)('a' + Term);
        }
      }

      // Utility function that is needed to sort the terms in the product,
      // used as a working functions for qsort().
      static int compareTermsInFMAProduct(const void *T1, const void *T2) {
        uint8_t C1 = *(const uint8_t *)T1;
        uint8_t C2 = *(const uint8_t *)T2;
        return (int)C1 - (int)C2;
      }

      // Sorts the terms in the product, e.g. (+cadb) --> (+abcd).
      void sortTerms() {
        qsort(Terms, NumTerms, sizeof(uint8_t), compareTermsInFMAProduct);
      }

      // This functions compares two products. It is used in std::stable_sort()
      // which is called to canonize sum of products.
      // This method must return true the same way as 'operator<(P1, P2) would
      // return.
      // The product with greater number of terms or which terms' indices
      // are greater should go first.
      static bool compareProducts(const FMAExprProduct &P1,
                                  const FMAExprProduct &P2) {
        // 1. Compare by lengths.
        int Diff = (int)P1.NumTerms - (int)P2.NumTerms;
        if (Diff > 0) // P1.NumTerms > P2.NumTerms ==> P1 must go first.
          return true;
        if (Diff < 0) // P1.NumTerms < P2.NumTerms ==> P1 must NOT go first.
          return false;

        // 2. Compare the products lexicographically.
        for (unsigned i = 0; i < P1.NumTerms; i++) {
          Diff = (int)P1.Terms[i] - (int)P2.Terms[i];
          if (Diff < 0)
            return true;
          if (Diff > 0)
            return false;
        }

        // 3. If one product has '+' sign and another has '-', then
        // we want the product with '+' sign to go first.
        // The reason why it may matter is that we want to be able to
        // do something with sums of products like: +abc+bd-bd.
        //
        // FIXME: The result of 2 identical products having opposit signs is
        // zero. Such products could be just removed in the method canonize().
        // Before fixing it we need to ensure that it cannot make the patterns
        // recognition any worse.
        if (!P1.Sign && P2.Sign)
          return true;

        // If (P1.Sign && !P2.Sign) then just return false.
        // If (P1 == P2) then return false as it is required by
        // std::stable_sort().
        return false;
      }
    };

    // Number of products in the sum of products.
    unsigned NumProducts;

    // Products composing the sum of products.
    FMAExprProduct *Products;

  public:
    // This is a very simplified form giving a quick snippet of the sum of the
    // products. It has 1 set bit per term in a product and 1 unset bit
    // separating products, e.g.:
    //   SP:    +aaabbbcc+ddd+1
    //   Shape:  11111111011101 (binary form)
    // TODO: Consider hiding this field and implementing a getter method.
    uint64_t Shape;

    // A reference to a DAG that can be lowered/flattened to the current sum
    // of products.
    // TODO: Consider hiding this field and implementing a getter method.
    FMADagCommon *Dag;

    // Default constructor which creates an empty sum of products.
    FMAExprSPCommon() {
      NumProducts = 0;
      Products = nullptr;
      Shape = 0;
      Dag = nullptr;
    };

    // Creates a sum of product for just one given term \p Term.
    FMAExprSPCommon(unsigned Term) {
      NumProducts = 1;
      Products = new FMAExprProduct[1];
      Shape = 1;
      Dag = nullptr;
      Products[0].setSingleton(false, Term);
    };

    // Creates a copy of the given sum of products \p SP.
    FMAExprSPCommon(const FMAExprSPCommon &SP) {
      NumProducts = SP.NumProducts;
      Products = new FMAExprProduct[NumProducts];
      memcpy(Products, SP.Products, NumProducts * sizeof(FMAExprProduct));
      Shape = SP.Shape;
      Dag = nullptr;
    }

    virtual ~FMAExprSPCommon() { delete Dag; delete[] Products; }

    // Returns the number of products in the sum of products.
    unsigned getNumProducts() const { return NumProducts; }

    // Returns the number of terms in the product with index \p ProdInd.
    unsigned getProductSize(unsigned ProdInd) const {
      assert(ProdInd < NumProducts && "Incorrect inputs at getProductSize()");
      return Products[ProdInd].NumTerms;
    }

    // Returns a term from the product with index \p ProdInd and the position
    // in the product \p PositionInProduct.
    unsigned getTerm(unsigned ProdInd, unsigned PositionInProduct) const {
      assert((ProdInd < NumProducts &&
              PositionInProduct < Products[ProdInd].NumTerms) &&
             "Incorrect access to a term in a product.");
      return Products[ProdInd].Terms[PositionInProduct];
    }

    // Returns the number of unique regular terms used in the sum of products.
    // The special terms 0.0 and 1.0 are not included into the returned number.
    unsigned getNumUniqueTerms() const {
      unsigned TermUsedBitVector = 0;
      unsigned NumUniqueTerms = 0;

      for (unsigned ProdInd = 0; ProdInd < NumProducts; ProdInd++) {
        unsigned NumTerms = Products[ProdInd].NumTerms;
        for (unsigned TermInd = 0; TermInd < NumTerms; TermInd++) {
          unsigned Term = Products[ProdInd].Terms[TermInd];
          if (Term == TermZERO || Term == TermONE)
            continue;

          unsigned Mask = 1U << Term;
          if ((TermUsedBitVector & Mask) == 0) {
            TermUsedBitVector |= Mask;
            NumUniqueTerms++;
          }
        }
      }
      return NumUniqueTerms;
    }

    // Uses the given \m TermsMapping array/map to re-map the regular terms.
    // For example, for the following input array:
    //   TermsMapping[] = {1, 1, 0}; // t0->t1, t1->t1, t2->t0
    // and the initial state of the SP:
    //   +t0*t1+t2   // SP has 3 unique terms
    // it changes the SP to the following:
    //   +t1*t1+t0   // SP has 2 unique terms
    void doTermsMapping(const unsigned TermsMapping[]) {
      for (unsigned ProdInd = 0; ProdInd < NumProducts; ProdInd++) {
        uint8_t *Terms = Products[ProdInd].Terms;
        unsigned ProdNumTerms = Products[ProdInd].NumTerms;
        for (unsigned TermInd = 0; TermInd < ProdNumTerms; TermInd++) {
          unsigned Term = Terms[TermInd];
          if (Term != TermZERO && Term != TermONE)
            Terms[TermInd] = TermsMapping[Term];
        }
      }

      if (Dag)
        Dag->doTermsMapping(TermsMapping);
    }

    // Returns true if one of products consists of only the value 1.0.
    // Otherwise, returns false.
    // For example, this method returns true for SP: +ab+1.
    bool hasTermOne() const {
      for (unsigned ProdInd = 0; ProdInd < NumProducts; ProdInd++) {
        if (Products[ProdInd].isOne())
          return true;
      }
      return false;
    }

    // Returns true iff the given sum of products \p SP is identical
    // to 'this' SP. The parameter \p IgnoreProductSigns specifies if the
    // signs of products must be ignored during the comparison.
    bool isEqualTo(const FMAExprSPCommon &SP,
                   bool IgnoreProductSigns = false) const {
      if (NumProducts != SP.NumProducts)
        return false;

      for (unsigned ProdInd = 0; ProdInd < NumProducts; ProdInd++) {
        unsigned ProdNumTerms = Products[ProdInd].NumTerms;
        if (ProdNumTerms != SP.Products[ProdInd].NumTerms)
          return false;
        if (!IgnoreProductSigns &&
            Products[ProdInd].Sign != SP.Products[ProdInd].Sign)
          return false;

        const uint8_t *Terms = Products[ProdInd].Terms;
        const uint8_t *SPTerms = SP.Products[ProdInd].Terms;

        for (unsigned TermInd = 0; TermInd < ProdNumTerms; TermInd++) {
          if (Terms[TermInd] != SPTerms[TermInd])
            return false;
        }
      }

      return true;
    }

    // Returns true iff the given \p SP has the same number of products and
    // all corresponding products have the same signs.
    // Otherwise, returns false.
    bool hasEqualProductSigns(const FMAExprSPCommon &SP) const {
      if (NumProducts != SP.getNumProducts())
        return false;

      for (unsigned ProdInd = 0; ProdInd < NumProducts; ProdInd++) {
        if (Products[ProdInd].Sign != SP.Products[ProdInd].Sign)
          return false;
      }
      return true;
    }

    // Initialize the sum of products as the result of a MUL operation of two
    // given sums of products \p A and \p B.
    //   A:      +abc-d
    //   B:      +ab+e
    //   Result: +abcab+abce-dab-de
    //
    // The returned value is true if the initialization passed successfully.
    // Otherwise, false is returned, which is possible when the result of
    // A and B multiplication is too big (i.e. has too many products, too big
    // products, or has too many terms).
    bool initForMul(const FMAExprSPCommon &A, const FMAExprSPCommon &B) {
      unsigned NewSPNumTerms = 0;
      unsigned NewSPProdInd = 0;

      assert(Products == nullptr &&
             "initForMul() must be used only for empty SP.");

      // Handle this special case to simplify the code below.
      if (A.isZero() || B.isZero()) {
        NumProducts = 1;
        Products = new FMAExprProduct[1];
        Products[0].setSingleton(false, TermZERO);
        return true;
      }

      // If A has M products and B has N products then A*B has M*N products.
      NumProducts = A.getNumProducts() * B.getNumProducts();
      Products = new FMAExprProduct[NumProducts];

      for (unsigned AProd = 0; AProd < A.NumProducts; AProd++) {
        bool AIsOne = A.Products[AProd].isOne();
        for (unsigned BProd = 0; BProd < B.NumProducts; BProd++) {
          unsigned NewProdNumTerms = 0;

          // Step1.
          // Copy the terms of the current product from 'A' to the new product,
          // i.e. start defining the new result product.
          // Skip this step if the current product from A has only one term
          // TermONE.
          if (!AIsOne) {
            for (unsigned i = 0; i < A.Products[AProd].NumTerms; i++) {
              if (NewProdNumTerms >= MaxNumOfTermsInProduct)
                return false;

              Products[NewSPProdInd].Terms[NewProdNumTerms] =
                A.Products[AProd].Terms[i];
              NewProdNumTerms++;
            }
          }

          // Step2.
          // Copy the terms of the current product from 'B' to the new product
          // and finalize the result product.
          // If AIsOne is true then this step cannot be skipped.
          // Otherwise, this step must be skipped when the current product
          // from B has only one term TermONE.
          if (AIsOne || !B.Products[BProd].isOne()) {
            for (unsigned i = 0; i < B.Products[BProd].NumTerms; i++) {
              if (NewProdNumTerms >= MaxNumOfTermsInProduct)
                return false;

              Products[NewSPProdInd].Terms[NewProdNumTerms] =
                B.Products[BProd].Terms[i];
              NewProdNumTerms++;
            }
          }

          // Step3.
          // The product is ready, finalize it now.
          NewSPNumTerms += NewProdNumTerms;
          Products[NewSPProdInd].NumTerms = NewProdNumTerms;
          Products[NewSPProdInd].Sign =
            A.Products[AProd].Sign != B.Products[BProd].Sign;
          NewSPProdInd++;
        } // end for (unsigned BProd = 0; BProd < B.NumProducts; BProd++)
      } // end for (unsigned AProd = 0; AProd < A.NumProducts; AProd++)

      assert(NumProducts == NewSPProdInd &&
             "The number of initialized products is incorrect.");
      return fitsInShape(NumProducts, NewSPNumTerms);
    }

    // Initialize the sum of products as the result of an ADD operation of two
    // given sums of products \p A and \p B.
    //   A:      +abc-d
    //   B:      +ab+e
    //   Result: +abc-d+ab+e
    // The parameters \p ASign and \BSign may be passed to invert the sign
    // of the passed sums of products. So, \p BSign set to true, means that
    // the newly initialized sum of products is the result of subtract
    // operation: (A - B).
    //
    // The returned value is true if the initialization passed successfully.
    // Otherwise, false is returned, which is possible when the sum of A and B
    // produces either too many products, too big products, or too many terms.
    bool initForAdd(const FMAExprSPCommon &A, const FMAExprSPCommon &B,
                    bool ASign, bool BSign) {
      assert(Products == nullptr &&
             "initForAdd() must be used only for empty SP.");

      bool AIsZero = A.isZero();
      bool BIsZero = B.isZero();
      NumProducts = (AIsZero ? 0 : A.getNumProducts()) +
                    (BIsZero ? 0 : B.getNumProducts());
      if (NumProducts == 0)
        NumProducts++;
      Products = new FMAExprProduct[NumProducts];

      unsigned NewSPNumTerms = 0;
      unsigned NewSPProdInd = 0;
      if (!AIsZero) {
        // Copy all products of A to the result.
        for (unsigned AProd = 0; AProd < A.NumProducts; AProd++) {
          Products[NewSPProdInd] = A.Products[AProd];
          if (ASign)
            Products[NewSPProdInd].Sign = !Products[NewSPProdInd].Sign;
          NewSPProdInd++;
          NewSPNumTerms += A.Products[AProd].NumTerms;
        }
      }

      if (!BIsZero || AIsZero) {
        // Copy all products of B to the result.
        for (unsigned BProd = 0; BProd < B.NumProducts; BProd++) {
          Products[NewSPProdInd] = B.Products[BProd];
          if (BSign)
            Products[NewSPProdInd].Sign = !Products[NewSPProdInd].Sign;
          NewSPProdInd++;
          NewSPNumTerms += B.Products[BProd].NumTerms;
        }
      }

      assert(NumProducts == NewSPProdInd &&
             "The number of initialized products is incorrect.");
      return fitsInShape(NumProducts, NewSPNumTerms);
    }

    // Initializes the current sum of products using the given DAG \p D.
    bool initForDag(const FMADagCommon &D) {
      FMAExprSPCommon *OpndSP[3];
      bool IsOk = false;
      unsigned NumNodes = D.getNumNodes();

      // This array keeps the sums of products computed for the FMA Dag nodes
      // with indices (1, 2, ..., NumNodes-1).
      FMAExprSPCommon *NodeSPs = new FMAExprSPCommon[NumNodes - 1];
      // This array of pointers keeps the references to SPs created for regular
      // and special terms;
      FMAExprSPCommon *TermSPs[FMADagCommon::MaxTermIndex + 1] = {};

      // The FMA DAGs (Directed Acyclic Graphs) have the following constraint:
      // if some operand of an FMA node with index 'i' is another FMA, then
      // that operand refers to an FMA node with greater than 'i' index.
      // So, the bottom-up iterative walk through the DAG can be used to
      // compute the sum of products for it.
      for (unsigned NodeInd = NumNodes - 1; (int)NodeInd >= 0; NodeInd--) {

        // 1. Compute Sum Of Products for 3 operands of the current FMA node.
        for (unsigned OpndInd = 0; OpndInd < 3; OpndInd++) {
          bool IsTerm;
          unsigned Opnd = D.getOperand(NodeInd, OpndInd, &IsTerm);
          if (!IsTerm)
            OpndSP[OpndInd] = &NodeSPs[Opnd - 1];
          else if (TermSPs[Opnd] != nullptr)
            OpndSP[OpndInd] = TermSPs[Opnd];
          else {
            OpndSP[OpndInd] = new FMAExprSPCommon(Opnd);
            TermSPs[Opnd] = OpndSP[OpndInd];
          }
        }

        // 2. Compute Sum Of Products for MUL part of the FMA node.
        //
        // TODO: If MUL operation for the operands 0 and 1 was computed some
        // time ago, then it may be a good idea to re-use that SP here.
        // It would require saving/hashing MUL SPs somewhere.
        // Similar could be done for ADD operation, but MULL is more expensive
        // than ADD, so SPs generated for MULs are more important.
        FMAExprSPCommon Mul;
        IsOk = Mul.initForMul(*OpndSP[0], *OpndSP[1]);
        if (!IsOk)
          break;

        // 3. Compute Sum of Products for ADD part of the FMA node.
        FMAExprSPCommon *AddSP = NodeInd == 0 ? this : &NodeSPs[NodeInd - 1];
        IsOk = AddSP->initForAdd(Mul, *OpndSP[2], D.getMulSign(NodeInd),
                                                  D.getAddSign(NodeInd));
        if (!IsOk)
          break;
      }

      for (auto I : TermSPs)
        delete I;

      return IsOk;
    }

    // Computes the SHAPE representation and initializes the correspondng
    // 'Shape' field.
    // SHAPE is a very simplified representation of the sum of products.
    // It uses 1 set bit per 1 term in product and 1 unset bit per + or -
    // operation applyed to the products.
    // For example:
    //   SP: +  abcd+abc+e-f
    //   SHAPE: 111101110101 // binary form
    void computeShape() {
      Shape = 0;
      for (unsigned ProdInd = 0; ProdInd < NumProducts; ProdInd++) {
        unsigned ProdNumTerms = Products[ProdInd].NumTerms;
        uint64_t ProdMask = (1ULL << ProdNumTerms) - 1;
        if (ProdInd) {
          // Shift left the previous product and set the least significant bit
          // to zero to indicate a + or - operation.
          Shape <<= 1;
        }
        Shape = (Shape << ProdNumTerms) | ProdMask;
      }
    }

    // Canonizes the sum of products. Here that means that the terms in each
    // of the products and the products itself must be lexicographically
    // ordered.
    void canonize() {
      unsigned ProdIndex;

      // 1. Sort terms in each of products.
      for (ProdIndex = 0; ProdIndex < NumProducts; ProdIndex++) {
        Products[ProdIndex].sortTerms();
      }

      // 2. Sort products.
      // std::stable_sort was intentionally used here to avoid situations
      // when the output code depends on the compiler host. That is a very
      // uncommon situation, but that might happen.
      std::vector<FMAExprProduct> ProductVector;
      ProductVector.assign(Products, Products + NumProducts);
      std::stable_sort(ProductVector.begin(), ProductVector.end(),
                       FMAExprProduct::compareProducts);
      ProdIndex = 0;
      for (const FMAExprProduct Prod : ProductVector) {
        Products[ProdIndex] = Prod;
        ProdIndex++;
      }
    }

    // Returns true iff all products have positive sign.
    bool allProductsArePositive() const {
      for (unsigned ProdInd = 0; ProdInd < NumProducts; ProdInd++) {
        if (Products[ProdInd].Sign)
          return false;
      }
      return true;
    }

    // Prints the sum of product to the given output stream \p OS.
    void print(raw_ostream &OS) const {
      for (unsigned ProdInd = 0; ProdInd < NumProducts; ProdInd++)
        Products[ProdInd].print(OS);
      OS << ";\n";
    }

  private:
    // Deletes the sums of products referenced in the passed array of pointers
    // \p SPToDelete. The parameter \p ElemNum specifies the number of elements
    // in the array of pointers.
    static void freeSPs(FMAExprSPCommon *SPToDelete[], unsigned ElemNum) {
      for (unsigned i = 0; i < ElemNum; i++)
        delete SPToDelete[i];
    }

    // Returns true iff the current sum of products consists of only one
    // product containing only one term TermZERO.
    bool isZero() const { return NumProducts == 1 && Products[0].isZero(); }

    // Returns true iff the current sum of products consists of only one
    // product containing only one term TermONE.
    bool isOne() const { return NumProducts == 1 && Products[0].isOne(); }

    // Returns true iff 'Shape' can be computed for a sum of products with
    // \p NumProducts and \p NumTerms.
    // The number of bits required to build 'Shape' is equal to the number of
    // required unset bits (unset bits are used to separate products) plus
    // the number of set bits (1 set bit per 1 term in a product).
    static bool fitsInShape(unsigned NumProducts, unsigned NumTerms) {
      return (NumProducts - 1 + NumTerms <= sizeof(uint64_t) * 8);
    }
};

// The purpose of this class is to match two sums of products.
// It has a very simple public interface. Basically it consists of
// one default constructor, one destructor and one method:
//   FMADagCommon *getDagToMatchSPs(const FMAExprSPCommon &FormalSPRef,
//                                  const FMAExprSPCommon &ActualSPRef);
// which does all the SP to SP matching work.
//
// Note that this class does huge amount of work and it is very sensitive to
// changes. The method getDagToMatchSPs() takes the majority (about 86%) of the
// time spent in the FMA TableGen component. It is also critical for the FMA
// optimization performed in CodeGen.
// In order to optimize the efficiency of this code, many of the private
// methods do not accept/pass parameters, but read/write from/to the fields
// defined in this class. That helped to reduce the time spent here by
// about 2-3 times.
class FMASPToSPMatcher {
  private:
    // This structure contains information describing one term that is included
    // into the sum of products.
    // It is needed to make the SP to SP matching faster and to significantly
    // clip the recursion depth in getDagToMatchSPsImpl().
    // The arrays of such structures are initialized in initTermsInfo() and
    // used in canMapFormalTermToActualTerm(). See those functions for details.
    struct FMASPTermInfo {
      unsigned NumUses;          // Number of term uses in SP.
      unsigned NumProducts;      // Number of products using this term.
      unsigned MaxProductUses;   // Maximal number of term uses in one product.
      unsigned MinProductUses;   // Minimal number of term uses in one product.
      unsigned MaxProductLength; // Max length of a product using this term.
      unsigned SumProductLengths;// Sum of lengths of products using this term.
      unsigned ValidNeighbors;   // Bit mask of valid neighbors.
    };

    // All of the fields defined below are stored in the instances of this
    // class just to avoid passing too many parameters to the hottest method
    // getDagToMatchSPsImpl(). The fields are initialized in the method
    // getDagToMatchSPs() and its callees, used and modified in the method
    // getDagToMatchSPsImpl(), and are invalid after return from
    // getDagToMatchSPs();

    // The following two fields are the references to the sums of products
    // being matched. We use the word _formal_ for the sum of products that is
    // the source, i.e. SP which terms are going to be renamed, and the word
    // _actual_ for the destination SP, i.e. the sum of product that we want to
    // get by renaming the terms in FormalSP.
    const FMAExprSPCommon *FormalSP;
    const FMAExprSPCommon *ActualSP;

    // Arrays of structures describing terms in FormsSP and ActualSP.
    FMASPTermInfo FormalTermsInfo[FMAExprSPCommon::MaxNumOfUniqueTermsInSP];
    FMASPTermInfo ActualTermsInfo[FMAExprSPCommon::MaxNumOfUniqueTermsInSP];

    // Number of unique terms in FormalSP.
    unsigned NumFormalTerms;
    // Number of unique terms in ActualSP.
    unsigned NumActualTerms;

    // This array is used to keep information about the numbers of users of
    // terms in ActualSP.
    unsigned ActualTermUses[FMAExprSPCommon::MaxNumOfUniqueTermsInSP];

    // This array is used to keep information about the numbers of terms in
    // FormalSP used to map to interested term of ActualSP.
    // E.g. if N = NumFormalTermsForActual[i], then there are N terms
    // in FormalSP which are mapped to the term with index=i in ActualSP.
    unsigned NumFormalTermsForActual[FMAExprSPCommon::MaxNumOfUniqueTermsInSP];

    // Mapping of the terms in FormalSP to terms of ActualSP.
    unsigned FormalToActualMapping[FMAExprSPCommon::MaxNumOfUniqueTermsInSP];

    // Walks through all products and terms of one of two sums of the products
    // and initializes the structures describing the terms of SP.
    //
    // The parameter \p InitActualSP specifies the sum of products and
    // corresponding array of structures to be initialized. If it is set to
    // true then this call initializes structures describing terms of ActualSP.
    // Otherwise, this calls initializes structures describing terms of
    // FormalSP.
    //
    // Inputs: FormalSP, ActualSP, NumFormalTerms, NumActualTerms.
    // Ouputs: FormalTermsInfo, ActualTermsInfo.
    //         The array ActualTermUses[] is clobbered in this method.
    void initTermsInfo(bool InitActualSP) {
      const FMAExprSPCommon *SP;
      FMASPTermInfo         *TermsInfo;
      unsigned               NumTerms;

      if (InitActualSP) {
        SP = ActualSP;
        TermsInfo = ActualTermsInfo;
        NumTerms = NumActualTerms;
      } else {
        SP = FormalSP;
        TermsInfo = FormalTermsInfo;
        NumTerms = NumFormalTerms;
      }

      // At this point we need some temporary array holding the numbers of uses
      // of each term in each of the products. In order to save some time on
      // memory allocation we temporarily use one of already allocated arrays,
      // i.e. ActualTermUses[]. The description of this method says that
      // this array is clobbered here.
      unsigned *TermUses = ActualTermUses;

      memset(TermsInfo, 0, NumTerms * sizeof(FMASPTermInfo));
      unsigned NumProducts = SP->getNumProducts();
      for (unsigned ProdInd = 0; ProdInd < NumProducts; ProdInd++) {
        unsigned ProdNumTerms = SP->getProductSize(ProdInd);

        // Count the numbers of uses of terms in one product.
        memset(TermUses, 0, NumTerms * sizeof(unsigned));
        for (unsigned TermInd = 0; TermInd < ProdNumTerms; TermInd++) {
          unsigned Term = SP->getTerm(ProdInd, TermInd);
          if (Term != FMAExprSPCommon::TermZERO &&
              Term != FMAExprSPCommon::TermONE)
            TermUses[Term]++;
        }

        for (unsigned Term = 0; Term < NumTerms; Term++) {
          unsigned ThisTermUses = TermUses[Term];
          if (ThisTermUses == 0)
            continue;

          TermsInfo[Term].NumUses += ThisTermUses;

          if (ProdNumTerms > TermsInfo[Term].MaxProductLength)
            TermsInfo[Term].MaxProductLength = ProdNumTerms;

          TermsInfo[Term].SumProductLengths += ProdNumTerms;
          TermsInfo[Term].NumProducts++;

          if (ThisTermUses > TermsInfo[Term].MaxProductUses)
            TermsInfo[Term].MaxProductUses = ThisTermUses;

          if (TermsInfo[Term].MinProductUses == 0)
            TermsInfo[Term].MinProductUses = ThisTermUses;
          else if (ThisTermUses < TermsInfo[Term].MinProductUses)
            TermsInfo[Term].MinProductUses = ThisTermUses;

          // Set ValidNeighbors.
          // Term X is a valid neighbor of Y iff there is at least one product
          // in which both X and Y appear, including the case X == Y as long
          // as X appears at least twice in such product.
          for (unsigned OtherTerm = 0; OtherTerm < NumTerms; OtherTerm++) {
            if (TermUses[OtherTerm] != 0 &&
                (OtherTerm != Term || ThisTermUses != 1)) {
              assert(OtherTerm < 32 &&
                    "Increase the size of ValidNeighbors field and make the "
                    "shift operation below safe.");
              TermsInfo[Term].ValidNeighbors |= 1 << OtherTerm;
            }
          }
        } // for (unsigned Term = 0; Term < NumTerms; Term++)
      } // for (unsigned ProdInd = 0; ProdInd < SP->NumProducts; ProdInd++)
    }

    // Returns true if it is possible to find such combination of MUL and ADD
    // signs at the given \p D DAG nodes that after lowered/flattened of the
    // DAG would give a sum of products with identical product signs as
    // \p SP has. In this case the signs of \p D are changed accordingly.
    // Otherwise, false is returned and the sign bits of \p D are set to some
    // random values.
    //
    // Inputs: The parameters \p D, and \p SP.
    // Outputs: Updated signs in \p D;
    //          A returned bool value.
    bool matchDagSignsToSP(FMADagCommon &D, const FMAExprSPCommon &SP) const {
      // A minor optimization: if all products in SP are positive then
      // the sign bits for all Mul and Add operations in DAG can be unset.
      // This situation is typical for DAGs and SPs generated in TableGen.
      if (SP.allProductsArePositive()) {
        D.unsetAllSignBits();
        return true;
      }

      FMAExprSPCommon TmpSP;

      // FIXME: The following approach is ugly as it is the exhaustive search,
      // which is especially bad as this method is called from another
      // exhaustive search code (even though it is limited by some heuristics).
      // The code below may call the method FMAExprSPCommon::initForDag()
      // up to 1024 times to find sign bits for a DAG with 5 nodes.
      //
      // One of doable solutions is to have special versions of the methods
      //   FMAExprSPCommon::initForMul()
      //   FMAExprSPCommon::initForAdd()
      //   FMAExprSPCommon::canonize()
      //   FMAExprSPCommon::initForDag()
      // which would not only do what they were initially designed for,
      // but also would accumulate and track sign bits of the initial DAG.
      // For example, for such DAG = {X = (-a*Y+b); Y = (c*d-1)} and signs
      // represented as S0=sign(-a*Y)=-1, S1=sign(b)=+1, S2=sign(c*d)=1,
      // S3=sign(-1)=-1 the method initForDag() would return not only SP:
      //   -acd+bcd-a+b
      // but also would build a system of equations:
      //   S0*S2=-1
      //   S0*S3=+1
      //   S1*S2=-1
      //   S1*S3=+1
      // The signs S0,..S3 could be found using some efficient mathematical
      // algorithm or by using an exhaustive search which would iterate through
      // all possible combinations of S0,..S3. Even though it would be an
      // exhaustive search again, it would not require calling the method
      // initForDag() many and many times, i.e. it would be hundreds or
      // thousands times faster.

      // The following {mul_sign, add_sign} combinations are allowed here:
      // NumNodes = 1:  {0,0}, {0,1}, {1,0}, {1,1};
      // NumNodes = 2:  {{0,0},{0,0}}, {{0,0},{0,1}}, ..., {{1,1},{1,1}};
      // NumNodes = 3:  {{0,0},{0,0},{0,0}}, ..., {{1,1},{1,1},{1,1}};
      // ...
      // So, '-a*b-c' can be matched against a*b+c
      unsigned NumNodes = D.getNumNodes();

      // It is safe to use the left shift operation here as NumNodes is small.
      unsigned Pow4NumNodes = 1 << (NumNodes * 2);

      for (unsigned Comb = 0; Comb < Pow4NumNodes; Comb++) {
        // 1. Set the signs combination to FMA DAG.
        unsigned Op = Comb;
        for (unsigned NodeInd = 0; NodeInd < NumNodes; NodeInd++) {
          D.setAddSign(NodeInd, (Op & 1) != 0);
          D.setMulSign(NodeInd, (Op & 2) != 0);
          Op >>= 2;
        }

        // 2. Init the temporary Sum Of Products for FMA DAG.
        TmpSP.initForDag(D);
        TmpSP.canonize();

        // 3. Compare the signs in the temporary SP computed in (2) and signs
        //    in the given SP passed to this method.
        //    If signs combination is found, then keep the current state,
        //    stop iterations and return.
        if (SP.hasEqualProductSigns(TmpSP))
          return true;
      }

      return false;
    }

    // If the terms mapping accumulated in the field FormalToActualMapping[]
    // can be applied to terms of FormalSP to get SP identical to ActualSP,
    // then a copy of the DAG associated with the \p FormalSP is created,
    // the terms in the copied DAG are renamed and the result copy is returned.
    // Otherwise, nullptr is returned.
    //
    // Inputs: FormalSP, ActualSP, FormalToActualMapping[].
    // Outputs: A returned DAG or nullptr.
    FMADagCommon *getDagIfMappingIsValid() const {
      // 1. Create a copy of the 'FormalSP' using the given terms mapping.
      FMAExprSPCommon TmpSP(*FormalSP);
      TmpSP.doTermsMapping(FormalToActualMapping);
      TmpSP.canonize();

      // 2. Check that 'this' SP is equal to TmpSP.
      if (!ActualSP->isEqualTo(TmpSP, true /* IgnoreProductSigns */))
        return nullptr;

      // 3. Create a copy of the dag associated with 'FormalSP' using
      //    the given terms mapping.
      FMADagCommon *ResDag = new FMADagCommon(*FormalSP->Dag);
      ResDag->doTermsMapping(FormalToActualMapping);

      // 4. Set sign bits in ResDag to make it equivalent to ActualSP.
      if (!matchDagSignsToSP(*ResDag, *ActualSP)) {
        delete ResDag;
        ResDag = nullptr;
      }

      return ResDag;
    }

    // This function returns true if the term \p FormalTerm of FormalSP
    // can potentially be mapped to the term \p ActualTerm of ActualSP and it
    // returns false if there are obvious reasons why such mapping is invalid.
    //
    // Inputs: The parameters \p FormalTerm, \p ActualTerm.
    //         The fields ActualTermsInfo[], FormalTermsInfo[], ActualTermUses.
    // Outputs: A returned bool value.
    bool canMapFormalTermToActualTerm(unsigned FormalTerm,
                                      unsigned ActualTerm) const {
      // Attention: the following checks are sorted by their probabilities.
      // The checks that allow to exit from the routine first are placed
      // first. That is done to minimize the time spent in this routine
      // as this routine is included into the hottest path of this TableGen.

      const FMASPTermInfo *ActualTermInfo = &ActualTermsInfo[ActualTerm];

      // Skip the ActualTerm if it is already used more times than in
      // the actual SP.
      if (ActualTermInfo->NumUses < ActualTermUses[ActualTerm])
        return false;

      const FMASPTermInfo *FormalTermInfo = &FormalTermsInfo[FormalTerm];

      // Skip the ActualTerm if the sum of lengths of products using it
      // is smaller than the sum of lengths of products using FormalTerm.
      if (ActualTermInfo->SumProductLengths <
          FormalTermInfo->SumProductLengths)
        return false;

      // Skip the ActualTerm if it is used in smaller number of products
      // than the given FormalTerm.
      //   Example: ActualSP: +ab+b; FormalSP: +ba+a;
      //   Actual 'a' cannot be mapped to formal 'a'.
      if (ActualTermInfo->NumProducts < FormalTermInfo->NumProducts)
        return false;

      // Skip the ActualTerm if the biggest product using it is smaller
      // than the biggest product using the FormalTerm.
      if (ActualTermInfo->MaxProductLength < FormalTermInfo->MaxProductLength)
        return false;

      // Skip the ActualTerm if it has smaller max number of uses in
      // some product than FormalTerm.
      if (ActualTermInfo->MaxProductUses < FormalTermInfo->MaxProductUses)
        return false;

      // Skip the ActualTerm if it has smaller min number of uses in
      // some product that FormalTerm.
      if (ActualTermInfo->MinProductUses < FormalTermInfo->MinProductUses)
        return false;

      // Use the ValidNeighbors field.
      // If FormalTerm and any other formal term (OtherFT), mapped to actual
      // terms ActualTerm and corresponding OtherAT, are valid neighbors
      // then the terms ActualTerm and OtherAT must also be valid neighbors.
      // Otherwise, mapping of FormalTerm to ActualTerm is not valid.
      unsigned FTValidNeighbors = FormalTermsInfo[FormalTerm].ValidNeighbors;
      unsigned ATValidNeighbors = ActualTermsInfo[ActualTerm].ValidNeighbors;
      for (unsigned OtherFT = 0; OtherFT <= FormalTerm; OtherFT++) {
        unsigned IsValidNeighborInFormal = (FTValidNeighbors >> OtherFT) & 1;
        if (IsValidNeighborInFormal) {
          unsigned OtherAT = FormalToActualMapping[OtherFT];
          unsigned IsValidNeighborInActual = (ATValidNeighbors >> OtherAT) & 1;
          if (!IsValidNeighborInActual)
            return false;
        }
      }

      return true;
    }

    // Tries to match the \p FormalSP to \p ActualSP. The matching is
    // successful if it is possible to rename/remap the term in \p FormalSP
    // such a way that the result of that re-mapping gives us \p ActualSP.
    // If matching is successful, then a pointer to a DAG, equivalent to
    // \p ActualSP, is returned. Otherwise, nullptr is returned.
    //
    // The method recursively calls itself, being initially called with
    // parameters \p FormalTerm = 0, \p NumActualTermsMapped = 0. Method adds
    // +1 to \p FormalTerm on each step of the recursion and also updates
    // \p NumActualTermsMapped parameter accordingly. Exit from the recursion
    // happens when \p FormalTerm gets equal to NumFormalTerms or when it is
    // discovered that \p FormalTerm cannot be matched to any of terms from
    // actual SP. The depth of the recursion cannot exceed
    // FMAExprSPCommon::MaxNumOfUniqueTermsInSP which is some relatively small
    // value.
    //
    // Inputs:  The parameters: \p FormalTerm (the formal term that should be
    //          mapped at this call), and \p NumActualTermsMapped (the number
    //          of terms in ActualSP which are not mapped yet).
    //          All fields of this class: FormalTermsInfo[], ActualTermsInfo[],
    //          FormalSP, ActualSP, NumFormalTerms, NumActualTerms,
    //          NumFormalTermsForActual, ActualTermUses[],
    //          FormalToActualMapping[];
    // Outputs: The fields: FormalToActualMapping[], ActualTermUses[],
    //          NumFormalTermsForActual[].
    //          A returned DAG or nullptr.
    FMADagCommon *getDagToMatchSPsImpl(unsigned FormalTerm,
                                       unsigned NumActualTermsMapped) {
      // All terms in FormalSP are already mapped to the terms in ActualSP.
      // Just check if the mapping is valid and return the result DAG.
      if (FormalTerm == NumFormalTerms)
        return getDagIfMappingIsValid();

      unsigned NumActualTermsToMap = NumActualTerms - NumActualTermsMapped;
      unsigned NumFormalTermsToMap = NumFormalTerms - FormalTerm;
      unsigned FormalTermNumUses = FormalTermsInfo[FormalTerm].NumUses;

      // Some of terms in FormalSP are not yet set/mapped. Try mapping
      // the 'FormalTerm' to one of term in ActualSP in the following loop.
      unsigned ActualTerm = 0;
      for (; ActualTerm < NumActualTerms; ActualTerm++) {
        unsigned NewActualTermMapped =
          NumFormalTermsForActual[ActualTerm] == 0 ? 1 : 0;

        // Mapping of FormalTerm to ActualTerm will set the number of free
        // formal terms to (NumFormalTermsToMap - 1) and the number of free
        // actual terms to (NumActualTermsToMap - NewActualTermMapped).
        // Here we check that the mapping will still be possible, i.e. that
        // there will be enough free formal terms to do the matching.
        if (NumActualTermsToMap - NewActualTermMapped >= NumFormalTermsToMap)
          continue;

        ActualTermUses[ActualTerm] += FormalTermNumUses;
        FormalToActualMapping[FormalTerm] = ActualTerm;

        if (canMapFormalTermToActualTerm(FormalTerm, ActualTerm)) {
          NumFormalTermsForActual[ActualTerm]++;
          FMADagCommon *ResDag =
            getDagToMatchSPsImpl(FormalTerm + 1,
                                 NumActualTermsMapped + NewActualTermMapped);
          if (ResDag)
            return ResDag;
          NumFormalTermsForActual[ActualTerm]--;
        }

        ActualTermUses[ActualTerm] -= FormalTermNumUses;
      }
      return nullptr;
    }

  public:
    FMASPToSPMatcher() { }
    ~FMASPToSPMatcher() { }

    // Tries to match \p FormalSP to \p ActualSP. The matching is successful
    // if it is possible to rename/remap the term in \p FormalSP such a way
    // that the result of that re-mapping gives us \p ActualSP.
    // If matching is successful, then a pointer to a DAG, equivalent to
    // \p ActualSP, is returned. Otherwise, nullptr is returned.
    //
    // Inputs: parameters \p FormalSPRef and \p ActualSPRef.
    // Outputs: All fields of this class.
    //          A returned DAG or nullptr.
    FMADagCommon *getDagToMatchSPs(const FMAExprSPCommon &FormalSPRef,
                                   const FMAExprSPCommon &ActualSPRef) {
      NumActualTerms = ActualSPRef.getNumUniqueTerms();
      NumFormalTerms = FormalSPRef.getNumUniqueTerms();
      if (NumFormalTerms < NumActualTerms)
        return nullptr;

      // Terms matching is not implemented for TermONE. It also does not make
      // sense implementing it as it would only complicate the code doing
      // the matching. It is always possible to use a regular term for 1.0
      // const. Actually, this check is always false and potentially it could
      // be replaced with an assert, but having return nullptr is more safe
      // and gentle way to handle such special cases.
      if (ActualSPRef.hasTermOne() || FormalSPRef.hasTermOne())
        return nullptr;

      ActualSP = &ActualSPRef;
      FormalSP = &FormalSPRef;

      initTermsInfo(true /* InitActualSP */);
      initTermsInfo(false /* InitFormalSP */);
      memset(ActualTermUses, 0, NumActualTerms * sizeof(unsigned));
      memset(FormalToActualMapping, 0, NumFormalTerms * sizeof(unsigned));
      memset(NumFormalTermsForActual, 0, NumActualTerms * sizeof(unsigned));

      return getDagToMatchSPsImpl(0, 0);
    }
};

} // End llvm namespace

#endif // LLVM_LIB_TARGET_X86_X86FMACOMMON_H
