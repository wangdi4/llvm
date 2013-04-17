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

File Name:  Image.cpp

\*****************************************************************************/
#include "Image.h"
#include "IContainerVisitor.h"
#include "llvm/Support/DataTypes.h"

namespace Validation
{

    ///////////////////////////////////
    /// Image implementation
    ///////////////////////////////////////
    void Image::AllocateMemoryForData()
    {
        // Compute size of memory to allocate
        std::size_t buffSize = m_desc.GetImageSizeInBytes();
        // Allocate memory
        m_data = new uint8_t[buffSize];
    }

    Image::Image( const ImageDesc& desc ) : m_desc(desc)
    {
        // Allocate memory
        AllocateMemoryForData();
    }

    Image::~Image()
    {
        if (0 != m_data)
        {
            delete[] m_data;
        }
    }

    void Image::Accept( IContainerVisitor& visitor ) const
    {
        visitor.visitImage(this);
    }

    ImageDesc GetImageDescription( const IMemoryObjectDesc* iDesc )
    {
        const ImageDesc * desc = static_cast<const ImageDesc *>(iDesc);
        if (NULL != desc)
            return *desc;
        else
            throw Exception::InvalidArgument("ImageDesc is expected");
    }

    ImageTypeVal GetImageTypeFromDimCount(uint32_t dim_count, bool isArray) {
        ImageTypeVal imageType = UNSPECIFIED_MEM_OBJECT_IMAGE;
        if (dim_count == 1) {
            imageType = OpenCL_MEM_OBJECT_IMAGE1D;
        }  else if (dim_count == 2) {
            imageType = isArray ? OpenCL_MEM_OBJECT_IMAGE1D_ARRAY : OpenCL_MEM_OBJECT_IMAGE2D;
        }   else if (dim_count == 3) {
            imageType = isArray ? OpenCL_MEM_OBJECT_IMAGE2D_ARRAY : OpenCL_MEM_OBJECT_IMAGE3D;
        }
        return imageType;
    }

} // End of Validation namespace

