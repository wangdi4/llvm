
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

///////////////////////////////////////////////////////////
//  lrb_cl_logger.h
//  Implementation of Helper functions for logger
///////////////////////////////////////////////////////////

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)

enum ELogLevel
	{   	
		LL_DEBUG     = 100,
		LL_INFO      = 200,
		LL_ERROR     = 300,
		LL_CRITICAL  = 400,	
		LL_STATISTIC	= 500,
		LL_OFF       = 1000
	};


#define InfoLog(CLIENT, CLIENT_ID, DBG_PRINT, ...)			\
	if (CLIENT.pfnclLogAddLine && CLIENT_ID) CLIENT.pfnclLogAddLine(CLIENT_ID, cl_int(LL_INFO),WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, DBG_PRINT,  __VA_ARGS__);
#define DbgLog(CLIENT, CLIENT_ID, DBG_PRINT, ...)			\
	if (CLIENT.pfnclLogAddLine && CLIENT_ID) CLIENT.pfnclLogAddLine(CLIENT_ID, cl_int(LL_DEBUG),WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, DBG_PRINT,  __VA_ARGS__);
#define ErrLog(CLIENT, CLIENT_ID, DBG_PRINT, ...)			\
	if (CLIENT.pfnclLogAddLine && CLIENT_ID) CLIENT.pfnclLogAddLine(CLIENT_ID, cl_int(LL_ERROR),WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, DBG_PRINT,  __VA_ARGS__);
#define CriticLog(CLIENT, CLIENT_ID, DBG_PRINT, ...)			\
	if (CLIENT.pfnclLogAddLine && CLIENT_ID) CLIENT.pfnclLogAddLine(CLIENT_ID, cl_int(LL_CRITICAL),WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, DBG_PRINT,  __VA_ARGS__);
