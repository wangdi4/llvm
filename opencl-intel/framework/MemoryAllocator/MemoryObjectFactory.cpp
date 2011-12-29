// Copyright (c) 2006-2010 Intel Corporation
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

///////////////////////////////////////////////////////////////////////////////////////////////////
//  cl_mem_allocator.cpp
//  Implementation of the Memory Allocator Class
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "MemoryObjectFactory.h"
#include "Context.h"

#include <CL/cl.h>
#include <assert.h>

using namespace std;
using namespace Intel::OpenCL::Framework;

MemoryObjectFactory* MemoryObjectFactory::GetInstance()
{
	static MemoryObjectFactory sMemObjFactory;

	return &sMemObjFactory;
}

bool MemoryObjectFactory::FactoryKey::operator<(const FactoryKey& _Right) const
{
	return ( ( clObjType < _Right.clObjType) ||
		     ( ( clObjType == _Right.clObjType) && ( iSupportedDevices < _Right.iSupportedDevices) ) ||
			 ( ( clObjType == _Right.clObjType) && ( iSupportedDevices == _Right.iSupportedDevices) && ( iGfxSysSharing < _Right.iGfxSysSharing) ) );
}

void MemoryObjectFactory::RegisterMemoryObjectCreator(cl_bitfield iSupportedDevices,
											int iGfxSysSharing,
											cl_mem_object_type clObjType,
											fn_MemoryObjectCreator* pMemObjCreator)
{
	FactoryKey key;

	key.clObjType = clObjType;
	key.iSupportedDevices = iSupportedDevices;
	key.iGfxSysSharing = iGfxSysSharing;

	assert(m_memObjMap.find(key) == m_memObjMap.end() );
	m_memObjMap[key] = pMemObjCreator;
}

cl_err_code MemoryObjectFactory::CreateMemoryObject( cl_bitfield iRequiredDevices,
								cl_mem_object_type clObjType, int iGfxSysSharing, Context* pContext, MemoryObject*	*pMemObject)
{
	FactoryKey key;

	assert(NULL != pMemObject);

	key.clObjType = clObjType;
	key.iSupportedDevices = iRequiredDevices;
	key.iGfxSysSharing = iGfxSysSharing;

	std::map<FactoryKey, fn_MemoryObjectCreator*>::iterator it = m_memObjMap.find(key);
	if ( it == m_memObjMap.end() )
	{
		return CL_ERR_FAILURE;
	}
	MemoryObject* pMemObj = it->second(pContext, (ocl_entry_points*)(pContext->GetHandle()->dispatch), clObjType );
	if (NULL == pMemObj)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

	*pMemObject = pMemObj;
	return CL_SUCCESS;
}
