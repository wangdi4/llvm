/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ImageCallbackManager.h

\*****************************************************************************/
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
