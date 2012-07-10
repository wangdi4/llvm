/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ImplicitGIDPass.cpp

\*****************************************************************************/

#include "ImplicitGIDPass.h"

namespace intel {

char ImplicitGlobalIdPass::ID = 0;

ImplicitGlobalIdPass::ImplicitGlobalIdPass() : ModulePass(ID) {}

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
  if (!F.isDeclaration()) {
      m_pSyncInstSet = &m_pDataPerBarrier->getSyncInstructions(&F);
      insertComputeGlobalIds(&F);
  }
  return true;
}

void ImplicitGlobalIdPass::runOnBasicBlock(unsigned i, Instruction *pGIDAlloca, Instruction *insertBefore)
{
    Value* gid_at_dim = m_util.createGetGlobalId(i, insertBefore);

    // get_global_id returns size_t, but we want to always pass a 64-bit
    // number. So we may need to extend the result to 64 bits.
    //
    const IntegerType* gid_type = dyn_cast<IntegerType>(gid_at_dim->getType());
    if (gid_type && gid_type->getBitWidth() != 64) {
        Value* zext_gid = new ZExtInst(
            gid_at_dim,
            IntegerType::getInt64Ty(*m_pContext),
            Twine("gid") + Twine(i) + Twine("_i64"),
            insertBefore);
        gid_at_dim = zext_gid;
    }
    new StoreInst(gid_at_dim, pGIDAlloca, insertBefore);
}

void ImplicitGlobalIdPass::insertComputeGlobalIds(Function* pFunc)
{
    bool functionHasBarriers = !m_pSyncInstSet->empty();
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

    // Prepare to create debug metadata for implicit gid variables
    //
    DIDescriptor scope;
    DebugLoc loc;
    getBBScope(entry_block, scope, loc);

    DIType gid_di_type = getOrCreateUlongDIType();

    for (unsigned i = 0; i <= 2; ++i) {
        // Create implicit local variables to hold the gids
        //
        Twine gid_name = Twine("__ocl_dbg_gid") + Twine(i);
        AllocaInst* gid_alloca = new AllocaInst(IntegerType::getInt64Ty(*m_pContext),
                                           0, gid_name, insert_before);
        DIVariable div = m_pDIB->createLocalVariable(dwarf::DW_TAG_auto_variable, scope,
                                                     StringRef(gid_name.str()), DIFile(0), 1, gid_di_type,
                                                     true, DIDescriptor::FlagArtificial, 0);
        Instruction* gid_declare = m_pDIB->insertDeclare(gid_alloca, div, insert_before);
        gid_declare->setDebugLoc(loc);
        if (m_pSyncInstSet->empty()) {
          runOnBasicBlock(i, gid_alloca, insert_before);
        } else {
	  for (TInstructionSet::iterator ii = m_pSyncInstSet->begin(),
		 ie = m_pSyncInstSet->end(); ii != ie; ++ii ) {
            Instruction *pNextInst = dyn_cast<Instruction>(&*(++BasicBlock::iterator(*ii)));
	    runOnBasicBlock(i, gid_alloca, pNextInst);
          }
        }
    }
}

bool ImplicitGlobalIdPass::getBBScope(const BasicBlock& BB, DIDescriptor& scope_out, DebugLoc& loc_out)
{
    for (BasicBlock::const_iterator BI = BB.begin(), BE = BB.end(); BI != BE; ++BI)
    {
        DebugLoc loc = BI->getDebugLoc();
        if (loc.isUnknown())
            continue;
        LLVMContext &context = BI->getContext();
        DIDescriptor scope(loc.getScope(context));
        if (scope.isLexicalBlock()) {
            scope_out = scope;
            loc_out = loc;
            return true;
        }
    }
    return false;
}

DIType ImplicitGlobalIdPass::getOrCreateUlongDIType()
{
    for (DebugInfoFinder::iterator t_i = m_DbgInfoFinder.type_begin(),
         t_end = m_DbgInfoFinder.type_end();
         t_i != t_end; ++t_i) {
        MDNode* t_mdn = *t_i;
        DIType t(t_mdn);

        if (t.getName() == "long unsigned int") {
            return t;
        }
    }
    // If the type wasn't found, create it now
    return m_pDIB->createBasicType("long unsigned int", 64, 64, dwarf::DW_ATE_unsigned);
}

  // Register this pass...
  static RegisterPass<ImplicitGlobalIdPass> IDIP("B-ImplicitGlobalIdPass",
    "Implicit Global Id Pass - Add parameters for native (gdb) debugging", false, true);

}

extern "C" {
  void* createImplicitGIDPass() {
    return new intel::ImplicitGlobalIdPass();
  }
}
