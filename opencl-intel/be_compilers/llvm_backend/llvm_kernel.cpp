/////////////////////////////////////////////////////////////////////////
// llvm_kernel.cpp:
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
// or disclosed in any way without Intel�s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel�s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "llvm_kernel.h"
#include "llvm_binary.h"
#include "llvm_program.h"
#include "cpu_dev_limits.h"

#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Module.h"
#include "llvm/Support/MemoryBuffer.h"

#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Constants.h"
#include "llvm/ModuleProvider.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Instructions.h"
#include "llvm/CallingConv.h"
#include "llvm/TypeSymbolTable.h"
#include "llvm/ValueSymbolTable.h"

#ifdef __ENABLE_VTUNE__
#include "VTune\JITProfiling.h"
#endif

#include <string>
#include <map>
#include <vector>

using namespace Intel::OpenCL::DeviceBackend;
using namespace std;
using namespace llvm;

// External declarations
extern ExecutionEngine* g_pExecEngine;

// Constructor/Destructor
LLVMKernel::LLVMKernel(LLVMProgram* pProgram) :
	m_pFuncPtr(NULL), m_szName(NULL), m_uiArgCount(0), m_pArguments(NULL),
	m_pReqdWGSize(NULL), m_pHintWGSize(NULL), m_uiOptWGSize(1),
	m_uiExplLocalMemCount(0), m_bBarrier(false), m_pModule(NULL), m_pFunction(NULL),
	m_uiTotalImplSize(0), m_uiStackSize(CPU_DEV_MIN_WI_PRIVATE_SIZE),
	m_pProgram(pProgram), m_pCtxPtr(NULL), m_uiVTuneId(-1),
	m_bVectorized(false), m_uiVectorWidth(0), m_szVectorizedName(NULL)
{
	memset(m_GlbIds, 0, sizeof(m_GlbIds));
}

LLVMKernel::~LLVMKernel()
{
#if defined(__ENABLE_VTUNE__) && 0	// Bug in VTune dll's causes an exception when called during PROCESS_DETACH
		if ( (-1 != m_uiVTuneId) && (iJIT_SAMPLING_ON == iJIT_IsProfilingActive()) )
		{
			iJIT_Method_Id MI;

			MI.method_id = m_uiVTuneId;

			iJIT_NotifyEvent(iJVM_EVENT_TYPE_METHOD_UNLOAD_START, &MI);
			m_uiVTuneId = -1;
		}
#endif

	if ( (NULL != m_pFuncPtr) && (m_pFunction != NULL) )
	{
		g_pExecEngine->freeMachineCodeForFunction(m_pFunction);
		m_pFuncPtr = NULL;
	}
	if ( NULL != m_pArguments )
	{
		delete []m_pArguments;
	}
	if ( NULL != m_szVectorizedName)
	{
		delete m_szVectorizedName;
	}
}

