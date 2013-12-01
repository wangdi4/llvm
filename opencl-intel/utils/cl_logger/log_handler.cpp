// Copyright (c) 2006-2012 Intel Corporation
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
//  LogHandler.cpp
//  Created on: 10-Dec-2008 11:42:24 AM
//  Implementation of the log handler class
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

// Eliminate Windows unsecure CRT code, to be able to use POSIX style functions.
#define _CRT_SECURE_NO_WARNINGS 1

#include <assert.h>
#include <malloc.h>
#include <string.h>

#include "cl_sys_defines.h"

#include "log_handler.h"
#include "cl_synch_objects.h"

using namespace Intel::OpenCL::Utils;

#if defined (_WIN32)
    #define WCSDUP   _wcsdup
#else
    #define WCSDUP    wcsdup
#endif

#define MAX_STRDUP_SIZE 1024

/**
 * Safe version of strdup.
 */
char *strdup_safe(const char *src)
{
    size_t actual = strlen(src);
    actual = (actual > MAX_STRDUP_SIZE) ? MAX_STRDUP_SIZE : actual;

    char *retStr = (char*)malloc((actual+1) * sizeof(char));
    if (NULL == retStr) return NULL;

    STRCPY_S(retStr, actual+1, src);
    return retStr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// FileDescriptorLogHandler Ctor Implementation
/////////////////////////////////////////////////////////////////////////////////////////
FileDescriptorLogHandler::FileDescriptorLogHandler(const char* handle) : LogHandler(), m_fileHandler(NULL), m_dupStderr(-1)
{
    if (NULL != handle)
    {
        m_handle = strdup_safe(handle);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// FileDescriptorLogHandler Dtor
/////////////////////////////////////////////////////////////////////////////////////////
FileDescriptorLogHandler::~FileDescriptorLogHandler()
{
	if ( NULL != m_handle )
	{
		free(m_handle);
		m_handle=NULL;
	}

	if (-1 != m_dupStderr)
	{
		// redirect back stderr
		DUP2(m_dupStderr, fileno(stderr));
		m_dupStderr = -1;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////
// FileDescriptorLogHandler::Init
/////////////////////////////////////////////////////////////////////////////////////////
cl_err_code FileDescriptorLogHandler::Init(ELogLevel level, const char* fileName, const char* title, FILE* fileDesc)
{
    if (NULL == m_handle)
	{
        return CL_ERR_INITILIZATION_FAILED;
	}

	if (NULL == fileDesc)
	{
		return CL_ERR_LOGGER_FAILED;
	}

	m_fileHandler = fileDesc;

    m_logLevel = level;       // retrieve this info from Logger (not implemented yet)

    // redirect stderr to fileDesc (in order to get log messages from MIC device)
	fflush(stderr);
	m_dupStderr = DUP(fileno(stderr));
	assert(-1 != m_dupStderr && "duplicate stderr failed");
	DUP2(fileno(m_fileHandler), fileno(stderr));

	const char* pTitle = (NULL == title) ?
		"\n##########################################################################################################\n" :
		title;

	// fputs is thread safe.
	if (EOF == fputs(pTitle, m_fileHandler))
	{
		printf("fwrite failed\n");
		assert(false);
		return CL_ERR_LOGGER_FAILED;
	}
	Flush();
	
	return CL_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////
// FileDescriptorLogHandler::Log
/////////////////////////////////////////////////////////////////////////////////////////
void FileDescriptorLogHandler::Log(LogMessage& logMessage)
{
    if (m_logLevel > logMessage.GetLogLevel())
    {
        // ignore messages with lower log level
        return;
    }

    char* formattedMsg = logMessage.GetFormattedMessage();
	// fputs is thread safe.
    if (EOF == fputs(formattedMsg, m_fileHandler))
    {
        printf("fwrite failed\n");
        assert(false);
        return;
    }
    Flush();

	return;
}

/////////////////////////////////////////////////////////////////////////////////////////
// FileDescriptorLogHandler::Flush
/////////////////////////////////////////////////////////////////////////////////////////
void FileDescriptorLogHandler::Flush()
{
	if (m_fileHandler)
	{
		fflush(m_fileHandler);     // thread safe
	}
}


/////////////////////////////////////////////////////////////////////////////////////////
// FileLogHandler Ctor Implementation
/////////////////////////////////////////////////////////////////////////////////////////
FileLogHandler::FileLogHandler(const char* handle) : FileDescriptorLogHandler(handle), m_fileName(NULL)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// FileLogHandler Dtor Implementation
/////////////////////////////////////////////////////////////////////////////////////////
FileLogHandler::~FileLogHandler()
{
	if (NULL != m_fileHandler)
	{
        fclose(m_fileHandler);
		m_fileHandler=NULL;
	}

	if ( NULL != m_fileName )
	{
		free(m_fileName);
		m_fileName=NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// FileLogHandler::Init
/////////////////////////////////////////////////////////////////////////////////////////
cl_err_code FileLogHandler::Init(ELogLevel level, const char* fileName, const char* title, FILE* fileDesc)
{
	if (m_handle == NULL)
	{
        return CL_ERR_INITILIZATION_FAILED;
	}
	if (NULL == fileName)
	{
		printf("logger initialization failed, fileName must be valid pointer\n");
		return CL_ERR_LOGGER_FAILED;
	}
    m_fileName = strdup_safe(fileName);
	FILE* tFileHandler = NULL;
    if (m_fileName)
    {
        tFileHandler = fopen(m_fileName, "w" );
        if (NULL == tFileHandler)
        {
            printf("can't open log file for writing\n");
            return CL_ERR_LOGGER_FAILED;
        }
	}
	return FileDescriptorLogHandler::Init(level, fileName, title, tFileHandler);
}
