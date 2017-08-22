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
// static char cvs_id[] = "$Id$";
//
// This tablegen backend is responsible for emitting efficient MUL/ADD/FMA
// patterns for the target code generator. The main purpose of having this
// backend is to do the most expensive part of the efficient MUL/ADD/FMA
// auto-generation work to the compiler build phase and to let code generator
// quickly find pre-computed efficient alternatives/patterns for the input
// expressions.
//
//  External interfaces:
//      void EmitMAPatterns(RecordKeeper &RK, raw_ostream &OS);
//

#include "CodeGenTarget.h"
#include "llvm/Support/Format.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/TableGenBackend.h"
#include <algorithm>
#include <vector>
using namespace llvm;

#define DEBUG_TYPE "x86-global-ma"
#include "llvm/Support/Debug.h"

#include "../../lib/Target/X86/Intel_X86FMACommon.h"

namespace {


// This enum defines known MUL/ADD-like operations that could be used by this
// tablegen. If the target platform supports some or all of those and such
// operation need to be used here then the target description file must
// contain corresponding definitions/instances of 'MAOperation' class.
//
// Potentially some new operations can be added to this enum if expressions
// with such expression can be transformed to a canonic form having only
// MUL and ADD operations.
enum MAOperation {
  ADD = 0, // A + B
  MUL = 1, // A * B
  FMA = 2, // A * B + C
  MAOperationsNum = 3
};

// Describes the known operations.
typedef struct MAOperationDesc {
  MAOperation Index;
  bool        IsSupported;
  const char  *Name;
  const char  *Description;
} MAOperationDesc;

class FMAExprSPTG;

// This class is a child of the class FMADagCommon representing FMA expression
// trees/dags and it adds only some methods required for TableGen.
class FMADagTG : public FMADagCommon {
  private:
    // Returns true iff the DAG node with the given index \p NodeInd is used
    // by at least one of the previous nodes.
    bool isNodeUsed(unsigned NodeInd) const;

    // Returns the number of times the node with the index \p NodeInd is used
    // as an operand in other DAG nodes.
    unsigned countNodeUses(unsigned NodeInd) const;

    // Sets the operand with the index \p OpndInd in the node \p NodeInd to
    // the specified value \p Operand. The parameter \p IsTerm specifies if
    // the parameter \p Operand is an index of a term or an FMA node.
    void setOperand(unsigned NodeInd, unsigned OpndInd,
                    bool IsTerm, unsigned Operand);

    // Sets the operand with the index \p OpndInd in the node \p NodeInd to
    // the specified value \p Operand. There is no any special parameter
    // specifying the kind of the \p Operand, i.e. if it is a term,
    // a special term or an FMA. So, the following agreement is used:
    // The \p Operand is:
    // - another FMA node,   if (Operand < NumNodes);
    // - a regular term,     if (Operand == NumNodes);
    // - a special term 1.0, if (Operand == NumNodes + 1);
    // - a special term 0.0, if (Operand == NumNodes + 2).
    void setOperandSpecial(unsigned NodeInd, unsigned OpndInd,
                           unsigned Operand);

    // Generates a sum of products for this Dag and stores it to the given
    // Dags storage \p SPsStorage.
    // If for some reasons the SP seems inefficient or unsupported then
    // it may not be added to the SPs storage to not pollute it.
    void generateAndStoreSP(std::vector<FMAExprSPTG *> &SPsStorage) const;

    // Generates all possible FMA Dags recursively, generates sums of products
    // for them and puts them to the storage \p SPsStorage.
    //
    // The parameter \p NodeInd specifies the index of the node that is not
    // formed yet. It is supposed that the nodes with indices
    // 0, ..., (\p NodeInd - 1) are already formed. The recursion ends when
    // \p NodeInd becomes equal to (NumNodes - 1) where NumNodes is the number
    // of nodes in DAG. Notice that the DAG size must be properly initialized
    // before the first call of generateNode().
    //
    // The parameter \p NumFMARefs specifies the number of times that FMA nodes
    // have been used as operands by the first (\p NodeInd - 1) DAG nodes,
    // which have been constructed so far. It is added to filter out odd and
    // too complicated FMA DAGs, which may exist, but often not wanted because
    // they are almost never met in real world, are lowered into too big sums
    // of products having too few terms (i.e. being only corner cases), etc.
    void generateNode(unsigned NodeInd, unsigned NumFMARefs,
                      std::vector<FMAExprSPTG *> &SPsStorage);

    // Checks if the DAG seems good, i.e. it is not a duplicate of some other
    // already generated DAG, it does not have obvious inefficiencies, etc.,
    // generates SP for the DAG, links SP and DAG and stores SP to the SPs
    // storage.
    void checkAndSaveDag(std::vector<FMAExprSPTG *> &SPsStorage);

    // Initializes the DAG using the given string \p SpecialDag.
    void initForString(const char *SpecialDag);

