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
//
//  Original author: rjiossy
///////////////////////////////////////////////////////////
#include "crt_config.h"
#include "crt_registry.h"

CrtConfig::CrtConfig()
{
}

CrtConfig::~CrtConfig()
{
}


cl_uint CrtConfig::getNumPlatforms()
{
    return (cl_uint)m_libraryNames.size();
}

std::string& CrtConfig::getPlatformLibName(cl_uint index)
{
    return m_libraryNames[index];
}

crt_err_code CrtConfig::Init()
{
    std::string cpuRuntimeLibName = "intelocl";
    std::string gpuRuntimeLibName = "igdrcl";

    // CPU-runtime library:
    std::string libName;
    std::string valueName;

    if (emulatorEnabled())
    {
        valueName = "cpu_2_1_emulator_path";
    }
    else
    {
        valueName = "cpu_path";
    }

    if( OCLCRT::Utils::GetCpuPathFromRegistry( valueName, libName ) )
    {
        // full path library name
        libName = libName + "\\" + OCLCRT::Utils::FormatLibNameForOS( cpuRuntimeLibName );
    }
    else
    {
        libName = OCLCRT::Utils::FormatLibNameForOS( cpuRuntimeLibName );
    }
    m_libraryNames.push_back( libName );

    if (!emulatorEnabled())
    {
        // GPU-runtime library:
        m_libraryNames.push_back( OCLCRT::Utils::FormatLibNameForOS( gpuRuntimeLibName ) );
    }

    return CRT_SUCCESS;
}

bool CrtConfig::emulatorEnabled()
{
#if defined( _WIN32 ) && defined( BUILD_EXPERIMENTAL_21 )
    return true;
#endif // defined( _WIN32 )
    return false;
}
