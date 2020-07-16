// INTEL CONFIDENTIAL
//
// Copyright 2012-2019 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "BarrierPass.h"
#include "BarrierUtils.h"
#include "CompilationUtils.h"
#include "InitializePasses.h"
#include "LoopUtils/LoopUtils.h"
#include "MetadataAPI.h"
#include "OCLPassSupport.h"

#include "llvm/Analysis/CFG.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"

#include <set>
#include <sstream>
#include <vector>

#define DEBUG_TYPE "B-Barrier"

using namespace Intel::MetadataAPI;

extern bool OptUseTLSGlobals;

static cl::opt<bool> OptIsNativeDebug("is-native-debug", cl::init(false),
                                      cl::Hidden, cl::desc("Use native debug"));

namespace intel {

  char Barrier::ID = 0;

  OCL_INITIALIZE_PASS_BEGIN(Barrier, "B-Barrier", "Barrier Pass - Handle special values & replace barrier/fiber with internal loop over WIs", false, true)
  OCL_INITIALIZE_PASS_DEPENDENCY_INTEL(DataPerBarrier)
  OCL_INITIALIZE_PASS_DEPENDENCY_INTEL(DataPerValue)
  OCL_INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
  OCL_INITIALIZE_PASS_END(Barrier, "B-Barrier", "Barrier Pass - Handle special values & replace barrier/fiber with internal loop over WIs", false, true)

  Barrier::Barrier(bool isNativeDebug, bool useTLSGlobals)
      : ModulePass(ID), m_DL(nullptr), m_pContext(nullptr), m_uiSizeT(0),
        m_sizeTType(nullptr), m_I32Type(nullptr),
        m_useTLSGlobals(useTLSGlobals || OptUseTLSGlobals),
        m_LocalIdAllocTy(nullptr), m_LocalIdArrayTy(nullptr), m_Zero(nullptr),
        m_One(nullptr), m_pDataPerValue(nullptr), m_pAllocaValues(nullptr),
        m_pSpecialValues(nullptr), m_pCrossBarrierValues(nullptr),
        m_pDataPerBarrier(nullptr), m_pSyncInstructions(nullptr),
        m_currFunction(nullptr), m_currBarrierKeyValues(nullptr),
        m_isNativeDBG(isNativeDebug || OptIsNativeDebug) {
    std::fill(m_PtrLocalId, m_PtrLocalId + MaxNumDims, nullptr);
    initializeBarrierPass(*llvm::PassRegistry::getPassRegistry());
  }

  bool Barrier::runOnModule(Module &M) {
    //Get Analysis data
    m_pDataPerBarrier = &getAnalysis<DataPerBarrier>();
    m_pDataPerValue = &getAnalysis<DataPerValue>();

    m_DL = &M.getDataLayout();

    //Initialize barrier utils class with current module
    m_util.init(&M);
    // This call is needed to initialize vectorization widths
    m_util.getAllKernelsWithBarrier();

    m_pContext = &M.getContext();
    //Initialize the side of size_t
    m_uiSizeT = M.getDataLayout().getPointerSizeInBits(0);
    m_sizeTType = IntegerType::get(*m_pContext, m_uiSizeT);
    m_I32Type = IntegerType::get(*m_pContext, 32);
    m_LocalIdArrayTy = ArrayType::get(m_sizeTType, MaxNumDims);
    m_LocalIdAllocTy = PointerType::get(m_LocalIdArrayTy, 0);
    m_Zero = ConstantInt::get(m_sizeTType, 0);
    m_One = ConstantInt::get(m_sizeTType, 1);

    bool ModuleHasAnyInternalCalls = false;

    if (m_useTLSGlobals) {
      // Add thread local variable for local ids
      m_LocalIds = new GlobalVariable(
          M, m_LocalIdArrayTy, false, GlobalValue::LinkOnceODRLinkage,
          UndefValue::get(m_LocalIdArrayTy), "LocalIds", nullptr,
          GlobalValue::GeneralDynamicTLSModel);
      m_LocalIds->setAlignment(
          MaybeAlign(M.getDataLayout().getPreferredAlignment(m_LocalIds)));
    }

    //Find all functions that call synchronize instructions
    TFunctionSet& functionsWithSync = m_util.getAllFunctionsWithSynchronization();

    //Collect data for each function with synchronize instruction
    for (TFunctionSet::iterator fi = functionsWithSync.begin(),
                                fe = functionsWithSync.end();
         fi != fe; ++fi) {
      Function* pFunc = *fi;

      //Check if function has no synchronize instructions!
      assert ( m_pDataPerBarrier->hasSyncInstruction(pFunc) &&
        "Cannot reach here with function that has no barrier");

      //Create new BB at the begining of the function for declarations
      BasicBlock *EntryBB = &pFunc->getEntryBlock();
      BasicBlock *FirstBB = pFunc->begin()->splitBasicBlock(EntryBB->begin(), "FirstBB");
      m_oldToNewSyncBBMap[pFunc][EntryBB] = FirstBB;

      //Initialize the argument values
      //This is needed for optimize pLocalId calculation
      bool hasNoInternalCalls = !m_util.doesCallModuleFunction(pFunc);
      ModuleHasAnyInternalCalls = ModuleHasAnyInternalCalls || !hasNoInternalCalls;
      createBarrierKeyValues(pFunc, hasNoInternalCalls);
    }

    //Fix non inlined internal functions that need special handling
    //Run over functions with synchronize instruction:
    // 1. Handle call instructions to non-inline functions
    for ( TFunctionSet::iterator fi = functionsWithSync.begin(),
        fe = functionsWithSync.end(); fi != fe; ++fi ) {
      Function* pFuncToFix = *fi;

      //Run over old users of pFuncToFix and prepare parameters as needed.
      for ( Value::user_iterator ui = pFuncToFix->user_begin(),
          ue = pFuncToFix->user_end(); ui != ue; ++ui ) {
        CallInst *pCallInst = dyn_cast<CallInst>(*ui);
        if ( !pCallInst ) continue;
        //Handle call instruction operands and return value, if needed.
        fixCallInstruction(pCallInst);
      }
    }
    // 2. Handle non-inline functions
    for ( TFunctionSet::iterator fi = functionsWithSync.begin(),
        fe = functionsWithSync.end(); fi != fe; ++fi ) {
      Function* pFuncToFix = *fi;
      //Load arguments from special buffer at specific offset as needed.
      fixNonInlineFunction(pFuncToFix);
    }


    //Run over functions with synchronize instruction:
    // 1. Handle Values from Group-A, Group-B.1 and Group-B.2
    // 2. Hanlde synchronize instructions
    for ( TFunctionSet::iterator fi = functionsWithSync.begin(),
        fe = functionsWithSync.end(); fi != fe; ++fi ) {
      Function* pFuncToFix = *fi;
      runOnFunction(*pFuncToFix);
    }

    // Update Map with structure stride size for each kernel.
    // fixAllocaValues may add new alloca.
    updateStructureStride(M, functionsWithSync);

    fixSynclessTIDUsers(M, functionsWithSync);
    // Fix get_local_id() and get_global_id() function calls
    fixGetWIIdFunctions(M);

    return true;
  }

  bool Barrier::runOnFunction(Function &F) {

    assert(!m_pDataPerBarrier->hasFiberInstruction(&F) && "handle case when having fiber instructions!");

    //Get key values for this functions.
    getBarrierKeyValues(&F);

    m_pSyncInstructions = &m_pDataPerBarrier->getSyncInstructions(&F);

    m_pSpecialValues = &m_pDataPerValue->getValuesToHandle(&F);
    m_pAllocaValues = &m_pDataPerValue->getAllocaValuesToHandle(&F);
    m_pCrossBarrierValues = &m_pDataPerValue->getUniformValuesToHandle(&F);

    Instruction* pInsertBefore = &*F.getEntryBlock().begin();
    if (m_isNativeDBG) {
      // Move alloca instructions for locals/parameters for debugging purposes
      for (TValueVector::iterator vi = m_pAllocaValues->begin(), ve = m_pAllocaValues->end();
           vi != ve; ++vi ) {
        AllocaInst *pAllocaInst = cast<AllocaInst>(*vi);
        pAllocaInst->moveBefore(pInsertBefore);
      }
    }

    //Clear container for new iteration on new function
    m_toRemoveInstructions.clear();
    m_preSyncLoopHeader.clear();

    //Fix special values
    fixSpecialValues();

    // Fix alloca values
    fixAllocaValues(F);

    //Fix cross barrier uniform values
    fixCrossBarrierValues(&*F.begin()->begin());

    //Replace sync instructions with internal loop over WI ID
    replaceSyncInstructions();

    //Remove all instructions in m_toRemoveInstructions
    eraseAllToRemoveInstructions();

    return true;
  }