    // Defines the number of nodes in the Dag. It is used when the allocated
    // memory and some/all of pre-defined Dag nodes need to be re-used.
    void setNumNodes(unsigned NumNodes) {
      writeToUInt64(EncodedDag, 0, 3, NumNodes);
    }

  public:
    // The default constructor creates an empty Dag, i.e. without any nodes.
    FMADagTG() : FMADagCommon(0) {};

    // This constructor creates a copy of the given \p Dag.
    FMADagTG(const FMADagTG &Dag) : FMADagCommon(Dag) {};

    // Generates a set of DAGs and SPs, links them together and puts SPs to
    // the given storage of sums of products \p SPsStorage.
    // This static method is added to the class FMADagTG because the DAGs
    // are generated first, and only then SPs are generated for DAGs. Also, in
    // the process of building DAGs the knowledge about the DAGs structures and
    // the access to private methods is required.
    static void generateDagsAndSPs(std::vector<FMAExprSPTG *> &SPsStorage);

    // Returns the number of operands in the node with index \p NodeInd, which
    // are references to other DAG nodes.
    unsigned getNumberOfFMAsInNode(unsigned NodeInd) const;

    // Returns the number of regular terms in the node with the index
    // \p NodeInd. Special terms 0.0 and 1.0 are not included into the
    // returned value.
    unsigned getNumberOfRegularTermsInNode(unsigned NodeInd) const;
};

// This class is a child of the class FMAExprSPCommon representing expressions
// in the canonical form (i.e. sum of products) and it adds only some methods
// required for TableGen.
class FMAExprSPTG : public FMAExprSPCommon {
  public:
    // Returns true if this sum of products is more general case of the given
    // sum of products \p SP.
    // For example, (+ab+c) is more general than (+ab+a) because it is always
    // possible to rename the terms in the first SP to get the second SP.
    // Also, (+ab+c) is considered similar to (+ac+b) because it is also
    // possible to rename terms in the first to get the second.
    bool isSimilarOrMoreGeneralCaseThan(const FMAExprSPTG &SP) const;

    // Returns true iff the DAG associated with this sum of products is similar
    // or better than the DAG associated with the given sum of products \p SP.
    // The first DAG is similar or better than the second if the first has
    // the same of smaller number of operations AND has the same or smaller
    // depth. If one DAG has smaller number of operations but has bigger
    // depth than another, then the first is not better (and not worse) than
    // another.
    // Usually this function is called to compare DAGs associated with SPs
    // having the same shape or even identical SPs, but that does not add
    // any constraints to this method and it potentially can be asked to
    // compare DAGs associated with SPs having different shapes as well.
    bool hasSimilarOrBetterDagThan(const FMAExprSPTG &SP) const;
};

bool FMAExprSPTG::isSimilarOrMoreGeneralCaseThan(const FMAExprSPTG &SP) const {
  // If 'this' can be transformed to the given SP, then 'this' SP is
  // equivalent to or more general than the passed SP.
  FMASPToSPMatcher SPMatcher;
  FMADagCommon *TmpDag = SPMatcher.getDagToMatchSPs(*this, SP);

  if (!TmpDag)
    return false;

  delete TmpDag;
  return true;
}

bool FMAExprSPTG::hasSimilarOrBetterDagThan(const FMAExprSPTG &SP) const {
  return (Dag->getDepth() <= SP.Dag->getDepth() &&
          Dag->getNumNodes() <= SP.Dag->getNumNodes());
}

bool FMADagTG::isNodeUsed(unsigned NodeInd) const {
  for (unsigned N = 0; N < NodeInd; N++) {
    bool AIsTerm, BIsTerm, CIsTerm;

    unsigned A = getOperand(N, 0, &AIsTerm);
    if (!AIsTerm && A == NodeInd)
      return true;

    unsigned B = getOperand(N, 1, &BIsTerm);
    if (!BIsTerm && B == NodeInd)
      return true;

    unsigned C = getOperand(N, 2, &CIsTerm);
    if (!CIsTerm && C == NodeInd)
      return true;
  }
  return false;
}

unsigned FMADagTG::countNodeUses(unsigned NodeInd) const {
  unsigned NumUses = 0;
  for (unsigned N = 0; N < NodeInd; N++) {
    bool AIsTerm, BIsTerm, CIsTerm;
    unsigned A = getOperand(N, 0, &AIsTerm);
    unsigned B = getOperand(N, 1, &BIsTerm);
    unsigned C = getOperand(N, 2, &CIsTerm);
    if (!AIsTerm && A == NodeInd)
      NumUses++;
    if (!BIsTerm && B == NodeInd)
      NumUses++;
    if (!CIsTerm && C == NodeInd)
      NumUses++;
  }
  return NumUses;
}

void FMADagTG::setOperand(unsigned NodeInd, unsigned OpndInd,
                          bool IsTerm, unsigned Opnd) {

  assert((NodeInd < getNumNodes() && OpndInd < 3) &&
         "Illegal parameters in the method setOperand()");

  unsigned BitsPerOpnd = getBitSizeForOperand(NodeInd);
  unsigned Offset = getBitOffsetForOperand(NodeInd, OpndInd);

  if (!IsTerm) {
    assert(Opnd < getNumNodes() && "FMA node index is out of range.");
    writeToUInt64(EncodedDag, Offset, BitsPerOpnd, Opnd + 2 - NodeInd);
  } else if (Opnd == TermZERO)
    writeToUInt64(EncodedDag, Offset, BitsPerOpnd, 0);
  else if (Opnd == TermONE)
    writeToUInt64(EncodedDag, Offset, BitsPerOpnd, 1);
  else {
    // It is a regular term.
    assert(Opnd < MaxNumOfUniqueTermsInDAG && "Term index is out of range.");
    writeToUInt64(EncodedDag, Offset, BitsPerOpnd, 2);
    setTerm(NodeInd, OpndInd, Opnd);
  }
}

void FMADagTG::setOperandSpecial(unsigned NodeInd, unsigned OpndInd,
                                 unsigned OpndVal) {
  unsigned NumNodes = getNumNodes();

  bool IsTerm;
  if (OpndVal < NumNodes)
    IsTerm = false;
  else {
    IsTerm = true;
    if (OpndVal == NumNodes)
      OpndVal = 0; // 1st term, it must be renamed later.
    else if (OpndVal == NumNodes + 1)
      OpndVal = TermONE;
    else if (OpndVal == NumNodes + 2)
      OpndVal = TermZERO;
  }
  setOperand(NodeInd, OpndInd, IsTerm, OpndVal);
}

class MAPatternsEmitter {
  RecordKeeper &Records;
private:
  // Parse the input Target Description files, find the supported MUL/ADD/etc
  // operations and configure the patterns emitter.
  void parseTDAndConfigurePatternsEmitter(raw_ostream &OS,
                                          MAOperationDesc AllMAOperations[]);

