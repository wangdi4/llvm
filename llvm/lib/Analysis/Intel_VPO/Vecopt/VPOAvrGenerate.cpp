//===-- VPOAvrGenerate.cpp ------------------------------------------------===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the AVR Generation Pass.
///
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrGenerate.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/Passes.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrVisitor.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionUtils.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"
#include <algorithm>

#define DEBUG_TYPE "avr-generation"

using namespace llvm;
using namespace llvm::vpo;
using namespace llvm::loopopt;

INITIALIZE_PASS_BEGIN(AVRGenerate, "avr-generate", "AVR Generate", false, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(PostDominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WRegionInfo);
INITIALIZE_PASS_END(AVRGenerate, "avr-generate", "AVR Generate", false, true)

char AVRGenerate::ID = 0;

INITIALIZE_PASS_BEGIN(AVRGenerateHIR, "hir-avr-generate", "AVR Generate HIR",
                      false, true)
INITIALIZE_PASS_DEPENDENCY(HIRParser)
INITIALIZE_PASS_END(AVRGenerateHIR, "hir-avr-generate", "AVR Generate HIR",
                    false, true)

char AVRGenerateHIR::ID = 0;

// Abstract Layer command line options

static cl::opt<bool>
    AvrStressTest("avr-stress-test", cl::init(false),
                  cl::desc("Construct full Avrs for stress testing"));

static cl::bits<ALOpts> DisableALOpt(
    "disable-avr-opt",
    cl::desc("Specify abstract layer optimization to disable: "), cl::Hidden,
    cl::values(clEnumVal(ALBuild, "Disable Abstract Layer Build"),
               clEnumVal(ALLoopOpt, "Disable Abstract Layer Loop Opt"),
               clEnumVal(ALBranchOpt, "Disable Abstract Layer Branch Opt"),
               clEnumVal(ALExprTreeOpt, "Disable Abstract Layer Expr Tree Opt"))
    );

// Pass Initialization

FunctionPass *llvm::createAVRGeneratePass() { return new AVRGenerate(); }
FunctionPass *llvm::createAVRGenerateHIRPass() { return new AVRGenerateHIR(); }

AVRGenerateBase::AVRGenerateBase(char &ID) : FunctionPass(ID) {
  setLLVMFunction(nullptr);
  setAvrFunction(nullptr);
  setAvrWrn(nullptr);
  AbstractLayer.clear();

  // Set Optimization Level
  // Default is Abstract Layer build with all optimizations enabled.
  DisableALBuild = DisableALOpt.isSet(ALBuild) ? true : false;
  DisableLoopOpt = DisableALOpt.isSet(ALLoopOpt) ? true : false;
  DisableAvrBranchOpt = DisableALOpt.isSet(ALBranchOpt) ? true : false;
  DisableAvrExprTreeOpt = DisableALOpt.isSet(ALExprTreeOpt) ? true : false;
}

void AVRGenerateBase::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<PostDominatorTreeWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
}

bool AVRGenerateBase::runOnFunction(Function &F) {
  setLLVMFunction(&F);

  // Build the base Abstract Layer representation.
  if (!DisableALBuild) {

    buildAbstractLayer();

    addLabelReferences();

    DEBUG(dbgs() << "Abstract Layer:\n");
    DEBUG(this->dump(PrintAvrType));
  }

  // Insert AVRLoops into Abstract Layer
  if (!DisableLoopOpt) {

    optimizeLoopControl();

    DEBUG(dbgs() << "Abstract Layer After Loop Formation:\n");
    DEBUG(this->dump(PrintAvrType));
  }

  // Insert AVRIfs into Abstract Layer
  if (!DisableAvrBranchOpt) {

#if 0
    // In the process of moving the optimization of the AL, improving
    // its stability. Temporary disable until next check-in 
    // AVR IFs are now constructed by default.
    optimizeAvrBranches();

    DEBUG(dbgs() << "Abstract Layer After If Formation:\n");
    DEBUG(this->dump(PrintAvrType));
#endif
  }

  // Insert AVRExpression and AVRValue nodes and build expression trees into
  // Abstract Layer.
  if (!DisableAvrExprTreeOpt) {

    optimizeAvrExpressions();    // Add expressions and values
    optimizeAvrSubExpressions(); // Build sub-expressions

    DEBUG(dbgs() << "Abstract Layer After Expression Tree Formation:\n");
    DEBUG(this->dump(PrintAvrType));
  }

  // Clean up unnecessary AVR nodes.
  removeAvrNOPs();

  return false;
}

class AVRAddLabelReferences {

private:
  /// AL - Abstract Layer to fix.
  AVRGenerateBase *AL;

  /// \brief Utility function for adding the AVR label of a successor basic
  /// block as a successor of the branch.
  void addBranchSuccessor(AVRBranchIR *ABranch, BasicBlock *SuccBBlock);

  /// \brief Utility function for adding the AVR label of a successor HLLabel as
  /// a successor of the branch.
  void addBranchSuccessor(AVRBranchHIR *ABranch, HLLabel *SuccHLabel);

public:
  AVRAddLabelReferences(AVRGenerateBase *AbstractLayer) : AL(AbstractLayer) {}

  /// Visit Functions
  void visit(AVR *ANode) {}
  void postVisit(AVR *ANode) {}
  void visit(AVRBranchIR *ABranchIR);
  void visit(AVRBranchHIR *ABranchHIR); 
  void postVisit(AVRPhiIR *APhiIR);
  bool isDone() { return false; }
  bool skipRecursion(AVR *ANode) { return false; }
};

void AVRAddLabelReferences::addBranchSuccessor(AVRBranchIR *ABranch,
                                               BasicBlock *SuccBBlock) {

  // Search AL for AVRLabel generated for this BB.
  auto Itr = AL->AvrLabels.find(SuccBBlock);
  assert(Itr != AL->AvrLabels.end() &&
         "Avr Label for BB not found in abstract layer!");
  AVRLabelIR *ChildrenBegin = Itr->second;
  ABranch->addSuccessor(ChildrenBegin);
}

void AVRAddLabelReferences::addBranchSuccessor(AVRBranchHIR *ABranch,
                                               HLLabel *SuccHLabel) {

  // Search AL for AVRLabel generated for this HLabel.
  auto Itr = AL->AvrLabelsHIR.find(SuccHLabel);
  assert(Itr != AL->AvrLabelsHIR.end() &&
         "Avr Label for HLabel not found in abstract layer!");
  AVRLabel *ALabel = Itr->second;
  ABranch->addSuccessor(ALabel);
}

void AVRAddLabelReferences::visit(AVRBranchIR *ABranchIR) {

  if (ABranchIR->isConditional()) {

    addBranchSuccessor(ABranchIR, ABranchIR->getThenBBlock());
    addBranchSuccessor(ABranchIR, ABranchIR->getElseBBlock());
  } else {

    addBranchSuccessor(ABranchIR, ABranchIR->getNextBBlock());
  }
}

