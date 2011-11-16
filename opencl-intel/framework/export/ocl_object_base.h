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

#pragma once

#include "cl_synch_objects.h"
#include <iostream>
#include <set>
#include <string>

namespace Intel { namespace OpenCL { namespace Framework {

/**
 * This represents a base class for all objects in the OpenCL runtime that can add dependency to
 * another object.
 * This super-class holds two multi-sets: a dependency set and a reverse dependency set. This
 * enables to maintains a dependency graph for each object, which can be printed at any moment.
 * In the destructor OCLObjectBase asserts that both the dependency and the reverse dependency sets
 * are empty, otherwise it means that there is a memory leak or a possible wild pointer. 
 */

class OCLObjectBase
{  
    const std::string m_typename;
    Intel::OpenCL::Utils::OclMutex m_muAcquireRelease;
    std::multiset<const OCLObjectBase*> m_dependencySet;
    std::multiset<const OCLObjectBase*> m_reverseDependencySet;

	void PrintDependencyGraphRecursive(std::ostream&, unsigned int uiIndent) const;
	
protected:

    /**
     * Constructor
     * 
     * @param typeName the name of the concrete object's type 
     */
    explicit OCLObjectBase(const std::string& typeName) : m_typename(typeName) { }

    /**
     * Insert an object to the dependency set - this object is dependent on the parameter object.
     *
     * @param pObj an object to insert to the dependency set
     */
    void InsertToDependencySet(OCLObjectBase* pObj);

    /**
     * Erase an object from the dependency set.
     * 
     * @param pObj an object to erase from the dependency set
     */
    void EraseFromDependecySet(OCLObjectBase* pObj);

public:

    /**
     * destructor
     */
    virtual ~OCLObjectBase();

    /**
     * Print the objects dependency graph
     * 
     * @param os an ostream to print the graph on
     */
    void PrintDependencyGraph(std::ostream& os);

};

}}}
