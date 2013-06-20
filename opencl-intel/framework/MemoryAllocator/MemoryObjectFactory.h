// Copyright (c) 2006-2012 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////
//  cl_mem_allocator.h
//  Declaration of the Memory Allocator
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "cl_framework.h"
#include <cl_types.h>

#include <map>
#include "cl_shared_ptr.h"

using Intel::OpenCL::Utils::SharedPtr;

namespace Intel { namespace OpenCL { namespace Framework {

	// define Graphics system sharing type
	enum tGfxSharing
	{
		CL_MEMOBJ_GFX_SHARE_NONE	= 0,
		CL_MEMOBJ_GFX_SHARE_GL		= 1,
		CL_MEMOBJ_GFX_SHARE_DX9		= 2,
		CL_MEMOBJ_GFX_SHARE_DX10	= 4,
        CL_MEMOBJ_GFX_SHARE_DX11    = 8
	};

	class Context;
	class MemoryObject;

#define REGISTER_MEMORY_OBJECT_CREATOR_INTERNAL(SUPPORTED_DEVICES,GFX_SHARE,OBJECT_TYPE,CLASS,IMPLEMETATION)	struct CLASS##CreatorRegister\
	{\
		CLASS##CreatorRegister()\
		{\
		MemoryObjectFactory::GetInstance()->RegisterMemoryObjectCreator(SUPPORTED_DEVICES,GFX_SHARE,OBJECT_TYPE,&CLASS##CreatorRegister::Create);\
		}\
		static SharedPtr<MemoryObject> Create(SharedPtr<Context> pContext, cl_mem_object_type clObjType)\
		{\
            /* This is to prevent class B that inherits from A to not define Allocate, while A does. This would compile if we just used 'return', but we'd get an object of class A instead of one of type B */\
            SharedPtr<IMPLEMETATION> pMemObj = IMPLEMETATION::Allocate(pContext, clObjType);\
			return pMemObj;\
		}\
	};\
	CLASS##CreatorRegister class##CLASS##CreatorRegister;

    typedef SharedPtr<MemoryObject> fn_MemoryObjectCreator(SharedPtr<Context> pContext, cl_mem_object_type clObjType);

	class MemoryObjectFactory
	{
	public:
		static MemoryObjectFactory* GetInstance();

		void	RegisterMemoryObjectCreator(cl_bitfield iSupportedDevices,
											int iGfxSysSharing,
											cl_mem_object_type clObjType,
											fn_MemoryObjectCreator* pMemObjCreator);

		cl_err_code CreateMemoryObject( cl_bitfield iRequiredDevices, cl_mem_object_type clObjType, int iGfxSysSharing,
											SharedPtr<Context> pContext, SharedPtr<MemoryObject> *pMemObject);

	protected:
		struct FactoryKey
		{
			cl_mem_object_type	clObjType;
			cl_bitfield			iSupportedDevices;
			int					iGfxSysSharing;

			bool operator<(const FactoryKey& _Right) const;
		};

		std::map<FactoryKey, fn_MemoryObjectCreator*>	m_memObjMap;
	};

// This macro level constructs an object creator class name as a concatenation of a memory object class name and random number
#define REGISTER_MEMORY_OBJECT_CREATOR_INTERNAL_WRAP(SUPPORTED_DEVICES,GFX_SHARE,OBJECT_TYPE,CLASS,NUMBER)\
	REGISTER_MEMORY_OBJECT_CREATOR_INTERNAL(SUPPORTED_DEVICES,GFX_SHARE,OBJECT_TYPE,CLASS##_##NUMBER##_,CLASS)

// This macro level just desacralizes __LINE__ macro and converts it into simple number 
#define REGISTER_MEMORY_OBJECT_CREATOR_INTERNAL_WRAP1(SUPPORTED_DEVICES,GFX_SHARE,OBJECT_TYPE,CLASS,NUMBER)\
	REGISTER_MEMORY_OBJECT_CREATOR_INTERNAL_WRAP(SUPPORTED_DEVICES,GFX_SHARE,OBJECT_TYPE,CLASS,NUMBER)

// This macro level is required to add a __LINE__ macro operand
#define REGISTER_MEMORY_OBJECT_CREATOR(SUPPORTED_DEVICES,GFX_SHARE,OBJECT_TYPE,CLASS)\
	REGISTER_MEMORY_OBJECT_CREATOR_INTERNAL_WRAP1(SUPPORTED_DEVICES,GFX_SHARE,OBJECT_TYPE,CLASS,__LINE__)

}}}