void AVRAddLabelReferences::visit(AVRBranchHIR *ABranchHIR) {

  assert(!ABranchHIR->isConditional() &&
         "Unexpected conditional branch in HIR");
  assert(isa<HLGoto>(ABranchHIR->getHIRInstruction()) &&
         "Unexpected ABranchHIR");
  const HLGoto *HGoto = cast<HLGoto>(ABranchHIR->getHIRInstruction());

  addBranchSuccessor(ABranchHIR, HGoto->getTargetLabel());
}

void AVRAddLabelReferences::postVisit(AVRPhiIR *APhiIR) {

  PHINode *Phi = cast<PHINode>(APhiIR->getLLVMInstruction());

  AVRUtils::setAVRPhiLHS(APhiIR,
                         AVRUtilsIR::createAVRValueIR(Phi, Phi, APhiIR));

  unsigned IncomingNum = Phi->getNumIncomingValues();
  for (unsigned Ind = 0; Ind < IncomingNum; ++Ind) {

    Value *Incoming = Phi->getIncomingValue(Ind);
    AVRValue *AValue = AVRUtilsIR::createAVRValueIR(Incoming, Phi, APhiIR);

    BasicBlock *Predecessor = Phi->getIncomingBlock(Ind);
    auto Itr = AL->AvrLabels.find(Predecessor);
    assert(Itr != AL->AvrLabels.end() && "Missing label for incoming value");

    AVRUtils::addAVRPhiIncoming(APhiIR, AValue, Itr->second);
  }
}

// Abstract Layer Visitor Classes

// Avr Branch Optimization: if-formation.
/// \brief AVRBranchOptVisitor class is a specialized visitor which walks the
/// Abstract Layer and idenitifies conditional AvrBranch nodes which can be
/// transformed to AvrIf nodes.
///
/// This visitor constructs a vector of objects of type CandidateIf. A
/// CandidateIf simply contains a pointer to the cond-branch along with
/// pointers to then and else blocks which the brnach jumps to. CandidateIf
/// objects are consumed in the tranformation phase of Avr Branch optimization.
///
class AVRBranchOptVisitor {

  typedef SmallVector<CandidateIf *, 16> CandidateIfTy;
  typedef CandidateIfTy::reverse_iterator reverse_iterator;
  typedef CandidateIfTy::const_reverse_iterator const_reverse_iterator;

private:
  /// AL - Abstract Layer to optimize.
  AVRGenerateBase *AL;

  /// CandidateIfs - Vector of CandidateIfs identified by this visitor.
  CandidateIfTy CandidateIfs;

  /// \brief Returns a CandidateIf if ABranch can be represnted as an
  /// AVRIf.
  CandidateIf *generateAvrIfCandidate(AVRBranchIR *ABranch);

  /// \brief Returns the CandidateIf which lexically first branches to ALabel
  /// for short ciruits. Returns null if ALabel is not part of a sc-chain.
  CandidateIf *identifyShortCircuitParent(AVRLabelIR *ALabel);

  /// \brief Returns an AvrBlock (range of avrs specified by a begin and end
  /// avr) that represents the given BBlock.
  AvrBlock *findIfChildrenBlock(BasicBlock *BBlock);

  /// \brief Returns true if ThenChildren and ElseChildren contain
  /// supported control-flow for AvrIf optimization
  bool isSupportedAvrIfChildren(AvrBlock *ThenChildren, AvrBlock *ElseChidlren);

public:
  AVRBranchOptVisitor(AVRGenerateBase *AbstractLayer) : AL(AbstractLayer) {}

  /// Visit Functions
  void visit(AVR *ANode) {}
  void visit(AVRBranchIR *ABranchIR);
  void postVisit(AVR *ANode) {}
  bool isDone() { return false; }
  bool skipRecursion(AVR *ANode) { return false; }

  /// \brief Return number of candidate ifs identified.
  unsigned getNumberOfCandidates() { return CandidateIfs.size(); }

  /// \brief Returns true if CandidateIfs is empty
  bool isEmpty() { return CandidateIfs.empty(); }

  // The If transformation will be bottom up. Only define reverse
  // iterators.
  reverse_iterator rbegin() { return CandidateIfs.rbegin(); }
  reverse_iterator rend() { return CandidateIfs.rend(); }
  const_reverse_iterator rbegin() const { return CandidateIfs.rbegin(); }
  const_reverse_iterator rend() const { return CandidateIfs.rend(); }
};

CandidateIf *
AVRBranchOptVisitor::identifyShortCircuitParent(AVRLabelIR *AvrLabel) {

  // Search for short circuit in else-block
  auto Itr = std::find_if(CandidateIfs.begin(), CandidateIfs.end(),
                          [AvrLabel](CandidateIf *obj) -> bool {
                            if (obj->hasElseBlock()) {
                              return obj->getElseBegin() == AvrLabel;
                            }
                            return false;
                          });

  if (Itr != CandidateIfs.end()) {
    return *Itr;
  }

  return nullptr;
}

bool AVRBranchOptVisitor::isSupportedAvrIfChildren(AvrBlock *ThenChildren,
                                                   AvrBlock *ElseChildren) {

  if (!ThenChildren || !ElseChildren)
    return false;

  if (AVRBranch *ThenTerm = dyn_cast<AVRBranch>(ThenChildren->getEnd())) {

    if (AVRBranch *ElseTerm = dyn_cast<AVRBranch>(ElseChildren->getEnd())) {

      if (!ThenTerm->isConditional() && !ElseTerm->isConditional())
        return true;

      // TODO: Check Successors and support more compilcated if structures.
    }
  }

  return false;
}

AvrBlock *AVRBranchOptVisitor::findIfChildrenBlock(BasicBlock *BBlock) {

  if (!BBlock)
    return nullptr;

  // Search AL for AVRLabel generated for this BB.
  auto Itr = AL->AvrLabels.find(BBlock);
  if (Itr != AL->AvrLabels.end()) {

    AVRLabelIR *ChildrenBegin = Itr->second;
    AVR *ChildrenEnd = ChildrenBegin->getTerminator();

    assert(ChildrenBegin && ChildrenEnd && "Malformed If-children block!");

    return new AvrBlock(ChildrenBegin, ChildrenEnd);
  }

  // Unable to find Avr Label for given BBlock.
  llvm_unreachable("Avr Label for BB not found in abstract layer!");

  return nullptr;
}

