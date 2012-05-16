// Copyright (c) 2008-2012 Intel Corporation
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

///////////////////////////////////////////////////////////
//  user_event.cpp
//  Implementation of the Class UserEvent
//  Created on:      04-Jul-2010 
//  Original author: Singer, Doron
///////////////////////////////////////////////////////////

#include <assert.h>
#include <cl_sys_info.h>

#include "build_event.h"
#include "Context.h"

using namespace Intel::OpenCL::Framework;

BuildEvent::BuildEvent( _cl_context_int* context ) : OclEvent(context)
{
	//AddPendency(this);	// why we need this?
	m_pContext = (Context*)context->object;
	SetEventState(EVENT_STATE_HAS_DEPENDENCIES);
    m_returnCode = 0xdead;
	m_pContext->AddPendency(this);
}

BuildEvent::~BuildEvent()
{
	m_pContext->RemovePendency(this);
}

void BuildEvent::SetComplete(cl_int returnCode)
{
	m_returnCode = returnCode;
	SetEventState(EVENT_STATE_DONE);
	//RemovePendency(this);	// why we need this?
}

