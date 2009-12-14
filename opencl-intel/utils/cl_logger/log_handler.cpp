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

///////////////////////////////////////////////////////////////////////////////////////////////////
//  LogHandler.cpp
//  Created on: 10-Dec-2008 11:42:24 AM                      
//  Implementation of the log handler class
//  Original author: ulevy                     
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "log_handler.h"
#include "cl_synch_objects.h"
#include <assert.h>
#include <malloc.h>
#include <string.h>
using namespace Intel::OpenCL::Utils;

/////////////////////////////////////////////////////////////////////////////////////////
// FileLogHandler Ctor Implementation
/////////////////////////////////////////////////////////////////////////////////////////
FileLogHandler::FileLogHandler(const wchar_t* handle)
{
    if (NULL != handle)
    {
        m_handle = _wcsdup(handle);

        if (NULL != m_handle)
		{
            wcscpy_s(m_handle, wcslen(handle) + 1, handle);           
		}
        else
		{
            m_handle = NULL;
		}
    }

    m_fileName   = NULL;
    m_fileHandler = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// FileLogHandler Dtor
/////////////////////////////////////////////////////////////////////////////////////////
FileLogHandler::~FileLogHandler() 
{
    if (m_fileHandler)
	{
        fclose(m_fileHandler);
	}
    
    if (m_fileName)
	{
        free(m_fileName);
	}
}


/////////////////////////////////////////////////////////////////////////////////////////
// FileLogHandler::Init
/////////////////////////////////////////////////////////////////////////////////////////
cl_err_code FileLogHandler::Init(ELogLevel level, const wchar_t* fileName, const wchar_t* title)
{   
    if (m_handle == NULL)
	{
        return CL_ERR_INITILIZATION_FAILED;
	}

    m_logLevel = level;       // retrieve this info from Logger (not implemented yet)

    m_fileName = _wcsdup(fileName);
    if (m_fileName)
    {
        errno_t err;

        if(err = _wfopen_s(&m_fileHandler, m_fileName, L"wt" ))
        {
            wprintf(L"can't open log file for writing\n");
            return CL_ERR_LOGGER_FAILED;
        }    
        assert (m_fileHandler != NULL);

		{
			// Lock 
			OclAutoMutex CS(&m_CS);
			wchar_t* pTitle = (NULL == title) ?
				L"\n##########################################################################################################\n" :
				title;

			if (!fwprintf(m_fileHandler, pTitle) )
			{
				wprintf(L"fwrite failed\n");
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
        if (!fprintf(m_fileHandler, formattedMsg) )       
        {
            wprintf(L"fwrite failed\n");
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
               
    wchar_t* formattedMsg = logMessage.GetFormattedMessageW();
    {
        // Lock 
        OclAutoMutex CS(&m_CS);  
        if (!fwprintf(m_fileHandler, formattedMsg) )       
        {
            wprintf(L"fwrite failed\n");
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
ConsoleLogHandler::ConsoleLogHandler(const wchar_t* handle)
{
    if (handle)
    {
        m_handle = _wcsdup(handle);
        
        if (m_handle)
            wcscpy_s(m_handle, wcslen(handle) + 1, handle);           
        else
            m_handle = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// ConsoleLogHandler::Init
/////////////////////////////////////////////////////////////////////////////////////////
cl_err_code ConsoleLogHandler::Init(ELogLevel level, wchar_t* fileName)
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
        fprintf ( stdout, formattedMsg) ;    
        Flush();         
        // UnLock
    }        
}
void ConsoleLogHandler::LogW(LogMessage& logMessage)
{                    
    if (m_logLevel > logMessage.GetLogLevel())
    {
        // ignore messages with lower log level
        return;
    }          

    wchar_t* formattedMsg = logMessage.GetFormattedMessageW();    
    {   
        // Lock
        OclAutoMutex CS(&m_CS);
        fwprintf ( stdout, formattedMsg) ;    
        Flush();         
        // UnLock
    }        
}