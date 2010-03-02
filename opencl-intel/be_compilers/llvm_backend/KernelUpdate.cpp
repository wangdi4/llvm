/////////////////////////////////////////////////////////////////////////
// BuiltInFunctionImport.cpp:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2009 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related 
// to the source code ("Material") are owned by Intel Corporation or its 
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and 
// proprietary and confidential information of Intel Corporation and its 
// suppliers and licensors, and is protected by worldwide copyright and trade 
// secret laws and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, distributed, 
// or disclosed in any way without Intel’s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////

// This pass is used for import of built-in functions from runtime module

#include "stdafx.h"
#include "cpu_dev_limits.h"
#include "cl_device_api.h"
#include "llvm_backend.h"
#include "Vectorizer.h"

#include <llvm/Pass.h>
#include <llvm/Module.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Instructions.h>
#include <llvm/Constants.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/transforms/utils/Cloning.h>


#include <map>

using namespace llvm;
using namespace std;
using namespace Intel::OpenCL::DeviceBackend;


namespace Intel { namespace OpenCL { namespace DeviceBackend {

	// Update kernel assembly to match our execution environment
	class KernelUpdate : public ModulePass
	{
	public:
		KernelUpdate(Vectorizer *pVect, SmallVectorImpl<Function*> &vectFunctions) : 
		  ModulePass(&ID) , m_pVectorizer (pVect) , m_pVectFunctions (&vectFunctions) {}

		// doPassInitialization - For this pass, it removes global symbol table
		// entries for primitive types.  These are never used for linking in GCC and
		// they make the output uglier to look at, so we nuke them.
		//
		// Also, initialize instance variables.
		//
		bool runOnModule(Module &M);

	protected:
		static char ID; // Pass identification, replacement for typeid

		Module*						m_pModule;
		ConstantArray*				m_pAnnotations;
		Vectorizer*					m_pVectorizer;
		SmallVectorImpl<Function*>*	m_pVectFunctions;

		Function *RunOnKernel(Function *pFunc, ConstantArray* pFuncLocals);
		bool	IsAKernel(const Function* pFunc);
		bool	ParseLocalBuffers(ConstantArray* pFuncLocals, Argument* pLocalMem);

		void	AddWIInfoDeclarations();
		void	AddBarrierDeclaration();
		void	AddAsyncCopyDeclaration();
		void	AddPrefetchDeclaration();
		bool	m_bBarrierDecl;
		bool	m_bAsyncCopyDecl;
		bool	m_bPrefetchDecl;

		TLLVMKernelInfo	m_sInfo;
		map<const Function*, TLLVMKernelInfo>	m_mapKernelInfo;

		// Calculate and return Global ID value for given dimension
		Value* SubstituteWIcall(llvm::CallInst *pCall,
			llvm::Argument* pWorkInfo, llvm::Argument* pWGid,
			llvm::Argument* pBaseGlbId, llvm::Argument* pLocalId);
		Value*	CalcGlobalId(llvm::CallInst *pCall, llvm::Argument* pWGinfo, llvm::Argument* pLocalId, bool bGlobal);
		Value*	m_GlbIds[MAX_WORK_DIM];

		// Barrier/AsyncCopy/Wait
		void	AddCtxAddress(llvm::CallInst* pCall, llvm::Argument* pLocalId);
		void	UpdateBarrier(llvm::CallInst* pCall, llvm::Argument* pLocalId);
		Value*	UpdateAsyncCopy(llvm::CallInst* pCall, llvm::Argument* pLocalId);
		void	UpdateWaitGroup(llvm::CallInst* pCall, llvm::Argument* pLocalId);
		void	UpdatePrefetch(llvm::CallInst* pCall);
		Value*	m_pCtxPtr;

		friend 	void getKernelInfoMap(ModulePass *pKUPath, map<const Function*, TLLVMKernelInfo>& infoMap);

	};

	char KernelUpdate::ID = 0;

	ModulePass *createKernelUpdatePass(Pass* pVect, SmallVectorImpl<Function*> &vectFunctions) {
		return new KernelUpdate((Vectorizer*)pVect, vectFunctions);
	}

	void getKernelInfoMap(ModulePass *pKUPath, map<const Function*, TLLVMKernelInfo>& infoMap)
	{
		KernelUpdate* pKU = dynamic_cast<KernelUpdate*>(pKUPath);

		infoMap.clear();
		if ( NULL != pKU )
		{
			infoMap.insert(pKU->m_mapKernelInfo.begin(), pKU->m_mapKernelInfo.end());
		}
	}

