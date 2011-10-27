/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  BufferContainerList.h

\*****************************************************************************/
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

    private:
        /// hide copy constructor
        BufferContainerList(const BufferContainerList& ) : 
           IBufferContainerList() {}
        /// hide assignment operator
        void operator =(BufferContainerList&){}

        typedef std::vector<BufferContainer*> BufferContainerVector;
        /// List of buffers
        BufferContainerVector m_BCV;
    };

} // End of Validation namespace

#endif // __BUFFER_CONTAINER_LIST_H__

