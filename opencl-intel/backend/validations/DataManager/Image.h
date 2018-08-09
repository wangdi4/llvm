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

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "IMemoryObject.h"                    // IMemoryObject declaration
#include "llvm/Support/DataTypes.h"      // LLVM data types
#include "Exception.h"
#include "ImageDesc.h"

namespace Validation
{
    class BufferContainer;
    class IContainerVisitor;

    /// Class for containing data.
    class Image : public IMemoryObject
    {
    public:
        /// @brief Initializing ctor. Allocates memory for buffer's data using
        /// values from buffer description.
        Image(const ImageDesc& desc);

        virtual ~Image();

        virtual void* GetDataPtr() const
        {
            return (void *)m_data;
        }

        virtual const IMemoryObjectDesc* GetMemoryObjectDesc() const
        {
            return &m_desc;
        }

        virtual std::string GetName() const {return GetImageName();}

        static std::string GetImageName() {return std::string("Image");}

        void Accept( IContainerVisitor& visitor ) const;
    private:
        /// hide copy constructor
        Image(const Image& ) : IMemoryObject(), m_desc(){}

        /// hide assignment operator
        void operator =(Image&){}

        /// @brief Allocates memory for image's data using existing buffer
        /// description values
        void AllocateMemoryForData();
        /// Image's data values
        uint8_t* m_data;
        /// Image description containing types of values, size of buffer, etc.
        ImageDesc m_desc;
        /// declare friend
        friend class BufferContainer;
    };

    ImageDesc GetImageDescription(const IMemoryObjectDesc* iDesc);

} // End of Validation namespace

#endif // __IMAGE_H__