	bool KernelUpdate::runOnModule(Module &M)
	{
		m_pModule = &M;

		m_bBarrierDecl = false;
		m_bAsyncCopyDecl = false;
		m_bPrefetchDecl = false;

		m_mapKernelInfo.clear();

		AddWIInfoDeclarations();

		// Extract pointer to module annotations
		GlobalVariable *annotation = m_pModule->getGlobalVariable("llvm.global.annotations");
		if ( NULL == annotation )
		{
			return false;
		}

		m_pAnnotations = dyn_cast<ConstantArray>(annotation->getInitializer());
		
		if ( NULL != m_pVectorizer )
		{
			getVectorizerFunctions(m_pVectorizer, *m_pVectFunctions);
		}

		// now we need to pass every kernel and make relevant changes to match our environment
		for (unsigned i = 0, e = m_pAnnotations->getType()->getNumElements(); i != e; ++i) 
		{
			// Obtain kernel function from annotation
			ConstantStruct *elt = cast<ConstantStruct>(m_pAnnotations->getOperand(i));
			Function *pFunc = dyn_cast<Function>(elt->getOperand(0)->stripPointerCasts());
			if ( NULL == pFunc )
			{
				continue;	// Not a function pointer
			}

			// Obtain local variables array
			GlobalVariable* pGlbVar = dyn_cast<GlobalVariable>(elt->getOperand(3)->stripPointerCasts());
			if ( NULL == pGlbVar )
			{
				return false;
			}
			ConstantArray *pFuncLocals = dyn_cast<ConstantArray>(pGlbVar->getInitializer());

			// Run on original kernel
			Function* pNewFunc = RunOnKernel(pFunc, pFuncLocals);
			assert(pNewFunc);

			// Run on vectorized kernel if available
			Function* pVectFunc;
			if ( !m_pVectFunctions->empty() && ( ( pVectFunc = (*m_pVectFunctions)[i]) != NULL ) )
			{
				(*m_pVectFunctions)[i] = RunOnKernel(pVectFunc, pFuncLocals);
			}
		}

		return true;
	}

	Instruction* CreateInstrFromConstantExpr(ConstantExpr* pCE, Value* From, Value* To)
	{  
		Instruction *Replacement = 0;

		if (pCE->getOpcode() == Instruction::GetElementPtr)
		{
			SmallVector<Value*, 8> Indices;
			Value *Pointer = pCE->getOperand(0);
			Indices.reserve(pCE->getNumOperands()-1);
			if (Pointer == From) Pointer = To;

			for (unsigned i = 1, e = pCE->getNumOperands(); i != e; ++i) {
				Value *Val = pCE->getOperand(i);
				if (Val == From) Val = To;
				Indices.push_back(Val);
			}
			Replacement = GetElementPtrInst::Create(Pointer, Indices.begin(), Indices.end());
		}
		else if (pCE->getOpcode() == Instruction::ExtractValue)
		{
			Value *Agg = pCE->getOperand(0);
			if (Agg == From) Agg = To;

			const SmallVector<unsigned, 4> &Indices = pCE->getIndices();
			Replacement = ExtractValueInst::Create(Agg,	Indices.begin(), Indices.end());
		}
		else if (pCE->getOpcode() == Instruction::InsertValue)
		{
			Value *Agg = pCE->getOperand(0);
			Value *Val = pCE->getOperand(1);
			if (Agg == From) Agg = To;
			if (Val == From) Val = To;

			const SmallVector<unsigned, 4> &Indices = pCE->getIndices();
			Replacement = InsertValueInst::Create(Agg, Val, Indices.begin(), Indices.end());
		}
		else if (pCE->isCast())
		{
			assert(pCE->getOperand(0) == From && "Cast only has one use!");
			Replacement = CastInst::Create((Instruction::CastOps)pCE->getOpcode(), To, pCE->getType());
		}
		else if (pCE->getOpcode() == Instruction::Select)
		{
			Value *C1 = pCE->getOperand(0);
			Value *C2 = pCE->getOperand(1);
			Value *C3 = pCE->getOperand(2);
			if (C1 == From) C1 = To;
			if (C2 == From) C2 = To;
			if (C3 == From) C3 = To;
			Replacement = SelectInst::Create(C1, C2, C3);
		}
		else if (pCE->getOpcode() == Instruction::ExtractElement)
		{
			Value *C1 = pCE->getOperand(0);
			Value *C2 = pCE->getOperand(1);
			if (C1 == From) C1 = To;
			if (C2 == From) C2 = To;
			Replacement = new ExtractElementInst(C1, C2);
		}
		else if (pCE->getOpcode() == Instruction::InsertElement)
		{
			Value *C1 = pCE->getOperand(0);
			Value *C2 = pCE->getOperand(1);
			Value *C3 = pCE->getOperand(1);
			if (C1 == From) C1 = To;
			if (C2 == From) C2 = To;
			if (C3 == From) C3 = To;
			Replacement = InsertElementInst::Create(C1, C2, C3);
		}
		else if (pCE->getOpcode() == Instruction::ShuffleVector)
		{
			Value *C1 = pCE->getOperand(0);
			Value *C2 = pCE->getOperand(1);
			Value *C3 = pCE->getOperand(2);
			if (C1 == From) C1 = To;
			if (C2 == From) C2 = To;
			if (C3 == From) C3 = To;
			Replacement = new ShuffleVectorInst(C1, C2, C3);
		}
		else if (pCE->isCompare())
		{
			Value *C1 = pCE->getOperand(0);
			Value *C2 = pCE->getOperand(1);
			if (C1 == From) C1 = To;
			if (C2 == From) C2 = To;
			Replacement = CmpInst::Create((Instruction::OtherOps)pCE->getOpcode(), pCE->getPredicate(), C1, C2);
		}
		else
		{
			assert(0 && "Unknown ConstantExpr type!");
			return NULL;
		}

		return Replacement;
	}

