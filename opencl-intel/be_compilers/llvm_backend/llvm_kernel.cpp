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
#include "llvm_backend.h"

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

// Constructor/Destructor
LLVMKernel::LLVMKernel(LLVMProgram* pProgram) :
	m_pFuncPtr(NULL), m_szName(NULL), m_uiArgCount(0), m_pArguments(NULL),
	m_pReqdWGSize(NULL), m_pHintWGSize(NULL), m_uiOptWGSize(1),
	m_uiExplLocalMemCount(0), m_bBarrier(false), m_pModule(NULL), m_pFunction(NULL),
	m_uiStackSize(CPU_DEV_MIN_WI_PRIVATE_SIZE), m_stTotalImplSize(0),
	m_pProgram(pProgram), m_uiVTuneId(-1),
	m_uiVectorWidth(0), m_pVectorizedKernel(NULL)
{
}

LLVMKernel::~LLVMKernel()
{
	if ( (NULL != m_pFuncPtr) && (m_pFunction != NULL) )
	{
		LLVMBackend::GetInstance()->GetExecEngine()->freeMachineCodeForFunction(m_pFunction);
		m_pFuncPtr = NULL;
	}
	if ( NULL != m_pArguments )
	{
		delete []m_pArguments;
	}
}

void LLVMKernel::Release()
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

	if ( NULL != m_pVectorizedKernel)
	{
		m_pVectorizedKernel->Release();
		m_pVectorizedKernel = NULL;
	}

	delete this;
}

cl_dev_err_code LLVMKernel::ParseArguments(Function *pFunc, ConstantArray* pFuncArgs)
{
	if ( NULL != m_pArguments )
	{
		delete []m_pArguments;
		m_uiArgCount = 0;
	}

	string strArgs = pFuncArgs->getAsString();
	m_uiArgCount = pFunc->getArgumentList().size() - KRNL_NUM_CONST_ARGS;

	m_pArguments = new cl_kernel_argument[m_uiArgCount];
	if ( NULL == m_pArguments )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	Function::arg_iterator arg_it = pFunc->arg_begin();
	for (unsigned i=0; i<m_uiArgCount; ++i)
	{
		Argument* pArg = arg_it;
		// Set argument sizes
		switch (arg_it->getType()->getTypeID())
		{
		case Type::FloatTyID:
			m_pArguments[i].type = CL_KRNL_ARG_FLOAT;
			m_pArguments[i].size_in_bytes = sizeof(float);
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
						m_pArguments[i].type = CL_KRNL_ARG_VECTOR;
						m_pArguments[i].size_in_bytes = uiNumElem & 0xFFFF;
						m_pArguments[i].size_in_bytes |= (uiElemSize << 16);
						break;
					}
				}
				m_pArguments[i].size_in_bytes = 0;
				// Detect pointer qualifier
				// Test for image
				std::string &imgArg = pFunc->getParent()->getTypeName(PTy->getElementType());
				unsigned int inx = imgArg.find("struct._image");
				if ( -1 != inx )	// Image identifier was found
				{
					// Get dimension of the image strlen("struct._image")
					char dim = imgArg.at(13);
					// Setup image pointer
					m_pArguments[i].type = ('2' == dim ? CL_KRNL_ARG_PTR_IMG_2D : CL_KRNL_ARG_PTR_IMG_3D);
					m_pArguments[i].size_in_bytes = ('5' == strArgs[i]) ? 0 : 1;	// Set RW/WR flag
					break;
				}
				switch (PTy->getAddressSpace())
				{
				case 0: case 1:	// Global Address space
					m_pArguments[i].type = CL_KRNL_ARG_PTR_GLOBAL;
					assert( ('2' == strArgs[i]) || ('1' == strArgs[i]) );
					break;
				case 2:
					m_pArguments[i].type = CL_KRNL_ARG_PTR_CONST;
					assert('8' == strArgs[i]);
					break;
				case 3: // Local Address space
					m_pArguments[i].type = CL_KRNL_ARG_PTR_LOCAL;
					++m_uiExplLocalMemCount;
					assert('9' == strArgs[i]);
					break;

				default:
					assert(0);
				}}
			break;

		case Type::IntegerTyID:
			{
				if ( '4' == strArgs[i] )
				{
					m_pArguments[i].type = CL_KRNL_ARG_SAMPLER;
					m_pArguments[i].size_in_bytes = 0;
				}
				else
				{
					const IntegerType *ITy = cast<IntegerType>(arg_it->getType());
					m_pArguments[i].type = CL_KRNL_ARG_INT;
					m_pArguments[i].size_in_bytes = ITy->getBitWidth()/8;
				}
			}
			break;

		case Type::DoubleTyID:
			m_pArguments[i].type = CL_KRNL_ARG_DOUBLE;
			m_pArguments[i].size_in_bytes = sizeof(double);
			break;

		case Type::VectorTyID:
			{
			const VectorType *pVector = dyn_cast<VectorType>(arg_it->getType());
			m_pArguments[i].type = CL_KRNL_ARG_VECTOR;
			m_pArguments[i].size_in_bytes = (unsigned int)pVector->getNumElements();
			m_pArguments[i].size_in_bytes |= (pVector->getContainedType(0)->getPrimitiveSizeInBits()/8)<<16;
			}
			break;

		default:
			assert(0 && "Unhelded parameter type");
		}
		++arg_it;
	}

	if ( m_uiExplLocalMemCount > CPU_MAX_LOCAL_ARGS )
	{
		return CL_DEV_ERROR_FAIL;
	}
	return CL_DEV_SUCCESS;
}