  void Barrier::fixSynclessTIDUsers(Module &M, const TFunctionSet &FuncsWithSync) {
    std::vector<Function*> Worklist;
    std::set<Function*> FuncsToPatch;
    std::set<CallInst*> CIsToPatch;
    std::map<ConstantExpr*, Function*> ConstBitcastsToPatch;
    std::vector<std::string> TIDFuncNames;
    using namespace Intel::OpenCL::DeviceBackend;
    TIDFuncNames.push_back(CompilationUtils::mangledGetLID());
    TIDFuncNames.push_back(CompilationUtils::mangledGetGID());
    // Initialize the set of functions that need patching by selecting the
    // functions which contain direct calls to get_*_id() and are w/o syncs
    for (unsigned I = 0; I < TIDFuncNames.size(); ++I) {
      Function *F = M.getFunction(TIDFuncNames[I]);
      if (!F) continue;
      for (Function::user_iterator ui = F->user_begin(), ue = F->user_end();
           ui != ue; ++ui) {
        CallInst *CI = dyn_cast<CallInst>(*ui);
        if (!CI) continue;
        Function *CallingF = CI->getParent()->getParent();
        assert(CallingF);
        if (FuncsWithSync.count(CallingF)) continue;
        FuncsToPatch.insert(CallingF);
        Worklist.push_back(CallingF);
      }
    }
    // Traverse back the call graph and find the set of all functions which need to be patched. Also find the coresponding call intructions
    // Function which need to be patched are either:
    // 1. Functions w/o sync instructions which are direct calls of get_*_id() (handled in loop above)
    // 2. Functions which are direct caller of functions described in 1. or (recursively) functions defined in this line which do not contain sync instructions
    for (unsigned WorkListIdx = 0; WorkListIdx < Worklist.size(); ++WorkListIdx) {
      Function *CalledF = Worklist[WorkListIdx];
      for (Function::user_iterator UI = CalledF->user_begin(), UE = CalledF->user_end();
           UI != UE; ++UI) {

        // OCL2.0. handle constant expression with bitcast of function pointer
        if(ConstantExpr *CE = dyn_cast<ConstantExpr>(*UI)) {
          if((CE->getOpcode() == Instruction::BitCast || CE->getOpcode() == Instruction::AddrSpaceCast) &&
            CE->getType()->isPointerTy()){
            ConstBitcastsToPatch[CE] = CalledF;
            continue;
          }
        }

        CallInst *CI = dyn_cast<CallInst>(*UI);
        if (!CI) continue;
        CIsToPatch.insert(CI);
        Function *CallingF = CI->getParent()->getParent();
        if (FuncsWithSync.count(CallingF)) continue;
        FuncsToPatch.insert(CallingF);
        Worklist.push_back(CallingF);
      }
    }

    typedef std::map<Function *, Function *> F2FMap;
    F2FMap OldF2PatchedF;
    if (!m_useTLSGlobals) {
      // Setup stuff needed for adding another argument to patched functions
      SmallVector<Attribute, 1> NoAlias(
          1, Attribute::get(M.getContext(), Attribute::NoAlias));
      SmallVector<AttributeSet, 1> NewAttrs(
          1, AttributeSet::get(*m_pContext, NoAlias));
      // Patch the functions
      for (std::set<Function *>::iterator I = FuncsToPatch.begin(),
                                          E = FuncsToPatch.end();
           I != E; ++I) {
        Function *OldF = *I;
        Function *PatchedF = CompilationUtils::AddMoreArgsToFunc(
            OldF, m_LocalIdAllocTy, "pLocalIdValues", NewAttrs, "BarrierPass");
        OldF2PatchedF[OldF] = PatchedF;
        assert(!m_pBarrierKeyValuesPerFunction.count(OldF));
        // So now the last arg of NewF is the base of the memory holding
        // LocalId's
        // Find the last arg
        Function::arg_iterator AI = PatchedF->arg_begin();
        for (unsigned I = 0; I < PatchedF->arg_size() - 1; ++I, ++AI) {
          // Skip over the original args
        }
        m_pBarrierKeyValuesPerFunction[PatchedF].m_TheFunction = PatchedF;
        m_pBarrierKeyValuesPerFunction[PatchedF].m_pLocalIdValues = &*AI;
      }
      // Patch the calls
      for (std::set<CallInst *>::iterator I = CIsToPatch.begin(),
                                          IE = CIsToPatch.end();
           I != IE; ++I) {
        CallInst *CI = *I;
        Function *CallingF = CI->getParent()->getParent();
        Function *CalledF = CI->getCalledFunction();
        assert(OldF2PatchedF.find(CalledF) != OldF2PatchedF.end());
        Function *PatchedF = OldF2PatchedF[CalledF];
        // Use calling functions's LocalIdValues as additional argument to
        // called function
        assert(m_pBarrierKeyValuesPerFunction.find(CallingF) !=
               m_pBarrierKeyValuesPerFunction.end());
        Value *NewArg = m_pBarrierKeyValuesPerFunction.find(CallingF)
                            ->second.m_pLocalIdValues;
        SmallVector<Value *, 1> NewArgs(1, NewArg);
        CompilationUtils::AddMoreArgsToCall(CI, NewArgs, PatchedF);
      }
    }

    // Patch the constant function ptr addr bitcasts. Used in OCL20. Extended execution
    for (std::map<ConstantExpr*, Function*>::iterator I = ConstBitcastsToPatch.begin(),
      E = ConstBitcastsToPatch.end();
      I != E; ++I) {
        ConstantExpr *CE = I->first;
        Function *F = I->second;

        if (!m_useTLSGlobals) {
          assert(OldF2PatchedF.find(F) != OldF2PatchedF.end() &&
                 "expected to find patched function in map");
          F = OldF2PatchedF[F];
        }

        // this case happens when global block variable is used
        Constant *newCE = ConstantExpr::getPointerCast(F, CE->getType());
        CE->replaceAllUsesWith(newCE);
    }
}

  void Barrier::useStackAsWorkspace(Instruction* insertBefore, Instruction* insertBeforeEnd) {
    //TODO: do we need to set DebugLoc for these instruction?
    //      These are debug instruction, so we assume there
    //      are not binded to any source instruction.
    IRBuilder<> builder(*m_pContext);

    for (TValueVector::iterator vi = m_pAllocaValues->begin(),
      ve = m_pAllocaValues->end(); vi != ve; ++vi ) {
        AllocaInst *pAllocaInst = dyn_cast<AllocaInst>(*vi);
        assert( pAllocaInst && "container of alloca values has non AllocaInst value!" );
        //Get offset of alloca value in special buffer
        unsigned int offset = m_pDataPerValue->getOffset(pAllocaInst);

        Value *pAddrInSpecialBufferCopyOut;
        Value *pAddrInSpecialBufferCopyIn;
        Type *pAllocaType = pAllocaInst->getAllocatedType();
        if (pAllocaType->isStructTy() || pAllocaType->isArrayTy()) {
          Constant *pSizeToCopy = ConstantExpr::getSizeOf(pAllocaType);
          if (insertBefore) {
            pAddrInSpecialBufferCopyOut = getAddressInSpecialBuffer(
                offset, pAllocaInst->getType(), insertBefore, nullptr);

            // create copy to work item buffer (from stack)
            builder.SetInsertPoint(insertBefore);
            builder.CreateMemCpy(
                pAddrInSpecialBufferCopyOut,
                MaybeAlign(pAllocaInst->getAlignment()), pAllocaInst,
                MaybeAlign(pAllocaInst->getAlignment()), pSizeToCopy, false);
          }

          if (insertBeforeEnd) {
            pAddrInSpecialBufferCopyIn = getAddressInSpecialBuffer(
                offset, pAllocaInst->getType(), insertBeforeEnd, nullptr);

            // create copy to stack (from work item buffer)
            builder.SetInsertPoint(insertBeforeEnd);
            builder.CreateMemCpy(
                pAllocaInst, MaybeAlign(pAllocaInst->getAlignment()),
                pAddrInSpecialBufferCopyIn,
                MaybeAlign(pAllocaInst->getAlignment()), pSizeToCopy, false);
          }
        } else {
          if (insertBefore) {
            pAddrInSpecialBufferCopyOut = getAddressInSpecialBuffer(
                offset, pAllocaInst->getType(), insertBefore, nullptr);
            // create copy to work item buffer (from stack)
            LoadInst *pLDInstCopyOut =
                new LoadInst(pAllocaType, pAllocaInst, "CopyOut", insertBefore);
            new StoreInst(pLDInstCopyOut, pAddrInSpecialBufferCopyOut, insertBefore);
          }

          if (insertBeforeEnd) {
            pAddrInSpecialBufferCopyIn = getAddressInSpecialBuffer(
                offset, pAllocaInst->getType(), insertBeforeEnd, nullptr);

            // create copy to stack (from work item buffer)
            LoadInst *pLDInstCopyIn =
                new LoadInst(pAllocaType, pAddrInSpecialBufferCopyIn, "CopyIn",
                             insertBeforeEnd);
            new StoreInst(pLDInstCopyIn, pAllocaInst, insertBeforeEnd);
          }
        }
    }
  }

  void Barrier::findSyncBBSuccessors() {
    TBasicBlockToBasicBlockSet SyncBBSuccs;
    for (Instruction *SyncI : *m_pSyncInstructions) {
      BasicBlock *BB = SyncI->getParent();
      m_syncPerBB[BB] = SyncI;

      TInstructionSet &SyncPreds =
          m_pDataPerBarrier->getBarrierPredecessors(SyncI).m_relatedBarriers;
      for (Instruction *Pred : SyncPreds) {
        // Note Pred->getParent() may be different from Pred->getParent()
        // in DataPerBarrierPass, because barrier basic block may change when
        // splitting basic block, see FirstBB and CallBB in this pass.
        BasicBlock *PredBB = Pred->getParent();
        SyncBBSuccs[PredBB].insert(BB);
      }
    }

    // Run DFS to find successors recursively
    for (auto &S : SyncBBSuccs) {
      BasicBlock *BB = S.first;
      SmallVector<BasicBlock *, 8> BBsToHandle;
      for (BasicBlock *Succ : S.second) {
        BBsToHandle.push_back(Succ);
        m_syncBBSuccessors[BB].insert(Succ);
      }
      while (!BBsToHandle.empty()) {
        BasicBlock *V = BBsToHandle.pop_back_val();
        if (!SyncBBSuccs.count(V))
          continue;
        for (BasicBlock *Succ : SyncBBSuccs[V]) {
          if (m_syncBBSuccessors[BB].count(Succ))
            continue;
          m_syncBBSuccessors[BB].insert(Succ);
          BBsToHandle.push_back(Succ);
        }
      }
    }
  }

