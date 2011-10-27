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

File Name:  Buffer.h

\*****************************************************************************/
#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <assert.h>
#include "IMemoryObject.h"                    // IMemoryObject declaration
#include "llvm/System/DataTypes.h"      // llvm data types
#include "Exception.h"


namespace Validation
{
    class BufferContainer;
    class IContainerVisitor;

    /// Class for containing data.
    class Buffer : public IMemoryObject
    {
    public:
        /// @brief Initializing ctor. Allocates memory for buffer's data using
        /// values from buffer description.
        Buffer(const BufferDesc& desc);

        virtual ~Buffer();

        virtual void* GetDataPtr() const
        {
            return (void *)m_data;
        }

        virtual const IMemoryObjectDesc* GetMemoryObjectDesc() const
        {
            return &m_desc;
        }

        void Accept( IContainerVisitor& visitor ) const;

        virtual std::string GetName() const {return GetBufferName();}

        static std::string GetBufferName() {return std::string("Buffer");}

    private:
        /// hide copy constructor
        Buffer(const Buffer& ) : IMemoryObject(), m_desc(0, INVALID_WIDTH, INVALID_DATA_TYPE, false){}

        /// hide assignment operator
        void operator =(Buffer&){}

        /// @brief Allocates memory for buffer's data using existing buffer
        /// description values
        void AllocateMemoryForData();
        /// Buffer's data values
        uint8_t* m_data;
        /// Buffer description containing types of values, size of buffer, etc.
        BufferDesc m_desc;
        /// declare friend
        friend class BufferContainer;
    };

    BufferDesc GetBufferDescription(const IMemoryObjectDesc* iDesc);

    template<typename T>
    class BufferAccessor
    {
    public:
        /// @brief ctor. Specify buffer that you want to work with
        BufferAccessor(const IMemoryObject& in_buf):
          m_buf(in_buf)
          {
              BufferDesc m_desc = GetBufferDescription(in_buf.GetMemoryObjectDesc());
              if (m_desc.GetElementDescription().IsStruct())
                  throw Exception::InvalidArgument("Stuctures are not supported by buffer accessor");
              if(m_desc.GetElementDescription().IsAggregate() && !m_desc.GetElementDescription().IsStruct() && 
                 m_desc.GetElementDescription().GetSubTypeDesc(0).GetSizeInBytes() != sizeof(T))
                  throw Exception::InvalidArgument("Buffer accessor type parameter size is different from element size in buffer desc");
              if (!m_desc.GetElementDescription().IsComposite() && m_desc.GetElementDescription().GetSizeInBytes() != sizeof(T))
                  throw Exception::InvalidArgument("Buffer accessor type parameter size is different from element size in buffer desc");
              m_data = m_buf.GetDataPtr();
              // Calculate address
              m_SizeOfVector = m_desc.GetElementDescription().GetSizeInBytes();
              m_SizeOfVectorElementDataType = m_desc.GetElementDescription().IsComposite() ?
                  m_desc.GetElementDescription().GetSubTypeDesc(0).GetSizeInBytes() :
                  m_desc.GetElementDescription().GetSizeInBytes();

          }

          /// @brief Gets
          /// @param in_vecIndex  Index of vector that contains element
          /// @param in_index     Index of element in the vector
          /// As this is template class we need to place declaration here
          inline T& GetElem(const std::size_t in_vecIndex, const std::size_t in_offset) const
          {
              // Calculate address
              T* dataPointer = reinterpret_cast<T*>(
                  reinterpret_cast<char*>(m_data) + 
                  m_SizeOfVector * in_vecIndex + in_offset
                  * m_SizeOfVectorElementDataType);
              T& res = *dataPointer;
              return res;
          }

          inline void SetElem(const std::size_t in_vecIndex, const std::size_t in_offset, const T& newVal)
          {
// TODO: switch to DEBUG when done with debugging WOLF
#ifdef DEBUG1
              {
                  // check we were constructed for the same BufferDesc params
                  // significantly slows element access in buffer
                  BufferDesc desc;
                  desc = GetBufferDescription(m_buf.GetMemoryObjectDesc());
                  assert(m_buf.GetDataPtr() == m_data);
                  assert(desc.GetElementDescription().GetSizeInBytes() == m_SizeOfVector);
              }
#endif

              // Calculate address
              T* dataPointer = reinterpret_cast<T*>(
                  reinterpret_cast<char*>(m_data) + 
                  m_SizeOfVector * in_vecIndex + in_offset * m_SizeOfVectorElementDataType);
              *dataPointer = newVal;
          }

    private:
        const IMemoryObject& m_buf;
        /// Hide assignment operator for BufferAccessor
        BufferAccessor & operator=(const BufferAccessor &ba) {}
        /// Hide copy constuctor
        BufferAccessor(const BufferAccessor &b) {}
        // pointer to data
        void *m_data;
        /// number of vector elements
        size_t m_SizeOfVector;
        //. size of vector element in bytes
        size_t m_SizeOfVectorElementDataType;
    };

} // End of Validation namespace

#endif // __BUFFER_H__