  // Emit X86MA patters emitter for X86 target.
  void emitX86Patterns(raw_ostream &OS, MAOperationDesc AllMAOperations[]);

  // Removes duplicated and inefficient sums of products from the storage
  // \p SPsStorage.
  void removeRedundantPatterns(std::vector<FMAExprSPTG *> &SPsStorage);

public:
  MAPatternsEmitter(RecordKeeper &R) : Records(R) {}

  // Output the MA patterns for the target code generator.
  void run(raw_ostream &OS);
};

unsigned FMADagTG::getNumberOfFMAsInNode(unsigned NodeInd) const {
  bool IsTerm0, IsTerm1, IsTerm2;
  getOperand(NodeInd, 0, &IsTerm0);
  getOperand(NodeInd, 1, &IsTerm1);
  getOperand(NodeInd, 2, &IsTerm2);
  return (IsTerm0 ? 0 : 1) + (IsTerm1 ? 0 : 1) + (IsTerm2 ? 0 : 1);
}

unsigned FMADagTG::getNumberOfRegularTermsInNode(unsigned NodeInd) const {
  bool IsTerm0, IsTerm1, IsTerm2;
  unsigned Opnd0 = getOperand(NodeInd, 0, &IsTerm0);
  unsigned Opnd1 = getOperand(NodeInd, 1, &IsTerm1);
  unsigned Opnd2 = getOperand(NodeInd, 2, &IsTerm2);

  unsigned NumTerms = 0;
  // We are looking for non-constant terms only.
  if (IsTerm0 && Opnd0 != TermZERO && Opnd0 != TermONE)
    NumTerms++;
  if (IsTerm1 && Opnd1 != TermZERO && Opnd1 != TermONE)
    NumTerms++;
  if (IsTerm2 && Opnd2 != TermZERO && Opnd2 != TermONE)
    NumTerms++;

  return NumTerms;
}

void
FMADagTG::generateAndStoreSP(std::vector<FMAExprSPTG *> &SPsStorage) const {
  FMAExprSPTG *SP = new FMAExprSPTG();
  if (!SP->initForDag(*this)) {
    delete SP;
    return;
  }

  // If it is possible to filter out redundant SPs ealy, then do that now.
  // For example, we do not need SPs like (+ab+cd+1) as it is always possible
  // to have more generalized SP (+ab+cd+e), i.e. use a regular term instead
  // of special TermONE. Another reason to remove such SP is that it makes
  // the code doing the patterns matching simpler.
  if (SP->hasTermOne()) {
    delete SP;
    return;
  }

  // Ok, finalize the SP initialization and store it to the SPs storage.
  SP->Dag = new FMADagTG(*this);
  SP->canonize();
  SP->computeShape();
  SPsStorage.push_back(SP);
}

// Initializes the DAG using the given string \p DagString.
//
// Example of a DAG encoded in a string:
// "7: A=B*C+0; B=D*E+0; C=F*G+0; D=T*T+0; E=T*T+0; F=T*T+0; G=T*T+0;"
void FMADagTG::initForString(const char *DagString) {
  unsigned NumNodes = DagString[0] - '0';
  assert(NumNodes > 0 && NumNodes <= MaxNumOfNodesInDAG &&
         DagString[1] == ':' &&
         "Special dag is incorrectly encoded in string.");
  DagString += 2;
  setNumNodes(NumNodes);

  for (unsigned CurNodeInd = 0; CurNodeInd < NumNodes; CurNodeInd++) {
    char     NodeSymbol;
    char     OpndSymbol[3];

    sscanf(DagString, " %c=%c*%c+%c;", &NodeSymbol,
           OpndSymbol, OpndSymbol + 1, OpndSymbol + 2);

    assert((unsigned)(NodeSymbol - 'A') == CurNodeInd &&
           "FMA DAG nodes are defined in wrong order.");
    DagString += 9;

    for (unsigned OpndInd = 0; OpndInd < 3; OpndInd++) {
      bool IsTerm = true;
      unsigned Opnd;
      switch (OpndSymbol[OpndInd]) {
        case '0': // Const 0.0
          Opnd = TermZERO;
          break;
        case '1': // Const 1.0
          Opnd = TermONE;
          break;
        case 'T': // Anonymous term.
          Opnd = 0;
          break;
        default: // FMA node.
          IsTerm = false;
          Opnd = OpndSymbol[OpndInd] - 'A';
          assert(Opnd > CurNodeInd &&
                 "FMA DAG nodes are defined in wrong order.");
      }
      setOperand(CurNodeInd, OpndInd, IsTerm, Opnd);
    }
  }

  renumberTerms();
}

void FMADagTG::checkAndSaveDag(std::vector<FMAExprSPTG *> &SPsStorage) {
  unsigned NumNodes = getNumNodes();

  // Reject DAGs having two nodes forming this pattern:
  //   CurFMA(OtherFMA, 1, *);
  //   OtherFMA(A, B, 0); // CurFMA is the only user of OtherFMA.
  // because we already should have some DAG where those 2 nodes are fused
  // into one:
  //   CurFMA(A, B, *);
  for (unsigned CurFMAInd = 0; CurFMAInd < NumNodes - 1; CurFMAInd++) {
    bool CurAIsTerm, CurBIsTerm, OtherCIsTerm;

    unsigned OtherFMAInd = getOperand(CurFMAInd, 0, &CurAIsTerm);
    if (CurAIsTerm)
      continue;

    unsigned CurBTerm = getOperand(CurFMAInd, 1, &CurBIsTerm);
    if (!CurBIsTerm || CurBTerm != TermONE)
      continue;

    unsigned OtherCTerm = getOperand(OtherFMAInd, 2, &OtherCIsTerm);
    if (OtherCIsTerm && OtherCTerm == TermZERO &&
        countNodeUses(OtherFMAInd) == 1)
      return;
  }

  // Reject DAGs that have nodes not sorted by depth (avoid repeats).
  unsigned CurNodeDepth = MaxNumOfNodesInDAG;
  for (unsigned NextNodeInd = 1; NextNodeInd < NumNodes - 1; NextNodeInd++) {
    unsigned NextNodeDepth = getDepth(NextNodeInd);

    // If the nodes of the DAG are not sorted by their depth, then skip
    // such DAG as a duplicate of a similar DAG but having nodes sorted.
    // TODO: consider adding some changes to the method generateNode() that
    // would filter out such DAGs at the moment they are constructed.
    if (NextNodeDepth > CurNodeDepth)
      return;

    if (NextNodeDepth == CurNodeDepth) {
      // The nodes have equal depth. Check how many terms are there.
      unsigned CurFMAsNum = getNumberOfFMAsInNode(NextNodeInd - 1);
      unsigned NextFMAsNum = getNumberOfFMAsInNode(NextNodeInd);
      if (CurFMAsNum < NextFMAsNum)
        return;

      if (CurFMAsNum == NextFMAsNum) {
        unsigned CurNumTerms = getNumberOfRegularTermsInNode(NextNodeInd - 1);
        unsigned NextNumTerms = getNumberOfRegularTermsInNode(NextNodeInd);
        if (CurNumTerms < NextNumTerms)
          return;
      }
    }
    CurNodeDepth = NextNodeDepth;
  }

  renumberTerms();
  generateAndStoreSP(SPsStorage);
}

void FMADagTG::generateNode(unsigned NodeInd, unsigned NumFMARefs,
                            std::vector<FMAExprSPTG *> &SPsStorage) {
  unsigned NumNodes = getNumNodes();
  assert(NodeInd < NumNodes && "Cannot add any more nodes to the DAG.");

  // The Dag is still under construction. NodeInd is the index of the node
  // is being currently constructed.
  //
  // In the following 3 nested loops iterate through the possible values of
  // the FMA node operands A, B and C using some special agreement.
  // The operand is:
  // - one of next DAG nodes (operand < NumNodes);
  // - a terminal            (operand == NumNodes);
  // - 1.0                   (operand == NumNodes + 1);
  // - 0.0                   (operand == NumNodes + 2);
  for (unsigned A = NodeInd + 1; A <= NumNodes + 1; A++) {
    // The first operand A can be anything except 0.0.
    setOperandSpecial(NodeInd, 0, A);

    for (unsigned B = A; B <= NumNodes + 1; B++) {
      // The second operand B can be enything except 0.0.
      // Also, B >= A to avoid commutative repeats
      // I.e. having FMA(F1,F3,x) makes FMA(F3,F1,x) redundant.
      setOperandSpecial(NodeInd, 1, B);

      // Reject FMA(1, 1, *)
      if (A == NumNodes + 1 && B == NumNodes + 1)
        continue;

      for (unsigned C = NodeInd + 1; C <= NumNodes + 2; C++) {
        // The third operand C can be anything, including the value 0.0.

        unsigned NumLocalFMARefs = 0;
        if (A < NumNodes)
          NumLocalFMARefs++;
        if (B < NumNodes)
          NumLocalFMARefs++;
        if (C < NumNodes)
          NumLocalFMARefs++;

        // Reject the DAG if it seems too complicated.
        // Removing this constraint would increase the number of DAGs by more
        // than two times. It is though clear that the majority of such
        // additional DAGs are quite odd, and unlikely to be met in real world
        // applications.
        // Here is example of DAG/SP filtered out by this constraint:
        //   +aaaaaaaabbbbbbbb+aaaaaabbbbbb+aaaaaabbbbbb+aaaabbbb+aaabbb+ab+ab;
        //   F0=+F1*F2+F4; F1=+F2*F3+1; F2=+F3*F4+F4; F3=+F4*F4+0; F4=+a*b+0;
        // Such DAGs would only pollute the auto-generated header file
        // with patterns.
        //                                 x  1  2  3  4  5  6  7
        static unsigned MaxNumFMARefs[] = {0, 0, 3, 5, 6, 6, 6, 6};
        if (NumFMARefs + NumLocalFMARefs > MaxNumFMARefs[NumNodes])
          continue;

        // Do not allow FMA(*, 1, 0).
        if (B == NumNodes + 1 && C == NumNodes + 2)
          continue;

        // Reject dags having FMA(F3, 1, F1) because
        // we already have FMA(F1, 1, F3).
        if (B == NumNodes + 1 && A > C)
          continue;

        setOperandSpecial(NodeInd, 2, C);

        unsigned NextNodeInd = NodeInd + 1;
        if (NextNodeInd < NumNodes) {
          // Need to construct at least one more node.

          // If next node is not reachable from already constructed nodes,
          // then there is no need in constructing it.
          if (!isNodeUsed(NextNodeInd))
            continue;

          // Construct the next node.
          generateNode(NodeInd + 1, NumFMARefs + NumLocalFMARefs, SPsStorage);
        } else
          // The DAG is constructed, check it and save to the DAGs storage.
          checkAndSaveDag(SPsStorage);
      }
    }
  }
}

void FMADagTG::generateDagsAndSPs(std::vector<FMAExprSPTG *> &SPsStorage) {

  // We can use string notation to encode special cases of DAGs when
  // we want to manually add them to the dags storage. This mechanism is used
  // for dags having many nodes, i.e. for those that are not generated using
  // the standard machanism implemented in the generateNode() method.
  // The following notation is used:
  //   Capital letter - FMA node. Subtracting 'A' from the used capital letter
  //                    gives the FMA node index.
  //                    For example, 'B' stands for FMA node with index=1.
  //   T - Regular term;
  //   0 - Special term 0.0;
  //   1 - Special term 1.0;
  // Currently only up to 7 nodes per 1 Dag can be used. The number of nodes
  // is specified before the definition of the first node.
  const char *SpecialDags[] = {
    // +abcdefg; compute as ((a*b)*(c*d))*((e*f)*g).
    "6: A=B*C+0; B=D*E+0; C=F*T+0; D=T*T+0; E=T*T+0; F=T*T+0;",

    // +a+b+c+d+e+f+g; compute as ((a+b)+(c+d))+((e+f)+g).
    "6: A=B*1+C; B=D*1+E; C=F*1+T; D=T*1+T; E=T*1+T; F=T*1+T;",

    // +abcdefgh, compute as ((a*b)*(c*d))*((e*f)*(g*h)).
    "7: A=B*C+0; B=D*E+0; C=F*G+0; D=T*T+0; E=T*T+0; F=T*T+0; G=T*T+0;",

    // +a+b+c+d+e+f+g+h; compute as ((a+b)+(c+d))+((e+f)+(g+h)).
    "7: A=B*1+C; B=D*1+E; C=F*1+G; D=T*1+T; E=T*1+T; F=T*1+T; G=T*1+T;",

    // +abi+cdi+efi+ghi+j;
    // latency optimized; compute as ((a*b+(c*d))+(e*f+(g*h))*i+j.
    "6: A=B*T+T; B=C*1+D; C=T*T+E; D=T*T+F; E=T*T+0; F=T*T+0;",

    // +ab+cd+ef+gh+ij+kl;
    // latency optimized; compute as (a*b+(c*d+(e*f)))+(g*h+(i*j+(k*l))).
    "7: A=B*1+C; B=T*T+D; C=T*T+E; D=T*T+F; E=T*T+G; F=T*T+0; G=T*T+0;",

    // +ab+cd+ef+gh+ij+kl+m;
    // latency optimized; compute as (a*b+(c*d+(e*f)))+(g*h+(i*j+(k*l+m))).
    "7: A=B*1+C; B=T*T+D; C=T*T+E; D=T*T+F; E=T*T+G; F=T*T+0; G=T*T+T;",

    // +ab+cd+ef+gh+ij+kl+m;
    // recurrence (m) optimized;
    // compute as (a*b+(c*d+(e*f+(g*h+(i*j+(k*l))))))+m.
    // (throughput optimized version is autogenerated).
    "7: A=B*1+T; B=T*T+C; C=T*T+D; D=T*T+E; E=T*T+F; F=T*T+G; G=T*T+0;",

    // +ab+cd+ef+gh+ij+lm+k;
    // throughput optimized; compute as (a*b+(c*d+(e*f+(g*h+(i*j+(l*m+k)))))).
    "6: A=T*T+B; B=T*T+C; C=T*T+D; D=T*T+E; E=T*T+F; F=T*T+T;",

    // +ab+cd+ef+gh+jk+l;
    // recurrence (l) optimized; compute as (a*b+(c*d+(e*f+(g*h+(j*k)))))+l;
    // (throughput optimized version is autogenerated).
    "6: A=B*1+T; B=T*T+C; C=T*T+D; D=T*T+E; E=T*T+F; F=T*T+0;",
  };

  FMADagTG Dag;
  for (unsigned i = 0; i < sizeof(SpecialDags) / sizeof(SpecialDags[0]); i++) {
    Dag.initForString(SpecialDags[i]);
    Dag.generateAndStoreSP(SPsStorage);
  }

  // Automatically generate DAGs, generate SPs for them, link DAGs and SPs,
  // and store SPs to a storage.
  for (unsigned NumNodes = 1; NumNodes <= 5; NumNodes++) {
    // We are going to initialize the first NumNodes nodes in the Dag.
    // Initialize the EncodedDag to 0 to clean the upper bits to clean
    // any garbage in upper bits of the EncodedDag.
    // That is needed only for more comfortable debugging.
    Dag.EncodedDag = 0ULL;

    Dag.setNumNodes(NumNodes);
    Dag.generateNode(0, 0, SPsStorage);
  }
}

// This is the worker function for std::stable_sort() that is used to
// sort the sums of products before removing inefficient/duplicated SPs.
// The priorities in this routine are:
// 1) SPs are sorted by shape to group SPs with the same shape and to make it
//    possible to use binary search using the SHAPE.
// 2) Put the most general SPs first to reduce the number SP-to-SP matchings
//    that must be performed to remove inefficient/duplicated SPs.
// 3) Put SPs having better DAGs first to slightly improve the removal of
//    inefficient/duplicated SPs.
static bool cmpSPsBeforeFiltering(const FMAExprSPTG *A, const FMAExprSPTG *B) {
  // 1. Compare shapes.
  if (A->Shape < B->Shape)
    return true;
  if (A->Shape > B->Shape)
     return false;

  // 2. Compare the numbers of terms in SPs.
  // The SP with bigger number of terms goes first.
  int Cmp = A->getNumUniqueTerms() - B->getNumUniqueTerms();
  if (Cmp > 0)
    return true;
  if (Cmp < 0)
    return false;

  // 3. Compare the numbers of nodes in the DAGs (the smaller DAG goes first).
  Cmp = A->Dag->getNumNodes() - B->Dag->getNumNodes();
  if (Cmp < 0)
    return true;
  if (Cmp > 0)
    return false;

  // 4. Compare depth (the DAG with smaller depth goes first).
  Cmp = A->Dag->getDepth() - B->Dag->getDepth();
  if (Cmp < 0)
    return true;

  // Return false by default.
  return false;
}

// This is the worker function for std::stable_sort() that is used to
// sort the sums of products after removing inefficient/duplicated SPs.
// The priorities in this routine are:
// 1) Preserve the sorting done by SHAPE.
// 2) Put the SPs with better DAGs first to simplify the work of
//    FMA optimization that is going to use the pre-generated SPs.
//    So, the first found SP-to-SP match would give the best DAG there.
// 3) Preserve the sorting by the number of terms in SPs. SPs having more terms
//    and thus more flexible/general go first.
static bool cmpSPsAfterFiltering(const FMAExprSPTG *A, const FMAExprSPTG *B) {
  // 1. Compare shapes.
  if (A->Shape < B->Shape)
    return true;
  if (A->Shape > B->Shape)
     return false;

  // 2. Compare the numbers of nodes in the DAGs (the smaller DAG goes first).
  int Cmp = A->Dag->getNumNodes() - B->Dag->getNumNodes();
  if (Cmp < 0)
    return true;
  if (Cmp > 0)
    return false;

  // 3. Compare depth (the DAG with smaller depth goes first).
  Cmp = A->Dag->getDepth() - B->Dag->getDepth();
  if (Cmp < 0)
    return true;
  if (Cmp > 0)
    return false;

  // 4. Compare the numbers of terms in SPs.
  // The SP with bigger number of terms goes first.
  Cmp = A->getNumUniqueTerms() - B->getNumUniqueTerms();
  if (Cmp > 0)
    return true;

  // Return false by default.
  return false;
}

// Remove redundant SPs, for example:
// Redundant   : +ab+a+a+a+a;
//               F0=+F1*a+0; F1=+F2*1+1; F2=+F3*1+1; F3=+F4*1+1; F4=+b*1+1;
// Already have: +ab+c+c+d+d;
//               F0=+F1*1+F2; F1=+a*b+F2; F2=+c*1+d;
// The second is more efficient and more general/flexible (4 terms vs 2 terms).
//
// Redundant   : +abcd+cd: F0:+F1*F2+F2; F1:+a*b+0; F2:+c*d+0;
// Already have: +abcd+ab: F0:+F1*F2+F1; F1:+a*b+0; F2:+c*d+0;
// The first DAG is just equivalent of the second one.
void MAPatternsEmitter::removeRedundantPatterns(
                                      std::vector<FMAExprSPTG *> &SPsStorage) {
  for (auto CurIt = SPsStorage.begin(); CurIt != SPsStorage.end(); CurIt++) {
    FMAExprSPTG *CurSP = *CurIt;

    // Now iterate NextIt while see the same shape as CurIt has.
    auto NextIt = CurIt;
    for (NextIt++; NextIt != SPsStorage.end();) {
      FMAExprSPTG *NextSP = *NextIt;

      if (CurSP->Shape != NextSP->Shape)
        break;

      if (CurSP->hasSimilarOrBetterDagThan(*NextSP) &&
          CurSP->isSimilarOrMoreGeneralCaseThan(*NextSP)) {
        delete NextSP;
        NextIt = SPsStorage.erase(NextIt);
      } else
        NextIt++;
    }
  }
}
} // End anonymous namespace