CandidateIf *AVRBranchOptVisitor::generateAvrIfCandidate(AVRBranchIR *ABranch) {

  if (!ABranch->isConditional()) {
    // Unconditional branches are not If candidates.
    return nullptr;
  }

  if (ABranch->isBottomTest()) {
    // Bottom tests do not form If candidates - just set the predecessors and
    // return.
    return nullptr;
  }

  BasicBlock *ThenBBlock = ABranch->getThenBBlock();
  BasicBlock *ElseBBlock = ABranch->getElseBBlock();
  AvrBlock *ThenChildren = nullptr, *ElseChildren = nullptr;
  CandidateIf *ShortCircuitParent = nullptr;
  AVRBranch *ShortCircuitBr = nullptr;

  if (ThenBBlock) {
    ThenChildren = findIfChildrenBlock(ThenBBlock);

    if (!ThenChildren)
      return nullptr;
  }

  if (ElseBBlock) {
    ElseChildren = findIfChildrenBlock(ElseBBlock);

    if (!ElseChildren)
      return nullptr;

    // Is Short Circuit?
    AVRLabelIR *ElseLabel = cast<AVRLabelIR>(ElseChildren->getBegin());
    ShortCircuitParent = identifyShortCircuitParent(ElseLabel);

    if (ShortCircuitParent) {

      AVRLabelIR *TargetLabel =
          cast<AVRLabelIR>(ShortCircuitParent->getElseBegin());
      ShortCircuitBr = AVRUtils::createAVRBranch(TargetLabel);
    }
  }

  // Current support only allows ThenChildren Terminator and ElseChildren
  // Terminator to branch to common label
  if (!isSupportedAvrIfChildren(ThenChildren, ElseChildren))
    return nullptr;

  return new CandidateIf(ABranch, ThenChildren, ElseChildren,
                         ShortCircuitParent, ShortCircuitBr);
}

void AVRBranchOptVisitor::visit(AVRBranchIR *ABranchIR) {

  // TODO: Convert optimzation to fully IR-independent opt.
  CandidateIf *CandidateIf = generateAvrIfCandidate(ABranchIR);

  if (CandidateIf) {
    CandidateIfs.push_back(CandidateIf);
  }
}

/// \brief This visitor walks the Abstract Layer and inserts AVRExpression and
/// AVRValue nodes into the AL. Each AVR Assignmment node is deconstructed into
/// LHS and RHS AVR Expression Nodes. Each Expression consists of AVR Value
/// nodes which represent the operands and of the expression.
class AddAvrExprVisitor {

private:
  /// AL - Abstract Layer to analze.
  AVRGenerateBase *AbstractLayer;

public:
  AddAvrExprVisitor(AVRGenerateBase *AL) : AbstractLayer(AL) {}

  /// Visit Functions
  void visit(AVR *ANode) {}
  void visit(AVRAssignIR *AssignIR);
  void visit(AVRAssignHIR *AssignHIR);
  void postVisit(AVR *ANode) {}
  bool isDone() { return false; }
  bool skipRecursion(AVR *ANode) { return false; }
};

// To Do: Improve AL visitor to virtual visit functions.
void AddAvrExprVisitor::visit(AVRAssignIR *AssignIR) {

  AVRExpression *LhsExpr =
      AVRUtilsIR::createAVRExpressionIR(AssignIR, LeftHand);
  AVRUtils::setAVRAssignLHS(AssignIR, LhsExpr);
  AVRExpression *RhsExpr =
      AVRUtilsIR::createAVRExpressionIR(AssignIR, RightHand);
  AVRUtils::setAVRAssignRHS(AssignIR, RhsExpr);
}

void AddAvrExprVisitor::visit(AVRAssignHIR *AssignHIR) {

  AVRExpressionHIR *LhsExpr =
      AVRUtilsHIR::createAVRExpressionHIR(AssignHIR, LeftHand);
  AVRUtils::setAVRAssignLHS(AssignHIR, LhsExpr);
  AVRExpressionHIR *RhsExpr =
      AVRUtilsHIR::createAVRExpressionHIR(AssignHIR, RightHand);
  AVRUtils::setAVRAssignRHS(AssignHIR, RhsExpr);
}

/// \brief This visitor walks the Abstract Layer and removes unnecessary avr
/// nodes. Avr nodes such as no-op nodes which were inserted during AL
/// generation are removed in this visitor.
class RemoveAvrNOPs {

private:
  /// AbstractLayer - Abstract Layer of avr nodes to analyze
  AVRGenerateBase *AbstractLayer;

public:
  RemoveAvrNOPs(AVRGenerateBase *AL) : AbstractLayer(AL) {}
  /// Visit Functions
  void visit(AVR *ANode) {}
  void visit(AVRNOP *ANOP);
  void postVisit(AVR *ANode) {}
  bool isDone() { return false; }
  bool skipRecursion(AVR *ANode) { return false; }
};

void RemoveAvrNOPs::visit(AVRNOP *ANOP) { AVRUtils::remove(ANOP); }

void AVRGenerateBase::optimizeLoopControl() {

  if (!isAbstractLayerEmpty()) {

    DEBUG(dbgs() << "\nInserting Avr Loops.\n");

    // AVRGenerate has created a collection of AVR sequences which represent
    // candidate loops for vectorization. At this point these AVR sequences do
    // not
    // have any control flow AVRs in them.
    //
    // The control flow is not added in the first build of AVR for two reasons:
    //   1. If there is an error in control flow analysis, we still want a base
    //      set of AVRS to fall back on for vectorization.
    //
    //   2. The algorithm for detecting loop control flow and insert nodes is
    //      simplier when done as a post processing on exisiting AL.
    //
    // This walk will iterate through each AVR sequence (which represents a
    // candidate loop nest) and insert AVRLoop nodes, and move the AVR nodes
    // which represent the body of the loop into AVRLoop's children, where
    // necessary.

    // TODO: Change iteration to visitor. In case of nested
    // WRN Nodes this will not properly recursively build loops
    // and link to WRN
    for (auto I = begin(), E = end(); I != E; ++I) {
      formAvrLoopNest(&*I);
    }
  }
}

void AVRGenerateBase::addLabelReferences() {

  AVRAddLabelReferences AALR(this);
  AVRVisitor<AVRAddLabelReferences> AVisitor(AALR);
  AVisitor.forwardVisitAll(this);
}

void AVRGenerateBase::formAvrLoopNest(AVR *AvrNode) {

  if (AVRWrn *AvrWrn = dyn_cast<AVRWrn>(AvrNode)) {
    formAvrLoopNest(AvrWrn);
  } else if (AVRFunction *AvrFunction = dyn_cast<AVRFunction>(AvrNode)) {
    formAvrLoopNest(AvrFunction);
  } else {
    llvm_unreachable("Unexpected Avr node for Loop formation!");
  }
}

//
// AVRIf nodes are formed in two steps.
// (1) Identification/ Setup Pass (AL visit traversal)
//     Before AVRCompare nodes  can be replaced with AVRIf nodes
//     we must determine if:
//       A. AVRCompare is a candidate if. It is not part of a special
//          compare/select sequence or IV loop check.
//       B. AVRCompare is in a short circuit compare chain. Short
//          circuits are nested ifs which share a common if block.
//          Example:
//          if (A && B) {
//            S1
//          }
//          else {
//            S2
//          }
//
//          We would need to generate an avr equivalent of:
//          (TODO: We can generate a more effiecnt sequence)
//          if (A) {
//             if (B) {
//               S1
//             }
//             else {
//               goto L1;
//             }
//          }
//          else {
//      L1:   S2
//          }
//
//          Each candidate if is recorded and SC-chains are marked
//          inside CandidateIF object.
//
// (2) AVRCompare replacement with AVRIf transformation.
//

