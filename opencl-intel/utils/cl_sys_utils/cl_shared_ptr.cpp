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

#include "cl_shared_ptr.hpp"

using namespace std;

namespace Intel { namespace OpenCL { namespace Utils {

#if _DEBUG
/* These are defined as pointers, since in Linux the order of unloading shared libraries in the exit flow is undefined and objects in data segment might be destroyed before they are used
   in a shared library that has not yet been unloaded. */
OclMutex* allocatedObjectsMapMutex;
map<string, map<const void*, long> >* allocatedObjectsMap;
#endif

void InitSharedPtrs()
{
#if _DEBUG
    if (NULL == allocatedObjectsMapMutex)
    {
        allocatedObjectsMapMutex = new OclMutex();
    }
    if (NULL == allocatedObjectsMap)
    {
        allocatedObjectsMap = new map<string, map<const void*, long> >();
    }
#endif
}

void FiniSharedPts()
{
#if _DEBUG
    if (NULL != allocatedObjectsMapMutex)
    {
        delete allocatedObjectsMapMutex;
        allocatedObjectsMapMutex = NULL;
    }
    if (NULL != allocatedObjectsMap)
    {
        delete allocatedObjectsMap;
        allocatedObjectsMap = NULL;
    }
#endif
}

}}}
