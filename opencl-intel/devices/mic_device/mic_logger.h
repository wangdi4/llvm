
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

enum MicELogLevel
	{
		MIC_LL_DEBUG     = 100,
		MIC_LL_INFO      = 200,
		MIC_LL_ERROR     = 300,
		MIC_LL_CRITICAL  = 400,
		MIC_LL_STATISTIC = 500,
		MIC_LL_OFF       = 1000
	};


#define MicInfoLog(CLIENT, CLIENT_ID, DBG_PRINT, ...)			\
	if (CLIENT && CLIENT_ID) CLIENT->clLogAddLine(CLIENT_ID, cl_int(MIC_LL_INFO),__FILE__, __FUNCTION__, __LINE__, DBG_PRINT,  __VA_ARGS__);
#ifdef _DEBUG
#define MicDbgLog(CLIENT, CLIENT_ID, DBG_PRINT, ...)			\
	if (CLIENT && CLIENT_ID) CLIENT->clLogAddLine(CLIENT_ID, cl_int(MIC_LL_DEBUG),__FILE__, __FUNCTION__, __LINE__, DBG_PRINT,  __VA_ARGS__);
#else
#define MicDbgLog(CLIENT, CLIENT_ID, DBG_PRINT, ...)
#endif

#define MicErrLog(CLIENT, CLIENT_ID, DBG_PRINT, ...)			\
	if (CLIENT && CLIENT_ID) CLIENT->clLogAddLine(CLIENT_ID, cl_int(MIC_LL_ERROR),__FILE__, __FUNCTION__, __LINE__, DBG_PRINT,  __VA_ARGS__);
#define MicCriticLog(CLIENT, CLIENT_ID, DBG_PRINT, ...)			\
	if (CLIENT && CLIENT_ID) CLIENT->clLogAddLine(CLIENT_ID, cl_int(MIC_LL_CRITICAL),__FILE__, __FUNCTION__, __LINE__, DBG_PRINT,  __VA_ARGS__);