void AVRGenerateBase::optimizeAvrBranches() {

  // Step 1: Identify Candidates using AL visitor
  AVRBranchOptVisitor AC(this);
  AVRVisitor<AVRBranchOptVisitor> AvrBranchOpt(AC);
  AvrBranchOpt.forwardVisitAll(this);

  if (!AC.isEmpty()) {

    DEBUG(dbgs() << "\nIdentified " << AC.getNumberOfCandidates()
                 << " candidates for AvrIf optimization\n");

    // Optimize AVRCompare: Replace AVRBranches with AVRIf and set
    // children as appropiate. Traverse bottom up.

    // Step 2: Perform Replacement.
    for (auto I = AC.rbegin(), E = AC.rend(); I != E; ++I) {

      AVRBranch *AvrBranch = (*I)->getAvrBranch();
      AVRIfIR *AvrIfIR = AVRUtilsIR::createAVRIfIR(AvrBranch);
      AVRUtils::insertBefore(AvrItr(AvrBranch), AvrIfIR);

      // Then-Children
      if ((*I)->hasThenBlock()) {

        AVR *ThenBegin = (*I)->getThenBegin();
        AVR *ThenEnd = (*I)->getThenEnd();

        assert(ThenBegin && ThenEnd && "Malformed AvrIf then-children!");
        AVRUtils::moveAsFirstThenChildren(AvrIfIR, AvrItr(ThenBegin),
                                          AvrItr(ThenEnd));
      }

      // Else-Children
      if ((*I)->hasElseBlock()) {

        if (!(*I)->hasShortCircuit()) {

          AVR *ElseBegin = (*I)->getElseBegin();
          AVR *ElseEnd = (*I)->getElseEnd();

          assert(ElseEnd && ElseEnd && "Malformed AvrIf else-children!");
          AVRUtils::moveAsFirstElseChildren(AvrIfIR, AvrItr(ElseBegin),
                                            AvrItr(ElseEnd));
        } else {

          AVRBranch *SCSuccessor = (*I)->getShortCircuitSuccessor();

          assert(SCSuccessor && "AvrIf missing short-circuit successor!");
          AVRUtils::insertFirstElseChild(AvrIfIR, SCSuccessor);
        }
      }
    }

    // Step 3: Remove conditional branches
    for (auto I = AC.rbegin(), E = AC.rend(); I != E; ++I) {
      cleanupBranchOpt(*I);
    }
  } else {
    DEBUG(dbgs() << "No AVRCompares identified for AvrIf transformation!\n");
  }
}

void AVRGenerateBase::cleanupBranchOpt(CandidateIf *CandIf) {

  AVRBranch *Branch = CandIf->getAvrBranch();

  const ALChange *OptRemoval;

  // TODO: Move the change log modifications to the AVR utilites
  // and make transparent to user.

  // OptRemoval = new ALChange(Condition, ALBranchOpt, Removal);
  // ALChangeLog.push_back(OptRemoval);

  // Remvove the condition from AL
  // AVRUtils::remove(Condition);

  OptRemoval = new ALChange(Branch, ALBranchOpt, Removal);
  ALChangeLog.push_back(OptRemoval);

  // Remove the conditional branch from AL
  AVRUtils::remove(Branch);
}

void AVRGenerateBase::optimizeAvrExpressions() {

  // Add Avr Expressions and Avr Value nodes to the AL.
  AddAvrExprVisitor ExprVisitor(this);
  AVRVisitor<AddAvrExprVisitor> AddExprOpt(ExprVisitor);
  AddExprOpt.forwardVisitAll(this);
}

void AVRGenerateBase::optimizeAvrSubExpressions() {

  // TODO: Finish Subexpressions
}

void AVRGenerateBase::removeAvrNOPs() {
  RemoveAvrNOPs NOPRemover(this);
  AVRVisitor<RemoveAvrNOPs> RemoveVisitor(NOPRemover);
  RemoveVisitor.forwardVisitAll(this);
}

void AVRGenerateBase::print(raw_ostream &OS, unsigned Depth,
                            VerbosityLevel VLevel) const {

  formatted_raw_ostream FOS(OS);

  if (AbstractLayer.empty()) {
    FOS << "No AVRs Generated!\n";
    return;
  }

  for (auto I = begin(), E = end(); I != E; ++I) {
    I->print(FOS, Depth, VLevel);
  }
}

void AVRGenerateBase::print(raw_ostream &OS, const Module *M) const {
  this->print(OS, 1, PrintBase);
}

void AVRGenerateBase::dump(VerbosityLevel VLevel) const {
  formatted_raw_ostream OS(dbgs());
  this->print(OS, 1, VLevel);
}

bool AVRGenerateBase::codeGen() {

  if (!AbstractLayer.empty()) {
    AVR *ANode = &AbstractLayer.back();
    ANode->codeGen();
    return true;
  }

  return false;
}

void AVRGenerateBase::releaseMemory() {
  AbstractLayer.clear();
  ALChangeLog.clear();

  // TODO: Free up all generated AVRs.
}

AVRGenerate::AVRGenerate() : AVRGenerateBase(ID) {
  llvm::initializeAVRGeneratePass(*PassRegistry::getPassRegistry());

  setLoopInfo(nullptr);

  // Set Stress Testing Level
  setStressTest(AvrStressTest);
}

void AVRGenerate::getAnalysisUsage(AnalysisUsage &AU) const {
  AVRGenerateBase::getAnalysisUsage(AU);
  AU.addRequired<WRegionInfo>();
}

bool AVRGenerate::runOnFunction(Function &F) {

  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  PDT = &getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree();
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  WR = &getAnalysis<WRegionInfo>();

  return AVRGenerateBase::runOnFunction(F);
}

void AVRGenerate::buildAbstractLayer() {
  if (ScalarStressTest) {

    DEBUG(dbgs() << "\nAVR: Generating AVRs for whole function.\n");

    // Build complete AVR node representation for function in stress testing
    // mode
    buildAvrsForFunction();
  } else {

    DEBUG(dbgs() << "\nAVR: Generating AVRs for vector candidates.\n");

    // Build the WRGraph based on incoming LLVM IR
    WR->buildWRGraph(WRegionCollection::LLVMIR);

    // Build AVR node representation for incoming vector candidates
    buildAvrsForVectorCandidates();
  }
}

bool AVRGenerate::postDominatesAllCases(SwitchInst *SI, BasicBlock *BB) const {

  if (!PDT->dominates(BB, SI->getDefaultDest()))
    return false;

  for (auto I = SI->case_begin(), E = SI->case_end(); I != E; ++I) {
    if (!PDT->dominates(BB, I->getCaseSuccessor()))
      return false;
  }

  return true;
}

