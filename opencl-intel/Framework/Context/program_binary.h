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
//  program.h
//  Implementation of the ProgramBinary class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(_OCL_PROGRAM_BINARY_H_)
#define _OCL_PROGRAM_BINARY_H_

#include <cl_types.h>
#include <cl_object.h>
#include <logger.h>
#include <cl_device_api.h>

namespace Intel { namespace OpenCL { namespace Framework {

	class Program;
	class Device;

	/**********************************************************************************************
	* Class name:	ProgramBinary
	*
	* Description:	represents a ProgramBinary object
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/		
	class ProgramBinary 
	{
	public:

		/******************************************************************************************
		* Function: 	ProgramBinary
		* Description:	The ProgramBinary class constructor - allocate memory and copy the data
		*				return result through pErr
		* Arguments:	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		ProgramBinary(cl_uint uiBinSize, const void * pBinData, Device * pDevice, cl_err_code * pErr);

		/******************************************************************************************
		* Function: 	~ProgramBinary
		* Description:	The ProgramBinary class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		virtual ~ProgramBinary();

		// get the binary size
		cl_uint		GetSize(){ return m_uiBinSize; }
		
		// get the binary data
		void *		GetData(){ return m_pBinData; }
		
		// get the deivce associated to the binary
		Device *	GetDevice(){ return m_pDevice; }


	private:

		cl_uint									m_uiBinSize;	// binary size

		void *									m_pBinData;		// binary data

		Device *								m_pDevice;		// associated device

		cl_prog_container						m_clDevProgContainer;

		Intel::OpenCL::Utils::LoggerClient *	m_pLoggerClient;	// logger client

	};

}}};


#endif //_OCL_PROGRAM_BINARY_H_