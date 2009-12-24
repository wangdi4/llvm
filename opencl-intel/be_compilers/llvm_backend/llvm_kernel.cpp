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
	m_uiExplLocalMemCount(0), m_bBarrier(false), m_pModule(NULL),
	m_uiTotalImplSize(0), m_uiStackSize(DEFAULT_STACK_SIZE), m_pProgram(pProgram), m_pCtxPtr(NULL)
{
	memset(m_GlbIds, 0, sizeof(m_GlbIds));
}

LLVMKernel::~LLVMKernel()
{
	if ( NULL != m_pArguments )
	{
		delete []m_pArguments;
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
		// Set argument sizes
		switch (arg_it->getType()->getTypeID())
		{
		case Type::FloatTyID:
			m_pArguments[m_uiArgCount].type = CL_KRNL_ARG_FLOAT;
			m_pArguments[m_uiArgCount].size_in_bytes = sizeof(float);
			break;

		case Type::PointerTyID:
			{
				m_pArguments[m_uiArgCount].size_in_bytes = 0;
				// Detect pointer qualifier
				const PointerType *PTy = cast<PointerType>(arg_it->getType());
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
			unsigned int uiVectorSize = 1;
			while ( NULL != pVector )
			{
				uiVectorSize *= (unsigned int)pVector->getNumElements();
				if ( pVector->getContainedType(0)->getTypeID() != Type::VectorTyID )
				{
					uiVectorSize *= pVector->getContainedType(0)->getPrimitiveSizeInBits()/8;
				}
				pVector = dyn_cast<VectorType>(pVector->getContainedType(0));
			}

			m_pArguments[m_uiArgCount].type = CL_KRNL_ARG_VECTOR;
			m_pArguments[m_uiArgCount].size_in_bytes = uiVectorSize;
			}
			break;

		default:
			assert(0 && "Unhelded parameter type");
		}
		++m_uiArgCount;
		++arg_it;
	}

	return CL_DEV_SUCCESS;
}

