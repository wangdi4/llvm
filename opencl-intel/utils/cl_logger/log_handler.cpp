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

#if !defined (_WIN32)
    #include <stdlib.h>
    #include "cl_secure_string_linux.h"
#endif

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
// FileLogHandler Ctor Implementation
/////////////////////////////////////////////////////////////////////////////////////////
FileLogHandler::FileLogHandler(const char* handle) :
    m_fileName(NULL), m_fileHandler(NULL)
{
    if (NULL != handle)
    {
        m_handle = strdup_safe(handle);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// FileLogHandler Dtor
/////////////////////////////////////////////////////////////////////////////////////////
FileLogHandler::~FileLogHandler()
{
    free(m_handle);
    if (m_fileHandler)
	{
        fclose(m_fileHandler);
	}

    free(m_fileName);
}


/////////////////////////////////////////////////////////////////////////////////////////
// FileLogHandler::Init
/////////////////////////////////////////////////////////////////////////////////////////
cl_err_code FileLogHandler::Init(ELogLevel level, const char* fileName, const char* title)
{
    if (m_handle == NULL)
	{
        return CL_ERR_INITILIZATION_FAILED;
	}

    m_logLevel = level;       // retrieve this info from Logger (not implemented yet)

    m_fileName = strdup_safe(fileName);
    if (m_fileName)
    {
        m_fileHandler = fopen(m_fileName, "w" );
        if(NULL == m_fileHandler)
        {
            printf("can't open log file for writing\n");
            return CL_ERR_LOGGER_FAILED;
        }

		{
			// Lock
			OclAutoMutex CS(&m_CS);
			const char* pTitle = (NULL == title) ?
				"\n##########################################################################################################\n" :
				title;

			if (!fprintf(m_fileHandler, pTitle) )
			{
				printf("fwrite failed\n");
				assert(false);
				return CL_ERR_LOGGER_FAILED;
			}
			Flush();
			// Unlock
		}

		return CL_SUCCESS;
    }

	return CL_ERR_LOGGER_FAILED;
}

/////////////////////////////////////////////////////////////////////////////////////////
// FileLogHandler::Log
/////////////////////////////////////////////////////////////////////////////////////////
void FileLogHandler::Log(LogMessage& logMessage)
{
    // get lock
    if (m_logLevel > logMessage.GetLogLevel())
    {
        // ignore messages with lower log level
        return;
    }

    char* formattedMsg = logMessage.GetFormattedMessage();
    {
        // Lock
        OclAutoMutex CS(&m_CS);
        if (!fprintf(m_fileHandler, "%s", formattedMsg) )
        {
            printf("fwrite failed\n");
            assert(false);
            return;
        }
        Flush();

        // Unlock
    }
    return;
}

void FileLogHandler::LogW(LogMessage& logMessage)
{
    // get lock
    if (m_logLevel > logMessage.GetLogLevel())
    {
        // ignore messages with lower log level
        return;
    }

    char* formattedMsg = logMessage.GetFormattedMessage();
    {
        // Lock
        OclAutoMutex CS(&m_CS);
        if (!fprintf(m_fileHandler, formattedMsg) )
        {
            printf("fwrite failed\n");
            assert(false);
            return;
        }
        Flush();

        // Unlock
    }
    return;
}

/////////////////////////////////////////////////////////////////////////////////////////
// FileLogHandler::Flush
/////////////////////////////////////////////////////////////////////////////////////////
void FileLogHandler::Flush()
{
	if (m_fileHandler)
	{
		fflush(m_fileHandler);     // thread safe
	}
}


/////////////////////////////////////////////////////////////////////////////////////////
// ConsoleLogHandler Ctor Implementation
/////////////////////////////////////////////////////////////////////////////////////////
ConsoleLogHandler::ConsoleLogHandler(const char* handle)
{
    if (handle)
    {
        free(m_handle);
        m_handle = strdup_safe(handle);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// ConsoleLogHandler::Init
/////////////////////////////////////////////////////////////////////////////////////////
cl_err_code ConsoleLogHandler::Init(ELogLevel level, const char* fileName, const char* title)
{
    if (m_handle == NULL)
	{
        return CL_ERR_INITILIZATION_FAILED;
	}

    m_logLevel = level;           // retrieve this info from Logger (not implemented yet)

	return CL_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////
// ConsoleLogHandler::Flush
/////////////////////////////////////////////////////////////////////////////////////////
void ConsoleLogHandler::Flush()
{
    fflush(stdout);     // thread safe
}

/////////////////////////////////////////////////////////////////////////////////////////
// ConsoleLogHandler::Log
/////////////////////////////////////////////////////////////////////////////////////////
void ConsoleLogHandler::Log(LogMessage& logMessage)
{
    if (m_logLevel > logMessage.GetLogLevel())
    {
        // ignore messages with lower log level
        return;
    }

    char* formattedMsg = logMessage.GetFormattedMessage();
    {
        // Lock
        OclAutoMutex CS(&m_CS);
        fprintf ( stdout, "%s", formattedMsg) ;
        Flush();
        // UnLock
    }
}