  BasicBlock *Barrier::findNearestDominatorSyncBB(DominatorTree & DT,
                                                  BasicBlock * BB) {
    BasicBlock *Dominator = nullptr;
    for (BasicBlock *SyncBB : m_BBToPredSyncBB[BB]) {
      if (BB == SyncBB || !DT.dominates(SyncBB, BB))
        continue;
      if (Dominator) {
        // Skip if Dominator is SyncBB's successor.
        if (m_syncBBSuccessors[SyncBB].count(Dominator))
          continue;
        Dominator = SyncBB;
        continue;
      }
      // Check that there isn't another barrier in any path from SyncBB to BB.
      auto noBarrierFromTo = [this](BasicBlock *SyncBB,
                                    BasicBlock *BB) -> bool {
        for (BasicBlock *Succ : m_syncBBSuccessors[SyncBB]) {
          if (isPotentiallyReachable(Succ, BB))
            return false;
        }
        return true;
      };
      if (noBarrierFromTo(SyncBB, BB))
        Dominator = SyncBB;
    }
    return Dominator;
  }

  // This function binds alloca's user instruction to a basic block.
  // It also does optimization based on dominance information so that we load
  // address from new alloca only if necessary. For example, if both basick
  // block A and B contain alloca's users, A dominates B and there is no barrier
  // between A and B, we can bind users in B to A. Then we only need to load
  // address in A.
  // For the following IR, there is a barrier in for loop. There is no barrier
  // between while.cond and while.body, we only need to load from %i.addr in
  // while.cond and reuse it in while.body.
  //
  // for.body:
  // "Barrier BB":
  // while.cond:
  //   %1 = load i32, i32* %i, align 4, !dbg !46
  //   br i1 %cmp1, label %while.body, label %while.end, !dbg !45
  // while.body:
  //   store i32 1, i32* %i, align 4, !dbg !48
  // while.end:
  // for.inc:
  //
  // Output is:
  //
  // for.body:
  // "Barrier BB":
  // while.cond:
  //   %SBIndex22 = load i64, i64* %pCurrSBIndex, align 8, !dbg !46
  //   %SB_LocalId_Offset23 = add nuw i64 %SBIndex22, 40, !dbg !46
  //   %11 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset23,
  //       !dbg !46
  //   %pSB_LocalId24 = bitcast i8* %11 to i32*, !dbg !46
  //   store i32* %pSB_LocalId24, i32** %i.addr, align 8, !dbg !46
  //   %12 = load i32*, i32** %i.addr, align 8, !dbg !46
  //   call void @llvm.dbg.value(metadata i32* %12, metadata !43,
  //       metadata !DIExpression(DW_OP_deref)), !dbg !41
  //   %13 = load i32, i32* %12, align 4, !dbg !46
  //   br i1 %cmp1, label %while.body, label %while.end, !dbg !45
  // while.body:
  //   store i32 1, i32* %12, align 4, !dbg !48
  // while.end:
  // for.inc:
  void Barrier::bindUsersToBasicBlock(
      AllocaInst * AI, DbgDeclareInst * DI,
      TBasicBlockToInstructionMapVector & BBUsers) {
    Function &F = *AI->getFunction();
    DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();

    SmallVector<Instruction *, 8> UIs;
    for (User *U : AI->users()) {
      Instruction *UI = dyn_cast<Instruction>(U);
      assert(UI && "uses of alloca instruction is not an instruction!");
      UIs.push_back(UI);
    }

    BasicBlock *DIBB = nullptr;
    if (DI) {
      UIs.push_back(DI);
      DIBB = DI->getParent();
    }

    DenseMap<Instruction *, BasicBlock *> UIToInsertPointBB;
    TBasicBlockSet BBs;
    for (Instruction *UI : UIs) {
      Instruction *InsertBefore = getInstructionToInsertBefore(AI, UI, false);
      // If UI is PHINode, BB is PHINode's previous basic block, otherwise
      // BB is UI's parent basic block.
      BasicBlock *BB = InsertBefore->getParent();
      UIToInsertPointBB[UI] = BB;
      BBs.insert(BB);

      if (m_syncPerBB.count(BB))
        continue;

      // Collect BB's predecessors that are SyncBB
      if (!m_BBToPredSyncBB.count(BB) && m_pDataPerBarrier->hasPredecessors(BB)) {
        TBasicBlockSet &Preds = m_pDataPerBarrier->getPredecessors(BB);
        for (BasicBlock *Pred : Preds) {
          if (m_syncPerBB.count(Pred))
            m_BBToPredSyncBB[BB].push_back(Pred);
        }
        for (auto &OldToNewSync : m_oldToNewSyncBBMap[&F]) {
          BasicBlock *Pred = OldToNewSync.second;
          if (Preds.count(OldToNewSync.first) && m_syncPerBB.count(Pred))
            m_BBToPredSyncBB[BB].push_back(Pred);
        }
      }

      // Find the nearest SyncBB that dominates BB
      if (!m_BBToNearestDominatorSyncBB.count(BB))
        m_BBToNearestDominatorSyncBB[BB] = findNearestDominatorSyncBB(DT, BB);
    }

    // FIXME IsSingleUserBB and it related code should be removed.
    // Currently a variable's new llvm.dbg.value intrinsic is correct after
    // removing IsSingleUserBB, however, a few debugger_test_type tests fail
    // because register is killed if the variable has only one use, making it
    // optimized-out at a later breakpoint.
    // DBG_VALUE renamable $rdx, $noreg, !"v", !DIExpression(DW_OP_deref)
    // MOV32mi killed renamable $rdx
    bool IsSingleUserBB = (BBs.size() == 1);

    // Map from user BB to its bound dominator that value loaded from AddrAI in
    // the dominator will be reused in the user BB.
    TBasicBlockToBasicBlock BBBindToDominator;
    // Basic blocks that are bound to their nearest dominator SyncBB.
    SmallSet<BasicBlock *, 8> BBBindToNearestSyncDominator;

    for (BasicBlock *BB : BBs)
      BBBindToDominator[BB] = BB;

    // Bind user basic blocks to SyncBB
    for (auto &BBToDominator : m_BBToNearestDominatorSyncBB) {
      BasicBlock *BB = BBToDominator.first;
      BasicBlock *Dominator = BBToDominator.second;
      if (!Dominator)
        continue;
      // If Dominator is DIBB's predecessor, we shouldn't bind BB to Dominator
      // so that AI's debug scope is within DIBB.
      if (DIBB && isPotentiallyReachable(Dominator, DIBB) && !IsSingleUserBB)
        continue;

      BBBindToDominator[BB] = Dominator;
      BBBindToNearestSyncDominator.insert(BB);
    }

    if (IsSingleUserBB) {
      BasicBlock *BB = UIs[0]->getParent();
      if (BBBindToNearestSyncDominator.count(BB)) {
        if (BasicBlock *Pred = BB->getUniquePredecessor())
          BBBindToDominator[BB] = Pred;
      }
    }

    for (BasicBlock *BB : BBs) {
      DenseMap<BasicBlock *, bool> &HasBarrierTo = m_hasBarrierFromTo[BB];

      if (!m_BBToDominatedBBs.count(BB))
        DT.getDescendants(BB, m_BBToDominatedBBs[BB]);

      for (auto *DominatedBB : m_BBToDominatedBBs[BB]) {
        // Skip if DominatedBB is a SyncBB because binding to SyncBB is already
        // handled above.
        if (BB == DominatedBB || !BBs.count(DominatedBB) ||
            m_syncPerBB.count(DominatedBB))
          continue;
        // Skip if binding is already done.
        if (BBBindToNearestSyncDominator.count(DominatedBB))
          continue;
        // Skip if DominatedBB is already bound to either the basic block
        // containing DbgDeclareInst or a basic block that BB doesn't dominate.
        if ((BB != DIBB) &&
            (!DT.dominates(BB, BBBindToDominator[DominatedBB]) ||
             BBBindToDominator[DominatedBB] == DIBB))
          continue;

        if (!HasBarrierTo.count(DominatedBB))
          HasBarrierTo[DominatedBB] =
              m_util.isCrossedByBarrier(*m_pSyncInstructions, DominatedBB, BB);
        if (!HasBarrierTo[DominatedBB])
          BBBindToDominator[DominatedBB] = BB;
      }
    }
    for (Instruction *UI : UIs) {
      BasicBlock *BB = UIToInsertPointBB[UI];
      BBUsers[BBBindToDominator[BB]].push_back(UI);
    }
  }

