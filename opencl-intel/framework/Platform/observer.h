// Copyright (c) 2008-2009 Intel Corporation
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
//  observer.h
//  Implementation of different observer interfaces to be used by the framework
//  Created on:      23-Dec-2008 3:23:02 PM
//  Original author: Levy, Uri
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(__OCL_OBSERVER_H__)
#define __OCL_OBSERVER_H__

#include <cl_device_api.h>
#include <cl_types.h>

namespace Intel { namespace OpenCL { namespace Framework {

    // Pure interface class 
	// implement this interface if you want to be notified when program build process was finished
	// in order to register the obsrever, call BuildProgram method in Device object
    class IBuildDoneObserver
    {
    public:
        virtual cl_err_code NotifyBuildDone(cl_device_id device, cl_build_status build_status) = 0;

        virtual ~IBuildDoneObserver(){};  // Virtual D'tor
    };

	class IFrontendBuildDoneObserver
	{
	public:
		virtual cl_err_code NotifyFEBuildDone(cl_device_id device, size_t szBinSize, void * pBinData, const char *pBuildLog) = 0;

        virtual ~IFrontendBuildDoneObserver(){}; // Virtual D'tor
    };


	// Pure interface class 
	// implement this interface if you want to be notified when specific command's status was changed
	// in order to register the observer, call 
	class ICmdStatusChangedObserver
	{
	public:
		virtual cl_err_code NotifyCmdStatusChanged(cl_dev_cmd_id clCmdId, cl_int iCmdStatus, cl_int iCompletionResult, cl_ulong ulTimer) = 0;

        virtual ~ICmdStatusChangedObserver(){}; // Virtual D'tor
	};

}}};    // Intel::OpenCL::Framework
#endif  // !defined(__OCL_OBSERVER_H__)