void AVRGenerate::sortDomChildren(
    DomTreeNode *Node, SmallVectorImpl<BasicBlock *> &SortedChildren) const {

  // Sort the dom children of Node using post dominator relationship. If child1
  // post dominates child2, it should be visited after child2 otherwise forward
  // edges can turn into back edges.

  // TODO: look into dom child ordering for multi-exit loops.

  for (auto &I : (*Node)) {
    SortedChildren.push_back(I->getBlock());
  }

  // This check orders children that post-dominate other children, before them.
  // This is because I couldn't think of an appropriate check for sorting in the
  // reverse order. So instead the children are visited in reverse order after
  // sorting.
  auto PostDomOrder = [this](BasicBlock *B1, BasicBlock *B2) {
    // First check satisfies the strict weak ordering requirements of
    // comparator function.
    return ((B1 != B2) && PDT->dominates(B1, B2));
  };

  std::sort(SortedChildren.begin(), SortedChildren.end(), PostDomOrder);
}

AvrItr AVRGenerate::preorderTravAvrBuild(BasicBlock *BB, AvrItr InsertionPos) {

  assert(BB && &*InsertionPos && "Avr preorder traversal failed!");

  SmallVector<BasicBlock *, 8> DomChildren;
  auto *DomNode = DT->getNode(BB);

  // Build AVR node sequence for current basic block
  InsertionPos = generateAvrInstSeqForBB(BB, InsertionPos);
  AvrItr LastAvr(InsertionPos);

  // Sort the dominator children
  sortDomChildren(DomNode, DomChildren);

  // Traverse dominator children in reverse order. Post dominating children
  // preceed the children they dominate.
  for (auto RI = DomChildren.rbegin(), RE = DomChildren.rend(); RI != RE;
       ++RI) {

    auto DomChildBB = (*RI);

    if (AVRIfIR *AvrIfIR = dyn_cast<AVRIfIR>(LastAvr)) {
      AVRBranch *Branch = AvrIfIR->getAvrBranch();

      if (Branch->isConditional()) {

        // Traverse the basic blocks in program if-then-else order.
        BranchInst *BI = cast<BranchInst>(BB->getTerminator());

        if ((DomChildBB == BI->getSuccessor(0)) &&
            // If one of the 'if' successors post-dominates the other, it is
            // better to link it after the 'if' instead of linking it as a
            // child.
            !PDT->dominates(DomChildBB, BI->getSuccessor(1))) {

          preorderTravAvrBuild(DomChildBB, AvrItr(AvrIfIR->getLastThenChild()));
          continue;

        } else if (DomChildBB == BI->getSuccessor(1) &&
                   !PDT->dominates(DomChildBB, BI->getSuccessor(0))) {

          preorderTravAvrBuild(DomChildBB, AvrItr(AvrIfIR->getLastElseChild()));
          continue;
        }
      }
    } else if (AVRSwitchIR *ASwitchIR = dyn_cast<AVRSwitchIR>(LastAvr)) {

      // Link switch's case children.
      auto SI = cast<SwitchInst>(BB->getTerminator());

      if (!postDominatesAllCases(SI, DomChildBB)) {

        if (DomChildBB == SI->getDefaultDest()) {
          preorderTravAvrBuild(DomChildBB,
                               AvrItr(ASwitchIR->getDefaultCaseLastChild()));
          continue;
        }

        unsigned Count = 1;
        bool IsCaseChild = false;
        for (auto Itr = SI->case_begin(), End = SI->case_end(); Itr != End;
             ++Itr, ++Count) {
          if (DomChildBB == Itr->getCaseSuccessor()) {
            preorderTravAvrBuild(DomChildBB,
                                 AvrItr(ASwitchIR->getCaseLastChild(Count)));
            IsCaseChild = true;
            break;
          }
        }

        if (IsCaseChild)
          continue;
      }
    }

    // Link remaining dominator children.
    InsertionPos = preorderTravAvrBuild(DomChildBB, InsertionPos);
  }

  return InsertionPos;
}

#if 0
/// \brief This visitor class traverses the WRN graph and inserts
/// AVRWRN nodes into the abstract layer for nested WRN nodes. 
/// For vectorization, this WRN graph insertion will happen for:
///
/// #pragma simd
/// for () {             WRN #1
///   ...                 |
///   #pragma simd        |
///   for() {             | -> WRN #2
///     ...
///   }
/// }
///
/// This nested WRN graph is produced when nested simd
/// is specified or similarily for nested auto vectorization 
/// pragmas emitted by par vec analysis.
/// (This is currently disabled until support in front-end
/// or parvec is enabled for nested pragmas)
///
class AvrWrnInsertVisitor {
public:
  /// AL - Abstract Layer to insert WRN nodes into.
  AVRGenerateBase *AL; 

  /// Visitor constructor
  AvrWrnInsertVisitor(AVRGenerateBase *Layer) :AL(Layer) {}

  bool quitVisit(WRegionNode* W) { return false; }
  void preVisit(WRegionNode* W)  {} 
  void postVisit(WRegionNode* W) { 
    if (WRNVecLoopNode *WRN = dyn_cast<WRNVecLoopNode>(W)) {

      // 1. Check if WRN is a child node of a top-level parent WRN.
      // 2. Create a new AVRWrn node and insert into abstract layer.
      // 3. Move the respective AVR nodes of this WRN loop nest 
      //    into the children container of newly created WRN avr. 
    }
  }

};
#endif

void AVRGenerate::buildAvrsForVectorCandidates() {

  // Traverse the top level WRN nodes and recursively build avrs for
  // each of the loop nests these top level nodes represent. If there
  // exist WRN nodes which have children WRN nodes, these will be
  // added to the Abstract Layer after the preorder build is completed.
  for (auto Itr = WR->begin(), End = WR->end(); Itr != End; ++Itr) {

    if (WRNVecLoopNode *WRNVecNode = dyn_cast<WRNVecLoopNode>(*Itr)) {

      AvrWrn = AVRUtils::createAVRWrn(WRNVecNode);
      preorderTravAvrBuild(WRNVecNode->getEntryBBlock(), AvrItr(AvrWrn));
      AbstractLayer.push_back(AvrWrn);
    }
  }

// Traverse the WRN graph, and update the Abstract Layer with any
// children (nested) WRN nodes which are present.
// (Need front-end support for nested simd and parvec support
// for nested autovect pragma)
#if 0
  AvrWrnInsertVisitor WrnChildInsertion(this);
  WRegionUtils::forwardVisit(WrnChildInsertion, WR->getWRGraph());
#endif
}

