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

File Name:  Image.h

\*****************************************************************************/
#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "IMemoryObject.h"                    // IMemoryObject declaration
#include "llvm/System/DataTypes.h"      // llvm data types
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

