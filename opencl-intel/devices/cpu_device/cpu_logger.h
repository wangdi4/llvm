
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
//  cpu_logger.h
//  Implementation of Helper functions for logger
///////////////////////////////////////////////////////////

#if defined (_WIN32)
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#else
#define WIDEN(x) x
#endif

enum CpuELogLevel
	{
		CPU_LL_DEBUG     = 100,
		CPU_LL_INFO      = 200,
		CPU_LL_ERROR     = 300,
		CPU_LL_CRITICAL  = 400,
		CPU_LL_STATISTIC = 500,
		CPU_LL_OFF       = 1000
	};


#define CpuInfoLog(CLIENT, CLIENT_ID, DBG_PRINT, ...)			\
	if (CLIENT && CLIENT_ID) CLIENT->clLogAddLine(CLIENT_ID, cl_int(CPU_LL_INFO),WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, DBG_PRINT,  __VA_ARGS__);
#ifdef _DEBUG
#define CpuDbgLog(CLIENT, CLIENT_ID, DBG_PRINT, ...)			\
	if (CLIENT && CLIENT_ID) CLIENT->clLogAddLine(CLIENT_ID, cl_int(CPU_LL_DEBUG),WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, DBG_PRINT,  __VA_ARGS__);
#else
#define CpuDbgLog(CLIENT, CLIENT_ID, DBG_PRINT, ...)
#endif

#define CpuErrLog(CLIENT, CLIENT_ID, DBG_PRINT, ...)			\
	if (CLIENT && CLIENT_ID) CLIENT->clLogAddLine(CLIENT_ID, cl_int(CPU_LL_ERROR),WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, DBG_PRINT,  __VA_ARGS__);
#define CpuCriticLog(CLIENT, CLIENT_ID, DBG_PRINT, ...)			\
	if (CLIENT && CLIENT_ID) CLIENT->clLogAddLine(CLIENT_ID, cl_int(CPU_LL_CRITICAL),WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, DBG_PRINT,  __VA_ARGS__);