AvrItr AVRGenerate::generateAvrInstSeqForBB(BasicBlock *BB,
                                            AvrItr InsertionPos) {
  AVRLabelIR *ALabel = AVRUtilsIR::createAVRLabelIR(BB);
  AVR *NewNode = nullptr;

  // Add Avr label to map for downstream AL optimizations
  AvrLabels[BB] = ALabel;

  // First BB of loop, function, split is inserted as first child
  if (isa<AVRLoop>(InsertionPos) || isa<AVRFunction>(InsertionPos) ||
      isa<AVRWrn>(InsertionPos)) {
    AVRUtils::insertFirstChild(&*InsertionPos, ALabel);
  } else {
    AVRUtils::insertAfter(InsertionPos, ALabel);
  }

  InsertionPos = AvrItr(ALabel);

  for (auto I = BB->begin(), E = std::prev(BB->end()); I != E; ++I) {

    switch (I->getOpcode()) {
    case Instruction::Call:
      NewNode = AVRUtilsIR::createAVRCallIR(&*I);
      break;
    case Instruction::PHI:
      NewNode = AVRUtilsIR::createAVRPhiIR(&*I);
      break;
    case Instruction::Br:
      assert(0 && "Encountered a branch before block terminator!");
      NewNode = AVRUtilsIR::createAVRBranchIR(&*I);
      break;
    case Instruction::Ret:
      assert(0 && "Encountered a return before block terminator!");
      NewNode = AVRUtilsIR::createAVRReturnIR(&*I);
      break;
    case Instruction::ICmp:
    case Instruction::FCmp:
      NewNode = AVRUtilsIR::createAVRCompareIR(&*I);
      break;
    case Instruction::Select: {
      auto *SI = cast<SelectInst>(I);
      auto *CI = cast<Instruction>(SI->getCondition());
      assert(CI && "Could not resolve condition inst for select inst!");

      AVR *AvrCondition = AvrInsts[CI];
      assert(AvrCondition && "Could not resolve avr condition for select!");

      NewNode = AVRUtilsIR::createAVRSelectIR(&*I, AvrCondition);
      break;
    }
    default:
      NewNode = AVRUtilsIR::createAVRAssignIR(&*I);
    }

    DEBUG(dbgs() << "VECREPORT: Generated New AVR = ");
    DEBUG(NewNode->dump());

    // Add newly created avr and it's corresponding instruction to map.
    AvrInsts[&*I] = NewNode;

    // Insert the newly created avr into the abstract layer.
    AVRUtils::insertAfter(InsertionPos, NewNode);
    InsertionPos = AvrItr(NewNode);
  }

  // Generate terminator and set pointer in avr label node.
  InsertionPos = AvrItr(generateAvrTerminator(BB, &*InsertionPos));
  ALabel->setTerminator(&*InsertionPos);

  return InsertionPos;
}

AVR *AVRGenerate::generateAvrTerminator(BasicBlock *BB, AVR *InsertionPos) {

  auto Terminator = BB->getTerminator();

  if (BranchInst *BI = dyn_cast<BranchInst>(Terminator)) {

    AVR *AvrCondition = nullptr;

    if (BI->isConditional()) {

      if (auto *CI = dyn_cast<Instruction>(BI->getCondition())) {

        AvrCondition = AvrInsts[CI];
        assert(AvrCondition && "Avr condition for branch could not be "
                               "resolved!");
      }
    }

    // Create a branch terminator
    AVRBranchIR *ABranch =
        AVRUtilsIR::createAVRBranchIR(Terminator, AvrCondition);

    // Insert newly created branch into the abstract layer.
    AVRUtils::insertAfter(AvrItr(InsertionPos), ABranch);
    InsertionPos = ABranch;

    // Construct the AVRIf in place. Do not generate AVRIfs for
    // loop latches. The loop latch conditional branch will instead be
    // represented by the Loop IV, UB, LB and trip count.
    // The conditional branch this AVRIf represents will be
    // cleaned up downstream. For now we leave it in the abstract layer.

    Loop *Lp = LI->getLoopFor(BB);
    auto LpLatchBB = Lp ? Lp->getLoopLatch() : nullptr;

    if (BI->isConditional() && BB != LpLatchBB) {

      // Create an AVRIf node for terminator and insert it into the
      // abstract layer.
      AVRIfIR *AvrIf = AVRUtilsIR::createAVRIfIR(ABranch);
      AVRUtils::insertAfter(AvrItr(InsertionPos), AvrIf);
      InsertionPos = AvrIf;

      // FIXME: Until this is actually cleaned up downstream
      AVRUtils::remove(ABranch);
    }

  } else if (SwitchInst *SI = dyn_cast<SwitchInst>(Terminator)) {

    // Create an avr switch node for terminator and insert it into the
    // abstract layer.
    AVRSwitchIR *NewSwitch = AVRUtilsIR::createAVRSwitchIR(SI);

    AVRUtils::insertAfter(AvrItr(InsertionPos), NewSwitch);
    InsertionPos = NewSwitch;

  } else if (ReturnInst *RI = dyn_cast<ReturnInst>(Terminator)) {

    // Create a return node terminator and insert it into the abstract
    // layer
    AVRReturnIR *AReturn = AVRUtilsIR::createAVRReturnIR(RI);
    AVRUtils::insertAfter(AvrItr(InsertionPos), AReturn);
    InsertionPos = AReturn;

  } else if (UnreachableInst *UI = dyn_cast<UnreachableInst>(Terminator)) {

    // Create an unreachable node terminator and insert into the abstract
    // layer.
    AVRUnreachableIR *AUnreach = AVRUtilsIR::createAVRUnreachableIR(UI);
    AVRUtils::insertAfter(AvrItr(InsertionPos), AUnreach);
    InsertionPos = AUnreach;

  } else if (InvokeInst *II = dyn_cast<InvokeInst>(Terminator)) {

    // TODO: Add avr type for invoke.
    // Create an invoke node terminator and insert into the abstract
    // layer.
    AVRAssignIR *AAssign = AVRUtilsIR::createAVRAssignIR(II);
    AVRUtils::insertAfter(AvrItr(InsertionPos), AAssign);
    InsertionPos = AAssign;

  } else if (ResumeInst *RI = dyn_cast<ResumeInst>(Terminator)) {

    // TODO: Add avr type for resume.
    // Create a resume node terminator and insert into the abstract
    // layer.
    AVRAssignIR *AAssign = AVRUtilsIR::createAVRAssignIR(RI);
    AVRUtils::insertAfter(AvrItr(InsertionPos), AAssign);
    InsertionPos = AAssign;

  } else if (CatchSwitchInst *CSI = dyn_cast<CatchSwitchInst>(Terminator)) {

    // TODO: Add avr type for catchswitch inst.
    // Create a catchswitch node terminator and insert into the abstract
    // layer.
    AVRAssignIR *AAssign = AVRUtilsIR::createAVRAssignIR(CSI);
    AVRUtils::insertAfter(AvrItr(InsertionPos), AAssign);
    InsertionPos = AAssign;

  } else if (CatchReturnInst *CRI = dyn_cast<CatchReturnInst>(Terminator)) {

    // TODO: Add avr type for catchret.
    // Create a catchret node terminator and insert into the abstract
    // layer.
    AVRAssignIR *AAssign = AVRUtilsIR::createAVRAssignIR(CRI);
    AVRUtils::insertAfter(AvrItr(InsertionPos), AAssign);
    InsertionPos = AAssign;

  } else if (CleanupReturnInst *CRI = dyn_cast<CleanupReturnInst>(Terminator)) {

    // TODO: Add avr type for cleanupret.
    // Create a cleanup node terminator and insert into the abstract
    // layer.
    AVRAssignIR *AAssign = AVRUtilsIR::createAVRAssignIR(CRI);
    AVRUtils::insertAfter(AvrItr(InsertionPos), AAssign);
    InsertionPos = AAssign;

  } else {

    DEBUG(Terminator->dump());
    llvm_unreachable("Unknown terminator type!");
  }

  DEBUG(dbgs() << "VECREPORT: Generated New AVR = ");
  DEBUG(InsertionPos->dump());

  return InsertionPos;
}