// Parses the input target description files and looks for the definitions
// of the class 'MAOperation' and instances of that class. Such instances
// define the MULL/ADD-like operations available in the target and let this
// tablegen using them.
void
MAPatternsEmitter::parseTDAndConfigurePatternsEmitter(
                       raw_ostream &OS,
                       MAOperationDesc AllMAOperations[]) {

  // Print the enumarator with all known MUL/ADD operations.
  OS << "// The following operations are known by this tablegen and\n"
        "// potentially can be generated by this patterns emitter:\n";
  for (unsigned OpInd = 0; OpInd < MAOperationsNum; OpInd++) {
    OS << "//   " << std::string(AllMAOperations[OpInd].Name)
       << " : MAOperation<" << OpInd << ">; // "
       << std::string(AllMAOperations[OpInd].Description) << "\n";
  }

  // Check if the the class MAOperation is defined, exit if it is not.
  Record *MAOperationClass = Records.getClass("MAOperation");
  if (!MAOperationClass) {
    OS << "//\n"
          "// The target does not define the class MAOperation \n"
          "// and thus it does not define any MUL/ADD/FMA operations.\n"
          "//\n";
    return;
  }

  // Find the MA operations, i.e. instances of MAOperation class.
  // Set the 'IsSupported' field to true when found supported instances.
  // The found operations are not emitted right here to avoid duplications
  // and to print them only after sort by the value of the 'Index' field.
  for (const auto &D : Records.getDefs()) {
    if (D.second->isSubClassOf(MAOperationClass)) {
      unsigned Val = D.second->getValueAsInt("Index");
      if (Val >= MAOperationsNum)
        OS << "\n!Warning: Found an unexpected MAOperation.\n\n";
      else
        AllMAOperations[Val].IsSupported = true;
    }
  }

  // Print the MA operations available in the target platform.
  OS << "\n// The following operations are available in the target platform:\n";
  for (unsigned OpInd = 0; OpInd < MAOperationsNum; OpInd++) {
    if (AllMAOperations[OpInd].IsSupported)
      OS << "//   " << std::string(AllMAOperations[OpInd].Name) << "\n";
  }
}

