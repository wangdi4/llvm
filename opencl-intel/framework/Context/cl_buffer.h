// Copyright (c) 2006-2007 Intel Corporation
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
//  cl_buffer.h
//  Implementation of the Class Buffer
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <cl_types.h>
#include <cl_memory_object.h>
#include <Logger.h>
#include <map>
#include <list>

namespace Intel { namespace OpenCL { namespace Framework {

	class Device;
	class SubBuffer;

	/**********************************************************************************************
	* Class name:	Buffer
	*
	* Inherit:		MemoryObject
	* Description:	represents a memory object
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/		
	class Buffer : public MemoryObject
	{
	public:

		/******************************************************************************************
		* Function: 	Buffer
		* Description:	The Buffer class constructor
		* Arguments:	
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/		
		Buffer(Context * pContext, cl_mem_flags clMemFlags, size_t szBufferSize, ocl_entry_points * pOclEntryPoints, cl_err_code * pErrCode);

		// MemoryObject methods
		cl_err_code CreateDeviceResource(cl_device_id clDeviceId);
		cl_err_code ReadData(void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch = 0, size_t szSlicePitch = 0);
		cl_err_code WriteData(const void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch = 0, size_t szSlicePitch = 0);
		size_t GetSize() const;
		size_t GetNumDimensions() const { return 1; }
		void GetLayout( OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch ) const;
        bool CheckBounds( const size_t* pszOrigin, const size_t* pszRegion) const;
		bool CheckBounds( const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch) const;
		void * GetData( const size_t * pszOrigin = NULL ) const;

		/******************************************************************************************
		* Function: 	IsSubBuffer 
		* Description:	Returns True if the actual object is a SubBuffer object. Default is false.
		* Arguments:	
		* Author:		Rami Jioussy
		* Date:			August 2010
		******************************************************************************************/	
		virtual bool IsSubBuffer() { return false; }
		
		/******************************************************************************************
		* Function: 	AddSubBuffer 
		* Description:	Attaches a new SubBuffer to this buffer.
		* Arguments:	pSubBuffer			- pointer to the subbuffer object.		
		* Author:		Rami Jioussy
		* Date:			August 2010
		******************************************************************************************/		
		void AddSubBuffer(SubBuffer* pSubBuffer);

		/******************************************************************************************
		* Function: 	GetSubBuffersCount 
		* Description:	Returns the number of SubBuffers in the Buffer object.
		* Arguments:	
		* Author:		Rami Jioussy
		* Date:			August 2010
		******************************************************************************************/		
		cl_uint GetSubBuffersCount() { return m_subBuffers->Count(); }
		
		/******************************************************************************************
		* Function: 	GetSubBuffer 
		* Description:	Returns a pointer the SubBuffers belong to this buffer); 
		* Arguments:	szCount		- number of sub buffers to return
		*				pSubBuffer	- holds the pointer to the returned SubBuffers
		*			    retCount    - number of returned SubBuffers
		* Author:		Rami Jioussy
		* Date:			August 2010
		******************************************************************************************/		
		void GetSubBuffers(int szCount,  SubBuffer** pSubBuffers, cl_uint* retCount);		 
		

		/******************************************************************************************
		* Function: 	CheckIfSupportedByDevice
		* Description:	Returns True for buffer which isn't a SubBuffer.
		* Arguments:	cl_device_id - the device we want to test for alignment.		
		* Author:		Rami Jioussy
		* Date:			August 2010
		******************************************************************************************/	
		virtual bool CheckIfSupportedByDevice(cl_device_id) { return true; }

	protected:
		/******************************************************************************************
		* Function: 	~Buffer
		* Description:	The Buffer class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/			
		virtual ~Buffer();				
		OCLObjectsMap<_cl_mem_int> *			m_subBuffers;		

	};

	/**********************************************************************************************
	* Class name:	SubBuffer
	*
	* Inherit:		Buffer
	* Description:	represents a SubBuffer in another parent buffer; kind of relation can be a sub 
	*				region in the parent buffer.
	* Author:		Rami Jioussy
	* Date:			August 2010
	**********************************************************************************************/	
	class SubBuffer: public Buffer
	{
	public:
		
		/******************************************************************************************
		* Function: 	SubBuffer (Ctor)
		* Description:	The SubBuffer class Constructor
		* Arguments:	pContext			- pointer to the context
		*				clMemFlags			- memory creation flags
		*				szBufferSize		- size of subbuffer region
		*				pOclEntryPointes	- handle ocl callbacks table
		*				pErrorCode			- holds return code of the constructor
		* Author:		Rami Jioussy
		* Date:			August 2010
		******************************************************************************************/			
		SubBuffer(Context * pContext, cl_mem_flags clMemFlags, ocl_entry_points * pOclEntryPoints, cl_err_code * pErrCode);
		
		/******************************************************************************************
		* Function: 	initialize
		* Description:	SubBuffer initializing function; need to be called after creation.
		* Arguments:	pBuffer				- pointer to the parent buffer
		*				buffer_create_type	- Specifies the kind of subBuffer; like Sub region representation.
		*				buffer_create_info  - info for creating the subBuffer; in case of Sub region type
		*									this includes (offset,size) in the parent buffer.				
		* Author:		Rami Jioussy
		* Date:			August 2010
		******************************************************************************************/	
		cl_err_code initialize(Buffer* pBuffer, cl_buffer_create_type buffer_create_type, const void * buffer_create_info);
		
		/******************************************************************************************
		* Function: 	IsSubBuffer 
		* Description:	Returns (True); indicating this is SubBuffer; 
		* Arguments:	
		* Author:		Rami Jioussy
		* Date:			August 2010
		******************************************************************************************/	
		bool IsSubBuffer() { return true; }		
		
		/******************************************************************************************
		* Function: 	~SubBuffer (Dtor)
		* Description:	The SubBuffer class destructor
		* Arguments:			
		* Author:		Rami Jioussy
		* Date:			August 2010
		******************************************************************************************/	
		~SubBuffer();

		/******************************************************************************************
		* Function: 	CheckIfSupportedByDevice
		* Description:	Check if device alignment support subBuffer offset in the specified region.
		* Arguments:	cl_device_id - the device we want to test for alignment.		
		* Author:		Rami Jioussy
		* Date:			August 2010
		******************************************************************************************/	
		bool CheckIfSupportedByDevice(cl_device_id);
		
			
	protected:
		
		friend class MemoryObject;

		size_t	m_Origin;				// Offset in bytes relative to the parent buffer 
		size_t  m_szSize;				// Size in bytes of the sub_buffer
		Buffer* m_pParentBuffer;		// Pointer to the parent buffer
	};
}}}

