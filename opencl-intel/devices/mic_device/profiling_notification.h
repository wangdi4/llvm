// Copyright (c) 2006-2008 Intel Corporation
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

#include "cl_synch_objects.h" 

#include <source/COIProcess_source.h>

namespace Intel { namespace OpenCL { namespace MICDevice {

class ProfilingNotification
{
public:

	static ProfilingNotification& getInstance() { return m_singleProfilingNotification; };

	// Assume that calling registration when first creating OCL MIC context.
	bool registerProfilingNotification(COIPROCESS coiProcess);
	
	// Assume that calling unregister when deleting the last OCL MIC context.
	bool unregisterProfilingNotification(COIPROCESS coiProcess);

	// Global function that will notify for each event.
	static void callbackNotifier(COI_NOTIFICATIONS in_Type, COIPROCESS in_Process, COIEVENT in_Event, const void* in_UserData);

private:

	ProfilingNotification() {};

	static ProfilingNotification m_singleProfilingNotification;

};

}}}