cl_dev_err_code LLVMKernel::ParseArguments(Function *pFunc)
{
	if ( NULL != m_pArguments )
	{
		delete []m_pArguments;
		m_uiArgCount = 0;
	}

	m_pArguments = new cl_kernel_argument[pFunc->getArgumentList().size()];
	if ( NULL == m_pArguments )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	Function::arg_iterator arg_it = pFunc->arg_begin(), e = pFunc->arg_end();
	while (arg_it != e)
	{
		Argument* pArg = arg_it;
		// Set argument sizes
		switch (arg_it->getType()->getTypeID())
		{
		case Type::FloatTyID:
			m_pArguments[m_uiArgCount].type = CL_KRNL_ARG_FLOAT;
			m_pArguments[m_uiArgCount].size_in_bytes = sizeof(float);
			break;

		case Type::PointerTyID:
			{
				const PointerType *PTy = cast<PointerType>(arg_it->getType());
				if ( pArg->hasByValAttr() && PTy->getElementType()->getTypeID() == Type::VectorTyID )
				{
					// Check by pointer vector passing, used in long16 and double16
					const VectorType *pVector = dyn_cast<VectorType>(PTy->getElementType());
					unsigned int uiNumElem = (unsigned int)pVector->getNumElements();;
					unsigned int uiElemSize = pVector->getContainedType(0)->getPrimitiveSizeInBits()/8;
					if ( (uiElemSize*uiNumElem) > 4*16 )
					{
						m_pArguments[m_uiArgCount].type = CL_KRNL_ARG_VECTOR;
						m_pArguments[m_uiArgCount].size_in_bytes = uiNumElem & 0xFFFF;
						m_pArguments[m_uiArgCount].size_in_bytes |= (uiElemSize << 16);
						break;
					}
				}
				m_pArguments[m_uiArgCount].size_in_bytes = 0;
				// Detect pointer qualifier
				// Test for image
				std::string &imgArg = pFunc->getParent()->getTypeName(PTy->getElementType());
				unsigned int inx = imgArg.find("struct._image");
				if ( -1 != inx )	// Image identifier was found
				{
					// Get dimension of the image strlen("struct._image")
					char dim = imgArg.at(13);
					// Setup image pointer
					m_pArguments[m_uiArgCount].type = ('2' == dim ? CL_KRNL_ARG_PTR_IMG_2D : CL_KRNL_ARG_PTR_IMG_3D);
					break;
				}
				switch (PTy->getAddressSpace())
				{
				case 0: case 1:	// Global Address space
					m_pArguments[m_uiArgCount].type = CL_KRNL_ARG_PTR_GLOBAL;
					break;
				case 2:
					m_pArguments[m_uiArgCount].type = CL_KRNL_ARG_PTR_CONST;
					break;
				case 3: // Local Address space
					m_pArguments[m_uiArgCount].type = CL_KRNL_ARG_PTR_LOCAL;
					++m_uiExplLocalMemCount;
					break;

				default:
					assert(0);
				}}
			break;

		case Type::IntegerTyID:
			{
				const IntegerType *ITy = cast<IntegerType>(arg_it->getType());
				m_pArguments[m_uiArgCount].type = CL_KRNL_ARG_INT;
				m_pArguments[m_uiArgCount].size_in_bytes = ITy->getBitWidth()/8;
			}
			break;

		case Type::DoubleTyID:
			m_pArguments[m_uiArgCount].type = CL_KRNL_ARG_DOUBLE;
			m_pArguments[m_uiArgCount].size_in_bytes = sizeof(double);
			break;

		case Type::VectorTyID:
			{
			const VectorType *pVector = dyn_cast<VectorType>(arg_it->getType());
			m_pArguments[m_uiArgCount].type = CL_KRNL_ARG_VECTOR;
			m_pArguments[m_uiArgCount].size_in_bytes = (unsigned int)pVector->getNumElements();
			m_pArguments[m_uiArgCount].size_in_bytes |= (pVector->getContainedType(0)->getPrimitiveSizeInBits()/8)<<16;
			}
			break;

		default:
			assert(0 && "Unhelded parameter type");
		}
		++m_uiArgCount;
		++arg_it;
	}

	if ( m_uiExplLocalMemCount > CPU_MAX_LOCAL_ARGS )
	{
		return CL_DEV_ERROR_FAIL;
	}
	return CL_DEV_SUCCESS;
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

// Substitutes a pointer to local buffer, with argument passed within kernel parameters
cl_dev_err_code LLVMKernel::ParseLocalBuffers(ConstantArray* pFuncLocals, Argument* pLocalMem)
{
	// Create code for local buffer
	Instruction* pFirstInst = dyn_cast<Instruction>(pLocalMem->getParent()->getEntryBlock().begin());

	// Iterate through local buffers
	for (unsigned i = 0, e = pFuncLocals->getType()->getNumElements(); i != e; ++i) 
	{
		// Obtain kernel function from annotation
		Constant *elt = cast<Constant>(pFuncLocals->getOperand(i));
		GlobalValue *val = dyn_cast<GlobalValue>(elt->stripPointerCasts());
		// Calculate required buffer size
		const ArrayType *pArray = dyn_cast<ArrayType>(val->getType()->getElementType());
		unsigned int uiArraySize = pArray ? 1 : val->getType()->getElementType()->getPrimitiveSizeInBits()/8;
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
			GetElementPtrInst::Create(pLocalMem, ConstantInt::get(IntegerType::get(32), m_uiTotalImplSize), "", pFirstInst);
		// Now add bitcast to required/original pointer type
		CastInst* pBC = CastInst::Create(Instruction::BitCast, pLocalAddr, val->getType(), "", pFirstInst);
		// Advance total implicit size
		m_uiTotalImplSize += ADJUST_SIZE_TO_DCU_LINE(uiArraySize);

		GlobalValue::use_iterator itVal = val->use_begin();
		// Now we need to check all uses
		while ( itVal != val->use_end() )
		{
			Use &U = val->use_begin().getUse();
			if (Constant *C = dyn_cast<Constant>(U.getUser()))
			{
				// We need substitute constant expression with real instruction
				ConstantExpr* pCE = dyn_cast<ConstantExpr>(C);
				if ( NULL != pCE )
				{
					Instruction* pInst = CreateInstrFromConstantExpr(pCE, val, pBC);
					// Add instruction to the block
					pInst->insertBefore(pFirstInst);
					// Change all non-constant references
					ConstantExpr::use_iterator itCE = pCE->use_begin();
					while( itCE != pCE->use_end() )
					{
						Use &W = itCE.getUse();
						if ( !isa<Constant>(W.getUser()) )
						{
							W.set(pInst);
							itCE = pCE->use_begin();		// Restart the scan
						}
						else
						{
							++itCE;
						}
					}
					if ( pCE->use_empty() )
					{
						delete pCE;
						itVal = val->use_begin();
					}
					else
					{
						++itVal;
					}
					continue;
				}
			}

			if ( !isa<Instruction>(U.getUser()) )
			{
				++itVal;
				continue;
			}

			U.set(pBC);
			itVal = val->use_begin();
		}
	}

	return CL_DEV_SUCCESS;
}


Value* LLVMKernel::SubstituteWIcall(llvm::CallInst *pCall,
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
				m_GlbIds[uiDim] = CalcGlobalId(pCall, pBaseGlbId, pLocalId);
			}
			return m_GlbIds[uiDim];
		}
		// Otherwise, calculate inplace
		return CalcGlobalId(pCall, pBaseGlbId, pLocalId);
	}

	return pResult;
}