// For explicit vectorization of loops and functions, the vectorizer
// should not generate AVRFunction nodes. Building AVR for function
// is for stress testing only.
void AVRGenerate::buildAvrsForFunction() {
  AvrFunction = AVRUtils::createAVRFunction(Func, LI);

  preorderTravAvrBuild(AvrFunction->getEntryBBlock(), AvrItr(AvrFunction));

  // Add generated AVRs to Abstract Layer.
  AbstractLayer.push_back(AvrFunction);
}

bool AVRGenerate::isLoopALSupported(const Loop &Lp) {

  // Abstract Layer currently doesn't support building AVRLoop nodes for LLVM
  // Loops not in "normal" form. Loop must have a preheader, a single backedge,
  // and all of its exits have all of their predecessors inside the loop.
  if (!Lp.isLoopSimplifyForm()) {
    DEBUG(dbgs() << "VECREPORT: Loop structure not supported.\n");
    return false;
  }

  // Multi-exit loop not yet supported.
  if (!Lp.getExitingBlock()) {
    DEBUG(dbgs() << "VECREPORT: Multi-exit loops not currently supported.\n");
    return false;
  }

  return true;
}

void AVRGenerate::formAvrLoopNest(AVRFunction *AvrFunction) {

  Function *Func = AvrFunction->getOrigFunction();
  const LoopInfo *LI = AvrFunction->getLoopInfo();

  for (auto I = Func->begin(), E = Func->end(); I != E; ++I) {

    if (!LI->isLoopHeader(&*I))
      continue;

    Loop *Lp = LI->getLoopFor(&*I);
    assert(Lp && "Loop not found for Loop Header BB!");

    if (!isLoopALSupported(*Lp))
      continue;

    BasicBlock *LoopLatchBB = Lp->getLoopLatch();
    assert(LoopLatchBB && "Loop Latch BB not found!");

    // Dominator and Post Dominator info is needed for avr loop
    // formation. Absent info is usually due to infinite or
    // unreachable loops
    if (!DT->getNode(LoopLatchBB) || !PDT->getNode(LoopLatchBB)) {
      DEBUG(dbgs() << "VECREPORT: Unreachable or infinite loops are not "
                      "supported.\n");
      continue;
    }

    AVR *AvrLbl = AvrLabels[&*I];
    AVRLabel *AvrTermLabel = AvrLabels[LoopLatchBB];
    AVR *AvrTerm = AvrTermLabel->getTerminator();

    if (AVRBranch *ABranch = dyn_cast<AVRBranch>(AvrTerm)) {

      // We only handle bottom test loops.
      if (!ABranch->isConditional()) {
        DEBUG(dbgs() << "VECREPORT: Unconditional loop branch instructions not "
                        "currently supported.\n");
        continue;
      }
    }

    if (AvrLbl && AvrTerm) {

      // Mark the bottom test (Exclude it from AvrBranch Opt)
      markLoopBottomTest(AvrTermLabel);

      // Create AvrLoop
      AVRLoopIR *AvrLoopIR = AVRUtilsIR::createAVRLoopIR(Lp);

      // Hook AVR Loop into AVR Sequence
      AVRUtils::insertBefore(AvrItr(AvrLbl), AvrLoopIR);
      AVRUtils::moveAsFirstChildren(AvrLoopIR, AvrItr(AvrLbl), AvrItr(AvrTerm));
    }
  }
}

// AVR If insertion walks all of the conditional branches and
// attempts to generate AVRIF for them.  We need to exclude the
// conditional branch which is in the loop latch otherwise we
// incorrectly generate an AVRIF.
void AVRGenerate::markLoopBottomTest(AVRLabel *LoopLatchLabel) {

  AvrItr BottomTest(LoopLatchLabel);

  while (&*BottomTest) {

    if (AVRBranch *BT = dyn_cast<AVRBranch>(BottomTest)) {
      BT->setBottomTest(true);
      return;
    }
    BottomTest = std::next(BottomTest);
  }
}

void AVRGenerate::formAvrLoopNest(AVRWrn *AvrWrn) {

  const LoopInfo *LI = AvrWrn->getLoopInfo();
  AvrWrn->populateWrnBBSet();

  for (auto I = AvrWrn->wrnbbset_begin(), E = AvrWrn->wrnbbset_end(); I != E;
       ++I) {

    // TODO: FIX THIS ASAP - Should not be using const_casts.
    // The BBSet build in WRN is returning const BBlocks, but the interfaces
    // for loop info cannot handle these.
    BasicBlock *LoopHeaderBB = const_cast<BasicBlock *>(*I);

    if (!LI->isLoopHeader(LoopHeaderBB))
      continue;

    Loop *Lp = LI->getLoopFor(LoopHeaderBB);
    assert(Lp && "Loop not found for Loop Header BB!");

    if (!isLoopALSupported(*Lp))
      continue;

    BasicBlock *LoopLatchBB = Lp->getLoopLatch();
    assert(LoopLatchBB && "Loop Latch BB not found!");

    AVR *AvrLbl = AvrLabels[LoopHeaderBB];
    AVRLabel *AvrTermLabel = AvrLabels[LoopLatchBB];
    AVR *AvrTerm = AvrTermLabel->getTerminator();

    if (AvrLbl && AvrTerm) {

      // Mark the bottom test (Exclude it from AvrBranch Opt)
      markLoopBottomTest(AvrTermLabel);

      // Create AvrLoop
      AVRLoopIR *AvrLoopIR = AVRUtilsIR::createAVRLoopIR(Lp);

      // TODO: For nested WRN, this needs to only be set for
      // top-level loop of WRN.
      AvrLoopIR->setWrnVecLoopNode(AvrWrn->getWrnNode());

      // Hook AVR Loop into AVR Sequence
      AVRUtils::insertBefore(AvrItr(AvrLbl), AvrLoopIR);
      AVRUtils::moveAsFirstChildren(AvrLoopIR, AvrItr(AvrLbl), AvrItr(AvrTerm));
    }
  }

  cleanupAvrWrnNodes();
}

void AVRGenerate::cleanupAvrWrnNodes() {
  // TODO
}

AVRGenerateHIR::AVRGenerateHIR() : AVRGenerateBase(ID) {
  llvm::initializeAVRGenerateHIRPass(*PassRegistry::getPassRegistry());
}