  void Barrier::fixAllocaValues(Function & F) {
    DIBuilder DIB(*F.getParent(), /*AllowUnresolved*/ false);
    uint64_t Addr[] = {llvm::dwarf::DW_OP_deref};
    DIExpression *Expr = DIB.createExpression(Addr);
    const DataLayout &DL = F.getParent()->getDataLayout();
    Instruction *AddrInsertBefore = &*F.getEntryBlock().begin();

    // Reset containers for the current function.
    m_syncPerBB.clear();
    m_syncBBSuccessors.clear();
    m_BBToDominatedBBs.clear();
    m_BBToPredSyncBB.clear();
    m_BBToNearestDominatorSyncBB.clear();
    m_hasBarrierFromTo.clear();

    // For the current function, find all successors of each basic block which
    // contains a synchronization instruction.
    findSyncBBSuccessors();

    for (Value *V : *m_pAllocaValues) {
      AllocaInst *AI = dyn_cast<AllocaInst>(V);
      assert(AI && "container of alloca values has non AllocaInst value!");

      // Don't fix implicit GID.
      if (m_isNativeDBG && m_util.isImplicitGID(AI))
        continue;

      // Insert new alloca which stores AI's address in special buffer.
      // AI's users will be replaced by result of load instruction from the new
      // alloca.
      StringRef allocaName = AI->getName();
      AllocaInst *AddrAI =
          new AllocaInst(AI->getType(), DL.getAllocaAddrSpace(),
                         allocaName + ".addr", AddrInsertBefore);
      uint64_t ASize = AddrAI->getAllocationSizeInBits(DL).getValue() / 8;
      AddrAI->setAlignment(assumeAligned(ASize));
      m_addrAllocaSize[&F] += ASize;

      // Collect debug intrinsic.
      TinyPtrVector<DbgDeclareInst *> DIs;
      if (m_isNativeDBG) {
        for (DbgVariableIntrinsic *DVI : FindDbgAddrUses(AI)) {
          if (auto *DDI = dyn_cast<DbgDeclareInst>(DVI))
            DIs.push_back(DDI);
        }
      }
      // Only use the first DbgDeclareInst.
      DbgDeclareInst *DI = DIs.empty() ? nullptr : DIs.front();

      // Get offset of alloca value in special buffer.
      unsigned int Offset = m_pDataPerValue->getOffset(AI);

      // Bind AI's users to basic blocks that update AddrAI.
      // MapVector is used so that access order is deterministic.
      TBasicBlockToInstructionMapVector BBUsers;
      bindUsersToBasicBlock(AI, DI, BBUsers);

      // Now each user is bound to a basic block, in which we insert instruction
      // to load AI's address in special buffer to AddrAI, and replace the user
      // with result of the load instruction.
      for (auto &BBUser : BBUsers) {
        BasicBlock *BB = BBUser.first;
        Instruction *InsertBefore;
        if (m_syncPerBB.count(BB)) {
          InsertBefore = m_syncPerBB[BB]->getNextNode();
          assert(
              InsertBefore->getParent() == BB &&
              "sync instruction must not be the last instruction in the block");
        } else
          InsertBefore = BB->getFirstNonPHI();
        assert(InsertBefore && "InsertBefore is invalid");
        // Calculate the pointer of the current alloca in the special buffer
        Value *AddrInSpecialBuffer = getAddressInSpecialBuffer(
            Offset, AI->getType(), InsertBefore, &InsertBefore->getDebugLoc());
        IRBuilder<> Builder(InsertBefore);
        Builder.CreateStore(AddrInSpecialBuffer, AddrAI);
        LoadInst *LI = Builder.CreateLoad(AddrAI);
        if (m_isNativeDBG && DI) {
          const DebugLoc *DB = &DI->getDebugLoc();
          DIB.insertDbgValueIntrinsic(LI, DI->getVariable(), Expr, DB->get(),
                                      InsertBefore);
        }
        for (Instruction *UI : BBUser.second) {
          if (!isa<DbgDeclareInst>(UI))
            UI->replaceUsesOfWith(AI, LI);
        }
      }

      m_toRemoveInstructions.push_back(AI);

      // Remove old DbgDeclareInst
      if (m_isNativeDBG) {
        for (auto *DI : DIs)
          DI->eraseFromParent();
      }
    }
    DIB.finalize();
  }

  void Barrier::fixSpecialValues() {
    TValueVector::iterator vi = m_pSpecialValues->begin();
    TValueVector::iterator ve = m_pSpecialValues->end();
    for ( ; vi != ve; ++vi ) {
      Instruction *pInst = cast<Instruction>(*vi);

      const DebugLoc& DB = pInst->getDebugLoc();
      //This will hold the real type of this value in the special buffer
      Type *pTypeInSP = pInst->getType();
      bool oneBitBaseType = m_pDataPerValue->isOneBitElementType(pInst);
      if (oneBitBaseType) {
        // base type is i1 need to ZEXT/TRUNC to/from i32
        VectorType *pVecType = dyn_cast<VectorType>(pInst->getType());
        if (pVecType) {
          pTypeInSP = VectorType::get(IntegerType::get(*m_pContext, 32), pVecType->getNumElements());
        } else {
          pTypeInSP = IntegerType::get(*m_pContext, 32);
        }
      }

      //Get offset of special value in special buffer
      unsigned int offset = m_pDataPerValue->getOffset(pInst);
      //Find next instruction so we can create new instruction before it
      Instruction *pNextInst = &*(++BasicBlock::iterator(pInst));
      if ( isa<PHINode>(pNextInst) ) {
        //pNextInst is a PHINode, find first non PHINode to add instructions before it
        pNextInst = pNextInst->getParent()->getFirstNonPHI();
      }
      //Get PointerType of value type
      PointerType *pType = pTypeInSP->getPointerTo(SPECIAL_BUFFER_ADDR_SPACE);
      //Handle Special buffer only if it is not a call instruction.
      //Special buffer value of call instruction will be handled in the callee.
      CallInst* pCallInst = dyn_cast<CallInst>(pInst);
      if( !( pCallInst && m_pDataPerValue->hasOffset(pCallInst->getCalledFunction()) ) ) {
        //Calculate the pointer of the current special in the special buffer
        Value *pAddrInSpecialBuffer = getAddressInSpecialBuffer(offset, pType, pNextInst, &DB);
        Instruction *pInstToStore = !oneBitBaseType ? pInst :
          CastInst::CreateZExtOrBitCast(pInst, pTypeInSP, "ZEXT-i1Toi32", pNextInst);
        //Need to set DebugLoc for the case is oneBitBaseType. It won't hart to set
        //Same DebugLoc for the other case, as DB = pInst->getDebugLoc();
        pInstToStore->setDebugLoc(DB);
        //Add Store instruction after the value instruction
        StoreInst* pStoreInst = new StoreInst(pInstToStore, pAddrInSpecialBuffer, pNextInst);
        pStoreInst->setDebugLoc(DB);
      }

      TInstructionSet userInsts;
      //Save all uses of pInst and add them to a container before start handling them!
      for (Instruction::user_iterator ui = pInst->user_begin(),
                                      ue = pInst->user_end();
           ui != ue; ++ui) {
          Instruction *pUserInst = cast<Instruction>(*ui);
          if (pInst->getParent() == pUserInst->getParent()) {
            //This use of pInst is at the same basic block (no barrier cross so far)
            //assert( !isa<PHINode>(pUserInst) && "user instruction is a PHINode and appears befre pInst in BB" );
            if (!isa<PHINode>(pUserInst)) {
              continue;
            }
          }
          if (isa<ReturnInst>(pUserInst)) {
            // We don't want to return the value from the Special buffer we will load it later by the caller
            continue;
          }
          userInsts.insert(pUserInst);
      }
      //Run over all saved user instructions and handle by adding
      //load instruction before each value use
      for ( TInstructionSet::iterator ui = userInsts.begin(),
        ue = userInsts.end(); ui != ue; ++ui ) {
          Instruction *pUserInst = *ui;
          Instruction *pInsertBefore = getInstructionToInsertBefore(pInst, pUserInst, true);
          if (!pInsertBefore) {
            //as no barrier in the middle, no need to load & replace the origin value
            continue;
          }
          const DebugLoc& DB = pUserInst->getDebugLoc();
          //Calculate the pointer of the current special in the special buffer
          Value *pAddrInSpecialBuffer = getAddressInSpecialBuffer(offset, pType, pInsertBefore, &DB);
          Instruction *pLoadedValue = new LoadInst(
              pTypeInSP, pAddrInSpecialBuffer, "loadedValue", pInsertBefore);
          Instruction *pRealValue = !oneBitBaseType ? pLoadedValue :
            CastInst::CreateTruncOrBitCast(pLoadedValue, pInst->getType(), "Trunc-i1Toi32", pInsertBefore);
          pLoadedValue->setDebugLoc(DB);
          pRealValue->setDebugLoc(DB);
          //Replace the use of old value with the new loaded value from special buffer
          pUserInst->replaceUsesOfWith(pInst, pRealValue);
      }
    }
  }