	bool ChangeConstantExpression(ConstantArray* pFuncLocals, Value* pTheValue, Value* pUser, Instruction* pBC)
	{
		// We need substitute constant expression with real instruction
		ConstantExpr* pCE = dyn_cast<ConstantExpr>(pUser);
		if ( NULL == pCE )
		{
			return false;
		}

		Instruction* pInst = CreateInstrFromConstantExpr(pCE, pTheValue, pBC);
		// Change all non-constant references
		ConstantExpr::use_iterator itCE = pCE->use_begin();
		while( itCE != pCE->use_end() )
		{
			Use &W = itCE.getUse();
			User* pLclUser = W.getUser();

			if ( pLclUser == pFuncLocals)
			{
				// Skip reference to the annotations
				++itCE;
				continue;
			}

			ConstantExpr* pLclCE = dyn_cast<ConstantExpr>(pLclUser);
			if ( NULL != pLclCE )
			{
				if ( ChangeConstantExpression(pFuncLocals, pLclUser, pLclUser, pInst) )
				{
					itCE = pCE->use_begin();		// Restart the scan
				}
				else
				{
					++itCE;
				}
				continue;	// continue to next usage
			}

			// Check if user is an instruction that belongs to the same function
			Instruction* pUsrInst = dyn_cast<Instruction>(pLclUser);
			if ( (NULL == pUsrInst) || (pUsrInst->getParent()->getParent() != pBC->getParent()->getParent()) )
			{
				++itCE;
				continue;
			}

			// Add instruction to the block, only the first time
			if ( pInst->use_empty() )
			{
				pInst->insertAfter(pBC);
			}
			W.set(pInst);
			itCE = pCE->use_begin();		// Restart the scan
		}

		// Check if the instruction was not used
		if ( pInst->use_empty() )
		{
			delete pInst;
		}

		if ( !pCE->use_empty() )
		{
			return false;
		}

		// No more references to the constant expression we can delete it
		delete pCE;
		return true;
	}

	// Substitutes a pointer to local buffer, with argument passed within kernel parameters
	bool KernelUpdate::ParseLocalBuffers(ConstantArray* pFuncLocals, Argument* pLocalMem)
	{
		Function* pFunc = pLocalMem->getParent();

		// Create code for local buffer
		Instruction* pFirstInst = dyn_cast<Instruction>(pFunc->getEntryBlock().begin());

		// Iterate through local buffers
		for (unsigned i = 0, e = pFuncLocals->getType()->getNumElements(); i != e; ++i) 
		{
			// Obtain kernel function from annotation
			Constant *elt = cast<Constant>(pFuncLocals->getOperand(i));
			GlobalValue *pLclBuff = dyn_cast<GlobalValue>(elt->stripPointerCasts());

			// Calculate required buffer size
			const ArrayType *pArray = dyn_cast<ArrayType>(pLclBuff->getType()->getElementType());
			unsigned int uiArraySize = pArray ? 1 : pLclBuff->getType()->getElementType()->getPrimitiveSizeInBits()/8;
			assert ( 0 != uiArraySize );
			while ( NULL != pArray )
			{
				uiArraySize *= (unsigned int)pArray->getNumElements();
				if ( pArray->getContainedType(0)->getTypeID() != Type::ArrayTyID )
				{
					uiArraySize *= pArray->getContainedType(0)->getPrimitiveSizeInBits()/8;
				}
				pArray = dyn_cast<ArrayType>(pArray->getContainedType(0));
			}

			// Now retrieve to the offset of the local buffer
			GetElementPtrInst* pLocalAddr =
				GetElementPtrInst::Create(pLocalMem, ConstantInt::get(IntegerType::get(32), m_sInfo.stTotalImplSize), "", pFirstInst);
			// Now add bitcast to required/original pointer type
			CastInst* pBitCast = CastInst::Create(Instruction::BitCast, pLocalAddr, pLclBuff->getType(), "", pFirstInst);
			// Advance total implicit size
			m_sInfo.stTotalImplSize += ADJUST_SIZE_TO_DCU_LINE(uiArraySize);

			GlobalValue::use_iterator itVal = pLclBuff->use_begin();
			// Now we need to check all uses
			while ( itVal != pLclBuff->use_end() )
			{
				Use &U = itVal.getUse();
				if (ConstantExpr *pCE = dyn_cast<ConstantExpr>(U.getUser()))
				{
					if ( ChangeConstantExpression(pFuncLocals, pLclBuff, pCE, pBitCast)  )
					{
						// The value was completely remove
						// Scan from the beginning
						itVal = pLclBuff->use_begin();
					} else
					{
						// The values is still in the list
						// Continue to next item
						++itVal;
					}
					continue;
				}

				if ( !isa<Instruction>(U.getUser()) )
				{
					++itVal;
					continue;
				}

				// Is instruction
				// Now we need check use in our same function
				if ( dyn_cast<Instruction>(U.getUser())->getParent()->getParent() == pFunc )
				{
					U.set(pBitCast);
					itVal = pLclBuff->use_begin();
				}
				else
				{
					++itVal;
				}
			}
		}

		return true;
	}