llvm::Value* LLVMKernel::CalcGlobalId(llvm::CallInst *pCall, llvm::Argument* pBaseGlbId, llvm::Argument* pLocalId)
{
	// Load local id values
	SmallVector<Value*, 4> params;
	params.push_back(ConstantInt::get(IntegerType::get(32), 0));
	params.push_back(ConstantInt::get(IntegerType::get(32), 0));
	params.push_back(pCall->getOperand(1));
	GetElementPtrInst* pLclIdAddr =
		GetElementPtrInst::Create(pLocalId, params.begin(), params.end(), "", pCall);
	// Load the value of local id
	Value* pLocalIdVal = new LoadInst(pLclIdAddr, "", pCall);

	// Load the value of base global index
	GetElementPtrInst* pGlbBaseAddr =
		GetElementPtrInst::Create(pBaseGlbId, pCall->getOperand(1), "", pCall);
	// Load the Value
	Value* pBaseGlbIdVal = new LoadInst(pGlbBaseAddr, "", pCall);

	// Now add these two values
	Value* pGlbId = BinaryOperator::CreateAdd(pLocalIdVal, pBaseGlbIdVal, "", pCall);

	return pGlbId;
}

void LLVMKernel::AddCtxAddress(llvm::CallInst* pCall, llvm::Argument* pLocalId)
{
	// Calculate address to the Context pointer
	// Load local id values
	SmallVector<Value*, 4> params;
	params.push_back(ConstantInt::get(IntegerType::get(32), 0));
	params.push_back(ConstantInt::get(IntegerType::get(32), 0));
	// It's the next address after LocalId's
	params.push_back(ConstantInt::get(IntegerType::get(32), MAX_WORK_DIM));
	m_pCtxPtr = GetElementPtrInst::Create(pLocalId, params.begin(), params.end(), "", pCall);
}