  void Barrier::fixCrossBarrierValues(Instruction *pInsertBefore) {
    TValueVector::iterator vi = m_pCrossBarrierValues->begin();
    TValueVector::iterator ve = m_pCrossBarrierValues->end();
    for ( ; vi != ve; ++vi ) {
      Instruction *pInst = dyn_cast<Instruction>(*vi);
      assert( pInst && "container of special values has non Instruction value!" );
      //Find next instruction so we can create new instruction before it
      Instruction *pNextInst = &*(++BasicBlock::iterator(pInst));
      if ( isa<PHINode>(pNextInst) ) {
        //pNextInst is a PHINode, find first non PHINode to add instructions before it
        pNextInst = pNextInst->getParent()->getFirstNonPHI();
      }
      //Create alloca of value type at begining of function
      AllocaInst *pAllocaInst = new AllocaInst(
        pInst->getType(), m_DL->getAllocaAddrSpace(), pInst->getName(), pInsertBefore);
      //Add Store instruction after the value instruction
      StoreInst* pStoreInst = new StoreInst(pInst, pAllocaInst, pNextInst);
      pStoreInst->setDebugLoc(pInst->getDebugLoc());

      TInstructionSet userInsts;
      //Save all uses of pInst and add them to a container before start handling them!
      for ( Instruction::user_iterator ui = pInst->user_begin(),
        ue = pInst->user_end(); ui != ue; ++ui ) {
          Instruction *pUserInst = dyn_cast<Instruction>(*ui);
          assert( pUserInst && "uses of special instruction is not an instruction!" );
          if ( pInst->getParent() == pUserInst->getParent() && !isa<PHINode>(pUserInst) ) {
            //This use of pInst is at the same basic block (no barrier cross so far)
            continue;
          }
          userInsts.insert(pUserInst);
      }
      //Run over all saved user instructions and handle by adding
      //load instruction before each value use
      for ( TInstructionSet::iterator ui = userInsts.begin(),
        ue = userInsts.end(); ui != ue; ++ui ) {
          Instruction *pUserInst = *ui;
          Instruction *pInsertBefore = getInstructionToInsertBefore(pInst, pUserInst, true);
          if ( !pInsertBefore ) {
            //as no barrier in the middle, no need to load & replace the origin value
            continue;
          }
          //Calculate the pointer of the current special in the special buffer
          Instruction *pLoadedValue =
              new LoadInst(pAllocaInst->getAllocatedType(), pAllocaInst,
                           "loadedValue", pInsertBefore);
          pLoadedValue->setDebugLoc(pUserInst->getDebugLoc());
          //Replace the use of old value with the new loaded value from special buffer
          pUserInst->replaceUsesOfWith(pInst, pLoadedValue);
      }
    }
  }

  BasicBlock *Barrier::createLatchNesting(unsigned Dim, BasicBlock *Body,
                                          BasicBlock *Dispatch, Value *Step,
                                          const DebugLoc &DL) {

    LLVMContext &C = Body->getContext();
    Function *F = Body->getParent();
    // BB that is jumped to if loop in current nesting finishes
    BasicBlock *LoopEnd =
        BasicBlock::Create(C, AppendWithDimension("LoopEnd_", Dim), F, Dispatch);

    {
      IRBuilder<> B(Body);
      B.SetCurrentDebugLocation(DL);
      Value *LocalId = createGetLocalId(Dim, B);
      LocalId = B.CreateNUWAdd(LocalId, Step);
      createSetLocalId(Dim, LocalId, B);

      // if(LocalId[Dim] < WGSize[dim]) {BB Dispatch} else {BB LoopEnd}
      Value *IsContinue = B.CreateICmpULT(LocalId, getLocalSize(Dim));
      B.CreateCondBr(IsContinue, Dispatch, LoopEnd);
    }

    {
      IRBuilder<> B(LoopEnd);
      B.SetCurrentDebugLocation(DL);
      createSetLocalId(Dim, m_Zero, B);
    }
    return LoopEnd;
  }
  BasicBlock *Barrier::createBarrierLatch(BasicBlock *pPreSyncBB,
                                          BasicBlock *pSyncBB,
                                          BarrierBBIdList &BBId,
                                          Value *UniqueID, const DebugLoc &DL) {
    Function *F = pPreSyncBB->getParent();
    unsigned NumDims = getNumDims();
    // A. change the preSync basic block as follow
    // A(1). remove the unconditional jump instruction
    pPreSyncBB->getTerminator()->eraseFromParent();
     // Create then and else basic-blocks
    BasicBlock *Dispatch = BasicBlock::Create(*m_pContext, "Dispatch", F, pSyncBB);
    BasicBlock *InnerMost = pPreSyncBB;
    assert(m_currBarrierKeyValues->m_currVectorizedWidthValue);
    Value* LoopSteps[MaxNumDims] = {m_currBarrierKeyValues->m_currVectorizedWidthValue, m_One, m_One};
    for (unsigned I = 0; I < NumDims; ++I)
      InnerMost = createLatchNesting(I, InnerMost, Dispatch, LoopSteps[I], DL);

    // A(2). add the entry tail code
    // if(LocalId < WGSize[dim]) {Dispatch} else {pElseBB}

    // B. Create LocalId++ and switch instruction in Dispatch

    // Create "LocalId+=VectorizationWidth" code

    // Create "CurrSBBase+=Stride" code
    {
      IRBuilder<> B(Dispatch);
      B.SetCurrentDebugLocation(DL);
      Value *CurrSBIndex = createGetCurrSBIndex(B);
      Value *pUpdatedCurrSB = B.CreateNUWAdd(
          CurrSBIndex, m_currBarrierKeyValues->m_pStructureSizeValue);
      createSetCurrSBIndex(pUpdatedCurrSB, B);

      if (BBId.size() == 1) {
        // Only one case, no need for switch, create unconditional jump
        B.CreateBr(BBId[0].second);
      } else {
        // More than one case, create a switch
        Value *CurrBarrierId = createGetCurrBarrierId(B);
        // The first sync instruction is chosen to be the switch Default case
        SwitchInst *S =
            B.CreateSwitch(CurrBarrierId, BBId[0].second, BBId.size() - 1);
        for (unsigned I = 1; I < BBId.size(); ++I)
          S->addCase(BBId[I].first, BBId[I].second);
      }
    }

    // C. Create initialization to LocalId, currSB and currBarrier in pElseBB
    // LocalId = 0
    // currSB = 0
    // currBarrier = id
    // And connect the pElseBB to the pSyncBB with unconditional jump
    {
      IRBuilder<> B(InnerMost);
      B.SetCurrentDebugLocation(DL);
      createSetCurrSBIndex(m_Zero, B);
      if (UniqueID) {
        createSetCurrBarrierId(UniqueID, B);
      }
      B.CreateBr(pSyncBB);
    }
    // Only if we are debugging, copy data into the stack from local buffer
    // for execution and copy data out when finished. This allows for proper
    // DWARF based debugging.
    if (m_isNativeDBG) {
      createDebugInstrumentation(Dispatch, InnerMost);
    }
    return InnerMost;
  }

  void Barrier::createDebugInstrumentation(BasicBlock *Then, BasicBlock *Else) {
    // Use the then and else blocks to copy local buffer data.
    Instruction &pThenFront = Then->front();
    Instruction &pElseFront = Else->front();

    // I add the function DebugCopy as a marker so it can be handled later
    // in LocalBuffers pass.
    // LocalBuffers pass is responsible for implementing __local variables
    // correctly in OpenCL
    // (ie. as work-group globals and not thread globals). I insert them
    // in these marked blocks
    // so that I know when I need to copy from the local buffer into the
    // thread local (global).
    // This is also how I know where the beginning of each work item
    // iteration is (in the presence
    // of barriers) which is where the copying occurs.

    // Maybe there is a better way, I'm not sure. The problem I found is
    // LocalBuffers finds all
    // uses of a __local variable and updates the references to a local
    // buffer memory location
    // rather then the thread specific global for which the __local
    // variable symbol is defined.
    // So any changes to __local variables would have to be delayed until
    // this pass or LocalBuffers
    // would have to behave very differently.

    // There is also a copy that occurs from the local buffer into the
    // global variable after each
    // use of the __local variable so that the thread specific global
    // stays updated. This is
    // independent of the function markers. This is done in LocalBuffers
    // pass.

    // This only allows for reading of __local variables and not setting.

    Type *pResult = Type::getVoidTy(*m_pContext);
    Module *M = Then->getParent()->getParent();
    FunctionCallee pFunc = M->getOrInsertFunction("DebugCopy.", pResult);
    CallInst::Create(pFunc, "", &pThenFront);
    CallInst::Create(pFunc, "", &pElseFront);
  }