	Value* KernelUpdate::SubstituteWIcall(llvm::CallInst *pCall,
		llvm::Argument* pWorkInfo, llvm::Argument* pWGid,
		llvm::Argument* pBaseGlbId, llvm::Argument* pLocalId)
	{
		assert(pCall && "Invalid CallInst");
		Function* pFunc = pCall->getCalledFunction();
		const char*	pFuncName = pFunc->getNameStart();
		Module* pModule = pFunc->getParent();

		Value*	pResult = NULL;	// Object that holds the resolved value

		if ( !strncmp(pFuncName, "get_work_dim", 12) )
		{
			// Now retrieve address of the DIM count
			SmallVector<Value*, 4> params;
			params.push_back(ConstantInt::get(IntegerType::get(32), 0));
			params.push_back(ConstantInt::get(IntegerType::get(32), 0));
			GetElementPtrInst* pDimCntAddr =
				GetElementPtrInst::Create(pWorkInfo, params.begin(), params.end(), "", pCall);
			// Load the Value
			pResult = new LoadInst(pDimCntAddr, "", pCall);
			return pResult;
		}

		// Calculate table index of appropriate value
		int iTableInx = 0;
		iTableInx += strncmp(pFuncName, "get_global_size", 15) ? 0 : 2;
		iTableInx += strncmp(pFuncName, "get_local_size", 14) ? 0 : 3;
		iTableInx += strncmp(pFuncName, "get_num_groups", 14) ? 0 : 4;
		if ( iTableInx > 0 )
		{
			// Now retrieve address of the DIM count
			SmallVector<Value*, 4> params;
			params.push_back(ConstantInt::get(IntegerType::get(32), 0));
			params.push_back(ConstantInt::get(IntegerType::get(32), iTableInx));
			params.push_back(pCall->getOperand(1));
			GetElementPtrInst* pSizeAddr =
				GetElementPtrInst::Create(pWorkInfo, params.begin(), params.end(), "", pCall);
			// Load the Value
			pResult = new LoadInst(pSizeAddr, "", pCall);
			return pResult;
		}

		if ( !strncmp(pFuncName, "get_local_id", 12) )
		{
			SmallVector<Value*, 4> params;
			params.push_back(ConstantInt::get(IntegerType::get(32), 0));
			params.push_back(ConstantInt::get(IntegerType::get(32), 0));
			params.push_back(pCall->getOperand(1));
			GetElementPtrInst* pIdAddr =
				GetElementPtrInst::Create(pLocalId, params.begin(), params.end(), "", pCall);
			// Load the Value
			pResult = new LoadInst(pIdAddr, "", pCall);
			return pResult;
		}

		if ( !strncmp(pFuncName, "get_group_id", 12) )
		{
			GetElementPtrInst* pIdAddr =
				GetElementPtrInst::Create(pWGid, pCall->getOperand(1), "", pCall);
			// Load the Value
			pResult = new LoadInst(pIdAddr, "", pCall);
			return pResult;
		}
		if ( !strncmp(pFuncName, "get_global_id", 13) )
		{
			ConstantInt* pVal = dyn_cast<ConstantInt>(pCall->getOperand(1));
			if ( NULL != pVal )
			{
				// We have constant in the hand, we can use pre-calculated value
				unsigned int uiDim = (unsigned int)*pVal->getValue().getRawData();
				if ( NULL == m_GlbIds[uiDim] )	// Check if we already have value for this dimension
				{
					// Create new reference for this dimension
					m_GlbIds[uiDim] = CalcGlobalId(pCall, pBaseGlbId, pLocalId, true);
				}
				return m_GlbIds[uiDim];
			}
			// Otherwise, calculate inplace
			return CalcGlobalId(pCall, pBaseGlbId, pLocalId, false);
		}

		return pResult;
	}