// Resolve calls to external functions
// return stack size for required by calls to external functions
size_t LLVMKernel::ResolveFunctionCalls(Function* pFunc)
{
	// Required stack
	size_t stStack = 0;
	ExecutionEngine* pExecEng = LLVMBackend::GetInstance()->GetExecEngine();

	// Go through function blocks, and resolve calls
	Function::BasicBlockListType::iterator bb_it = pFunc->getBasicBlockList().begin();
	while ( bb_it != pFunc->getBasicBlockList().end() )
	{
		BasicBlock::InstListType::iterator inst_it = bb_it->getInstList().begin();
		llvm::Value* pArgVal = NULL;
		while ( inst_it != bb_it->getInstList().end() )
		{
			switch (inst_it->getOpcode())
			{
				// Call instruction
			case Instruction::Call:
				// Check call to not inlined functions/ kernels
				Function* pCallee = dyn_cast<Function>(inst_it->getOperand(0));
				if ( NULL != pCallee && !pCallee->isDeclaration() )
				{
					if ( !m_pProgram->IsKernel(pCallee->getNameStart()) )
					{
						size_t stLclStack = ResolveFunctionCalls(pCallee);
						pExecEng->getPointerToFunction(pCallee);
						stLclStack += pExecEng->getJitFunctionStackSize(pCallee);
						stStack = max(stStack, stLclStack);
					}
				}
				break;
			}
			++inst_it;
		}
		++bb_it;
	}

	return stStack;
}

cl_int LLVMKernel::Init(Function *pFunc, ConstantArray* pFuncArgs)
{
	m_pFunction = pFunc;
	m_pModule = pFunc->getParent();
	m_szName = pFunc->getNameStart();

	// Set Kernel arguments
	cl_dev_err_code rc;
	if ( pFuncArgs && CL_DEV_FAILED(rc = ParseArguments(pFunc, pFuncArgs)) )
	{
		return rc;
	}

	m_uiStackSize = ResolveFunctionCalls(pFunc);

	m_pFuncPtr = LLVMBackend::GetInstance()->GetExecEngine()->getPointerToFunction(pFunc);

	TLLVMKernelInfo& info = m_pProgram->m_mapKernelInfo[pFunc];
	m_bBarrier = info.bBarrier;
	m_stTotalImplSize = info.stTotalImplSize;

	m_uiStackSize = CPU_DEV_MIN_WI_PRIVATE_SIZE;

	if ( info.bDbgPrint )
	{
		m_uiStackSize = max(1024*32, m_uiStackSize);	// We need large stack here
	} else
	{
		if (info.bAsynCopy || m_bBarrier)
		{
			m_uiStackSize = max(1024*16, m_uiStackSize);
		}
	}

	// JIT generated get info from function
	m_uiStackSize = (unsigned int)(m_uiStackSize+LLVMBackend::GetInstance()->GetExecEngine()->getJitFunctionStackSize(pFunc));

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

	*pBinary = pBin;

	return CL_DEV_SUCCESS;
}

void LLVMKernel::setVectorizerProperties(LLVMKernel* pVectKernel, unsigned int vectorWidth)
{
	m_pVectorizedKernel	= pVectKernel;
	m_uiVectorWidth		= vectorWidth;
}