  void Barrier::replaceSyncInstructions() {
    //Run over all sync instructions and split its basic-block
    //in order to create an empty basic-block previous to the sync basic block
    unsigned ID = 0;
    std::stringstream Name;
    for (TInstructionSet::iterator ii = m_pSyncInstructions->begin(),
                                   ie = m_pSyncInstructions->end();
         ii != ie; ++ii) {
      Instruction *pInst = dyn_cast<Instruction>(*ii);
      assert(pInst && "sync instruction container contains non instruction!");
      BasicBlock *pLoopHeaderBB = pInst->getParent();
      Name.str("");
      Name << "SyncBB" << ID++;
      BasicBlock *pLoopEntryBB = pInst->getParent()->splitBasicBlock(
          BasicBlock::iterator(pInst), Name.str());
      m_preSyncLoopHeader[pLoopEntryBB] = pLoopHeaderBB;
      m_toRemoveInstructions.push_back(pInst);
    }
    for (TInstructionSet::iterator ii = m_pSyncInstructions->begin(),
                                   ie = m_pSyncInstructions->end();
         ii != ie; ++ii) {
        Instruction *pInst = *ii;
        DebugLoc DL = pInst->getDebugLoc();
        unsigned int id = m_pDataPerBarrier->getUniqueID(pInst);
        Value* UniqueID = ConstantInt::get(m_I32Type, APInt(32, id));
        SYNC_TYPE type = m_pDataPerBarrier->getSyncType(pInst);
        BasicBlock *pSyncBB = pInst->getParent();
        BasicBlock *pPreSyncBB = m_preSyncLoopHeader[pSyncBB];
        assert( pPreSyncBB && "pSyncBB assumed to have sync loop header basic block!" );
        if (SYNC_TYPE_DUMMY_BARRIER == type) {
          //This is a dummy barrier replace with the following
          // LocalId = 0
          // currSB = 0
          // currBarrier = id
          IRBuilder<> B(&*pPreSyncBB->begin());
          unsigned NumDimsToZero = getNumDims();
          assert(
              (!m_isNativeDBG || NumDimsToZero == MaxNumDims) &&
              "Debugger requires local/global_id in all dimensions to be valid");
          for (unsigned Dim = 0; Dim < NumDimsToZero; ++Dim) {
            createSetLocalId(Dim, m_Zero, B);
          }
          createSetCurrSBIndex(m_Zero, B);
          createSetCurrBarrierId(UniqueID, B);
          continue;
        }
        //This is a barrier/fiber instruction.
        // For the innermost loop, replace with the following code
        // if (LocalId.0 < GroupSize.0) {
        //   LocalId.0+=VecWidth
        //   switch (currBarrier) {
        //     case i: goto barrier_i;
        //   }
        // } else {
        //   LocalIdi.0 = 0;
        //   currBarrier = id
        //   if (LocalId.1 < GroupSize.1) {
        //    LocalId.1+=1
        //   } else {
        //    LocalId.1 = 0;
        //    if (LocalId.2 < GroupSize.2) {
        //     LocalId.2+=1
        //   }
        // }

        BarrierBBIdList BBId;
        // Create List of barrier label that may be jumped to
        DataPerBarrier::SBarrierRelated *pRelated = &m_pDataPerBarrier->getBarrierPredecessors(pInst);
        assert( !pRelated->m_hasFiberRelated && "We reach here only if function has no fiber!" );
        TInstructionSet *pSyncPreds = &pRelated->m_relatedBarriers;
        for (TInstructionSet::iterator ii = pSyncPreds->begin(),
                                       ie = pSyncPreds->end();
             ii != ie; ++ii) {
          Instruction *pSyncInst = *ii;
          unsigned int predId = m_pDataPerBarrier->getUniqueID(pSyncInst);
          BBId.push_back(
              std::make_pair(ConstantInt::get(*m_pContext, APInt(32, predId)),
                             pSyncInst->getParent()));
        }
        createBarrierLatch(pPreSyncBB, pSyncBB, BBId, UniqueID, DL);
    }

  }

  void Barrier::createBarrierKeyValues(Function* pFunc, bool hasNoInternalCalls) {
    SBarrierKeyValues* pBarrierKeyValues = &m_pBarrierKeyValuesPerFunction[pFunc];

    const auto AllocaAddrSpace = m_DL->getAllocaAddrSpace();

    pBarrierKeyValues->m_TheFunction = pFunc;
    unsigned NumDims = computeNumDim(pFunc);
    pBarrierKeyValues->m_NumDims = NumDims;
    Instruction* pInsertBefore = &*pFunc->getEntryBlock().begin();
    //Add currBarrier alloca
    pBarrierKeyValues->m_pCurrBarrierId = new AllocaInst(
      Type::getInt32Ty(*m_pContext), AllocaAddrSpace,
      "pCurrBarrier", pInsertBefore);

    //Will hold the index in special buffer and will be increased by stride size
    pBarrierKeyValues->m_pCurrSBIndex = new AllocaInst(
      m_sizeTType, AllocaAddrSpace, "pCurrSBIndex", pInsertBefore);

    if (!m_useTLSGlobals) {
      // get_local_id()
      pBarrierKeyValues->m_pLocalIdValues =
          new AllocaInst(m_LocalIdAllocTy->getElementType(), AllocaAddrSpace,
                         "pLocalIds", pInsertBefore);
    }

    //get_special_buffer()
    pBarrierKeyValues->m_pSpecialBufferValue = m_util.createGetSpecialBuffer(pInsertBefore);

    //get_local_size()
    for (unsigned i = 0; i < NumDims; ++i)
      pBarrierKeyValues->m_pLocalSize[i] = m_util.createGetLocalSize(i, pInsertBefore);

    unsigned int structureSize = m_pDataPerValue->getStrideSize(pFunc);
    pBarrierKeyValues->m_pStructureSizeValue =
      ConstantInt::get(m_sizeTType, APInt(m_uiSizeT, structureSize));
    pBarrierKeyValues->m_currVectorizedWidthValue =
        ConstantInt::get(m_sizeTType, m_util.getKernelVectorizationWidth(pFunc));
 }

  void Barrier::getBarrierKeyValues(Function* pFunc) {
    m_currFunction = pFunc;
    assert(m_pBarrierKeyValuesPerFunction.count(pFunc) &&
      "initiation of argument values is broken");
    m_currBarrierKeyValues = &m_pBarrierKeyValuesPerFunction[pFunc];
  }

  Instruction* Barrier::getInstructionToInsertBefore(Instruction *pInst, Instruction *pUserInst, bool expectNULL) {
    if (!isa<PHINode>(pUserInst)) {
      //pUserInst is not a PHINode, we can insert instruction before it.
      return pUserInst;
    }
    //pUserInst is a PHINode, find previous basic block
    BasicBlock *pPrevBB = BarrierUtils::findBasicBlockOfUsageInst(pInst, pUserInst);

    if ( expectNULL && pPrevBB == pInst->getParent() ) {
      //In such case no need to load & replace the origin value
      //as no barrier in the middle, return NULL to indecate that.
      return nullptr;
    }
    return pPrevBB->getTerminator();
  }

  Value* Barrier::getAddressInSpecialBuffer(
    unsigned int offset, PointerType *pType, Instruction *pInsertBefore, const DebugLoc* DB){
      Value *pOffsetVal = ConstantInt::get(m_sizeTType, APInt(m_uiSizeT, offset));
      //If hit this assert then need to handle PHINode!
      assert(!isa<PHINode>(pInsertBefore) && "cannot add instructions before a PHI node!" );
      IRBuilder<> B(pInsertBefore);
      if (DB)
        B.SetCurrentDebugLocation(*DB);
      //Calculate the pointer of the given offset for LocalId in the special buffer
      Value *CurrSB = createGetCurrSBIndex(B);
      CurrSB = B.CreateNUWAdd(CurrSB, pOffsetVal, "SB_LocalId_Offset");
      Value *Idxs[1] = { CurrSB };
      Value *pAddrInSBinBytes =
          B.CreateInBoundsGEP(m_currBarrierKeyValues->m_pSpecialBufferValue,
                              ArrayRef<Value *>(Idxs));
      //Bitcast pointer according to alloca type!
      Value *pAddrInSpecialBuffer =
          B.CreatePointerCast(pAddrInSBinBytes, pType, "pSB_LocalId");
      return pAddrInSpecialBuffer;
  }

  Instruction* Barrier::createOOBCheckGetLocalId(CallInst *pCall) {
    // if we are going in this path, then no chance that we can run less than 3D
    //
    // Create three basic blocks to contain the dim check as follows
    // entry: (old basic block tail)
    //   %0 = icmp ult i32 %dimndx, MAX_WORK_DIM
    //   br i1 %0, label %get.wi.properties, label %split.continue
    //
    // get.wi.properties:  (new basic block in case of in bound)
    //   ... ; load the property
    //   br label %split.continue
    //
    // split.continue:  (the second half of the splitted basic block head)
    //   %4 = phi i32 [ %res, %get.wi.properties ], [ out-of-bound-value, %entry ]

    BasicBlock *pBlock = pCall->getParent();
    Function *F = pBlock->getParent();
    // first need to split the current basic block to two BB's and create new BB
    BasicBlock *getWIProperties = BasicBlock::Create(*m_pContext, "get.wi.properties", F);
    BasicBlock *splitContinue = pBlock->splitBasicBlock(BasicBlock::iterator(pCall), "split.continue");

    // A.change the old basic block to the detailed entry
    // Entry:1. remove the unconditional jump instruction
    pBlock->getTerminator()->eraseFromParent();

    // Entry:2. add the entry tail code (as described up)
    {
      IRBuilder<> B(pBlock);
      ConstantInt *max_work_dim_i32 = ConstantInt::get(
          *m_pContext, APInt(32U, uint64_t(MaxNumDims), false));
      Value *checkIndex = B.CreateICmpULT(
          pCall->getArgOperand(0), max_work_dim_i32, "check.index.inbound");
      B.CreateCondBr(checkIndex, getWIProperties, splitContinue);
    }

    // B.Build the get.wi.properties block
    // Now retrieve address of the DIM count

    BranchInst::Create(splitContinue, getWIProperties);
    IRBuilder<> B(getWIProperties->getTerminator());
    B.SetCurrentDebugLocation(pCall->getDebugLoc());
    Value *LocalIds;
    if (m_useTLSGlobals) {
      LocalIds = m_LocalIds;
    } else {
      LocalIds = m_currBarrierKeyValues->m_pLocalIdValues;
    }
    Instruction *pResult =
        createGetLocalId(LocalIds, pCall->getArgOperand(0), B);

    // C.Create Phi node at the first of the splitted BB
    PHINode *pAttrResult = PHINode::Create(IntegerType::get(*m_pContext, m_uiSizeT), 2, "", splitContinue->getFirstNonPHI());
    pAttrResult->addIncoming(pResult, getWIProperties);
    // The overflow value
    pAttrResult->addIncoming(m_Zero, pBlock);

    return pAttrResult;
  }
  Value *Barrier::resolveGetLocalIDCall(CallInst *Call) {
    Value *Dimension = Call->getOperand(0);
    if (ConstantInt *C = dyn_cast<ConstantInt>(Dimension)) {
      uint64_t Dim = C->getZExtValue();
      if (Dim >= MaxNumDims) {
        // OpenCL Spec says to return zero for OOB dim value
        return m_Zero;
      }
      // assert(m_pBarrierKeyValuesPerFunction[pFunc].m_NumDims > Dim);
      IRBuilder<> B(Call);
      return createGetLocalId(Dim, B);
    }
    //assert(m_pBarrierKeyValuesPerFunction[pFunc].m_NumDims == MaxNumDims);
    return createOOBCheckGetLocalId(Call);
  }