	llvm::Value* KernelUpdate::CalcGlobalId(llvm::CallInst *pCall, llvm::Argument* pBaseGlbId, llvm::Argument* pLocalId, bool bGlobal)
	{
		// Load local id values
		SmallVector<Value*, 4> params;
		params.push_back(ConstantInt::get(IntegerType::get(32), 0));
		params.push_back(ConstantInt::get(IntegerType::get(32), 0));
		params.push_back(pCall->getOperand(1));

		llvm::BasicBlock::iterator nextInst = pCall;
		if(bGlobal)
		{
			nextInst = pCall->getParent()->getParent()->getEntryBlock().begin();
		}

		GetElementPtrInst* pLclIdAddr =
			GetElementPtrInst::Create(pLocalId, params.begin(), params.end(), "", nextInst);
		// Load the value of local id
		Value* pLocalIdVal = new LoadInst(pLclIdAddr, "", nextInst);

		// Load the value of base global index
		GetElementPtrInst* pGlbBaseAddr =
			GetElementPtrInst::Create(pBaseGlbId, pCall->getOperand(1), "", nextInst);
		// Load the Value
		Value* pBaseGlbIdVal = new LoadInst(pGlbBaseAddr, "", nextInst);

		// Now add these two values
		Value* pGlbId = BinaryOperator::CreateAdd(pLocalIdVal, pBaseGlbIdVal, "", nextInst);

		return pGlbId;
	}

	void KernelUpdate::AddCtxAddress(llvm::CallInst* pCall, llvm::Argument* pLocalId)
	{
		// Calculate address to the Context pointer
		// Load local id values
		SmallVector<Value*, 4> params;
		params.push_back(ConstantInt::get(IntegerType::get(32), 0));
		params.push_back(ConstantInt::get(IntegerType::get(32), 0));
		// It's the next address after LocalId's
		params.push_back(ConstantInt::get(IntegerType::get(32), MAX_WORK_DIM));

		m_pCtxPtr = GetElementPtrInst::Create(pLocalId, params.begin(), params.end(), "", pCall->getParent()->getParent()->getEntryBlock().begin());
	}

	void KernelUpdate::UpdateBarrier(llvm::CallInst* pCall, llvm::Argument* pLocalId)
	{
		if ( NULL == m_pCtxPtr )
		{
			AddCtxAddress(pCall, pLocalId);
		}
		assert(m_pCtxPtr);

		SmallVector<Value*, 4> params;
		// Create new call instruction with extended parameters
		params.clear();
		params.push_back(pCall->getOperand(1));
		params.push_back(m_pCtxPtr);
		Function* pNewBarrier = m_pModule->getFunction("lbarrier");
		CallInst::Create(pNewBarrier, params.begin(), params.end(), "", pCall);
	}

	Value* KernelUpdate::UpdateAsyncCopy(llvm::CallInst* pCall, llvm::Argument* pLocalId)
	{
		if ( NULL == m_pCtxPtr )
		{
			AddCtxAddress(pCall, pLocalId);
		}
		assert(m_pCtxPtr);

		// Create new call instruction with extended parameters
		SmallVector<Value*, 4> params;
		// push original parameters
		// Need bitcast to a general pointer
		CastInst* pBCDst = CastInst::Create(Instruction::BitCast, pCall->getOperand(1),
			PointerType::get(IntegerType::get(8), 0), "", pCall);
		params.push_back(pBCDst);
		CastInst* pBCSrc = CastInst::Create(Instruction::BitCast, pCall->getOperand(2),
			PointerType::get(IntegerType::get(8), 0), "", pCall);
		params.push_back(pBCSrc);
		params.push_back(pCall->getOperand(3));
		params.push_back(pCall->getOperand(4));
		// Distinguish operator size
		const PointerType* pPTy = dyn_cast<PointerType>(pCall->getOperand(1)->getType());
		assert(pPTy && "Must be a pointer");
		const Type* pPT = pPTy->getElementType();
		unsigned int uiSize = pPT->getPrimitiveSizeInBits()/8;	
		if ( 0 == uiSize )
		{
			const VectorType* pVT = dyn_cast<VectorType>(pPT);
			uiSize = pVT->getBitWidth()/8;
		}
		params.push_back(ConstantInt::get(IntegerType::get(32), uiSize));
		params.push_back(m_pCtxPtr);
		assert(pPTy && "Must by a pointer type");
		Function* pNewAsyncCopy = m_pModule->getFunction(
			pPTy->getAddressSpace() == 3 ?
			"lasync_wg_copy_g2l" : "lasync_wg_copy_l2g");
		Value* res = CallInst::Create(pNewAsyncCopy, params.begin(), params.end(), "", pCall);
		return res;
	}