void LLVMKernel::UpdateBarrier(llvm::CallInst* pCall, llvm::Argument* pLocalId)
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

Value* LLVMKernel::UpdateAsyncCopy(llvm::CallInst* pCall, llvm::Argument* pLocalId)
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

void LLVMKernel::UpdateWaitGroup(llvm::CallInst* pCall, llvm::Argument* pLocalId)
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

void LLVMKernel::UpdatePrefetch(llvm::CallInst* pCall)
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


cl_int LLVMKernel::ParseLLVM(Function *pFunc, ConstantArray* pFuncArgs, ConstantArray* pFuncLocals)
{
	bool	bDbgPrint = false;
	bool	bAsynCopy = false;

	m_pFunction = pFunc;
	m_pModule = pFunc->getParent();

	// Local Memories Lookup table
	map<string, llvm::Argument *>	mapLocalMemBuffers;

	// Set Kernel arguments
	cl_dev_err_code rc;
	if ( CL_DEV_FAILED(rc = ParseArguments(pFunc)) )
	{
		return rc;
	}

	m_szName = pFunc->getNameStart();

	// Apple LLVM-IR workaround
	// 1.	Pass WI information structure as the next parameter after given function parameters
	// 2.	We don't want to use TLS for local memory.
	//		Our solution to move all internal local memory blocks to be allocated
	//		by the execution engine and passed within additional parameters to the kernel,
	//		those parameters are not exposed to the user

	// Add Implicit memory block pointer
	Argument *pLocalMem = new Argument(PointerType::get(IntegerType::get(8), 0), "pLocalMem", pFunc);
	// Add Working dimension information structure pointer
	Argument *pWorkDim = new Argument(PointerType::get(m_pModule->getTypeByName("struct.WorkDim"), 0),
										"pWorkDim", pFunc);
//	pWGInfo->addAttr(Attribute::ByVal);

	unsigned int uiSizeT = m_pModule->getPointerSize()*32;
	// Add WG id parameter
	Argument *pWGId = new Argument(PointerType::get(IntegerType::get(uiSizeT), 0),
						"pWGId", pFunc);

	Argument *pBaseGlbId = new Argument(PointerType::get(IntegerType::get(uiSizeT), 0),
		"pBaseGlbId", pFunc);
	//	pBaseGlobalId->addAttr(llvm::Attribute::ByVal);

	// Add WI indexing information structure
	Argument *pLocalId = new Argument(PointerType::get(m_pModule->getTypeByName("struct.LocalId"), 0),
		"LocalIds", pFunc);
	pLocalId->addAttr(llvm::Attribute::ByVal);

	if ( pFuncLocals && CL_DEV_FAILED(rc = ParseLocalBuffers(pFuncLocals, pLocalMem)) )
	{
		return rc;
	}

	// Go through function blocks
	Function::BasicBlockListType::iterator bb_it = pFunc->getBasicBlockList().begin();
	while ( bb_it != pFunc->getBasicBlockList().end() )
	{
		BasicBlock::InstListType::iterator inst_it = bb_it->getInstList().begin();
		llvm::Value* pArgVal = NULL;
		while ( inst_it != bb_it->getInstList().end() )
		{
			bool bAddWIInfo = false;

			switch (inst_it->getOpcode())
			{
				// Call instruction
				case Instruction::Call:
					// Recognize WI info functions
					if ( !strncmp("get_", inst_it->getOperand(0)->getNameStart(), 4))
					{
						CallInst* pCall = dyn_cast<CallInst>(inst_it);
						// Substitute extern operand with function parameter
						Value *pNewRes = SubstituteWIcall(pCall, pWorkDim, pWGId, pBaseGlbId, pLocalId);
						if ( NULL != pNewRes)
						{
							pCall->uncheckedReplaceAllUsesWith(pNewRes);
							--inst_it;
							pCall->removeFromParent();
							delete pCall;
						}
						break;
					}
					// Check barrier()
					if ( !strcmp("barrier", inst_it->getOperand(0)->getNameStart()) )
					{
						m_pProgram->AddBarrierDeclaration();
						m_bBarrier = true;
						CallInst* pCall = dyn_cast<CallInst>(inst_it);
						UpdateBarrier(pCall, pLocalId);
						--inst_it;
						pCall->removeFromParent();
						delete pCall;
						break;
					}

					if (!strcmp("dbg_print", inst_it->getOperand(0)->getNameStart()))
					{
						bDbgPrint = true;
						break;
					}

					if ( !strncmp("__async_work_group_copy", inst_it->getOperand(0)->getNameStart(), 23) )
					{
						bAsynCopy = true;
						m_pProgram->AddAsyncCopyDeclaration();
						CallInst* pCall = dyn_cast<CallInst>(inst_it);
						// Substitute extern operand with function parameter
						Value *pNewRes = UpdateAsyncCopy(pCall, pLocalId);
						if ( NULL != pNewRes)
						{
							pCall->uncheckedReplaceAllUsesWith(pNewRes);
							--inst_it;
							pCall->removeFromParent();
							delete pCall;
						}
						break;
					}

					if ( !strncmp("wait_group", inst_it->getOperand(0)->getNameStart(), 10) )
					{
						m_pProgram->AddAsyncCopyDeclaration();
						CallInst* pCall = dyn_cast<CallInst>(inst_it);
						// Substitute extern operand with function parameter
						UpdateWaitGroup(pCall, pLocalId);
						--inst_it;
						pCall->removeFromParent();
						delete pCall;
						break;
					}

					if ( !strncmp("__prefetch", inst_it->getOperand(0)->getNameStart(), 10) )
					{
						m_pProgram->AddPrefetchDeclaration();
						CallInst* pCall = dyn_cast<CallInst>(inst_it);
						// Substitute extern operand with function parameter
						UpdatePrefetch(pCall);
						--inst_it;
						pCall->removeFromParent();
						delete pCall;
						break;
					}

					// Check call to not inlined functions/ kernels
					Function* pCallee = dyn_cast<Function>(inst_it->getOperand(0));
					if ( NULL != pCallee && !pCallee->isDeclaration() )
					{
						if ( !m_pProgram->IsKernel(pCallee->getNameStart()) )
						{
							g_pExecEngine->getPointerToFunction(pCallee);
						}
					}
					break;
			}
			++inst_it;
		}
		++bb_it;
	}

	m_pFuncPtr = g_pExecEngine->getPointerToFunction(pFunc);

	if ( bDbgPrint )
	{
		m_uiStackSize = 1024*32;	// We need large stack here
	} else
	{
		if (bAsynCopy || m_bBarrier)
		{
			m_uiStackSize = 1024*16;
		}
	}

	// JIT generated get info from function
	m_uiStackSize = (unsigned int)(m_uiStackSize+g_pExecEngine->getJitFunctionStackSize(pFunc));

#ifdef __ENABLE_VTUNE__
	if ( iJIT_SAMPLING_ON == iJIT_IsProfilingActive() )
	{
		m_uiVTuneId = iJIT_GetNewMethodID();

		iJIT_Method_Load ML;

		memset(&ML, 0, sizeof(iJIT_Method_Load));

		//Parameters
		ML.method_id = m_uiVTuneId;              // uniq method ID - can be any uniq value, (such as the mb)
		ML.method_name = pFunc->getNameStart();          // method name (can be with or without the class and signature, in any case the class name will be added to it)
		ML.method_load_address = m_pFuncPtr;  // virtual address of that method  - This determines the method range for the iJVM_EVENT_TYPE_ENTER/LEAVE_METHOD_ADDR events
		ML.method_size = g_pExecEngine->getJitFunctionSize(pFunc);        // Size in memory - Must be exact
#if 0
		// Used of DebugInfo
		// Constants in this example
		ML.line_number_size = 0;        // Line Table size in number of entries - Zero if none
		ML.line_number_table = NULL;    // Pointer to the begining of the line numbers info array
		ML.class_id = 0;                // uniq class ID
		ML.class_file_name = NULL;      // class file name 
		ML.source_file_name = NULL;     // source file name
		ML.user_data = NULL;            // bits supplied by the user for saving in the JIT file...
		ML.user_data_size = 0;          // the size of the user data buffer
		ML.env = iJDE_JittingAPI;
#endif
		iJIT_NotifyEvent(iJVM_EVENT_TYPE_METHOD_LOAD_FINISHED, &ML);
	}
#endif

	// Set optimal WG size
	if ( m_bBarrier )
	{
		m_uiOptWGSize = 16;	// TODO: to be checked 
	} else
	{
		m_uiOptWGSize = 16; // TODO: to be checked
	}

	// Set WG size hint
	if ( pFunc->hasFnAttr(llvm::Attribute::WGSizeHint) )
	{
		m_pHintWGSize = pFunc->getWGSizeHint();

		m_uiOptWGSize = 1;
		for(int i=0; i<MAX_WORK_DIM; ++i)
		{
			m_uiOptWGSize*=m_pHintWGSize[i];
		}
	}
	// Set required WG size
	if ( pFunc->hasFnAttr(llvm::Attribute::ReqdWGSize) )
	{
		m_pReqdWGSize = pFunc->getReqdWGSize();
		m_uiOptWGSize = 1;
		for(int i=0; i<MAX_WORK_DIM; ++i)
		{
			m_uiOptWGSize*=m_pReqdWGSize[i];
		}
	}

	// Add VTune information

	return CL_DEV_SUCCESS;
}

