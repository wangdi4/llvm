/*****************************************************************************\

Copyright (c) Intel Corporation (2013).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  Prefetch.cpp

\*****************************************************************************/

#define DEBUG_TYPE "prefetch"

#include "llvm/ADT/Statistic.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/BranchProbability.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/Debug.h"
#include "llvm/Instructions.h"
#include "llvm/Type.h"
#include "OCLAddressSpace.h"
#include "mic_dev_limits.h"

#include "Prefetch.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include <sstream>
#include <string>
#include <climits>
#include <map>

// Un-comment the following line if you want to get auto-prefetch statistics and decision information.
// Will run in debug and release mode. Should be disabled while the code is committed.
//#define TUNE_PREFETCH

#ifdef TUNE_PREFETCH
    #define TUNEPF(X)   do { X; } while (0)
#else // TUNE_PREFETCH
    #define TUNEPF(X)
#endif // TUNE_PREFETCH

#ifdef TUNE_PREFETCH
#include <stdio.h>
#endif // TUNE_PREFETCH

namespace intel{

// KNC Micro-architecture specific information
const int UarchInfo::L2MissLatency = 512;
const int UarchInfo::L1MissLatency = 32;
const int UarchInfo::L1PrefetchSlots = 8;
const int UarchInfo::L2PrefetchSlots = 32;
const int UarchInfo::MaxThreads = 4;
const int UarchInfo::CacheLineSize = 64;
const int UarchInfo::defaultL1PFType = 1;
const int UarchInfo::defaultL2PFType = 2;

const std::string Prefetch::m_prefetchIntrinsicName = "llvm.x86.mic.prefetch";

const int Prefetch::defaultTripCount = 16;
} // namespace intel

#ifdef TUNE_PREFETCH
STATISTIC(PFStat_SerialBB, "Accesses not considered for prefetch since their BB has no vector instructions");
STATISTIC(PFStat_LargeDiff256, "Accesses that were merged although had distance > 256 < 1024");
STATISTIC(PFStat_LargeDiff1024, "Accesses that were not merged since had distance > 1024");
STATISTIC(PFStat_BBNotInLoop, "Basic blocks that are not in a loop");
STATISTIC(PFStat_NonSimpleLoad, "Non simple loads that were ignored for prefetching");
STATISTIC(PFStat_LocalLoad, "Loads from the local space that were ignored for prefetching");
STATISTIC(PFStat_PrivateLoad, "Loads from the private space that were ignored for prefetching");
STATISTIC(PFStat_NonSimpleStore, "Non simple stores that were ignored for prefetching");
STATISTIC(PFStat_LocalStore, "Stores to the local space that were ignored for prefetching");
STATISTIC(PFStat_PrivateStore, "Stores to the private space that were ignored for prefetching");
STATISTIC(PFStat_ManualPFAbortAPF, "Abort memory access search since prefetch intrinsic was found");
STATISTIC(PFStat_GlobalGather, "Gather from Global address space");
STATISTIC(PFStat_LocalGather, "Gather from Local address space");
STATISTIC(PFStat_ConstantGather, "Gather from Constant address space");
STATISTIC(PFStat_PrivateGather, "Gather from Private address space");

STATISTIC(PFStat_GlobalMaskGather, "Mask Gather from Global address space");
STATISTIC(PFStat_LocalMaskGather, "Mask Gather from Local address space");
STATISTIC(PFStat_ConstantMaskGather, "Mask Gather from Constant address space");
STATISTIC(PFStat_PrivateMaskGather, "Mask Gather from Private address space");

STATISTIC(PFStat_GlobalScatter, "Scatter from Global address space");
STATISTIC(PFStat_LocalScatter, "Scatter from Local address space");
STATISTIC(PFStat_ConstantScatter, "Scatter from Constant address space");
STATISTIC(PFStat_PrivateScatter, "Scatter from Private address space");

STATISTIC(PFStat_GlobalMaskScatter, "Mask Scatter from Global address space");
STATISTIC(PFStat_LocalMaskScatter, "Mask Scatter from Local address space");
STATISTIC(PFStat_ConstantMaskScatter, "Mask Scatter from Constant address space");
STATISTIC(PFStat_PrivateMaskScatter, "Mask Scatter from Private address space");

STATISTIC(PFStat_nonSCAVAbleAccess, "Memory access in loop w/o scalar evolution");
STATISTIC(PFStat_accessStepIsLoopVariant, "Memory access pattern is non-stride");
STATISTIC(PFStat_accessStepIsLoopInvariant, "Memory access is constant in inner-most loop, has scalar evolution in an outer loop");
STATISTIC(PFStat_accessMatchInSameBB, "Access match in the same BB");
STATISTIC(PFStat_accessMatchInDOMBB, "Access match in a dominating BB");
STATISTIC(PFStat_matchExact, "Access address exact match");
STATISTIC(PFStat_matchSameLine, "Access address match same cache line");
STATISTIC(PFStat_partLine, "PF triggered by partial cache line access");
STATISTIC(PFStat_oneLine, "PF triggered by full cache line access");
STATISTIC(PFStat_multLines, "PF triggered by multiple line access");
STATISTIC(PFStat_pfMaskedLoads, "PF triggered by masked unaligned loads");
STATISTIC(PFStat_pfMaskedStores, "PF triggered by masked unaligned stores");
STATISTIC(PFStat_pfMaskedRandomGather, "PF triggered by masked random gathers");
STATISTIC(PFStat_pfMaskedRandomScatter, "PF triggered by masked random scatters");
STATISTIC(PFStat_pfRandomGather, "PF triggered by full random gathers");
STATISTIC(PFStat_pfRandomScatter, "PF triggered by full random scatters");
STATISTIC(PFStat_variableStep, "Access step is variable (non-constant or unknown constant))");
#endif // TUNE_PREFETCH

static cl::opt<int>
PFL1Distance("pfl1dist", cl::init(0), cl::Hidden,
  cl::desc("Number of iterations ahead to prefetch to the L1"));

static cl::opt<int>
    PFL2Distance("pfl2dist", cl::init(0), cl::Hidden,
        cl::desc("Number of iterations ahead to prefetch to the L2"));
static cl::opt<int>
    PFL1Type("pfl1type",cl::init(intel::UarchInfo::defaultL1PFType), cl::Hidden,
        cl::desc("Prefetch type (L1). See SDM for details. Negative number "
            "disables these prefetches"));

static cl::opt<int>
    PFL2Type("pfl2type",cl::init(intel::UarchInfo::defaultL2PFType), cl::Hidden,
        cl::desc("Prefetch type (L2). See SDM for details. Negative number "
            "disables these prefetches"));


using namespace Intel::OpenCL::DeviceBackend;

namespace intel{

// PrefetchCandidateUtils - Support memory accesses that are KNC architecture
// specific.
class PrefetchCandidateUtils {

private:
  // inrinsic names of KNC specific accesses
  static const std::string m_intrinsicName;
  static const std::string m_prefetchIntrinsicName;
  static const std::string m_prefetchGatherIntrinsicName;
  static const std::string m_prefetchMaskGatherIntrinsicName;
  static const std::string m_prefetchScatterIntrinsicName;
  static const std::string m_prefetchMaskScatterIntrinsicName;
  static const std::string m_gatherPrefetchIntrinsicStr;
  static const std::string m_scatterPrefetchIntrinsicStr;
  static const std::string m_gatherIntrinsicName;
  static const std::string m_maskGatherIntrinsicName;
  static const std::string m_scatterIntrinsicName;
  static const std::string m_maskScatterIntrinsicName;

private:
  // address space detection
  static unsigned detectAddressSpace(Value *addr);

  // identify if an index vector refers to one cache line load
  static bool isTightConstantVect (Value *index, Type *indexedType);

  // get the index operand of a gather or scatter intrinsic
  static bool getIndexOperand(Instruction *I);

public:
  // identify whether an intrinsic represents a memory access
  static bool isPrefetchCandidate (CallInst *pCallInst, bool &pfExclusive,
      bool &isRandomV, unsigned &addrSpace, Value *&addrV,
      int &accessSizeV, int level);

  // identify if 2 instructions have the same index operand
  static bool indexMatch(Instruction *I1, Instruction *I2);