	void KernelUpdate::UpdateWaitGroup(llvm::CallInst* pCall, llvm::Argument* pLocalId)
	{
		if ( NULL == m_pCtxPtr )
		{
			AddCtxAddress(pCall, pLocalId);
		}
		assert(m_pCtxPtr);

		// Create new call instruction with extended parameters
		SmallVector<Value*, 4> params;
		params.push_back(pCall->getOperand(1));
		params.push_back(pCall->getOperand(2));
		params.push_back(m_pCtxPtr);
		Function* pNewWait = m_pModule->getFunction("lwait_group_events");
		CallInst::Create(pNewWait, params.begin(), params.end(), "", pCall);
	}

	void KernelUpdate::UpdatePrefetch(llvm::CallInst* pCall)
	{
		// Create new call instruction with extended parameters
		SmallVector<Value*, 4> params;
		// push original parameters
		// Need bitcast to a general pointer
		CastInst* pBCPtr = CastInst::Create(Instruction::BitCast, pCall->getOperand(1),
			PointerType::get(IntegerType::get(8), 0), "", pCall);
		params.push_back(pBCPtr);
		// Put number of elements
		params.push_back(pCall->getOperand(2));
		// Distinguish element size
		const PointerType* pPTy = dyn_cast<PointerType>(pCall->getOperand(1)->getType());
		assert(pPTy && "Must be a pointer");
		const Type* pPT = pPTy->getElementType();
		unsigned int uiSize = pPT->getPrimitiveSizeInBits()/8;	
		if ( 0 == uiSize )
		{
			const VectorType* pVT = dyn_cast<VectorType>(pPT);
			uiSize = pVT->getBitWidth()/8;
		}
		params.push_back(ConstantInt::get(IntegerType::get(32), uiSize));
		Function* pPrefetch = m_pModule->getFunction("lprefetch");
		CallInst::Create(pPrefetch, params.begin(), params.end(), "", pCall);
	}

	BasicBlock::iterator removeInstruction(BasicBlock* pBB, BasicBlock::iterator it)
	{
		BasicBlock::InstListType::iterator prev;

		if ( pBB->begin() == it)
		{
			prev = pBB->end();
		}
		else
		{
			prev = it;
			--prev;
		}

		Instruction* pInst = it;
		pInst->removeFromParent();
		delete pInst;

		if ( pBB->end() == prev)
		{
			return pBB->begin();
		}

		return ++prev;
	}