  bool Barrier::fixGetWIIdFunctions(Module &M) {
    //clear container for new iteration on new function
    m_toRemoveInstructions.clear();

    std::string Name;
    //Find all get_local_id instructions
    TInstructionVector& getLIDInstructions = m_util.getAllGetLocalId();
    for (TInstructionVector::iterator ii = getLIDInstructions.begin(),
      ie = getLIDInstructions.end(); ii != ie; ++ii ) {
        CallInst *pOldCall = dyn_cast<CallInst>(*ii);
        assert( pOldCall && "Something other than CallInst is using get_local_id function!" );
        Function *pFunc = pOldCall->getParent()->getParent();
        if (!m_useTLSGlobals)
          getBarrierKeyValues(pFunc);
        else
          m_currFunction = pFunc;
        Value *LID = resolveGetLocalIDCall(pOldCall);
        pOldCall->replaceAllUsesWith(LID);
        m_toRemoveInstructions.push_back(pOldCall);
    }

    // Maps [Function, ConstDimension] --> BaseGlobalId
    typedef std::pair<Function *, ConstantInt *> FuncDimPair;
    std::map<FuncDimPair, Value *> FuncToBaseGID;
    //Find all get_global_id instructions
    TInstructionVector& getGIDInstructions = m_util.getAllGetGlobalId();
    for (TInstructionVector::iterator ii = getGIDInstructions.begin(),
      ie = getGIDInstructions.end(); ii != ie; ++ii ) {
        CallInst *pOldCall = dyn_cast<CallInst>(*ii);
        assert( pOldCall && "Something other than CallInst is using get_global_id function!" );
        Function *pFunc = pOldCall->getParent()->getParent();
        if (!m_useTLSGlobals)
          getBarrierKeyValues(pFunc);
        else
          m_currFunction = pFunc;
        Value *BaseGID = 0;
        Value *Dim = pOldCall->getOperand(0);
        // Computation of BaseGID: If the dimension is a constant, cache it and reuse in function
        if (ConstantInt *ConstDim = dyn_cast<ConstantInt>(Dim)) {
           FuncDimPair Key = std::make_pair(pFunc, ConstDim);
           Value *&Val = FuncToBaseGID[Key];
           if (!Val) {
             Val = m_util.createGetBaseGlobalId(Dim,
                                                &*pFunc->getEntryBlock().begin());
           }
           BaseGID = Val;
        } else
          BaseGID = m_util.createGetBaseGlobalId(Dim, pOldCall);
        Value *LID = resolveGetLocalIDCall(pOldCall);
        //Replace get_global_id(arg) with global_base_id + local_id
        Name = AppendWithDimension("GlobalID_", Dim);
        Value *GlobalID =
            BinaryOperator::CreateAdd(LID, BaseGID, Name, pOldCall);
        pOldCall->replaceAllUsesWith(GlobalID);
        m_toRemoveInstructions.push_back(pOldCall);
    }

    //Remove all instructions in m_toRemoveInstructions
    eraseAllToRemoveInstructions();

    return true;
  }

  void Barrier::fixNonInlineFunction(Function *pFuncToFix) {
    //TODO: do we need to set DebugLoc for these instructions?
    //Get key values for this functions.
    getBarrierKeyValues(pFuncToFix);

    unsigned int numOfArgs = pFuncToFix->getFunctionType()->getNumParams();
    //Use offsets instead of original parameters
    Function::arg_iterator argIter = pFuncToFix->arg_begin();
    for ( unsigned int i = 0; i < numOfArgs; ++i, ++argIter ) {
      Value *pArgVal = &*argIter;
      if ( m_pDataPerValue->hasOffset(pArgVal) ) {
        unsigned int offset = m_pDataPerValue->getOffset(pArgVal);
        fixArgumentUsage(pArgVal, offset);
      }
    }
    if ( m_pDataPerValue->hasOffset(pFuncToFix) ) {
      unsigned int offset = m_pDataPerValue->getOffset(pFuncToFix);

      std::vector<BasicBlock*> pVecBB;
      for ( Function::iterator bi = pFuncToFix->begin(), be = pFuncToFix->end(); bi != be; ++bi ) {
        BasicBlock *pBB = &*bi;
        pVecBB.push_back(pBB);
      }
      //Run over all basic blocks of the new function and handle return terminators
      for ( std::vector<BasicBlock*>::iterator bi = pVecBB.begin(), be = pVecBB.end(); bi != be; ++bi ) {
        BasicBlock *pBB = *bi;
        ReturnInst *pRetInst = dyn_cast<ReturnInst>(pBB->getTerminator());
        if ( !pRetInst ) {
          //It is not return instruction terminator, check next basic block
          continue;
        }
        Value *pRetVal = pRetInst->getOperand(0);
        Instruction *pNextInst;
        if (Instruction *pInst = dyn_cast<Instruction>(pRetVal)) {
          //Find next instruction so we can create new instruction before it
          pNextInst = &*(++BasicBlock::iterator(pInst));
          if ( isa<PHINode>(pNextInst) ) {
            //pNextInst is a PHINode, find first non PHINode to add instructions before it
            pNextInst = pNextInst->getParent()->getFirstNonPHI();
          }
        } else {
          //In this case the return value is not an instruction and
          //it cannot be assumed that it is inside the barrier loop.
          //Thus, need to create a new barrier loop that store this value
          //in the special buffer, that is why we needed to find the values:
          // m_pCurrSBIndex, m_pLocalIdValue, m_pWIIterationCountValue
          //Before:
          //  BB:
          //      ret pRetVal
          //After:
          //  BB:
          //      br loopBB
          //  loopBB:
          //      pSB[pCurrSBValue+offset] = pRetVal
          //      cond LocalId < IterCount
          //      LocalId++
          //      pCurrSBValue += Stride
          //      br cond, loopBB, RetBB
          //  RetBB:
          //      ret pRetVal
          BasicBlock *LoopBB =
            pBB->splitBasicBlock(BasicBlock::iterator(pRetInst), "LoopBB");
          BasicBlock *RetBB = LoopBB->splitBasicBlock(LoopBB->begin(), "RetBB");
          BarrierBBIdList BBId(
              1, std::make_pair(ConstantInt::get(*m_pContext, APInt(32, 0)),
                                LoopBB));
          DebugLoc DL = pRetInst->getDebugLoc();
          Value* UniqueID = 0;
          createBarrierLatch(LoopBB, RetBB, BBId, UniqueID, DL);

          pNextInst = LoopBB->getFirstNonPHI();
        }
        fixReturnValue(pRetVal, offset, pNextInst);
      }
    }
  }

  void Barrier::fixArgumentUsage(Value *pOriginalArg, unsigned int offsetArg) {
    //TODO: do we need to set DebugLoc for these instructions?
    assert( (!m_pDataPerValue->isOneBitElementType(pOriginalArg) ||
      !isa<VectorType>(pOriginalArg->getType())) && "pOriginalArg with base type i1!");
    TInstructionSet userInsts;
    for ( Value::user_iterator ui = pOriginalArg->user_begin(),
      ue = pOriginalArg->user_end(); ui != ue; ++ui) {
        Instruction *pUserInst = dyn_cast<Instruction>(*ui);
        userInsts.insert(pUserInst);
    }
    for (TInstructionSet::iterator ui = userInsts.begin(), ue = userInsts.end();
         ui != ue; ++ui) {
      Instruction *pUserInst = *ui;
      assert(pUserInst &&
             "Something other than Instruction is using function argument!");
      Instruction *pInsertBefore = pUserInst;
      if (isa<PHINode>(pUserInst)) {
        BasicBlock *pPrevBB =
            BarrierUtils::findBasicBlockOfUsageInst(pOriginalArg, pUserInst);
        pInsertBefore = pPrevBB->getTerminator();
      }
      // In this case we will always get a valid offset and need to load the
      // argument from the special buffer using the offset corresponding argument
      PointerType *pType =
          pOriginalArg->getType()->getPointerTo(SPECIAL_BUFFER_ADDR_SPACE);
      Value *pAddrInSpecialBuffer =
          getAddressInSpecialBuffer(offsetArg, pType, pInsertBefore, nullptr);
      Value *pLoadedValue =
          new LoadInst(pOriginalArg->getType(), pAddrInSpecialBuffer,
                       "loadedValue", pInsertBefore);
      pUserInst->replaceUsesOfWith(pOriginalArg, pLoadedValue);
    }
  }

  void Barrier::fixReturnValue(Value *pRetVal, unsigned int offsetRet, Instruction* pInsertBefore) {
    //TODO: do we need to set DebugLoc for these instructions?
    assert( (!m_pDataPerValue->isOneBitElementType(pRetVal) ||
      !isa<VectorType>(pRetVal->getType())) && "pRetVal with base type i1!");
    // pRetVal might be a result of calling other function itself
    // in such case no need to handle it here as it will be saved
    // to the special buffer by the called function itself.
    // Calculate the pointer of the current special in the special buffer
    PointerType *pType =
        pRetVal->getType()->getPointerTo(SPECIAL_BUFFER_ADDR_SPACE);
    Value *pAddrInSpecialBuffer =
        getAddressInSpecialBuffer(offsetRet, pType, pInsertBefore, nullptr);
    //Add Store instruction after the value instruction
    new StoreInst(pRetVal, pAddrInSpecialBuffer, pInsertBefore);
  }

