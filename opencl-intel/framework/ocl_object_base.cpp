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

#include <cassert>
#include "ocl_object_base.h"

namespace Intel { namespace OpenCL { namespace Framework {

void OCLObjectBase::PrintDependencyGraphRecursive(std::ostream& os, unsigned int uiIndent) const
{
    for (std::multiset<const OCLObjectBase*>::const_iterator iter = m_dependencySet.begin();
		 iter != m_dependencySet.end();
         iter++)
    {
        for (unsigned int i = 0; i < uiIndent; i++)
        {
            os << '\t';
        }
        const OCLObjectBase* const pObj = *iter;
        os << pObj->m_typename << " " << *iter << std::endl;
        pObj->PrintDependencyGraphRecursive(os, uiIndent + 1);
    }
}

void OCLObjectBase::PrintDependencyGraph(std::ostream& os)
{
    m_muAcquireRelease.Lock();
    os << std::hex;
    os << "Dependency graph for " << m_typename << " " << this << ":" << std::endl;
    PrintDependencyGraphRecursive(os, 1);
    m_muAcquireRelease.Unlock();
}

void OCLObjectBase::InsertToDependencySet(OCLObjectBase* pObj)
{
#if defined DEBUG_DEPENDENCY
    m_muAcquireRelease.Lock();
    m_dependencySet.insert(pObj);
    pObj->m_reverseDependencySet.insert(this);    
    m_muAcquireRelease.Unlock();
#endif
}

void OCLObjectBase::EraseFromDependecySet(OCLObjectBase* pObj)
{
#if defined DEBUG_DEPENDENCY
    m_muAcquireRelease.Lock();
    m_dependencySet.erase(m_dependencySet.find(pObj));   // remove only one occurrence
    pObj->m_reverseDependencySet.erase(pObj->m_reverseDependencySet.find(this));
    m_muAcquireRelease.Unlock();
#endif
}

OCLObjectBase::~OCLObjectBase()
{
#if defined DEBUG_DEPENDENCY
    assert(0 == m_dependencySet.size());
    assert(0 == m_reverseDependencySet.size());
#endif
}

}}}
