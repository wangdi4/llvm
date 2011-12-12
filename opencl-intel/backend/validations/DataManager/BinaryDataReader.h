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

File Name:  BinaryDataReader.h

\*****************************************************************************/
#ifndef __BINARY_DATA_READER_H__
#define __BINARY_DATA_READER_H__

#include <string>
#include <fstream>
#include <vector>

#include "llvm/System/DataTypes.h"
#include "IDataReader.h"
#include "IBufferContainerList.h"
#include "IContainerVisitor.h"
#include "Exception.h"
#include "IMemoryObject.h"
#include "Buffer.h"
#include "Image.h"

namespace Validation
{
    /// @brief IBufferContainerList binary reader
    class BinaryContainerListReader: public IDataReader
    {
    public:

        /// @brief Ctor
        /// @param [in] - filename to read the from
        BinaryContainerListReader( const std::string& filename):
          m_stream( filename.c_str(), std::ios_base::in | std::ios_base::binary)
        {
          if( m_stream.fail() )
              throw Exception::IOError("BinaryContainerListReader: cannot open file");
        }

        /// @brief reads the file data and populate the given container
        /// @param [in] - pointer to IBufferContainerList
        void Read(IContainer * pContainer)
        {
          assert(NULL != pContainer);
          readBufferContainerList(static_cast<IBufferContainerList*>(pContainer));
        }

    private:

        template<class T> void read_value( T& value)
        {
            m_stream.read((char*)&value, sizeof(T));
            if( m_stream.fail() )
                throw Exception::IOError("BinaryContainerListReader: read failed");
        }

        void readBufferContainerList( IBufferContainerList* pContainerList)
        {
            assert(NULL != pContainerList);
            uint32_t containerCount = 0;
            read_value(containerCount);

            for( uint32_t i = 0; i < containerCount; ++i )
            {
                IBufferContainer* pBufferContainer = pContainerList->CreateBufferContainer();
                readBufferContainer(pBufferContainer);
            }
        }

        void read_marker(std::string& marker)
        {
            uint32_t markerSize;
            read_value(markerSize);
            std::vector<char> markerVal(markerSize+1);
            markerVal[markerSize] = '\0';
            m_stream.read(&markerVal[0], markerSize);
            if (m_stream.gcount() != markerSize)
            {
                throw Exception::IOError("Can't read memory object marker from the data file.");
            }
            marker = std::string(&markerVal[0]);
        }

        void read_element_desc(TypeDesc& td)
        {
            uint32_t typeValue;
            read_value(typeValue);
            td.SetType(TypeVal(typeValue));
            uint32_t sizeInBytes;
            read_value(sizeInBytes);
            td.SetTypeAllocSize(sizeInBytes);

            if (td.IsAggregate() || td.IsPointer())
            {
                uint64_t numOfSubElements;
                read_value(numOfSubElements);
                td.SetNumberOfElements(numOfSubElements);

                uint64_t numOfSubTypes;
                read_value(numOfSubTypes);
                td.SetNumOfSubTypes(numOfSubTypes);

                TypeDesc subTypeDesc;
                read_element_desc(subTypeDesc);
                td.SetSubTypeDesc(0, subTypeDesc);
                if (td.IsStruct())
                {
                    uint64_t offset;
                    read_value(offset);
                    subTypeDesc.SetOffsetInStruct(offset);
                    td.SetSubTypeDesc(0, subTypeDesc);
                    for (uint64_t i = 1; i < numOfSubTypes; ++i)
                    {
                        TypeDesc subTD;
                        read_element_desc(subTD);
                        read_value(offset);
                        subTD.SetOffsetInStruct(offset);
                        td.SetSubTypeDesc(i, subTD);
                    }
                }
            }
        }

        void readBufferContainer( IBufferContainer* pContainer)
        {
            assert( NULL != pContainer);

            uint32_t bufferCount = 0;
            read_value(bufferCount);

            for(uint32_t i = 0; i < bufferCount; ++i )
            {
                std::string marker;
                read_marker(marker);

                if (marker == Buffer::GetBufferName())
                {
                    // TODO: Fix all size_t in writing/reading.
                    // Potentially it is not portable.

                    //first read the buffer descriptor
                    uint32_t numOfElements;
                    read_value(numOfElements);
                    TypeDesc elemDesc;
                    read_element_desc(elemDesc);
                    uint32_t isNeat;
                    read_value(isNeat);
                    // then the buffer data
                    BufferDesc bd;
                    bd.SetNumOfElements((size_t)numOfElements);
                    bd.SetElementDecs(elemDesc);
                    bd.SetNeat(bool(isNeat));
                    IMemoryObject* pBuffer = pContainer->CreateBuffer(bd);
                    m_stream.read( (char*)pBuffer->GetDataPtr(), bd.GetBufferSizeInBytes());
                }
                else if (marker == Image::GetImageName())
                {
                    uint32_t signature; // it could be 2 or 3 for old version and it is ffffffff for other
                    uint32_t imageType2Read;
                    ImageTypeVal imageType;
                    ImageSizeDesc sizes;
                    ImageChannelOrderVal imageOrder;
                    ImageChannelDataTypeVal imageDataType;
                    uint32_t pixelSize;
                    bool isNEAT;
                    read_value(signature);
                    if(signature == 0xffffffff) {
                        // OpenCl 1.2 and higher
                        uint32_t versionHigh;
                        uint32_t versionLow;
                        read_value(versionHigh);
                        read_value(versionLow);
                        read_value(imageType2Read);
                        imageType = (ImageTypeVal)imageType2Read;
                        read_value(sizes); 
                    } else {
                        // OpenCl 1.1 read
                        ImageSizeDesc_1_1 oldSizes;
                        // here signature actually is num of dimensions
                        imageType = GetImageTypeFromDimCount(signature);
                        read_value(oldSizes);

                        sizes.width = oldSizes.width;
                        sizes.height = oldSizes.height;
                        sizes.depth = oldSizes.depth;
                        sizes.row = oldSizes.row;
                        sizes.slice = oldSizes.slice;
                    }                   
                    read_value(imageOrder);
                    read_value(imageDataType);
                    read_value(pixelSize);
                    read_value(isNEAT);
                    
                    ImageDesc imd(imageType, sizes, imageDataType, imageOrder, isNEAT);
                    assert((size_t)pixelSize == imd.GetElementSize() );

                    IMemoryObject* pImage = pContainer->CreateImage(imd);
                    m_stream.read( (char*)pImage->GetDataPtr(), imd.GetImageSizeInBytes());
                }
                else
                {
                    throw Exception::IOError(std::string("Buffer container do not support ")+marker);
                }
            }
        }

    private:
        std::fstream m_stream;
    };
} // End of Validation namespace

#endif // __BINARY_DATA_READER_H__