// Substitutes a pointer to local buffer, with argument passed within kernel parameters
llvm::Value* LLVMKernel::SubstituteImplLocalPtr(llvm::Instruction* pInst, llvm::Argument* pLocalMem, llvm::Value* pArgVal)
{
	const GlobalValue *pGV = dyn_cast<GlobalVariable>(pArgVal);
	if ( (NULL == pGV) || (pGV->getType()->getAddressSpace() != 3) )
	{
		return NULL;
	}

	const PointerType *PTy = cast<PointerType>(pArgVal->getType());
	assert(( NULL != PTy ) && "Expected pointer type");
	if ( (PTy == NULL) )
	{
		// Not a "Local" pointer
		return NULL;
	}

	// Check if already have reference to the buffer
	string &strBuffName = pArgVal->getName();
	map<string, Value*>::iterator	it = m_mapImplLocalPtr.find(strBuffName);
	if ( m_mapImplLocalPtr.end() != it)	// reference to an old buffer
	{
		return it->second;
	}

	// Reference to a new local buffer
	// Create buffer size
	const ArrayType *pArray = dyn_cast<ArrayType>(PTy->getElementType());
	unsigned int uiArraySize = 1;
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
		GetElementPtrInst::Create(pLocalMem, ConstantInt::get(IntegerType::get(32), m_uiTotalImplSize), "", pInst);
	// Now add bitcast to required/original pointer type
	CastInst* pBC = CastInst::Create(Instruction::BitCast, pLocalAddr, PTy, "", pInst);
	// Advance total implicit size
	m_uiTotalImplSize += ADJUST_SIZE_TO_DCU_LINE(uiArraySize);
	// Add to map
	m_mapImplLocalPtr[strBuffName] = pBC;

	return pBC;
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

cl_int LLVMKernel::ParseLLVM(Function *pFunc)
{
	bool	bDbgPrint = false;
	bool	bAsynCopy = false;

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
				case llvm::Instruction::Call:
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

					break;

				// getelementptr instruction
				case llvm::Instruction::GetElementPtr:
					// Retrieve array operand
					pArgVal = inst_it->getOperand(0);
					// Substitute "local" variables with parameter one
					pArgVal = SubstituteImplLocalPtr(inst_it, pLocalMem, pArgVal);
					if ( NULL != pArgVal )
					{
						inst_it->setOperand(0, pArgVal);
					}
					break;

				case llvm::Instruction::Load: case llvm::Instruction::Store:
					// Retrieve operand number according to command
					unsigned uiOpId = llvm::Instruction::Store == inst_it->getOpcode();
					pArgVal = inst_it->getOperand( uiOpId );
					ConstantExpr *pCE = dyn_cast<ConstantExpr>(pArgVal);
					if ( (NULL != pCE) && (pCE->getNumOperands() >= 1) )
					{
						pArgVal = SubstituteImplLocalPtr(inst_it, pLocalMem, pCE->getOperand(0));
						// For Load/Store operations, we need substitute ConstantExpression with real instruction
						if ( NULL != pArgVal )
						{
							SmallVector<Value*, 4> operands;
							for(unsigned i=1; i<pCE->getNumOperands(); ++i)
							{
								operands.push_back(pCE->getOperand(i));
							}
							// Create new instruction and insert it before the original load/store
							GetElementPtrInst* pNewInst = GetElementPtrInst::Create(
								pArgVal, operands.begin(), operands.end(), "", inst_it);
							// Use the result in the original instruction
							inst_it->setOperand(uiOpId, pNewInst);

							// Change all other reference
							pCE->uncheckedReplaceAllUsesWith(pArgVal);
						}
						break;
					}
					GlobalValue *pGV = dyn_cast<GlobalVariable>(pArgVal);
					if ( (NULL != pGV) && (pGV->getType()->getAddressSpace() == 3) )
					{
						// Retrieve array operand
						// Substitute "local" variables with parameter one
						pArgVal = SubstituteImplLocalPtr(inst_it, pLocalMem, pArgVal);
						if ( NULL != pArgVal )
						{
							inst_it->setOperand(uiOpId, pArgVal);
						}
						break;
					}
					break;
			}
			++inst_it;
		}
		++bb_it;
	}

	m_pFuncPtr = g_pExecEngine->getPointerToFunction(pFunc);

	// Set optimal WG size
	if ( m_bBarrier )
	{
		m_uiOptWGSize = 16;	// TODO: to be checked 
	} else
	{
		m_uiOptWGSize = 16; // TODO: to be checked
	}

	if ( bDbgPrint )
	{
		m_uiStackSize = 1024*32;	// We need large stack here
	} else
	{
		if (bAsynCopy && m_bBarrier)
		{
			m_uiStackSize = 1024*16;
		}
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

	LLVMBinary*	pBin = new LLVMBinary(this, pArgsBuffer, BufferSize,
												WorkDimension, pGlobalOffeset,
												pGlobalWorkSize, pLocalWorkSize);

	if ( NULL == pBin )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	// Allocate memory for local parameters
	char*	pLocalParams = new char[BufferSize];
	if ( NULL == pLocalParams )
	{
		delete pBin;
		return CL_DEV_OUT_OF_MEMORY;
	}
	// Allocate buffer to store pointer to explicit local buffers inside the parameters buffer
	// These positions will be replaced with real pointers just before the execution
	cl_uint	uiLocalCount = 0;
	size_t*	pLocalBufferOffsets = NULL;
	if ( m_uiExplLocalMemCount > 0)
	{
		pLocalBufferOffsets = new size_t[m_uiExplLocalMemCount];
		if ( NULL == pLocalBufferOffsets )
		{
			delete []pLocalParams;
			delete pBin;
			return CL_DEV_OUT_OF_MEMORY;
		}
	}

	size_t	stTotalLocalSize = 0;
	size_t	stOffset = 0;

	memcpy_s(pLocalParams, BufferSize, pArgsBuffer, BufferSize);
	// Calculate actual local buffer size
	// Store in local buffer in reverse order
	for(unsigned int i=0; i<m_uiArgCount; ++i)
	{
		// Argument is buffer object or local memory size
		if ( CL_KRNL_ARG_PTR_GLOBAL <= m_pArguments[i].type )

		{
			stOffset += sizeof(void*);
		}
		else if (CL_KRNL_ARG_PTR_LOCAL == m_pArguments[i].type)
		{
			// Retrieve sizes of explicit local objects
			size_t origSize = *(((size_t*)(pLocalParams+stOffset)));
			size_t locSize = ADJUST_SIZE_TO_DCU_LINE(origSize); 
			stTotalLocalSize += locSize;
			pLocalBufferOffsets[uiLocalCount] = stOffset;	// the offset is from the end
			++uiLocalCount;
			*(((size_t*)(pLocalParams+stOffset))) = locSize;
			stOffset += sizeof(void*);
		}
		else
		{
			stOffset += m_pArguments[i].size_in_bytes;
		}
	}

	stTotalLocalSize += m_uiTotalImplSize;
	// Check local size
	if ( CPU_DEV_LCL_MEM_SIZE <  stTotalLocalSize)
	{
		delete []pLocalBufferOffsets;
		delete []pLocalParams;
		delete pBin;
		return CL_DEV_ERROR_FAIL;
	}

	assert(pBinary);
	*pBinary = pBin;
	pBin->m_pLocalParams = pLocalParams;
	pBin->m_uiLocalCount = uiLocalCount;
	pBin->m_pLocalBufferOffsets = pLocalBufferOffsets;

	return CL_DEV_SUCCESS;
}