	Function *KernelUpdate::RunOnKernel(Function *pFunc, ConstantArray* pFuncLocals)
	{
		std::vector<const llvm::Type *> newArgsVec;

		Function::ArgumentListType::iterator argIt = pFunc->getArgumentList().begin();
		while(argIt != pFunc->getArgumentList().end())
		{
			newArgsVec.push_back(argIt->getType());
			argIt++;
		}

		unsigned int uiSizeT = m_pModule->getPointerSize()*32;

		newArgsVec.push_back(PointerType::get(IntegerType::get(8), 0));
		newArgsVec.push_back(PointerType::get(m_pModule->getTypeByName("struct.WorkDim"), 0));
		newArgsVec.push_back(PointerType::get(IntegerType::get(uiSizeT), 0));
		newArgsVec.push_back(PointerType::get(IntegerType::get(uiSizeT), 0));
		newArgsVec.push_back(PointerType::get(m_pModule->getTypeByName("struct.LocalId"), 0));

		FunctionType *FTy = FunctionType::get( pFunc->getReturnType(),newArgsVec, pFunc->isVarArg());

		Function *NewF = Function::Create(FTy, pFunc->getLinkage(), pFunc->getName());

		DenseMap<const Value*, Value*> ValueMap;

		Function::arg_iterator DestI = NewF->arg_begin();
		for (Function::const_arg_iterator I = pFunc->arg_begin(), E = pFunc->arg_end(); I != E; ++I, ++DestI)
		{
			DestI->setName(I->getName());
			ValueMap[I] = DestI;
		}

		DestI->setName("pLocalMem");
		Argument *pLocalMem = DestI;
		++DestI;
		DestI->setName("pWorkDim");
		Argument *pWorkDim = DestI;
		++DestI;
		DestI->setName("pWGId");
		Argument *pWGId = DestI;
		++DestI;
		DestI->setName("pBaseGlbId");
		Argument *pBaseGlbId = DestI;
		++DestI;
		DestI->setName("LocalIds");
		Argument *pLocalId = DestI;
		pLocalId->addAttr(llvm::Attribute::ByVal);

		std::vector<ReturnInst*> Returns;
		CloneFunctionInto(NewF, pFunc, ValueMap, Returns, "", NULL);

		// Initialize kernel related variables
		memset(&m_sInfo, 0, sizeof(TLLVMKernelInfo));
		memset(m_GlbIds, 0, sizeof(m_GlbIds));
		m_pCtxPtr = NULL;

		// Apple LLVM-IR workaround
		// 1.	Pass WI information structure as the next parameter after given function parameters
		// 2.	We don't want to use TLS for local memory.
		//		Our solution to move all internal local memory blocks to be allocated
		//		by the execution engine and passed within additional parameters to the kernel,
		//		those parameters are not exposed to the user


		if ( pFuncLocals && !ParseLocalBuffers(pFuncLocals, pLocalMem) )
		{
			return false;
		}

		// Go through function blocks
		Function::BasicBlockListType::iterator bb_it = NewF->getBasicBlockList().begin();
		while ( bb_it != NewF->getBasicBlockList().end() )
		{
			BasicBlock::iterator inst_it = bb_it->begin();
			llvm::Value* pArgVal = NULL;
			while ( inst_it != bb_it->end() )
			{
				bool bAddWIInfo = false;

				switch (inst_it->getOpcode())
				{
					// Call instruction
				case Instruction::Call:
					{
					// Recognize WI info functions
					if ( !strncmp("get_", inst_it->getOperand(0)->getNameStart(), 4))
					{
						CallInst* pCall = dyn_cast<CallInst>(inst_it);
						// Substitute extern operand with function parameter
						Value *pNewRes = SubstituteWIcall(pCall, pWorkDim, pWGId, pBaseGlbId, pLocalId);
						if ( NULL != pNewRes)
						{
							pCall->uncheckedReplaceAllUsesWith(pNewRes);
							inst_it = removeInstruction(bb_it, inst_it);
						} else
						{
							++inst_it;
						}
						break;
					}
					// Check barrier()
					if ( !strcmp("barrier", inst_it->getOperand(0)->getNameStart()) )
					{
						AddBarrierDeclaration();
						CallInst* pCall = dyn_cast<CallInst>(inst_it);
						UpdateBarrier(pCall, pLocalId);
						inst_it = removeInstruction(bb_it, inst_it);
						m_sInfo.bBarrier = true;
						break;
					}

					if ( !strncmp("__async_work_group_copy", inst_it->getOperand(0)->getNameStart(), 23) )
					{
						AddAsyncCopyDeclaration();
						CallInst* pCall = dyn_cast<CallInst>(inst_it);
						// Substitute extern operand with function parameter
						Value *pNewRes = UpdateAsyncCopy(pCall, pLocalId);
						if ( NULL != pNewRes)
						{
							pCall->uncheckedReplaceAllUsesWith(pNewRes);
							inst_it = removeInstruction(bb_it, inst_it);
						} else
						{
							++inst_it;
						}
						m_sInfo.bAsynCopy = true;
						break;
					}

					if ( !strncmp("wait_group", inst_it->getOperand(0)->getNameStart(), 10) )
					{
						AddAsyncCopyDeclaration();
						CallInst* pCall = dyn_cast<CallInst>(inst_it);
						// Substitute extern operand with function parameter
						UpdateWaitGroup(pCall, pLocalId);
						inst_it = removeInstruction(bb_it, inst_it);
						break;
					}

					if ( !strncmp("__prefetch", inst_it->getOperand(0)->getNameStart(), 10) )
					{
						AddPrefetchDeclaration();
						CallInst* pCall = dyn_cast<CallInst>(inst_it);
						// Substitute extern operand with function parameter
						UpdatePrefetch(pCall);
						inst_it = removeInstruction(bb_it, inst_it);
						break;
					}

					// Check call to not inlined functions/ kernels
					Function* pCallee = dyn_cast<Function>(inst_it->getOperand(0));
					if ( NULL != pCallee && !pCallee->isDeclaration() )
					{
						if ( IsAKernel(pCallee) )
						{
							// TODO: Add special treatment for kernel calling kernel
						}
					}
					++inst_it;
					break;
					}

				default:
					++inst_it;
					break;
				}
			}
			++bb_it;
		}

		m_mapKernelInfo[NewF] = m_sInfo;

		pFunc->uncheckedReplaceAllUsesWith(NewF);
		pFunc->setName("__" + pFunc->getName() + "_original");
		m_pModule->getFunctionList().push_back(NewF);

		pFunc->deleteBody();

		return NewF;
	}