  // insert a prefetch for a random access
  static void insertPF(Instruction *I);

#ifdef TUNE_PREFETCH
  static void statAccess(Instruction *I, bool isRandom, bool pfExclusive);
#endif // TUNE_PREFETCH
};


//////////////////////////////////////////////////////////////////
/// Prefetch Class implementation
//////////////////////////////////////////////////////////////////

// Mask of all address spaces from which prefetch is likely to be beneficial
const int Prefetch::PrefecthedAddressSpaces =
    getAddressSpaceMask(Utils::OCLAddressSpace::Global) |
    getAddressSpaceMask(Utils::OCLAddressSpace::Constant);

/// Support for dynamic loading of modules under Linux
char Prefetch::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(Prefetch, "prefetch", "Auto Prefetch in Function", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(LoopInfo)
OCL_INITIALIZE_PASS_DEPENDENCY(ScalarEvolution)
OCL_INITIALIZE_PASS_DEPENDENCY(BranchProbabilityInfo)
OCL_INITIALIZE_PASS_DEPENDENCY(DominatorTree)
OCL_INITIALIZE_PASS_END(Prefetch, "prefetch", "Auto Prefetch in Function", false, false)


Prefetch::Prefetch(int level) : FunctionPass(ID), m_level(level) {
  initializePrefetchPass(*PassRegistry::getPassRegistry());
  init();
}

Prefetch::~Prefetch() {
  TUNEPF (PrintStatistics());
}

void Prefetch::init() {
  const char *val;
  int ival;

  if ((val = getenv("PFL1DIST")) != NULL) {
    std::istringstream(std::string(val)) >> ival;
    PFL1Distance = ival;
  }
  if ((val = getenv("PFL2DIST")) != NULL) {
    std::istringstream(std::string(val)) >> ival;
    PFL2Distance = ival;
  }
  if ((val = getenv("PFL1TYPE")) != NULL) {
    std::istringstream(std::string(val)) >> ival;
    PFL1Type = ival;
  }
  if ((val = getenv("PFL2TYPE")) != NULL) {
    std::istringstream(std::string(val)) >> ival;
    PFL2Type = ival;
  }

  m_disableAPF = false;
  if (getenv("DISAPF")) {
    m_disableAPF = true;
  }

  m_disableAPFGS = false;
  m_disableAPFGSTune = false;
  if (getenv("DISAPFGS")) {
    m_disableAPFGS = true;
#ifndef TUNE_PREFETCH
    m_disableAPFGSTune = true;
#endif // TUNE_PREFETCH
  }

  m_calcFactor = getenv("APFDISSMALL") == NULL;
  m_prefetchScalarCode = getenv("APFSCALAR") != NULL;

  TUNEPF (EnableStatistics());
}

// getConstStep - calculate loop step.
// step is non zero iff SAddr is AddRecExpr, i.e. it has step, and the
// step is constant
static int getConstStep (const SCEV *S, ScalarEvolution &SE) {
  if (S->getSCEVType() == scAddRecExpr) {
    const SCEVAddRecExpr *AR = cast<SCEVAddRecExpr>(S);
    const SCEV * SAddrStep = AR->getStepRecurrence(SE);
    if (SAddrStep->getSCEVType() == scConstant)
      return (cast<SCEVConstant>(SAddrStep)->getValue()->getZExtValue());
    TUNEPF(PFStat_variableStep++);
  }
  return 0;
}

// OffsetOfSCEV - get SCEV offset
static const SCEV *OffsetOfSCEV(const SCEV *S, ScalarEvolution &SE) {

  // if offset is not computable - return
  if (!S) {
    return NULL;
  }

  if (S->getSCEVType() == scConstant) {
    if (SE.getTypeSizeInBits(S->getType()) < 64)
      return SE.getZeroExtendExpr(S, IntegerType::get(SE.getContext(), 64));
    else
      return S;
  }

  // IMPORTANT NOTE: here is the base assumption that the value of Unknown
  // operations is always aligned, or at least, all unknown values have
  // the same alignment
  if (S->getSCEVType() == scUnknown)
    return SE.getConstant(IntegerType::get(SE.getContext(), 64), 0);

  // Travers operands of expressions of all other types and get their offset
  if (const SCEVCastExpr *Cast = dyn_cast<SCEVCastExpr>(S)) {
    return OffsetOfSCEV(Cast->getOperand(), SE);
  }

  if (const SCEVNAryExpr *NAry = dyn_cast<SCEVNAryExpr>(S)) {
    SmallVector<const SCEV *, 8> Operands;

    // Transform each operand.
    for (unsigned i = 0; i < NAry->getNumOperands(); i++) {
      const SCEV *newSCEV = OffsetOfSCEV(NAry->getOperand(i), SE);
      Operands.push_back(newSCEV);
    }

    // Apply operation on operand offset
    switch (S->getSCEVType()) {
      case scAddRecExpr: return SE.getAddExpr(Operands,
        (SCEV::NoWrapFlags)(NAry->getNoWrapFlags() & ~SCEV::FlagNW));
      case scAddExpr: return SE.getAddExpr(Operands, NAry->getNoWrapFlags());
      case scMulExpr: return SE.getMulExpr(Operands, NAry->getNoWrapFlags());
      case scSMaxExpr: return SE.getSMaxExpr(Operands[0], Operands[1]);
      case scUMaxExpr: return SE.getUMaxExpr(Operands[0], Operands[1]);

      default: llvm_unreachable("Unexpected SCEVNAryExpr kind!");
    }
  }

  if (const SCEVUDivExpr *Div = dyn_cast<SCEVUDivExpr>(S)) {
    const SCEV *LN = OffsetOfSCEV(Div->getLHS(), SE);
    const SCEV *RN = OffsetOfSCEV(Div->getRHS(), SE);
    if (SE.getConstant(IntegerType::get(SE.getContext(), 64), 0) == RN)
      return LN;
    return SE.getUDivExpr(LN, RN);
  }

  llvm_unreachable("Unexpected SCEV kind!");
  return 0;
}

// OffsetOfSCEV - get SCEV offset
static int getOffsetOfSCEV(const SCEV *S, ScalarEvolution &SE) {
  const SCEV *offset = OffsetOfSCEV(S, SE);
  return offset ? (cast<SCEVConstant>(offset)->getValue()->getZExtValue() % 64)
                : 0;
}

// SimplifySCev Searches for recurring expressions and mark them as NoWrap.
// it also extends all unknown values to 64 bit so upper calculations are all
// in 64 bits and sign/zero extend operations are eliminated.
// This may fold expressions that include them into the recurring expression
// hence simplyfy it
static const SCEV *SimplifySCEV(const SCEV *S, ScalarEvolution &SE) {

  // the SCEV is simplified by promoting constants and unknowns to 64 bit
  if (S->getSCEVType() == scConstant || S->getSCEVType() == scUnknown) {
    if (SE.getTypeSizeInBits(S->getType()) < 64)
      return SE.getZeroExtendExpr(S, IntegerType::get(SE.getContext(), 64));
    else
      return S;
  }

  // Travers operands of expressions of all other types and promote them if
  // they are not loop invariants
  if (const SCEVCastExpr *Cast = dyn_cast<SCEVCastExpr>(S)) {
    const SCEV *newSCEV = SimplifySCEV(Cast->getOperand(), SE);
    return newSCEV;
  }

  if (const SCEVNAryExpr *NAry = dyn_cast<SCEVNAryExpr>(S)) {
    SmallVector<const SCEV *, 8> Operands;

    // Transform each operand.
    for (unsigned i= 0; i < NAry->getNumOperands(); i++) {
      const SCEV *newSCEV = SimplifySCEV(NAry->getOperand(i), SE);
      Operands.push_back(newSCEV);
    }

    // If any operand actually changed, return a transformed result.
    switch (S->getSCEVType()) {
      case scAddRecExpr:
        return SE.getAddRecExpr(Operands, cast<SCEVAddRecExpr>(S)->getLoop(),
                                NAry->getNoWrapFlags());
      case scAddExpr: return SE.getAddExpr(Operands, NAry->getNoWrapFlags());
      case scMulExpr: return SE.getMulExpr(Operands, NAry->getNoWrapFlags());
      case scSMaxExpr: return SE.getSMaxExpr(Operands);
      case scUMaxExpr: return SE.getUMaxExpr(Operands);
      default: llvm_unreachable("Unexpected SCEVNAryExpr kind!");
    }
  }

  if (const SCEVUDivExpr *Div = dyn_cast<SCEVUDivExpr>(S)) {
    const SCEV *LN = SimplifySCEV(Div->getLHS(), SE);
    const SCEV *RN = SimplifySCEV(Div->getRHS(), SE);
    return SE.getUDivExpr(LN, RN);
  }

  llvm_unreachable("Unexpected SCEV kind!");
  return 0;
}

// PromoteSCEV - promote a SCEV N iterations ahead.
static const SCEV *PromoteSCEV(const SCEV *S, Loop *L, ScalarEvolution &SE,
                               unsigned count) {

  if (!SE.hasComputableLoopEvolution(S, L))
    return S;

  // Promote recurring expressions by count iterations ahead
  if (const SCEVAddRecExpr *AR = dyn_cast<SCEVAddRecExpr>(S)) {
    assert (AR->getLoop() == L &&
        "Expected AddRec expression recurring in this loop");
    const SCEV *SStep = AR->getStepRecurrence(SE);
    if (count != 1) {
      const SCEV *stepFactor =
          SE.getConstant(SStep->getType(), count, false);
      SStep = SE.getMulExpr(stepFactor, SStep);
    }
    S = SE.getAddExpr(SStep, S);
    return S;
  }

  // Travers operands of expressions of all other types and promote them if
  // they are not loop invariants
  if (const SCEVCastExpr *Cast = dyn_cast<SCEVCastExpr>(S)) {
    const SCEV *newSCEV = PromoteSCEV(Cast->getOperand(), L, SE, count);
      switch (S->getSCEVType()) {
        default: llvm_unreachable("Unexpected SCEVCastExpr kind!");
        case scTruncate: return SE.getTruncateExpr(newSCEV, S->getType());
        case scZeroExtend: return SE.getZeroExtendExpr(newSCEV, S->getType());
        case scSignExtend: return SE.getSignExtendExpr(newSCEV, S->getType());
      }
  }

  if (const SCEVNAryExpr *NAry = dyn_cast<SCEVNAryExpr>(S)) {
    SmallVector<const SCEV *, 8> Operands;

    // Transform each operand.
    for (unsigned i = 0; i < NAry->getNumOperands(); i++) {
      const SCEV *newSCEV = PromoteSCEV(NAry->getOperand(i), L, SE, count);
      Operands.push_back(newSCEV);
    }

    // If any operand actually changed, return a transformed result.
    switch (S->getSCEVType()) {
      case scAddExpr: return SE.getAddExpr(Operands, NAry->getNoWrapFlags());
      case scMulExpr: return SE.getMulExpr(Operands, NAry->getNoWrapFlags());
      case scSMaxExpr: return SE.getSMaxExpr(Operands);
      case scUMaxExpr: return SE.getUMaxExpr(Operands);
      default: llvm_unreachable("Unexpected SCEVNAryExpr kind!");
    }
  }

  if (const SCEVUDivExpr *Div = dyn_cast<SCEVUDivExpr>(S)) {
    const SCEV *LN = PromoteSCEV(Div->getLHS(), L, SE, count);
    const SCEV *RN = PromoteSCEV(Div->getRHS(), L, SE, count);
    return SE.getUDivExpr(LN, RN);
  }

  llvm_unreachable("Unexpected SCEV kind!");
  return 0;
}

// memAccessExists - checks if access overlaps with another access
// recorded in MAV.
// accessIsReady tells whether all access details are already calculated
bool Prefetch::memAccessExists (memAccess &access, memAccessV &MAV,
                                bool accessIsReady) {

  // Check first if an exact access is already detected. If yes - no need
  // to go into other analysis
  if (!access.isRandom()) {
    for (unsigned i = 0; i < MAV.size(); i++) {
      // If the exact same address is already detected drop the new one
      if (!MAV[i].isRandom() && access.S == MAV[i].S) {
        TUNEPF(PFStat_matchExact++);
        return true;
      }
    }
  } else { // access isRandom
    for (unsigned i = 0; i < MAV.size(); i++) {
      // If the exact same base address and index vector are already detected
      // drop the new one
      if (MAV[i].isRandom() && access.S == MAV[i].S &&
          PrefetchCandidateUtils::indexMatch(access.I, MAV[i].I)) {
        TUNEPF(PFStat_matchExact++);
        return true;
      }
    }
  }

  DEBUG (dbgs() << "Found SCEV in this BB of type " <<
      (int)access.S->getSCEVType() << "\n";
      access.S->dump(););

  // if checking a new access need to calculate some more info about it
  if (!accessIsReady) {
    access.step = getConstStep(access.S, *m_SE);
    access.offset = getOffsetOfSCEV(access.S, *m_SE);
  }

  // Identify and dismiss close addresses which are prefetched by a
  // close iteration
  for (unsigned i = 0; i < MAV.size(); i++) {

    // compare only if they have the same Random property
    if (access.isRandom() != MAV[i].isRandom())
      continue;

    // if the steps of the 2 accesses are different they are not in constant
    // distance
    if (access.step != MAV[i].step)
      continue;

    // random accesses base address are comparable only if index vector is the
    // same
    if (access.isRandom() &&
        !PrefetchCandidateUtils::indexMatch(access.I, MAV[i].I))
      continue;

    // analyze the distance between accesses
    const SCEV *diff = m_SE->getMinusSCEV(access.S, MAV[i].S);
    if (diff->getSCEVType() == scConstant) {
      int constDiff = cast<SCEVConstant>(diff)->getValue()->getZExtValue();
      // normalize distance between accesses to cache line boundary
      constDiff = constDiff - access.offset + MAV[i].offset;

      // if accesses are in the same cache line discard this one
      if (constDiff >= 0 && constDiff < UarchInfo::CacheLineSize) {
        TUNEPF(PFStat_matchSameLine++);
        return true;
      }
      // if the step is not constant we can't evaluate further
      if (access.step == 0)
        continue;

      int iterOffset = constDiff % access.step;
      int diffSteps = constDiff / access.step;
      if (diffSteps < 0)
        diffSteps = -diffSteps;
      // if accesses are in the same cache line in another iteration
      if (iterOffset >= 0 && iterOffset < UarchInfo::CacheLineSize) {
        TUNEPF (
          if (diffSteps > 256 && diffSteps <=1024)
            PFStat_LargeDiff256++;
          if (diffSteps > 1024)
            PFStat_LargeDiff1024++;
          );
        // don't merge accesses if iteration distance between accesses is too
        // large so the code may suffer from long cache warmup at the
        // beginning and from prefetches that hit the caches later
        if (diffSteps < 1024) {
          // prefetch for the more advanced access (step and diff are in the
          // same direction) is better, since the back access will catch up in
          // later iterations
          if ((constDiff < 0) == (access.step < 0)) {
            MAV[i].S = access.S;
            MAV[i].offset = access.offset;
          }
          TUNEPF (PFStat_matchSameLine++);
          return true;
        }
      }
    }
  }

  return false;
}

static BasicBlock *getIDom (DominatorTree *DT, BasicBlock *BB) {
  DomTreeNode *N = DT->getNode(BB);
  assert (N && "expecting a BB to have a DT Node");
  N = N->getIDom();
  if (!N) return NULL;
  BB = N->getBlock();
  assert (BB && "DT node is empty");
  return BB;
}

static int getSize(Type *Ty) {
  int size = Ty->getScalarSizeInBits() / 8;
  assert (size != 0 && "load/store is not scalar or size is 0");
  if (Ty->isVectorTy())
    size *= cast<VectorType>(Ty)->getNumElements();
  return size;
}

/// detectReferencesForPrefetch() - Traverese all BB in a function and detect
/// accesses that deserve prefetching.
/// If a prefetch intrinsic is detected the process is stopped.
bool Prefetch::detectReferencesForPrefetch(Function &F) {
  DominatorTree *DT = &getAnalysis<DominatorTree>();
  assert(DT && "Unable to get Dominators in Prefetch");

  unsigned NTSKind = F.getParent()->getMDKindID("nontemporal");

  SmallVector<BasicBlock *, 20> domBB;
  domBB.reserve(F.size());
  domBB.push_back(DT->getRoot());

  // Access size in bytes. If it's more than 64 bits onsider keeping more than
  // one record per access
  int accessSize;

  // collect all addresses accessed by loads and stores, which are from Global
  // address space and simple. Stores should not be to non temporal locations.
  for (unsigned int bbi = 0; bbi < domBB.size(); bbi++) {
    BasicBlock *BB = domBB[bbi];

    // insert to the list BBs dominated by this one
    DomTreeNode *Node = DT->getNode(BB);
    for (DomTreeNode::iterator CI = Node->begin(), CE = Node->end();
         CI != CE; ++CI) {
      domBB.push_back((*CI)->getBlock());
    }

    // prefetch only inside loops
    Loop *L = m_LI->getLoopFor(BB);
    if (!L) {
      TUNEPF (PFStat_BBNotInLoop++);
      continue;
    }

    for (BasicBlock::iterator II = BB->begin(), IE = BB->end();
        II != IE; ++II) {
      Value *addr = NULL;
      Instruction *I = II;

      if (I->getType()->isVectorTy())
        m_isVectorized.insert(L);

      LoadInst *pLoadInst;
      StoreInst *pStoreInst;
      CallInst* pCallInst;
      bool pfExclusive = false;
      bool isRandom = false;
      if ((pLoadInst = dyn_cast<LoadInst>(I)) != NULL) {
        if (pLoadInst->isSimple() &&
            Utils::isInSpace(pLoadInst->getPointerAddressSpace(), PrefecthedAddressSpaces)) {
          addr = pLoadInst->getPointerOperand();
          accessSize = getSize(I->getType());
        } else {
          TUNEPF (
            if (!pLoadInst->isSimple()) PFStat_NonSimpleLoad++;
            if (pLoadInst->getPointerAddressSpace() == Utils::OCLAddressSpace::Local)
              PFStat_LocalLoad++;
            if (pLoadInst->getPointerAddressSpace() == Utils::OCLAddressSpace::Private)
              PFStat_PrivateLoad++;
            );
          continue;
        }
      } else if ((pStoreInst = dyn_cast<StoreInst>(I)) != NULL) {
        if (pStoreInst->isSimple() &&
            Utils::isInSpace(pStoreInst->getPointerAddressSpace(), PrefecthedAddressSpaces) &&
            pStoreInst->getMetadata(NTSKind) == NULL) {
          addr = pStoreInst->getPointerOperand();
          accessSize = getSize(pStoreInst->getValueOperand()->getType());
          pfExclusive = true;
        } else {
          TUNEPF (
            if (!pStoreInst->isSimple()) PFStat_NonSimpleStore++;
            if (pStoreInst->getPointerAddressSpace() == Utils::OCLAddressSpace::Local)
              PFStat_LocalStore++;
            if (pStoreInst->getPointerAddressSpace() == Utils::OCLAddressSpace::Private)
              PFStat_PrivateStore++;
          );
          continue;
        }
      } else if (m_level > APFLEVEL_1_APF_LOAD_STORE && !m_disableAPFGSTune &&
                 (pCallInst = dyn_cast<CallInst>(I)) != NULL &&
                  pCallInst->getCalledFunction()) {
        unsigned addrSpace;
        bool isOtherPFCandidate =
            PrefetchCandidateUtils::isPrefetchCandidate(pCallInst, pfExclusive,
                isRandom, addrSpace, addr, accessSize, m_level);
        if (isOtherPFCandidate)
          isOtherPFCandidate = Utils::isInSpace(addrSpace, PrefecthedAddressSpaces);
        if (!isOtherPFCandidate || m_disableAPFGS)
          continue;
      } else {
        continue;
      }

      assert (addr && "Detected Load/Store for PF w/o an address?");
      assert (accessSize > 0 && "found non positive access size");

      // verify that scalar evolution is possible for this address type
      if (!m_SE->isSCEVable(addr->getType())) {
        TUNEPF (PFStat_nonSCAVAbleAccess++);
        continue;
      }

      // Get the symbolic expression for this address.
      const SCEV *SAddr = m_SE->getSCEVAtScope(addr, L);

      // keep only addresses which have computable evolution in this loop
      // TODO: hoist prefetches for addresses that have computable evolution
      // in outer loop
      // Random accesses can be loop variant
      if (!isRandom && !m_SE->hasComputableLoopEvolution(SAddr, L)) {
        TUNEPF (
          switch (m_SE->getLoopDisposition(SAddr, L)) {
            default:
            case ScalarEvolution::LoopComputable:
              assert (false && "unexpected loop disposition detected in SCEV");
              break;
            case ScalarEvolution::LoopVariant:
              PFStat_accessStepIsLoopVariant++; break;
            case ScalarEvolution::LoopInvariant:
              PFStat_accessStepIsLoopInvariant++; break;
          }
        );
        continue;
      }

      // if this is not an Add Recurrence expression already
      // simplify address scev so step is always assumed to be non wrapping.
      // this works with the vectorizer assumption of element consecutivity
      // we don't do this for random accesses since the prefetch will use the
      // original instruction address
      if (!isRandom && SAddr->getSCEVType() != scAddRecExpr)
        SAddr = SimplifySCEV(SAddr, *m_SE);

      // if this is the first access detected for this BB insert it t the list
      BBAccesses::iterator it = m_addresses.find(BB);
      if (it == m_addresses.end()) {
        DEBUG (dbgs() << "Found first SCEV in this BB of size " << accessSize <<
            " type " << (int)SAddr->getSCEVType() << "\n";
            SAddr->dump(););
        it = m_addresses.insert(std::make_pair(BB, memAccessV())).first;
        memAccessV &MAV = it->second;
        int step = 0;
        int offset = 0;
        if (!isRandom) {
          step = getConstStep(SAddr, *m_SE);
          offset = getOffsetOfSCEV(SAddr, *m_SE);
        }
        MAV.push_back(memAccess(I, SAddr, step, offset, isRandom,
            pfExclusive));

        // if access is for more than one cache line record references for all
        // accessed cache lines
        assert (accessSize <= 2 * UarchInfo::CacheLineSize &&
            "Accesses larger than 128 bytes are not expected");
        if (accessSize > UarchInfo::CacheLineSize) {
          memAccess access (I, SAddr, step, offset, isRandom, pfExclusive);
          const SCEV *nextSAddr = SAddr;
          for (int i = UarchInfo::CacheLineSize; i < accessSize;
              i+= UarchInfo::CacheLineSize) {
            nextSAddr =
                m_SE->getAddExpr(nextSAddr, m_SE->getConstant(m_i64, 64));
            access.S = nextSAddr;
            MAV.push_back(access);
            DEBUG (dbgs() << "Added access at offset " << i <<
                " bytes from base\n   ";
            access.S->dump(););
          }
        }
        continue;
      }

      // This BB already has vector of accesses that are candidate for prefetch
      memAccessV &MAV = it->second;

      // if no overlapping access was found add this one to the list
      memAccess access (I, SAddr, 0, 0, isRandom, pfExclusive);
      if (!memAccessExists(access, MAV, false)) {
        MAV.push_back(access);
        DEBUG (dbgs() << "Found new access of " << accessSize << " bytes\n   ";
               access.S->dump(););
      } else {
        TUNEPF (PFStat_accessMatchInSameBB++);
      }
      // if access is for more than one cache line record references for all
      // accessed cache lines
      assert (accessSize <= 2 * UarchInfo::CacheLineSize &&
          "Accesses larger than 128 bytes are not expected");
      if (accessSize > UarchInfo::CacheLineSize) {
        const SCEV *nextSAddr = SAddr;
        for (int i = UarchInfo::CacheLineSize; i < accessSize;
            i+= UarchInfo::CacheLineSize) {
          nextSAddr = m_SE->getAddExpr(nextSAddr, m_SE->getConstant(m_i64, 64));
          access.S = nextSAddr;
          if (!memAccessExists(access, MAV, true)) {
            MAV.push_back(access);
            DEBUG (dbgs() << "Added access at offset " << i <<
                  " bytes from base\n   ";
            access.S->dump(););
          } else {
            TUNEPF (PFStat_accessMatchInSameBB++);
          }
        }
      }
    } // end loop over instructions
  } // end loop over BBs

  // Merge overlapping accesses into dominating BBs
  for (unsigned int bbi = domBB.size()-1; bbi != 0; bbi--) {
    BasicBlock *BB = domBB[bbi];

    // if no accesses recorded for this BB - skip it
    BBAccesses::iterator it = m_addresses.find(BB);
    if (it == m_addresses.end())
      continue;

    memAccessV &MAV = it->second;

    // non vectorized code will not prefetch as long as this pass does not
    // implement loop unrolling, to avoid excessive prefetching.
    // it can hurt corner cases like scalar load followed by vectorized
    // function call that are the only instructions in a BB
    // if this BB belongs to a loop that is not vectorized - do not emit
    // prefetches for it.
    Loop *L = m_LI->getLoopFor(BB);
    assert (L && "BB with PF candidates is expected to be inside a loop");
    if ((m_prefetchScalarCode == false) &&
        (m_isVectorized.find(L) == m_isVectorized.end())) {
      TUNEPF (PFStat_SerialBB += MAV.size());
      m_addresses.erase(BB);
      continue;
    }

    for (BasicBlock *iDom = getIDom(DT, BB); iDom != 0;
         iDom = getIDom(DT, iDom)) {

      // if no accesses recorded for this BB dominator - skip it
      BBAccesses::iterator it = m_addresses.find(iDom);
      if (it == m_addresses.end())
        continue;

      memAccessV &iDomMAV = it->second;

      // check for each access that is not detected as recurring already if
      // it's recurring in its dominator
      for (unsigned i = 0; i < MAV.size(); i++) {
        if (!MAV[i].isRecurring() && memAccessExists(MAV[i], iDomMAV, true)) {
          MAV[i].setRecurring();
          TUNEPF(PFStat_accessMatchInDOMBB++);
        }
      }

      // maintenance:
      // find the last non-recurring occurrence, so future vector traversals
      // won't go beyond it.
      unsigned size = MAV.size();
      unsigned vSize = size;
      for (unsigned i = 0; i < MAV.size(); i++)
        size = MAV[i].isRecurring() ? size-1 : vSize;

      // if all are recurring - remove MAV for this BB
      if (size == 0) {
        m_addresses.erase(BB);
        break;
      }
      // if the recurring are the last ones - shorten the vector
      if (size < MAV.size())
        MAV.resize(size);
    }
  }

  return (m_addresses.size() > 0);
}

// countPFPerLoop() - Count the number of accesses that deserve a prefetch
// in each loop. Also record the number of consecutive iterations that
// target the same cache line for each access.
void Prefetch::countPFPerLoop () {
  assert (m_LoopInfo.size() == 0 &&
      "PF count should be called once per runOnFunction");

  bool hasFactor = false;

  for (BBAccesses::iterator it = m_addresses.begin(), itEnd = m_addresses.end();
      it != itEnd; ++it) {
    memAccessV &MAV = it->second;
    Loop *L = m_LI->getLoopFor(it->first);
    assert (L && "a BB that has prefetches must have a loop");

    // count the number of non-recurring accesses in this BB
    unsigned MAVSize = 0;
    int numRandom = 0;
    int factor = 1;
    for (unsigned int i = 0; i < MAV.size(); i++)
      if (!MAV[i].isRecurring()) {
        MAVSize++;
        MAV[i].factor = 1;
        // calculate the number of consecutive iterations that target the same
        // cache line.
        // Find the maximum of this factor for this BB.
        if (m_calcFactor) {
          if (MAV[i].step > 0 && MAV[i].step <= UarchInfo::CacheLineSize / 2) {
            MAV[i].factor = (UarchInfo::CacheLineSize - 1) / MAV[i].step + 1;
            if (MAV[i].factor > factor)
              factor = MAV[i].factor;
          }
        }
        if (MAV[i].isRandom())
          numRandom++;
        TUNEPF (
          int accessSize = 0;
          StoreInst *pStoreInst;
          LoadInst *pLoadInst;
          if ((pStoreInst = dyn_cast<StoreInst>(MAV[i].I)) != NULL)
            accessSize = getSize(pStoreInst->getValueOperand()->getType());
          else if ((pLoadInst = dyn_cast<LoadInst>(MAV[i].I)) != NULL)
            accessSize = getSize(MAV[i].I->getType());
          if (pStoreInst || pLoadInst) {
            if (accessSize < UarchInfo::CacheLineSize)
              PFStat_partLine++;
            else if (accessSize == UarchInfo::CacheLineSize)
              PFStat_oneLine++;
            else // (accessSize > UarchInfo::CacheLineSize)
              PFStat_multLines++;
          }
          else
            PrefetchCandidateUtils::statAccess(MAV[i].I, MAV[i].isRandom(),
                MAV[i].isExclusive());
          );
      }

    // BB with all recurring accesses were already removed from the map
    assert (MAVSize &&
        "expecting to have in address map only BB with prefetch candidates");

    if (factor > 1)
      hasFactor = true;

    // add number of prefetch candidates in this BB to loop counter
    if (m_LoopInfo.find(L) == m_LoopInfo.end())
      m_LoopInfo.insert(std::make_pair(L, loopPFInfo(MAVSize, factor,
                                                     numRandom)));
    else {
      loopPFInfo &info = m_LoopInfo[L];
      info.numRefs += MAVSize;
      info.numRandom += numRandom;
      // Find the maximum of the iteration factor for this loop
      if (factor > info.factor)
        info.factor = factor;
    }
  }

  // if none of the accesses target the same cache lines in different iterations
  // return
  if (!hasFactor)
    return;

  // calculate the number of accesses to different cache lines assuming that
  // loop iteration length is factored
  for (BBAccesses::iterator it = m_addresses.begin(), itEnd = m_addresses.end();
      it != itEnd; ++it) {
    Loop *L = m_LI->getLoopFor(it->first);
    assert (L && "a BB that has prefetches must have a loop");

    loopPFInfo &info = m_LoopInfo[L];
    int factor = info.factor;

    if(factor == 1)
      continue;

    memAccessV &MAV = it->second;
    int numRefs = 0;
    // count the number of non-recurring accesses in this BB
    for (unsigned int i = 0; i < MAV.size(); i++)
      if (!MAV[i].isRecurring())
        numRefs += factor / MAV[i].factor;

    info.factNumRefs += numRefs;
  }
}

unsigned int Prefetch::IterLength(Loop *L)
{
  LoopToLengthMap::iterator it;
  if ((it = m_iterLength.find(L)) != m_iterLength.end())
    return it->second;

  BranchProbabilityInfo *BPI = &getAnalysis<BranchProbabilityInfo>();
  assert (BPI && "Branch Probability is not available");
  unsigned int len = 0;
  BasicBlock *BB = L->getHeader();
  Loop *BBL;
#ifndef NDEBUG
  std::map<BasicBlock *, int> walkedBBs;
#endif //NDEBUG
  do {
    TerminatorInst *TI = BB->getTerminator();
    assert (TI->getNumSuccessors() > 0 && "Not expecting 0 successors");
    assert (TI->getNumSuccessors() <= 2 && "Not expecting more than 2 successors");
#ifndef NDEBUG
    assert (walkedBBs.find(BB) == walkedBBs.end() && "this BB length was already taken");
    walkedBBs[BB] = 1;
#endif //NDEBUG

    BBL = m_LI->getLoopFor(BB);
    if (BBL == L) {
      len += BB->size();
        // select the next BB in the path
      if (TI->getNumSuccessors() == 2 &&
          BPI->getEdgeProbability(BB, TI->getSuccessor(0)) < BranchProbability(1, 2)) {
        BB = TI->getSuccessor(1);
      }
      else
        BB = TI->getSuccessor(0);
    }
    else {
      assert (BBL->getHeader() == BB &&
          "expected to enter subloop from its header");
      unsigned internalLoopLen = IterLength(BBL);
      {
        ScalarEvolution *SE = &getAnalysis<ScalarEvolution>();
        SmallVector<BasicBlock *, 8> ExitBlocks;
        L->getExitBlocks(ExitBlocks);
        unsigned tripCount = 0;
        for (unsigned i = 0; i < ExitBlocks.size(); ++i)
        {
          tripCount = std::max(tripCount, SE->getSmallConstantTripMultiple(L, ExitBlocks[i]));
        }
        if (tripCount == 0)
            tripCount = defaultTripCount;
        len += internalLoopLen * tripCount;
      }

      SmallVector<BasicBlock *, 8> ExitBlocks;
      BBL->getExitBlocks(ExitBlocks);
      assert (ExitBlocks.size() != 0 && "internal loop has no exit block");
      if (ExitBlocks.size() == 1)
        BB = ExitBlocks[0];
      else {
        // unfortunatelly the following is not deterministic for a certain
        // code, but depends on the order in which BBs are layed out in
        // ExitBlocks
        unsigned i = 0;
        for (; i < ExitBlocks.size(); i++) {
          if (m_LI->getLoopFor(ExitBlocks[i]) == L)
            break;
        }
        BB = (i < ExitBlocks.size()) ? ExitBlocks[i] : ExitBlocks[0];
      }
      assert (BB && "loop has more than one exit");
    }

    BBL = m_LI->getLoopFor(BB);

  } while (L->getHeader() != BB && (BBL == L || BBL->getParentLoop() == L));

  // assume 2 instructions per cycle
  len *= 2;

  m_iterLength[L] = len;
  DEBUG (dbgs() << L->getHeader()->getName() << " length: " << len << "\n");
  return len;
}

// getPFDistance(Loop *, loopPFInfo &) - calculate the prefetch distance
// for a loop
void Prefetch::getPFDistance(Loop *L, loopPFInfo &info) {

  // start with the minimum number of threads set by previously processed loops
  info.numThreads = m_numThreads;

  // if the loop iteration factor is more than 1
  int factIterLen = info.iterLen;
  int factNumRefs = info.numRefs;
  if (info.factor > 1) {
    factIterLen *= info.factor;
    factNumRefs = info.factNumRefs;
  }

  // If one loop iteration is longer than miss latency, then once a miss latency
  // has passed there's a free prefetch slot again. so overall the practical
  // number of available prefetch slots can be considered as higher, and it can
  // handle more accesses in each iteration.
  int L1PrefetchSlotsExt = (factIterLen <= UarchInfo::L1MissLatency) ?
      UarchInfo::L1PrefetchSlots :
      UarchInfo::L1PrefetchSlots * factIterLen / UarchInfo::L1MissLatency;
  int L2PrefetchSlotsExt = (factIterLen <= UarchInfo::L2MissLatency) ?
      UarchInfo::L2PrefetchSlots :
      UarchInfo::L2PrefetchSlots * factIterLen / UarchInfo::L2MissLatency;

  // if there are not enough L2 prefetch slots for one loop iterations use 1
  // thread to avoid mutual thread stalls, and prefetch distance of 1 to the L1
  // and 2 to the L2, so the data is ready when the L1 prefetch comes. Or let's
  // be a bit more agressive and optimistic, just in case that some of the
  // accesses are in the cache after all
  if (factNumRefs >= L2PrefetchSlotsExt) {
    info.L1Distance = 2;
    info.L2Distance = 3;
    info.numThreads = 1;

    m_numThreads = 1;
    return;
  }

  // if there are not enough L1 prefetch slots for one loop iterations use 1
  // thread to avoid mutual thread stalls.
  if (factNumRefs >= L1PrefetchSlotsExt) {
    info.L1Distance = 2;
    info.numThreads = 1;

    // recalculate iteration length assuming L1 cache misses will happen
    int iterLen = UarchInfo::L1MissLatency * factNumRefs / L1PrefetchSlotsExt;
    if (iterLen < factIterLen)
      iterLen = factIterLen;

    // calculate the distance for L2 prefetches: it's the upper bound of L2
    // miss latency divided by iteration length
    info.L2Distance = (UarchInfo::L2MissLatency - 1) / iterLen + 1;

    // if there are not enough slots for this distance reduce the distance
    // accordingly. We already know it's more than 1
    if (info.L2Distance * factNumRefs > L2PrefetchSlotsExt)
      info.L2Distance = L2PrefetchSlotsExt / factNumRefs;
    // put cache line in the L2 ahead of time, so it's ready when the L1
    // prefetchcomes to get it
    if (info.L2Distance < 3)
      info.L2Distance = 3;

    m_numThreads = 1;
    return;
  }

  // there are enough L1 and L2 prefetch slots for at least one iteration
  // calculate the prefetch distances.
  assert (factNumRefs < L2PrefetchSlotsExt &&
      factNumRefs < L1PrefetchSlotsExt &&
      "There are not enough prefetch slots for one access");

  // calculate the number of threads with which there's enough space for all
  // references. It shouldn't exceed the maximum number of HW threads or the
  // limit chosen for already processed loops
  int numThreads1 = L1PrefetchSlotsExt / factNumRefs;
  int numThreads2 = L2PrefetchSlotsExt / factNumRefs;
  int numThreads = (numThreads1 < numThreads2) ? numThreads1 : numThreads2;
  if (numThreads > m_numThreads)
    numThreads = m_numThreads;
  int iterLen = factIterLen;
  // calculate the distance for L2 prefetches: its the upper bound of L2
  // miss latency divided by iteration length
  info.L2Distance = (UarchInfo::L2MissLatency - 1) / iterLen + 1;
  // if there are not enough slots for this distance reduce the distance
  // accordingly. We already know it's more than 1
  if (info.L2Distance * factNumRefs * numThreads > L2PrefetchSlotsExt) {
    // calculate the l2 distance accordingly. It should be >= 1
    info.L2Distance = L2PrefetchSlotsExt / (factNumRefs * numThreads);
    assert (info.L2Distance > 0 && "L2 distance is 0");
  }
  // we don't want the L1 and L2 distance to be the same. we want the L2 data
  // to be there when the L1 prefetch happens
  if (info.L2Distance == 1)
    info.L2Distance = 2;
  // if there's only one thread other threads won't block on this one,
  // so we can be more aggressive
  if (numThreads == 1)
    info.L2Distance++;
  // iteration length grows
  iterLen = UarchInfo::L2MissLatency / info.L2Distance;
  if (iterLen < factIterLen)
    iterLen = factIterLen;

  // do the same to calculate the L1 distance
  info.L1Distance = (UarchInfo::L1MissLatency - 1) / iterLen + 1;
  if (info.L1Distance * factNumRefs * numThreads > L1PrefetchSlotsExt) {
    // calculate the l1 distance accordingly. It should be >= 1
    info.L1Distance = L1PrefetchSlotsExt / (factNumRefs * numThreads);
    assert (info.L1Distance > 0 && "L1 distance is 0");
  }
  // if L1 distance is larger than the L2 distance the L1 distance is made
  //  one less than the L2 distance, so the line is in the L2 once the L1
  //  prefetch is done
  if (info.L2Distance <= info.L1Distance)
    info.L1Distance = info.L2Distance - 1;
  else if (numThreads == 1)
    info.L1Distance++;

  assert (info.L2Distance > info.L1Distance && "L1 distance >= L2 distance");

  assert (numThreads <= m_numThreads &&
      "this loop tries to increase number of threads");

  info.numThreads = m_numThreads = numThreads;

  return;
}

// getPFDistance() - calculate prefetch distance for all loops in a function
void Prefetch::getPFDistance() {
  // start with the maximum number of threads
  m_numThreads = UarchInfo::MaxThreads;
  int firstNumThreads = UarchInfo::MaxThreads;
  bool first = true;

  // traverse all loops that have references that deserve prefetches and
  // calculate the L1 and L2 prefetch distance
  for (LoopInfoMap::iterator it = m_LoopInfo.begin(), itEnd = m_LoopInfo.end();
       it != itEnd ; it++) {
    Loop *L = it->first;
    loopPFInfo &info = it->second;
    if (info.numRefs != info.numRandom) {
      info.iterLen = IterLength(L);
      getPFDistance(L, info);
      if (first) {
        first = false;
        firstNumThreads = m_numThreads;
      }
    }

    TUNEPF (
      printf ("First Loop %llx Num Refs %d (factor %d, factored Num Refs %d)"
          "Num Random Refs %d NumThreads %d distL1 %d distL2 %d iterLen %d\n",
            (long long int )((void *)L), info.numRefs, info.factor,
             info.factNumRefs, info.numRandom, info.numThreads, info.L1Distance,
             info.L2Distance, info.iterLen);
    );

    DEBUG (dbgs() << "First Loop " << (void *)L << " Num Refs " << info.numRefs
      << " (factor " << info.factor << " factored Num Refs " << info.factNumRefs
      << ") NumRandom Refs " << info.numRandom << " NumThreads " <<
      info.numThreads << " distL1 " << info.L1Distance <<
      " distL2 " << info.L2Distance << " iterLen " << info.iterLen << "\n");
  }

  if (m_numThreads == firstNumThreads)
    return;

  // if the optimal number of threads has changed by the calculation
  // recalculate the distance for loops that prefered to use more threads.
  // this will increase the prefetch distance for those loops
  for (LoopInfoMap::iterator it = m_LoopInfo.begin(), itEnd = m_LoopInfo.end();
       it != itEnd ; it++) {
    loopPFInfo &info = it->second;
    if (info.numRefs == info.numRandom)
      continue;
    Loop *L = it->first;
    assert (info.numThreads >= m_numThreads &&
        "there's a loop that prefers less threads");
    if (info.numThreads > m_numThreads)
      getPFDistance(L, info);

    TUNEPF (
      printf ("Final Loop %llx Num Refs %d (factor %d, factored Num Refs %d)"
          "Num Random Refs %d NumThreads %d distL1 %d distL2 %d iterLen %d\n",
            (long long int )((void *)L), info.numRefs, info.factor,
             info.factNumRefs, info.numRandom, info.numThreads, info.L1Distance,
             info.L2Distance, info.iterLen);
    );

    DEBUG (dbgs() << "Final Loop " << (void *)L << " Num Refs " << info.numRefs
      << " (factor " << info.factor << " factored Num Refs " << info.factNumRefs
      << ") NumRandom Refs " << info.numRandom << " NumThreads " <<
      info.numThreads << " distL1 " << info.L1Distance <<
      " distL2 " << info.L2Distance << " iterLen " << info.iterLen << "\n");
  }
}

/// Insert a prefetch instruction just before instruction I, for address SADDR
/// count iterations ahead. The prefetched address is loop variant for loop L.
void Prefetch::insertPF (Instruction *I, Loop *L, int PFType,
                         const SCEV *SAddr, unsigned count, bool pfExclusive) {
  std::vector<Value*> args;
  std::vector<Type *> types;

  // get the expression for the address count iteration ahead
  SAddr = PromoteSCEV(SAddr, L, *m_SE, count);

  Value *V = m_ADRExpander->expandCodeFor(SAddr, SAddr->getType(), I);

  // Remove address space from pointer type
  Instruction *addr = new BitCastInst(V, m_pi8, "pfPtrTypeCast", I);

  // if the first instruction that accesses this location is a store bring this
  // line as exclusive
  if (pfExclusive)
    PFType |= 4;

  Constant *hint = ConstantInt::get(m_i32, PFType);

  args.push_back(addr);      // pointer to loaded data
  args.push_back(hint);

  types.push_back(m_pi8);
  types.push_back(m_i32);

  FunctionType *intr = FunctionType::get(m_void, types, false);
  Constant *new_f = addr->getParent()->getParent()->getParent()->
      getOrInsertFunction(m_prefetchIntrinsicName, intr);

  CallInst *callInst = CallInst::Create(new_f, ArrayRef<Value*>(args), "", I);

  // set debug info
  if (!I->getDebugLoc().isUnknown()) {
    addr->setDebugLoc(I->getDebugLoc());
    callInst->setDebugLoc(I->getDebugLoc());
  }

  DEBUG(dbgs() << "Generated PF in BB " << I->getParent()->getName() <<
      " type " << PFType << " distance " << count << "\n");
}

/// emitPrefetches() - Emit prefetch instructions into all BB that need them
void Prefetch::emitPrefetches() {
  for (BBAccesses::iterator it = m_addresses.begin(), itEnd = m_addresses.end();
        it != itEnd; ++it) {
    memAccessV &MAV = it->second;
    Loop *L = m_LI->getLoopFor(it->first);

    assert (m_LoopInfo.find(L) != m_LoopInfo.end() &&
        "a loop with PF candidates has no distance calculation");
    loopPFInfo &info = m_LoopInfo[L];
    int L1Dist = (PFL1Distance > 0) ? PFL1Distance : info.L1Distance;
    int L2Dist = (PFL2Distance > 0) ? PFL2Distance : info.L2Distance;

    // go over the recorded memory accesses in the BB and emit prefetches
    for (unsigned int i = 0; i < MAV.size(); i++)
    {
      // ignore recurring accesses
      if (MAV[i].isRecurring())
        continue;

      Instruction *I = MAV[i].I;

      if (!MAV[i].isRandom()) {
        const SCEV *SAddr = MAV[i].S;

        // do not insert L1 prefetch if it was disabled
        if (PFL1Type > 0)
          insertPF (I, L, (int)PFL1Type, SAddr, L1Dist * MAV[i].factor,
              MAV[i].isExclusive());

        // do not insert L2 prefetch if it was disabled
        if (PFL2Type > 0)
          insertPF (I, L, (int)PFL2Type, SAddr, L2Dist * MAV[i].factor,
              MAV[i].isExclusive());
      }
      else
        PrefetchCandidateUtils::insertPF(I);
    }
    MAV.clear();
  }
}

/// autoPrefetch() - perform all steps of aut-prefetch detection and insertions
bool Prefetch::autoPrefetch(Function &F) {
  // find references that deserve prefetching
  if (detectReferencesForPrefetch(F) == false)
    return false;

  // count how many there are per loop
  countPFPerLoop();
  if (m_LoopInfo.size() == 0)
    return false;

  // calculate prefetch distance for all loops in a function
  if (PFL1Distance == 0 || PFL2Distance == 0) {
    TUNEPF (printf ("Function: %s\n", F.getName().data()));
    DEBUG (dbgs() << "Function: " << F.getName() << "\n");
    getPFDistance();
  }

  // Emit prefetch instructions into all BBs that need them
  emitPrefetches();

  return true;
}

bool Prefetch::runOnFunction(Function &F) {
  // don't bother if prefetching is disabled.
  DEBUG (dbgs() << "prefetch go " << F.getName() << "\n";);
  if (m_level == APFLEVEL_0_DISAPF || m_disableAPF)
    return false;

  // connect some analysis passes
  m_LI = &getAnalysis<LoopInfo>();
  assert(m_LI && "Unable to get LoopInfo in Prefetch");

  m_SE = &getAnalysis<ScalarEvolution>();
  assert(m_SE && "Unable to get ScalarEvolution in Prefetch");

  // prepare some constants in function context
  LLVMContext* context = &F.getContext();
  m_i8 =  IntegerType::get(*context, 8);
  m_i32 =  IntegerType::get(*context, 32);
  m_i64 = IntegerType::get(*context, 64);
  m_pi8 = PointerType::get(m_i8, 0);
  m_void = Type::getVoidTy(*context);

  SCEVExpander SCEVE(*m_SE, "prefetch");
  m_ADRExpander = &SCEVE;

  bool modified = autoPrefetch(F);

  // cleanup
  m_ADRExpander->clear();
  m_iterLength.clear();
  m_addresses.clear();
  m_LoopInfo.clear();
  m_isVectorized.clear();

  return modified;
}

//////////////////////////////////////////////////////////////////
/// PrefetchCandidateUtils Class implementation
//////////////////////////////////////////////////////////////////
unsigned PrefetchCandidateUtils::detectAddressSpace(Value *addr) {
  BitCastInst *BCI = NULL;
  GetElementPtrInst *GEPI = NULL;

  // get address pointer type
  PointerType * PType = dyn_cast<PointerType>(addr->getType());

  // if it's a pointer as expected
  while (PType) {

    // get address space
    unsigned addrSpace = PType->getAddressSpace();
    // if address space is not the default value return it
    if (addrSpace != 0)
      return addrSpace;

    // get the instruction representing this value
    // return if it's not an instruction
    Instruction *I = dyn_cast<Instruction>(addr);
    if (I == NULL)
      return 0;

    // if it's a bit cast - get the source pointer type and the source
    // instruction
    if ((BCI = dyn_cast<BitCastInst>(I)) != NULL) {
       PType = cast<PointerType>(BCI->getSrcTy());
       addr = I->getOperand(0);
    }
    // if its a GEP - get its pointer operand
    else if((GEPI = dyn_cast<GetElementPtrInst>(I)) != NULL) {
      PType = cast<PointerType>(GEPI->getType());
      addr = I->getOperand(0);
    }
    // otherwise no specific address space was detected
    else
      return 0;
  }

  // no specific address space was detected
  return 0;
}

// isTightConstantVect - check if all vector elements are constants with
// maximum difference between the smallest and largest value, such that
// the type indexed by this vector is contained in one cache line.
bool PrefetchCandidateUtils::isTightConstantVect(Value *index, Type *indexedType) {
  VectorType *VType = dyn_cast<VectorType>(indexedType);
  if (VType == NULL)
    return false;
  assert (!isa<ConstantVector>(index) && "need to handle ConstantVector type in Prefetch.cpp");
/*  ConstantVector *CV = dyn_cast<ConstantVector>(index);
  // verify that the accessed type is a vector type (although this should be
  // guaranteed)
  // check if the index vector is of constant int values.
  if (VType == NULL || CV == NULL ||
      dyn_cast<ConstantInt>(CV->getOperand(0)) == NULL)
    return false;
  unsigned N = VType->getNumElements();
  unsigned destSize = VType->getElementType()->getScalarSizeInBits() / 8;
  if (CV->getNumOperands() < N)
    return false;
  assert (CV->getType()->getElementType()->getScalarSizeInBits() == 32 &&
      "expecting gather index to be v16i32");

  int i32Min = 0x7FFFFFFF;
  int i32Max = 0x80000000;

  for (unsigned i = 0; i < N; ++i) {
    int v = cast<ConstantInt>(CV->getOperand(i))->getSExtValue();
    if (v < i32Min) i32Min = v;
    if (v > i32Max) i32Max = v;
  }
  return ((i32Max - i32Min) * destSize <= unsigned(UarchInfo::CacheLineSize));*/
  ConstantDataVector *CV = dyn_cast<ConstantDataVector>(index);
  // verify that the accessed type is a vector type (although this should be
  // guaranteed)
  // check if the index vector is of constant int values.
  if (CV == NULL || CV->getElementType() !=
      IntegerType::get(CV->getContext(), 32))
    return false;
  unsigned N = VType->getNumElements();
  unsigned destSize = VType->getElementType()->getScalarSizeInBits() / 8;
  if (CV->getNumElements() < N)
    return false;

  int i32Min = INT_MAX;
  int i32Max = INT_MIN;

  for (unsigned i = 0; i < N; ++i) {
    int v = CV->getElementAsInteger(i);
    if (v < i32Min) i32Min = v;
    if (v > i32Max) i32Max = v;
  }
  return ((i32Max - i32Min) * destSize <= unsigned(UarchInfo::CacheLineSize));
}

bool PrefetchCandidateUtils::isPrefetchCandidate (CallInst *pCallInst,
    bool &pfExclusive, bool &isRandomV, unsigned &addrSpace, Value *&addrV,
    int &accessSizeV, int level) {

  Value *addr = NULL;

  StringRef Name = pCallInst->getCalledFunction()->getName();

  // ignore calls that are non intrinsic
  if (Name.find(m_intrinsicName) == std::string::npos)
    return false;

  // check if gather intrinsic for fetching data randomly
  // gather: <vector> = gather (<indexes>, <address> ...)
  if (Name.find(m_gatherIntrinsicName) != std::string::npos) {
    assert (pCallInst->getNumOperands() >= 2 &&
        "gather intrinsic doesn't have enough operands");
    if (pCallInst->getNumOperands() < 2)
      return false;
    assert (pCallInst->getOperand(1)->getType()->isPointerTy() &&
        "expected pointer type");
    addr = pCallInst->getOperand(1);
    if (!addr)
      return false;
    addrV = addr;
    addrSpace = detectAddressSpace(addr);
    TUNEPF (
      switch (addrSpace) {
        default: assert(false && "Unknown address space detected for gather");
          break;
        case Utils::OCLAddressSpace::Global: PFStat_GlobalGather++; break;
        case Utils::OCLAddressSpace::Local: PFStat_LocalGather++; break;
        case Utils::OCLAddressSpace::Constant: PFStat_ConstantGather++; break;
        case Utils::OCLAddressSpace::Private: PFStat_PrivateGather++; break;
      }
    );
    accessSizeV = UarchInfo::CacheLineSize;
    pfExclusive = false;
    isRandomV = !isTightConstantVect(pCallInst->getOperand(2),
        pCallInst->getType());
    if (isRandomV && level < APFLEVEL_3_RANDOM) return false;
    return true;
  }

  // check if masked gather intrinsic. For MIC can be consecutive masked load
  // or masked gather of random data
  // masked gather:
  //     <vector> = gather (<old-value>, <mask>, <indexes>, <address> ...)
  if (Name.find(m_maskGatherIntrinsicName) != std::string::npos) {
    assert (pCallInst->getNumOperands() >= 4 &&
        "masked gather intrinsic doesn't have enough operands");
    if (pCallInst->getNumOperands() < 4)
      return false;
    assert (pCallInst->getOperand(3)->getType()->isPointerTy() &&
        "expected pointer type");
    addr = pCallInst->getOperand(3);
    if (!addr)
      return false;
    addrV = addr;
    addrSpace = detectAddressSpace(addr);
    TUNEPF (
      switch (addrSpace) {
        default: assert(false &&
            "Unknown address space detected for masked gather"); break;
        case Utils::OCLAddressSpace::Global: PFStat_GlobalMaskGather++; break;
        case Utils::OCLAddressSpace::Local: PFStat_LocalMaskGather++; break;
        case Utils::OCLAddressSpace::Constant: PFStat_ConstantMaskGather++;
          break;
        case Utils::OCLAddressSpace::Private: PFStat_PrivateMaskGather++;
          break;
      }
    );
    accessSizeV = UarchInfo::CacheLineSize;
    pfExclusive = false;
    // check if the index operand is constant vector with close values
    isRandomV = !isTightConstantVect(pCallInst->getOperand(2),
        pCallInst->getType());
    if (isRandomV && level < APFLEVEL_3_RANDOM) return false;
    return true;
  }

  // check if scatter
  // scatter: scatter (<address>, <indexes>, <data> ...)
  if (Name.find(m_scatterIntrinsicName) != std::string::npos) {
    assert (pCallInst->getNumOperands() >= 3 &&
        "scatter intrinsic doesn't have enough operands");
    if (pCallInst->getNumOperands() < 3)
      return false;
    assert (pCallInst->getOperand(0)->getType()->isPointerTy() &&
        "expected pointer type");
    addr = pCallInst->getOperand(0);
    if (!addr)
      return false;
    addrV = addr;
    addrSpace = detectAddressSpace(addr);
    TUNEPF (
      switch (addrSpace) {
        default: assert(false && "Unknown address space detected for scatter");
        break;
        case Utils::OCLAddressSpace::Global: PFStat_GlobalScatter++; break;
        case Utils::OCLAddressSpace::Local: PFStat_LocalScatter++; break;
        case Utils::OCLAddressSpace::Constant: PFStat_ConstantScatter++; break;
        case Utils::OCLAddressSpace::Private: PFStat_PrivateScatter++; break;
      }
    );
    accessSizeV = UarchInfo::CacheLineSize;
    pfExclusive = true;
    isRandomV = !isTightConstantVect(pCallInst->getOperand(1),
        pCallInst->getOperand(2)->getType());
    if (isRandomV && level < APFLEVEL_3_RANDOM) return false;
    return true;
  }

  // check if masked scatter
  // masked scatter: scatter (<address>, <mask>, <indexes>, <data> ...)
  if (Name.find(m_maskScatterIntrinsicName) != std::string::npos) {
    assert (pCallInst->getNumOperands() >= 4 &&
        "scatter intrinsic doesn't have enough operands");
    if (pCallInst->getNumOperands() < 4)
      return false;
    assert (pCallInst->getOperand(0)->getType()->isPointerTy() &&
        "expected pointer type");
    addr = pCallInst->getOperand(0);
    if (!addr)
      return false;
    addrV = addr;
    addrSpace = detectAddressSpace(addr);
    TUNEPF (
      switch (addrSpace) {
        default: assert(false &&
            "Unknown address space detected for masked scatter"); break;
        case Utils::OCLAddressSpace::Global: PFStat_GlobalMaskScatter++; break;
        case Utils::OCLAddressSpace::Local: PFStat_LocalMaskScatter++; break;
        case Utils::OCLAddressSpace::Constant: PFStat_ConstantMaskScatter++;
          break;
        case Utils::OCLAddressSpace::Private: PFStat_PrivateMaskScatter++;
          break;
      }
    );
    accessSizeV = UarchInfo::CacheLineSize;
    pfExclusive = true;
    isRandomV = !isTightConstantVect(pCallInst->getOperand(2),
        pCallInst->getOperand(3)->getType());
    if (isRandomV && level < APFLEVEL_3_RANDOM) return false;
    return true;
  }

  TUNEPF(
    if (Name.find(m_prefetchIntrinsicName) != std::string::npos ||
        Name.find(m_gatherPrefetchIntrinsicStr) != std::string::npos ||
        Name.find(m_scatterPrefetchIntrinsicStr) != std::string::npos) {
      PFStat_ManualPFAbortAPF++;
    }
  );
  return false;
}

bool PrefetchCandidateUtils::getIndexOperand(Instruction *I) {
  assert (isa<CallInst>(I) && "Call instruction expected");
  CallInst *pCallInst = cast<CallInst>(I);
  StringRef Name = pCallInst->getCalledFunction()->getName();

  if (Name.find(m_gatherIntrinsicName) != std::string::npos)
    return 0;

  if (Name.find(m_scatterIntrinsicName) != std::string::npos)
    return 1;

  return 2;
}

bool PrefetchCandidateUtils::indexMatch(Instruction *I1, Instruction *I2) {
  return (I1->getOperand(getIndexOperand(I1)) ==
      I2->getOperand(getIndexOperand(I2)));
}

void PrefetchCandidateUtils::insertPF (Instruction *I) {
  const char *pfIntrinName = NULL;
  std::vector<Value*> args;
  std::vector<Type *> types;

  Module *M = I->getParent()->getParent()->getParent();
  LLVMContext &context = M->getContext();
  Type *i8 =  IntegerType::get(context, 8);
  Type *pi8 = PointerType::get(i8, 0);
  Type *i16 =  IntegerType::get(context, 16);
  Type *i32 =  IntegerType::get(context, 32);
  Type *v16i32 = VectorType::get(i32, 16);
  Value *mask8Bit = NULL;

  bool isExclusive = false;

  CallInst *pCallInst = cast<CallInst>(I);
  StringRef Name = pCallInst->getCalledFunction()->getName();

  Constant *hint = ConstantInt::get(i32, UarchInfo::defaultL1PFType);
  // gather: <vector> = gather (<indexes>, <address>, i, i, i)
  // gather PF: <vector> = gather (<indexes>, <address> , i, i, i)
  if (Name.find(m_gatherIntrinsicName) != std::string::npos) {
    // create gatherpf intrinsic operand list
    // push indexes
    args.push_back(pCallInst->getOperand(0));
    types.push_back(v16i32);

    assert (I->getType()->isVectorTy() &&
        (cast<VectorType>(I->getType())->getNumElements() == 16 ||
         cast<VectorType>(I->getType())->getNumElements() == 8) &&
        "gathered data expected to be vector of 8 or 16");
    if (cast<VectorType>(I->getType())->getNumElements() == 16) {
      pfIntrinName = m_prefetchGatherIntrinsicName.c_str();
    } else {
      // if the instruction gathers 8 elements need to pf only the first 8
      // indexes
      pfIntrinName = m_prefetchMaskGatherIntrinsicName.c_str();
      mask8Bit = ConstantInt::get(Type::getInt16Ty(context), 127);
      args.push_back(mask8Bit);
      types.push_back(i16);
    }

    // push the address and modifiers - values and then types
    args.push_back(pCallInst->getOperand(1));
    args.push_back(pCallInst->getOperand(2));
    args.push_back(pCallInst->getOperand(3));
    args.push_back(hint);

    types.push_back(pi8);
    types.push_back(i32);
    types.push_back(i32);
    types.push_back(i32);
  }

  // masked gather:
  //     <vector> = gather (<old-value>, <mask>, <indexes>, <address>, i, i, i)
  // masked gather PF:
  //     <vector> = gather (<indexes>, <mask>, <address>, i, i, i)
  else if (Name.find(m_maskGatherIntrinsicName) != std::string::npos) {
    pfIntrinName = m_prefetchMaskGatherIntrinsicName.c_str();
    // create gatherpf intrinsic operand list
    // push indexes
    args.push_back(pCallInst->getOperand(2));
    types.push_back(v16i32);

    // push mask. zero extend it first to 16 bits as required by the gatherpf
    if (pCallInst->getOperand(1)->getType() == i8) {
      mask8Bit = CastInst::Create(Instruction::ZExt, pCallInst->getOperand(1),
          i16, "", I);
      args.push_back(mask8Bit);
    }
    else
      args.push_back(pCallInst->getOperand(1));

    types.push_back(i16);

    // push the address and modifiers - values and then types
    args.push_back(pCallInst->getOperand(3));
    args.push_back(pCallInst->getOperand(4));
    args.push_back(pCallInst->getOperand(5));
    args.push_back(hint);

    types.push_back(pi8);
    types.push_back(i32);
    types.push_back(i32);
    types.push_back(i32);
  }

  // scatter: scatter (<address>, <indexes>, <data>, i,i,i)
  // scatter PF: scatter (<address>, <indexes>, i, i, i)
  else if (Name.find(m_scatterIntrinsicName) != std::string::npos) {
    // create scatterpf intrinsic operand list
    // push address
    args.push_back(pCallInst->getOperand(0));
    types.push_back(pi8);

    assert (pCallInst->getOperand(2)->getType()->isVectorTy() &&
        (cast<VectorType>(pCallInst->getOperand(2)->getType())->getNumElements() == 16 ||
         cast<VectorType>(pCallInst->getOperand(2)->getType())->getNumElements() == 8) &&
        "scatter data expected to be vector of 8 or 16");
    if (cast<VectorType>(pCallInst->getOperand(2)->getType())->getNumElements() == 16) {
      pfIntrinName = m_prefetchScatterIntrinsicName.c_str();
    } else {
      // if the instruction scatters 8 elements need to pf only the first 8
      // indexes
      pfIntrinName = m_prefetchMaskScatterIntrinsicName.c_str();
      mask8Bit = ConstantInt::get(Type::getInt16Ty(context), 127);
      args.push_back(mask8Bit);
      types.push_back(i16);
    }

    // push the indexes and modifiers - values and then types
    args.push_back(pCallInst->getOperand(1));
    args.push_back(pCallInst->getOperand(3));
    args.push_back(pCallInst->getOperand(4));
    args.push_back(hint);

    types.push_back(v16i32);
    types.push_back(i32);
    types.push_back(i32);
    types.push_back(i32);

    isExclusive = true;
  }

  // masked scatter: scatter (<address>, <mask>, <indexes>, <data>, i, i, i)
  // masked scatter PF: scatter (<address>, <mask>, <indexes>, i, i, i)
  else if (Name.find(m_maskScatterIntrinsicName) != std::string::npos) {
    pfIntrinName = m_prefetchMaskScatterIntrinsicName.c_str();
    // create scatterpf intrinsic operand list
    // push address
    args.push_back(pCallInst->getOperand(0));
    types.push_back(pi8);

    // push mask. zero extend it first to 16 bits as required by the gatherpf
    if (pCallInst->getOperand(1)->getType() == i8) {
      mask8Bit = CastInst::Create(Instruction::ZExt, pCallInst->getOperand(1),
          i16, "", I);
      args.push_back(mask8Bit);
    }
    else
      args.push_back(pCallInst->getOperand(1));

    types.push_back(i16);

    // push the indexes and modifiers - values and then types
    args.push_back(pCallInst->getOperand(2));
    args.push_back(pCallInst->getOperand(4));
    args.push_back(pCallInst->getOperand(5));
    args.push_back(hint);

    types.push_back(v16i32);
    types.push_back(i32);
    types.push_back(i32);
    types.push_back(i32);

    isExclusive = true;
  }
  else
    return;

  assert (pfIntrinName != NULL && "Expected gather/scatter intrinsic");

  Type *voidTy = Type::getVoidTy(context);
  FunctionType *intr = FunctionType::get(voidTy, types, false);
  Constant *new_f = M->getOrInsertFunction(pfIntrinName, intr);

  CallInst *callInst = CallInst::Create(new_f, ArrayRef<Value*>(args), "", I);

  // set debug info
  if (!I->getDebugLoc().isUnknown()) {
    callInst->setDebugLoc(I->getDebugLoc());
  }

  DEBUG(dbgs() << "Generated PF in BB " << I->getParent()->getName() <<
      " type " << (isExclusive ? "ScatterPF" : "GatherPF") << "\n");
}

#ifdef TUNE_PREFETCH
void PrefetchCandidateUtils::statAccess(Instruction *I, bool isRandom,
    bool pfExclusive) {
  CallInst *pCallInst = dyn_cast<CallInst>(I);
  assert (pCallInst && "Expecting pf stat for gather/scatter");

  StringRef Name = pCallInst->getCalledFunction()->getName();
  // ignore calls that are non intrinsic
  assert (Name.find(m_intrinsicName) != std::string::npos &&
    "Expecting pf stat for gather/scatter");

  if (!isRandom) {
    if (pfExclusive)
      PFStat_pfMaskedStores++;
    else
      PFStat_pfMaskedLoads++;
    return;
  }

  // all our known intrinsic access cache line
  if (pfExclusive) {
    if (Name.find(m_scatterIntrinsicName) != std::string::npos)
      PFStat_pfRandomScatter++;
    else
      PFStat_pfMaskedRandomScatter++;
  } else {
    if (Name.find(m_gatherIntrinsicName) != std::string::npos)
      PFStat_pfRandomGather++;
    else
      PFStat_pfMaskedRandomGather++;
  }
}
#endif // TUNE_PREFETCH

const std::string PrefetchCandidateUtils::m_intrinsicName = "llvm.x86.mic.";
const std::string PrefetchCandidateUtils::m_prefetchIntrinsicName =
    "llvm.x86.mic.prefetch";
const std::string PrefetchCandidateUtils::m_prefetchGatherIntrinsicName =
    "llvm.x86.mic.gatherpf.ps";
const std::string PrefetchCandidateUtils::m_prefetchMaskGatherIntrinsicName =
    "llvm.x86.mic.mask.gatherpf.ps";
const std::string PrefetchCandidateUtils::m_prefetchScatterIntrinsicName =
    "llvm.x86.mic.scatterpf.ps";
const std::string PrefetchCandidateUtils::m_prefetchMaskScatterIntrinsicName =
    "llvm.x86.mic.mask.scatterpf.ps";
const std::string PrefetchCandidateUtils::m_gatherPrefetchIntrinsicStr =
    "gatherpf";
const std::string PrefetchCandidateUtils::m_scatterPrefetchIntrinsicStr =
    "scatterpf";
const std::string PrefetchCandidateUtils::m_gatherIntrinsicName = "mic.gather.";
const std::string PrefetchCandidateUtils::m_maskGatherIntrinsicName =
    "mic.mask.gather.";
const std::string PrefetchCandidateUtils::m_scatterIntrinsicName =
    "mic.scatter.";
const std::string PrefetchCandidateUtils::m_maskScatterIntrinsicName =
    "mic.mask.scatter.";

} // namespace intel

extern "C" {
FunctionPass * createPrefetchPassLevel(int level) {
  return new intel::Prefetch(level);
}
FunctionPass * createPrefetchPass() {
  return new intel::Prefetch();
}
}
