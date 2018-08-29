// INTEL CONFIDENTIAL
//
// Copyright 2011-2018 Intel Corporation.
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

#ifndef __BUFFER_CONTAINER_LIST_H__
#define __BUFFER_CONTAINER_LIST_H__

#include <vector>
#include <cstddef>              // for std::size_t

#include "IContainer.h"
#include "IContainerVisitor.h"
#include "IBufferContainer.h"
#include "BufferContainer.h"
#include "IBufferContainerList.h"

namespace Validation
{

    /// @brief implementation class of IBufferContainerList
    class BufferContainerList : public IBufferContainerList
    {
    public:
        /// constructor
        BufferContainerList() {}
        // destructor deletes all BufferContainer objects in list
        virtual ~BufferContainerList()
        {
            if(!m_BCV.empty())
            {
                for(BufferContainerVector::iterator it = m_BCV.begin();
                    it != m_BCV.end();
                    ++it)
                {
                    delete *it;
                }
                m_BCV.clear();
            }
        }

        virtual std::size_t GetBufferContainerCount() const
        {
            return m_BCV.size();
        }

        virtual IBufferContainer* CreateBufferContainer()
        {
            // Create a Buffer and put it into container.
            BufferContainer* pBC = new BufferContainer();
            m_BCV.push_back(pBC);
            return pBC;
        }

        virtual IBufferContainer* GetBufferContainer(std::size_t id) const
        {
            return (m_BCV[id]);
        }

        void Accept( IContainerVisitor& visitor ) const
        {
            visitor.visitBufferContainerList(this);
            for(BufferContainerVector::const_iterator it = m_BCV.begin(); it != m_BCV.end(); ++it)
            {
                (*it)->Accept(visitor);
            }
        }

        uint32_t GetDataVersion() {
            return readVersion;
        }

        void SetDataVersion(uint32_t const inDataVersion) {
            readVersion = inDataVersion;
        }

    private:
        /// hide copy constructor
        BufferContainerList(const BufferContainerList& ) : 
           IBufferContainerList() {}
        /// hide assignment operator
        void operator =(BufferContainerList&){}

        typedef std::vector<BufferContainer*> BufferContainerVector;
        /// List of buffers
        BufferContainerVector m_BCV;

        /// data version read from data file
        uint32_t readVersion;

    };

} // End of Validation namespace

#endif // __BUFFER_CONTAINER_LIST_H__