	// ------------------------------------------------------------------------------
	void KernelUpdate::AddWIInfoDeclarations()
	{
		// Detect size_t size
		unsigned int uiSizeT = m_pModule->getPointerSize()*32;
	/*
		struct sLocalId 
		{
			size_t	Id[MAX_WORK_DIM];
		};
	*/
		// Create Work Group/Work Item info structures
		std::vector<const Type*> members;
		members.push_back(ArrayType::get(IntegerType::get(uiSizeT), MAX_WORK_DIM)); // Local Id's
		StructType* pLocalId = StructType::get(members, true);
		m_pModule->addTypeName("struct.LocalId", pLocalId);

		/*
		struct sWorkInfo
		{
			unsigned int	uiWorkDim;
			size_t			GlobalOffset[MAX_WORK_DIM];
			size_t			GlobalSize[MAX_WORK_DIM];
			size_t			LocalSize[MAX_WORK_DIM];
			size_t			WGNumber[MAX_WORK_DIM];
		};
	*/
		members.clear();
		members.push_back(IntegerType::get(32));
		members.push_back(ArrayType::get(IntegerType::get(uiSizeT), MAX_WORK_DIM)); // Global offset
		members.push_back(ArrayType::get(IntegerType::get(uiSizeT), MAX_WORK_DIM)); // Global size
		members.push_back(ArrayType::get(IntegerType::get(uiSizeT), MAX_WORK_DIM)); // WG size/Local size
		members.push_back(ArrayType::get(IntegerType::get(uiSizeT), MAX_WORK_DIM)); // Number of groups
		StructType* pWorkDimType = StructType::get(members, true);
		m_pModule->addTypeName("struct.WorkDim", pWorkDimType);
	}

	void KernelUpdate::AddBarrierDeclaration()
	{
		if ( m_bBarrierDecl )
			return;

		std::vector<const Type*> params;
		params.push_back(IntegerType::get(32));
		params.push_back(PointerType::get(IntegerType::get(32), 0));
		FunctionType* pNewType = FunctionType::get(Type::VoidTy, params, false);
		Function* pNewFunc = Function::Create(pNewType, (GlobalValue::LinkageTypes)0, "lbarrier", m_pModule);
		m_bBarrierDecl = true;
	}

	//event_t lasync_work_group_copy(void* pDst, void* pSrc, size_t numElem, event_t event,
	//							   size_t elemSize, LLVMExecMultipleWIWithBarrier* *ppExec);
	void KernelUpdate::AddAsyncCopyDeclaration()
	{
		if ( m_bAsyncCopyDecl )
			return;

		unsigned int uiSizeT = m_pModule->getPointerSize()*32;

		std::vector<const Type*> params;
		params.push_back(PointerType::get(IntegerType::get(8), 0));
		params.push_back(PointerType::get(IntegerType::get(8), 0));
		params.push_back(IntegerType::get(uiSizeT));
		params.push_back(IntegerType::get(uiSizeT));
		params.push_back(IntegerType::get(uiSizeT));
		params.push_back(PointerType::get(IntegerType::get(32), 0));
		FunctionType* pNewType = FunctionType::get(IntegerType::get(uiSizeT), params, false);
		Function::Create(pNewType, (GlobalValue::LinkageTypes)0, "lasync_wg_copy_l2g", m_pModule);
		Function::Create(pNewType, (GlobalValue::LinkageTypes)0, "lasync_wg_copy_g2l", m_pModule);

		// Add wait
		params.clear();
		params.push_back(IntegerType::get(32));
		params.push_back(PointerType::get(IntegerType::get(32), 0));
		params.push_back(PointerType::get(IntegerType::get(32), 0));
		FunctionType* pWaitType = FunctionType::get(Type::VoidTy, params, false);
		Function::Create(pWaitType, (GlobalValue::LinkageTypes)0, "lwait_group_events", m_pModule);

		m_bAsyncCopyDecl = true;
	}

	void KernelUpdate::AddPrefetchDeclaration()
	{
		if ( m_bPrefetchDecl )
			return;

		unsigned int uiSizeT = m_pModule->getPointerSize()*32;

		std::vector<const Type*> params;
		// Source Pointer
		params.push_back(PointerType::get(IntegerType::get(8), 0));
		// Number of elements
		params.push_back(IntegerType::get(uiSizeT));
		// Element size
		params.push_back(IntegerType::get(uiSizeT));
		FunctionType* pNewType = FunctionType::get(Type::VoidTy, params, false);
		Function* pNewFunc = Function::Create(pNewType, (GlobalValue::LinkageTypes)0, "lprefetch", m_pModule);
		m_bPrefetchDecl = true;
	}

	bool KernelUpdate::IsAKernel(const Function* pFunc)
	{
		for (unsigned i = 0, e = m_pAnnotations->getType()->getNumElements(); i != e; ++i) 
		{
			// Obtain kernel function from annotation
			ConstantStruct *elt = cast<ConstantStruct>(m_pAnnotations->getOperand(i));
			Function *pFuncVal = dyn_cast<Function>(elt->getOperand(0)->stripPointerCasts());
			if ( NULL == pFuncVal )
			{
				continue;	// Not a function pointer
			}

			if ( pFuncVal == pFunc )
			{
				return true;
			}
		}

		// Function not found
		return false;
	}

}}}