  void Barrier::fixCallInstruction(CallInst *pCallToFix) {
    Function* pCalledFunc = pCallToFix->getCalledFunction();
    assert(pCalledFunc && "Call instruction has no called function");
    Function* pFunc = pCallToFix->getParent()->getParent();

    //Get key values for this functions.
    getBarrierKeyValues(pFunc);

    const DebugLoc& DB = pCallToFix->getDebugLoc();
    Instruction* pInsertBefore = nullptr;
    Function::arg_iterator argIter = pCalledFunc->arg_begin();
    for ( CallInst::const_op_iterator opi = pCallToFix->arg_begin(),
      ope = pCallToFix->arg_end(); opi != ope; ++opi, ++argIter) {
        if ( !m_pDataPerValue->hasOffset(&*argIter) ) continue;

        if ( !pInsertBefore ) {
          //Split sync instruction basic-block that contains the call instruction
          BasicBlock* pPreBB = pCallToFix->getParent();
          BasicBlock::iterator firstInst = pPreBB->begin();
          assert( m_pDataPerBarrier->getSyncInstructions(pFunc).count(&*firstInst) &&
            "assume first instruction to be sync instruction" );
          BasicBlock *callBB = pPreBB->splitBasicBlock(firstInst, "CallBB");
          pInsertBefore = pPreBB->getTerminator();
          m_oldToNewSyncBBMap[pFunc][pPreBB] = callBB;
        }
        //Need to handle operand
        Value* pOpVal = *opi;
        unsigned int offset = m_pDataPerValue->getOffset(&*argIter);

        //Calculate the pointer of the current special in the special buffer
        PointerType *pType = pOpVal->getType()->getPointerTo(SPECIAL_BUFFER_ADDR_SPACE);
        Value *pAddrInSpecialBuffer = getAddressInSpecialBuffer(offset, pType, pInsertBefore, &DB);
        //Add Store instruction before the synchronize instruction (in the pre basic block)
        StoreInst* pStoreInst = new StoreInst(pOpVal, pAddrInSpecialBuffer, pInsertBefore);
        pStoreInst->setDebugLoc(DB);
    }
    //Check if return value has usages
    if ( !pCallToFix->getNumUses() ) return;

    if ( !m_pDataPerValue->hasOffset(pCalledFunc) ) return;
    //Need to handle return value

    //Validate that next basic block is a synchronize basic block
    BasicBlock* pCallBB = pCallToFix->getParent();
    BranchInst* pBrInst = dyn_cast<BranchInst>(pCallBB->getTerminator());
    assert(pBrInst && pBrInst->getNumSuccessors() == 1 && "callInst BB has more than one successor");
    BasicBlock::iterator firstInst = pBrInst->getSuccessor(0)->begin();
    assert( m_pDataPerBarrier->getSyncInstructions(pFunc).count(&*firstInst) &&
            "assume first instruction to be sync instruction" );
    //Find next instruction so we can create new instruction before it
    Instruction *pNextInst = &*(++firstInst);

    unsigned int offset = m_pDataPerValue->getOffset(pCalledFunc);

    //Calculate the pointer of the current special in the special buffer
    PointerType *pType = pCallToFix->getType()->getPointerTo(SPECIAL_BUFFER_ADDR_SPACE);
    Value *pAddrInSpecialBuffer = getAddressInSpecialBuffer(offset, pType, pNextInst, &DB);
    //Add Load instruction from special buffer at function offset
    LoadInst *pLoadedValue = new LoadInst(
        pCallToFix->getType(), pAddrInSpecialBuffer, "loadedValue", pNextInst);
    pLoadedValue->setDebugLoc(DB);

    if ( m_pDataPerValue->hasOffset(pCallToFix) ) {
      //CallInst return value has an offset in the special buffer
      //Store the value to this offset.
      unsigned int offsetRet = m_pDataPerValue->getOffset(pCallToFix);

      //Calculate the pointer of the current special in the special buffer
      Value *pAddrInSpecialBuffer = getAddressInSpecialBuffer(offsetRet, pType, pNextInst, &DB);
      //Add Store instruction to special buffer at return value offset
      StoreInst* pStoreInst = new StoreInst(pLoadedValue, pAddrInSpecialBuffer, pNextInst);
      pStoreInst->setDebugLoc(DB);
    }
    else {
      pCallToFix->replaceAllUsesWith(pLoadedValue);
    }
  }

  void Barrier::eraseAllToRemoveInstructions() {
    //Remove all instructions in m_toRemoveInstructions
    for( TInstructionVector::iterator ii = m_toRemoveInstructions.begin(),
      ie = m_toRemoveInstructions.end(); ii != ie; ++ii ) {
        Instruction *pInst = dyn_cast<Instruction>(*ii);
        assert( pInst && "remove instruction container contains non instruction!" );
        pInst->eraseFromParent();
    }
  }

  unsigned Barrier::computeNumDim(Function *F) {
    auto maxWGDimMDApi = KernelInternalMetadataAPI(F).MaxWGDimensions;
    if (maxWGDimMDApi.hasValue()) {
      return maxWGDimMDApi.get();
    }
    return MaxNumDims;
  }
  // use DFS to calculate a function's non-barrier memory usage.
  static
  unsigned getPrivateSize(Function* pFunc, const CallGraph& cg, DataPerValue* dataPerVal,
                          DenseMap<Function *, uint64_t> &addrAllocaSize,
                          llvm::DenseMap<Function*, unsigned>& fnPrivSize,
                          TFunctionSet& fnsWithSync) {
    // external function or function pointer
    if (!pFunc || pFunc->isDeclaration())
      return 0;
    if (fnPrivSize.count(pFunc))
      return fnPrivSize[pFunc];
    unsigned maxSubPrivSize = 0;
    const CallGraphNode *nodeCG = cg[pFunc];
    for (auto &ci : *nodeCG) {
      Function *calledFunc = ci.second->getFunction();
      maxSubPrivSize =
          std::max(maxSubPrivSize,
                   getPrivateSize(calledFunc, cg, dataPerVal, addrAllocaSize,
                                  fnPrivSize, fnsWithSync));
    }
    fnPrivSize[pFunc] = maxSubPrivSize +
        (addrAllocaSize.count(pFunc) ? addrAllocaSize[pFunc] : 0) +
        (fnsWithSync.count(pFunc) ? 0 : dataPerVal->getStrideSize(pFunc));

    return fnPrivSize[pFunc];
  }

  void Barrier::updateStructureStride(Module & M, TFunctionSet& functionsWithSync) {
    // collect functions to process
    CallGraph cg{M};
    llvm::DenseMap<Function*, unsigned> funcToPrivSize;
    auto TodoList = BarrierUtils::getAllKernelsAndVectorizedCounterparts(
        KernelList(&M).getList());

    // Get the kernels using the barrier for work group loops.
    for (auto pFunc : TodoList) {
      auto kimd = KernelInternalMetadataAPI(pFunc);
      // Need to check if Vectorized Width Value exists, it is not guaranteed
      // that  Vectorized is running in all scenarios.
      int vecWidth =
          kimd.VectorizedWidth.hasValue() ? kimd.VectorizedWidth.get() : 1;
      unsigned int strideSize = m_pDataPerValue->getStrideSize(pFunc);
      assert(vecWidth && "vecWidth should not be 0!");
      strideSize = (strideSize + vecWidth - 1) / vecWidth;

      auto privateSize =
          getPrivateSize(pFunc, cg, m_pDataPerValue, m_addrAllocaSize,
                         funcToPrivSize, functionsWithSync);
      //Need to check if NoBarrierPath Value exists, it is not guaranteed that
      //KernelAnalysisPass is running in all scenarios.
      // CSSD100016517, CSSD100018743: workaround
      // Private memory is always considered to be non-uniform. I.e. it is not shared by each WI per vector lane.
      // If it is uniform (i.e. its content doesn't depend on non-uniform values) the private memory
      // query returns a smaller value than actual private memory usage. This subtle is taken into account
      // in the query for the maximum work-group.
      if (kimd.NoBarrierPath.hasValue() && kimd.NoBarrierPath.get()) {
        kimd.BarrierBufferSize.set(0);
        // if there are no barrier in the kernel, strideSize is the kernel
        // body's private memory usage. So need to add sub-function's memory size.
        kimd.PrivateMemorySize.set(strideSize + privateSize -
                                   m_pDataPerValue->getStrideSize(pFunc));
      } else {
        kimd.BarrierBufferSize.set(strideSize);
        // if there are some barriers in the kernel, stiderSize is barrier
        // buffer size. So need to add non barrier private memory.
        kimd.PrivateMemorySize.set(strideSize + privateSize);
      }
    }
  }
} // namespace intel


/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
void *createBarrierPass(bool isNativeDebug, bool useTLSGlobals) {
  return new intel::Barrier(isNativeDebug, useTLSGlobals);
  }

  void getBarrierPassStrideSize(Pass *pPass, std::map<std::string, unsigned int>& bufferStrideMap) {
    ((intel::Barrier*)pPass)->getStrideMap(bufferStrideMap);
  }
}
