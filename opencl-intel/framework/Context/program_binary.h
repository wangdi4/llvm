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
#include <logger.h>
#include <cl_synch_objects.h>
#include <cl_device_api.h>
#include <observer.h>

using namespace Intel::OpenCL::Utils;

namespace Intel { namespace OpenCL { namespace Framework {

	class OclMutex;
	class Program;
	class Device;

	/**********************************************************************************************
	* Enumaration:	EProgramBinaryBuildStatus
	*
	* Description:	define the resource from wich the program was created
	* Values:		PST_NONE		- Empty program
	*				PST_SOURCE_CODE	- Create program with source
	*				PST_BINARY		- Create program with binaries
	**********************************************************************************************/	
	enum EProgramBinaryBuildStatus
	{
		BBS_NEW = 0,				// new object
		BBS_EXTERNAL_BIN = 1,		// load with binary
		BBS_INTERNAL_BIN = 2,		// load with source - compliled with FE compiler
		BBS_BUILDING = 3,			// building with BE compiler
		BBS_READY = 5,				// binary ready
	};

	/**********************************************************************************************
	* Class name:	ProgramBinary
	*
	* Description:	represents a ProgramBinary object
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/		
	class ProgramBinary : public IBuildDoneObserver
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
		ProgramBinary(	cl_uint						uiBinSize,
						const void *				pBinData,
						cl_dev_binary_prop			clDevBinaryProp,
						Device *					pDevice,
						LoggerClient *				pLoggerClient,
						cl_err_code *				pErr );

		/******************************************************************************************
		* Function: 	~ProgramBinary
		* Description:	The ProgramBinary class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		virtual ~ProgramBinary();

		// build program binary
		cl_err_code Build(	const char *	pcOptions,
							IBuildDoneObserver * pBuildDoneObserver );

		// get the binary size and data
		cl_err_code		GetBinary(cl_uint uiBinSize, void * pBin, cl_uint * puiBinSizeRet);
		
		// get the deivce associated to the binary
		const Device *	GetDevice(){ return m_pDevice; }

		// get the build status of the current binary
		const cl_build_status GetStatus();

		// IBuildDoneObserver
		virtual cl_err_code NotifyBuildDone(cl_device_id device, cl_build_status build_status);

		const cl_dev_program GetId(){ return m_clDevProgram; }

		const cl_dev_binary_prop GetBinaryProp(){ return m_clDevBinaryProp; }

	private:

		cl_dev_program							m_clDevProgram;
		
		cl_dev_binary_prop						m_clDevBinaryProp;

		cl_build_status							m_clBuildStatus;	// build status

		IBuildDoneObserver *					m_pBuildDoneObserver;
		
		Device *								m_pDevice;		// associated device

		cl_uint									m_uiBinSize;	// binary size

		void *									m_pBinData;		// binary data

		//char *								m_pcBuildOptions; // build options

		//cl_prog_container	*					m_pclDevProgContainer;

		Intel::OpenCL::Utils::OclMutex			m_CS;				// Critical Section object for quering the build status

		DECLARE_LOGGER_CLIENT;	// logger client

	};

}}};


#endif //_OCL_PROGRAM_BINARY_H_