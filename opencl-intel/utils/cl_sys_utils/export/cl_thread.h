/////////////////////////////////////////////////////////////////////////
// cl_thread.h:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2008 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related 
// to the source code ("Material") are owned by Intel Corporation or its 
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and 
// proprietary and confidential information of Intel Corporation and its 
// suppliers and licensors, and is protected by worldwide copyright and trade 
// secret laws and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, distributed, 
// or disclosed in any way without Intel’s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////
#ifndef __CL_THREAD_H__
#define __CL_THREAD_H__

#include <string>

namespace Intel { namespace OpenCL { namespace Utils {

    /************************************************************************
     * OclThread is an abstract thread class.
     * To implement a thread in the system, inherit this class and implement the
     * Run function.
     * This class provides basic thread managing methods such as: start, join, terminate
     * Set bAutoDelete to true only for dynamicaly created objects and expected to be terminated by themself
    /************************************************************************/ 
    class OclThread
    {
    public:
		OclThread(std::string name = "", bool bAutoDelete = false);
	    virtual ~OclThread();

        virtual int         Start();
        virtual int         Join();
        virtual int         WaitForCompletion();
        void                Terminate(unsigned int exitCode);
	    int                 SetAffinity(unsigned char ucAffinity);
        bool                IsRunning() const               {return m_running;}    
        unsigned int        GetThreadId() const             {return m_threadId;}
        void                Clean();

    protected:
        virtual int         Run()=0;          // The actual thread running loop.    
        void                Exit(unsigned int exitCode);

        static unsigned int _stdcall ThreadEntryPoint( void* threadObject );
       
        void*               m_threadHandle;
        unsigned int        m_threadId;
        bool                m_running;
        bool                m_join;
		bool				m_bAutoDelete;
		std::string			m_Name;

    private:
        // A thread object cannot be copied
        OclThread(const OclThread&);           // copy constructor
        OclThread& operator=(const OclThread&);// assignment operator
    };

}}};    // Intel::OpenCL::Utils
#endif // __CL_THREAD_H__