void
MAPatternsEmitter::emitX86Patterns(raw_ostream &OS,
                                   MAOperationDesc AllMAOperations[]) {

  // FIXME: We cannot proceed without ADD/MUL/FMA operations.
  // It should be possible to relax this constraint later, for example,
  // generate patterns when only ADD and MUL operations are available.
  if (!AllMAOperations[ADD].IsSupported ||
      !AllMAOperations[MUL].IsSupported ||
      !AllMAOperations[FMA].IsSupported) {
    OS << "\n\n// Warning: Cannot generate patterns without "
          "ADD/MUL/FMA operations!\n";
    return;
  }

  // 1. Try all possible Dags and generate/collect SPs for relatively
  //    good Dags.
  std::vector<FMAExprSPTG *> SPsStorage;
  FMADagTG::generateDagsAndSPs(SPsStorage);

  // 2. Sort SPs to make the removal of inefficient/duplicated faster.
  std::stable_sort(SPsStorage.begin(), SPsStorage.end(),
                   cmpSPsBeforeFiltering);

  OS << "\n// Initial number of dags:                             "
     << SPsStorage.size() << "\n";

  // 3. Remove duplicated or inefficient SPs/Dags.
  removeRedundantPatterns(SPsStorage);

  // 4. Sort SPs to make the FMA optimization working faster.
  //    Put the SPs with better DAGs first. So, the first found SP match
  //    would give the best DAG.
  std::stable_sort(SPsStorage.begin(), SPsStorage.end(), cmpSPsAfterFiltering);

  // 5. Count the number of different SHAPEs, group Dags with the same SHAPE
  //    and count the number of Dags in each of such groups.
  uint64_t CurShape = 0;
  unsigned NumShapes = 0;
  std::vector<unsigned> NumDagsWithSameShapeVec;
  unsigned NumDagsWithSameShape = 0;
  for (auto SP : SPsStorage) {
    if (CurShape != SP->Shape) {
      CurShape = SP->Shape;
      if (NumDagsWithSameShape > 0) {
        NumDagsWithSameShapeVec.push_back(NumDagsWithSameShape);
        NumDagsWithSameShape = 0;
      }
      NumShapes++;
    }
    NumDagsWithSameShape++;
  }
  if (NumDagsWithSameShape > 0)
    NumDagsWithSameShapeVec.push_back(NumDagsWithSameShape);

  OS << "// Number of dags after elimination of redundant dags: "
     << SPsStorage.size() <<"\n//\n";
  OS << "// Number of shapes:                                   "
     << NumShapes << "\n\n\n";
  OS << "Dags.resize(" << NumShapes << ");\n\n";

  // 6. Print the SHAPEs and DAGs to the auto-generated include file.
  unsigned DagInd = 0;
  for (unsigned ShapeInd = 0; ShapeInd < NumShapes; ShapeInd++) {
    FMAExprSPTG *SP = SPsStorage[DagInd];
    CurShape = SP->Shape;

    // Start printing a new group of dags with the shape equal to 'CurShape'.
    OS << "// SHAPE = " << format_hex(CurShape, 2) << "\n";
    unsigned NumDagsInGroup = NumDagsWithSameShapeVec[ShapeInd];
    OS << "static const uint64_t DagsSet" << ShapeInd
       << "[" << NumDagsInGroup << "] = {\n";

    // Print the group of dags.
    for (unsigned i = 0; i < NumDagsInGroup; i++) {
      SP = SPsStorage[DagInd++];

      OS << "  // ";
      SP->print(OS);
      OS << "  // ";
      SP->Dag->print(OS);

      OS << "  " << format_hex(SP->Dag->getEncodedDag(), 2) << "LL";
      if (i < NumDagsInGroup)
        OS << ",";
      OS << "\n";
    }

    // Close the group.
    OS << "};\nDags[" << ShapeInd << "] = new FMAPatternsSet(DagsSet"
       << ShapeInd << ", " << NumDagsInGroup << ");\n\n";
  }
}

void MAPatternsEmitter::run(raw_ostream &OS) {
  CodeGenTarget Target(Records);

  emitSourceFileHeader("MA Patterns Header Fragment", OS);

  MAOperationDesc AllMAOperations[] = {
    {ADD, false, "MAOperationADD", "ADD: A + B"},
    {MUL, false, "MAOperationMUL", "MUL: A * B"},
    {FMA, false, "MAOperationFMA", "FMA: A * B + C"}
  };
  parseTDAndConfigurePatternsEmitter(OS, AllMAOperations);

  // Target.getName returns an object of the type string.
  // So, it is ok to use '==' operation below.
  if (Target.getName() == "X86")
    emitX86Patterns(OS, AllMAOperations);
}

namespace llvm {

void EmitMAPatterns(RecordKeeper &RK, raw_ostream &OS) {
  MAPatternsEmitter(RK).run(OS);
}

} // End llvm namespace
