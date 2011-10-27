/*****************************************************************************\

Copyright (c) Intel Corporation (2010, 2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  BinaryDataWriter.h

\*****************************************************************************/
#ifndef __BINARY_DATA_WRITER_H__
#define __BINARY_DATA_WRITER_H__

#include <string>
#include <fstream>
#include "llvm/System/DataTypes.h"

#include "Exception.h"
#include "IDataWriter.h"
#include "IBufferContainerList.h"
#include "IContainerVisitor.h"
#include "BufferDesc.h"
#include "Buffer.h"
#include "ImageDesc.h"
#include "Image.h"

namespace Validation
{
    /// @brief IBufferContainerList object writer to data file in Binary format
    /// Implements IDataWriter interface
    /// Usage:
    ///     Could be used in two main ways:
    ///     1.  BufferContainerList list;
    ///         BinaryContainerListWriter writer("filename");
    ///         writer.Write(&list);
    /// 
    ///     2.  BinaryContainerListWriter writer("filename");
    ///         BufferContainerList list;
    ///         list.Accept(&writer);
    ///         writer.Flush();
    class BinaryContainerListWriter: public IDataWriter, public IContainerVisitor
    {
    public:
        /// @brief Ctor.
        /// @param [in] - fileName name of file to write to
        BinaryContainerListWriter( const std::string& filename):
          m_stream( filename.c_str(), std::ios_base::out | std::ios_base::binary)
          {}

          /// @brief write data from IBufferContainerList object to stream
          /// @param [in] - pContainer pointer to object with IBufferContainerList interface
          void Write( const IContainer *pContainer )
          {
              assert( NULL != pContainer);
              pContainer->Accept(*this);
              Flush();
          }

        /// @brief flushes the stream to the file
        /// @param [in] - pContainer pointer to object with IBufferContainerList interface
        void Flush()
        {
            m_stream.flush();
        }
    private:

        template<class T> void write_value( T value)
        {
            m_stream.write((const char*)&value, sizeof(T));
        }

        void write_marker(const std::string& marker)
        {
            uint32_t markerSize = (uint32_t)marker.size();
            m_stream.write((const char*)&markerSize, sizeof(uint32_t));
            m_stream.write((const char*)marker.c_str(), markerSize);
        }

        void write_element_desc (TypeDesc td)
        {
            write_value((uint32_t)td.GetType());
            write_value((uint32_t)td.GetSizeInBytes());
            // if we are saving vector, array or pointer data type, no need 
            // to store description of all elements of array or vector. They are all the same.
            if (td.IsAggregate() || td.IsPointer())
            {
                uint64_t numOfSubElements = td.GetNumberOfElements();
                write_value(numOfSubElements);

                uint64_t numOfSubTypes = td.GetNumOfSubTypes();
                write_value(numOfSubTypes);
                write_element_desc(td.GetSubTypeDesc(0));
                if (td.IsStruct())
                {
                    write_value(td.GetSubTypeDesc(0).GetOffsetInStruct());
                    for (uint64_t i = 1; i < numOfSubTypes; ++i)
                    {
                        write_element_desc(td.GetSubTypeDesc(i));
                        write_value(td.GetSubTypeDesc(i).GetOffsetInStruct());
                    }
                }
            }
        }

        void visitBuffer( const IMemoryObject* pBuffer )
        {
            assert( NULL != pBuffer );

            // Write marker.
            write_marker(pBuffer->GetName());

            // first we write down the descriptor
            BufferDesc bd  = GetBufferDescription(pBuffer->GetMemoryObjectDesc());
            write_value((uint32_t)bd.NumOfElements());
            write_element_desc(bd.GetElementDescription());
            write_value((uint32_t)bd.IsNEAT());

            size_t size = bd.GetBufferSizeInBytes();
            // then the buffer itself
            m_stream.write( (const char*)pBuffer->GetDataPtr(), size);
        }

        void visitImage( const IMemoryObject* pImage )
        {
            assert( NULL != pImage );

            write_marker(pImage->GetName());

            // first we write down the descriptor
            ImageDesc imDesc = GetImageDescription(pImage->GetMemoryObjectDesc());
            write_value((uint32_t)imDesc.GetNumOfDimensions());
            write_value(imDesc.GetSizes());
            write_value(imDesc.GetImageChannelOrder());
            write_value(imDesc.GetImageChannelDataType());
            write_value((uint32_t)imDesc.GetElementSize());
            write_value(imDesc.IsNEAT());
            // then the image itself
            m_stream.write( (const char*)pImage->GetDataPtr(), imDesc.GetImageSizeInBytes());
        }

        void visitBufferContainer( const IBufferContainer* pBufferContainer)
        {
            assert( NULL != pBufferContainer);
            write_value((uint32_t)pBufferContainer->GetMemoryObjectCount());
        }

        void visitBufferContainerList( const IBufferContainerList* pBufferContainerList )
        {
            assert( NULL != pBufferContainerList );
            write_value((uint32_t)pBufferContainerList->GetBufferContainerCount());
        }

    private:
        std::fstream m_stream;
    };

} // End of Validation namespace

#endif // __BINARY_DATA_WRITER_H__