void AVRGenerateHIR::getAnalysisUsage(AnalysisUsage &AU) const {
  AVRGenerateBase::getAnalysisUsage(AU);
  AU.addRequiredTransitive<HIRFramework>();
  AU.addRequiredTransitive<HIRLocalityAnalysis>();
  AU.addRequiredTransitive<HIRDDAnalysis>();
}

bool AVRGenerateHIR::runOnFunction(Function &F) {
  HIRF = &getAnalysis<HIRFramework>();
  return AVRGenerateBase::runOnFunction(F);
}

void AVRGenerateHIR::buildAbstractLayer() {
  AVRGenerateVisitor AG(AvrLabelsHIR);

  // Walk the HIR and build WRGraph based on HIR
  WRContainerImpl *WRGraph = WRegionUtils::buildWRGraphFromHIR(*HIRF);
  DEBUG(errs() << "WRGraph #nodes= " << WRGraph->size() << "\n");
  for (auto I = WRGraph->begin(), E = WRGraph->end(); I != E; ++I) {
    DEBUG((*I)->dump());
  }

  // TBD: Using WRN nodes directly for now. This needs to be changed
  // to depend on identify vector candidates. We also need to create
  // AVRLoop variants for LLVM/HIR variants and use these going
  // forward.
  for (auto I = WRGraph->begin(), E = WRGraph->end(); I != E; ++I) {
    DEBUG(errs() << "Starting AVR gen for \n");
    DEBUG((*I)->dump());
    AVRWrn *AWrn;
    AVR *Avr;
    WRNVecLoopNode *WVecNode;

    if (!(WVecNode = dyn_cast<WRNVecLoopNode>(*I)))
      continue;

    // Create an AVRWrn and insert AVR for contained loop as child
    AWrn = AVRUtils::createAVRWrn(WVecNode);
    Avr = AG.visit(WVecNode->getHLLoop());
    AVRUtils::insertFirstChild(AWrn, Avr);

    AbstractLayer.push_back(AWrn);
  }

  // We have generated AL from HIR, do not invoke LLVM IR AL opts
  if (!AbstractLayer.empty()) {
    DisableLoopOpt = true;
    DisableAvrBranchOpt = true;
  }
}

// AVRGenerateVisitor - Generates HIR-based AL
AVR *AVRGenerateHIR::AVRGenerateVisitor::visitInst(HLInst *I) {
  return AVRUtilsHIR::createAVRAssignHIR(I);
}

AVR *AVRGenerateHIR::AVRGenerateVisitor::visitLabel(HLLabel *L) {
  AVRLabelHIR *ALabel = AVRUtilsHIR::createAVRLabelHIR(L);
  AvrLabels[L] = ALabel;
  return ALabel;
}

AVR *AVRGenerateHIR::AVRGenerateVisitor::visitGoto(HLGoto *G) {
  return AVRUtilsHIR::createAVRBranchHIR(G);
}

AVR *AVRGenerateHIR::AVRGenerateVisitor::visitLoop(HLLoop *L) {
  AVRLoopHIR *ALoop;
  AVR *ChildAVR;

  DEBUG(formatted_raw_ostream FOS(dbgs()); FOS << "VISITING HLLOOP:\n";
        L->print(FOS, 0, true);
        FOS << "\n+++++++++++++++++++++++++++++++++++++++++++++++\n");

  ALoop = AVRUtilsHIR::createAVRLoopHIR(L);

  // Visit loop preheader
  for (auto It = L->pre_begin(), End = L->pre_end(); It != End; ++It) {
    ChildAVR = visit(*It);
    AVRUtils::insertLastPreheaderChild(ALoop, ChildAVR);
  }

  // Visit loop children
  for (auto It = L->child_begin(), End = L->child_end(); It != End; ++It) {
    DEBUG(formatted_raw_ostream FOS(dbgs()); FOS << "LOOP CHILD:\n";
          It->print(FOS, 0, true);
          FOS << "\n-----------------------------------------------\n");
    ChildAVR = visit(*It);

    AVRUtils::insertLastChild(ALoop, ChildAVR);
  }

  // Visit loop postexit
  for (auto It = L->post_begin(), End = L->post_end(); It != End; ++It) {
    ChildAVR = visit(*It);
    AVRUtils::insertLastPostexitChild(ALoop, ChildAVR);
  }

// TODO: Set zero trip test.

#if 0
  formatted_raw_ostream OS(dbgs());
  ALoop->print(OS, 1, 1);
#endif

  return ALoop;
}

AVR *AVRGenerateHIR::AVRGenerateVisitor::visitRegion(HLRegion *R) {
  AVRWrn *AWrn;

  // TODO - for now use AVRWrn to represent a region. AVR generation
  // for HIR will change once we figure out how SIMD/AUTOVEC intrinsics
  // are represented and what we consider as potential vectorization
  // candidates.
  AWrn = AVRUtils::createAVRWrn(nullptr);

  // Visit region children
  for (auto It = R->child_begin(), E = R->child_end(); It != E; ++It) {
    AVR *ChildAVR;
    ChildAVR = visit(*It);
    AVRUtils::insertLastChild(AWrn, ChildAVR);
  }

  return (AVR *)AWrn;
}

AVR *AVRGenerateHIR::AVRGenerateVisitor::visitIf(HLIf *HIf) {
  AVRIf *AIf;

  AIf = AVRUtilsHIR::createAVRIfHIR(HIf);

  // Visit then children
  for (auto It = HIf->then_begin(), E = HIf->then_end(); It != E; ++It) {
    AVR *ChildAVR;
    ChildAVR = visit(*It);
    AVRUtils::insertLastThenChild(AIf, ChildAVR);
  }

  // Visit else children
  for (auto It = HIf->else_begin(), E = HIf->else_end(); It != E; ++It) {
    AVR *ChildAVR;
    ChildAVR = visit(*It);
    AVRUtils::insertLastElseChild(AIf, ChildAVR);
  }

  return AIf;
}

AVR *AVRGenerateHIR::AVRGenerateVisitor::visitSwitch(HLSwitch *S) {

  AVRSwitch *ASwitch = AVRUtilsHIR::createAVRSwitchHIR(S);

  // Visit default case children
  for (auto It = S->default_case_child_begin(), E = S->default_case_child_end();
       It != E; ++It) {

    AVR *ChildAVR = visit(*It);

    if (It == S->default_case_child_begin())
      AVRUtils::insertFirstDefaultChild(ASwitch, ChildAVR);
    else
      AVRUtils::insertLastDefaultChild(ASwitch, ChildAVR);
  }

  // Visit case children
  for (unsigned Case = 1, NumCases = S->getNumCases(); Case <= NumCases;
       ++Case) {

    AVRUtils::addCase(ASwitch);
    for (auto It = S->case_child_begin(Case), E = S->case_child_end(Case);
         It != E; ++It) {

      AVR *ChildAVR = visit(*It);

      if (It == S->case_child_begin(Case))
        AVRUtils::insertFirstChild(ASwitch, ChildAVR, Case);
      else
        AVRUtils::insertLastChild(ASwitch, ChildAVR, Case);
    }
  }

  return ASwitch;
}
