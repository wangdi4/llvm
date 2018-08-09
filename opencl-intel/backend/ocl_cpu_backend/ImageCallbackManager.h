// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once
#include "CPUDetect.h"
#include "ICompilerConfig.h"
#include "ImageCallbackLibrary.h"

#include <map>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class ImageCallbackLibrary;
class ImageCallbackFunctions;

//*****************************************************************************************
// Responsible for loading builtin modules in a lazy fashion. This is a singleton class.
//
class ImageCallbackManager
{
private:
    ImageCallbackManager();
    ~ImageCallbackManager();

public:
    /**
     * Static singleton intialization
     */
    static void Init();
    /**
     * Static singleton termination
     */
    static void Terminate();
    /**
     * Singleton instance
     */
    static ImageCallbackManager* GetInstance();

    /**
     * Initializes the \see BuiltinsLibrary for the given cpu if it hasn't been loaded before.
     */
    bool InitLibrary(const ICompilerConfig& config, bool isCpu, Intel::CPUId& cpuId);

    /***
    * Returns the image callback functions per architecture
    ****/

    ImageCallbackFunctions* getCallbackFunctions(const Intel::CPUId&);

private:
    typedef std::map<Intel::CPUId, ImageCallbackLibrary*> ImageCallbackMap;

    static ImageCallbackManager* s_pInstance;
    ImageCallbackMap m_ImageCallbackLibs;
};

}}}
