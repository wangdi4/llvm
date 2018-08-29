// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#include "ImplicitGIDPass.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"

namespace intel {

char ImplicitGlobalIdPass::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(ImplicitGlobalIdPass, "B-ImplicitGlobalIdPass", "Implicit Global Id Pass - Add parameters for native (gdb) debugging", false, true)
OCL_INITIALIZE_PASS_DEPENDENCY(DataPerBarrier)
OCL_INITIALIZE_PASS_END(ImplicitGlobalIdPass, "B-ImplicitGlobalIdPass", "Implicit Global Id Pass - Add parameters for native (gdb) debugging", false, true)

ImplicitGlobalIdPass::ImplicitGlobalIdPass() :
    ModulePass(ID), m_pDataPerBarrier(nullptr), m_pModule(nullptr),
    m_pContext(nullptr), m_pSyncInstSet(nullptr)
{
    initializeImplicitGlobalIdPassPass(*llvm::PassRegistry::getPassRegistry());
}

bool ImplicitGlobalIdPass::runOnModule(Module& M)
{
    m_pModule = &M;
    m_pDIB = std::auto_ptr<DIBuilder>(new DIBuilder(M));

    m_pContext = &M.getContext();

    m_pDataPerBarrier = &getAnalysis<DataPerBarrier>();

    m_util.init(&M);

    // Prime a DebugInfoFinder that can be queried about various bits of
    // debug information in the module.
    m_DbgInfoFinder = DebugInfoFinder();
    m_DbgInfoFinder.processModule(M);

    for ( Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi) {
      runOnFunction(*fi);
    }

    return true;
}

bool ImplicitGlobalIdPass::runOnFunction(Function& F)
{
  // Skip all functions without debug info (including built-ins, externals, etc)
  // We can't do anything for them
  if (!F.getSubprogram())
    return false;

  m_pSyncInstSet = 0;
  if (m_pDataPerBarrier->hasSyncInstruction(&F))
    m_pSyncInstSet = &m_pDataPerBarrier->getSyncInstructions(&F);
  insertComputeGlobalIds(&F);
  return true;
}

void ImplicitGlobalIdPass::runOnBasicBlock(unsigned i, Instruction *pGIDAlloca, Instruction *insertBefore)
{
  IRBuilder<> B(insertBefore);
  // **********************************************************************
  // DO NOT ADD DEBUG INFO TO THE CODE WHICH GENERATES GET_GLOBAL_ID AND THE
  // STORE, OR THE DEBUGGER WILL NOT BREAK AT A GIVEN GLOBAL_ID
  B.SetCurrentDebugLocation(DebugLoc());
  // **********************************************************************

  Value *gid_at_dim = m_util.createGetGlobalId(i, B);

  // get_global_id returns size_t, but we want to always pass a 64-bit
  // number. So we may need to extend the result to 64 bits.
  //
  const IntegerType *gid_type = dyn_cast<IntegerType>(gid_at_dim->getType());
  if (gid_type && gid_type->getBitWidth() != 64) {
    Value *zext_gid =
        B.CreateZExt(gid_at_dim, IntegerType::getInt64Ty(*m_pContext),
                     Twine("gid") + Twine(i) + Twine("_i64"));
    gid_at_dim = zext_gid;
  }
  B.CreateStore(gid_at_dim, pGIDAlloca);
}

void ImplicitGlobalIdPass::insertComputeGlobalIds(Function* pFunc)
{
    const bool functionHasBarriers = m_pSyncInstSet != 0;
    DIScope *scope = nullptr;
    DebugLoc loc;

    // Find the first basic block that has an instruction with debug
    // metadata attached to it (usually a "call void @llvm.dbg.declare"
    // or an LLVM instruction with a suffix like "!dbg !27").
    //
    auto func = [&](const BasicBlock& b) {
      return getBBScope(b, &scope, loc);
    };
    auto I = std::find_if(pFunc->begin(), pFunc->end(), func);

    assert( I != pFunc->end() &&
            "Failed to find at least one instruction with debug metadata attached to it. "
            "The gid variables require a valid scope and location information in order "
            "to be inserted into the function." );
    (void)I;

    // Find the first and second instructions in the function
    //
    BasicBlock& entry_block = pFunc->getEntryBlock();
    Instruction* first_instr = &(entry_block.front());

    // Insert after first instruction in the case of barrier calls otherwise
    // this breaks barrier pass (need to investigate why). In this case, expect
    // basic block to have more then 1 IR instruction.
    //
    Instruction* insert_before = functionHasBarriers ?
      dyn_cast<Instruction>(&*(++BasicBlock::iterator(first_instr))) :
      first_instr;
    assert( insert_before && "There is only one instruction in the current basic block!" );

    for (unsigned i = 0; i <= 2; ++i) {
      // Create implicit local variables to hold the gids
      //
      DIBuilder *diBuilder = m_pDIB.get();
      Twine gid_name = Twine("__ocl_dbg_gid") + Twine(i);
      AllocaInst *gid_alloca = new AllocaInst(
          IntegerType::getInt64Ty(*m_pContext), 0, gid_name, insert_before);
      auto File = diBuilder->createFile("", "");
      DILocalVariable *div = diBuilder->createAutoVariable(
          scope, StringRef(gid_name.str()),
        File, 1, getOrCreateUlongDIType(), true, DINode::FlagArtificial);

      // LLVM 3.6 UPGRADE: TODO: uncomment the line below if the new DIVariable
      // does need dwarf::DW_OP_deref expression
      (void) diBuilder->insertDeclare(gid_alloca,
                                      div,
                                      //diBuilder->createExpression(dwarf::DW_OP_deref),
                                      diBuilder->createExpression(),
                                      loc,
                                      insert_before);
      if (!functionHasBarriers) {
        runOnBasicBlock(i, gid_alloca, insert_before);
      } else {
        for (TInstructionSet::iterator ii = m_pSyncInstSet->begin(),
                                       ie = m_pSyncInstSet->end();
             ii != ie; ++ii) {
          Instruction *CallBarrier = *ii;
          Instruction *pNextInst = insert_before;
          if (CallBarrier->getParent() != gid_alloca->getParent()) {
            // Insert right after the barrier()
            pNextInst = &*(++BasicBlock::iterator(*ii));
          }
          runOnBasicBlock(i, gid_alloca, pNextInst);
        }
      }
    }
}

bool ImplicitGlobalIdPass::getBBScope(const BasicBlock& BB, DIScope** scope_out, DebugLoc& loc_out)
{
    for (BasicBlock::const_iterator BI = BB.begin(), BE = BB.end(); BI != BE; ++BI)
    {
        DebugLoc loc = BI->getDebugLoc();
        if (!loc)
            continue;
        assert(dyn_cast<DIScope>(loc.getScope()) && "DIScope is expected");
        DIScope *scope = dyn_cast<DIScope>(loc.getScope());
        if (dyn_cast<DILexicalBlock>(scope) || dyn_cast<DISubprogram>(scope)) {
            *scope_out = scope;
            loc_out = loc;
            return true;
        }
    }
    return false;
}

DIType * ImplicitGlobalIdPass::getOrCreateUlongDIType() const
{
    for ( auto const& t: m_DbgInfoFinder.types()) {
        if (t->getName() == "long unsigned int") {
            return t;
        }
    }
    // If the type wasn't found, create it now
    DIBuilder *diBuilder = m_pDIB.get();
    return diBuilder->createBasicType("long unsigned int", 64, dwarf::DW_ATE_unsigned);
}


}

extern "C" {
  void* createImplicitGIDPass() {
    return new intel::ImplicitGlobalIdPass();
  }
}