cl_int LLVMKernel::GetKernelParams( const cl_kernel_argument* OUT *pArgsBuffer, cl_uint* OUT ArgCount ) const
{
	assert(pArgsBuffer && ArgCount);

	*pArgsBuffer = m_pArguments;
	*ArgCount = m_uiArgCount;
	return CL_DEV_SUCCESS;
}

cl_int LLVMKernel::CreateBinary(void* IN pArgsBuffer, 
						size_t IN BufferSize, 
						cl_uint IN WorkDimension,
						const size_t* IN pGlobalOffeset,
						const size_t* IN pGlobalWorkSize, 
						const size_t* IN pLocalWorkSize,
						ICLDevBackendBinary** OUT pBinary) const
{
	assert(pBinary);

	LLVMBinary*	pBin = new LLVMBinary(this, WorkDimension, pGlobalOffeset,
												pGlobalWorkSize, pLocalWorkSize);

	if ( NULL == pBin )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	cl_uint rc = pBin->Init((char*)pArgsBuffer, BufferSize);
	if ( CL_DEV_FAILED(rc) )
	{
		return rc;
	}

	pBin->setVectorizerProperties(m_bVectorized, m_szVectorizedName, m_uiVectorWidth);

	*pBinary = pBin;

	return CL_DEV_SUCCESS;
}

bool LLVMKernel::isVectorized()
{
	return m_bVectorized;
}

unsigned int LLVMKernel::getVectorWidth()
{
	return m_uiVectorWidth;
}

void LLVMKernel::setVectorizerProperties(bool isVectorized, const char *vectorizedName, unsigned int vectorWidth)
{
	m_bVectorized      = isVectorized;
	m_szVectorizedName = strdup(vectorizedName);
	m_uiVectorWidth    = vectorWidth;
}
