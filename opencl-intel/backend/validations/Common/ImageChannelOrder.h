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

File Name:  ImageChannelOrder.h

\*****************************************************************************/
#ifndef __IMAGE_CHANNEL_ORDER_H__
#define __IMAGE_CHANNEL_ORDER_H__

#include <cstddef>                      // for std::size_t
#include <string>
#include <map>
#include <cassert>

#include "Exception.h"

namespace Validation
{
    /// Structure describing type of elements in Buffer
    /// ImageChannelOrderVal's first element must be 0 and last element must be INVALID_CHANNEL_ORDER
    enum ImageChannelOrderVal
    {
        OpenCL_R = 0,
        OpenCL_Rx,
        OpenCL_A,
        OpenCL_INTENSITY,
        OpenCL_LUMINANCE,
        OpenCL_RG,
        OpenCL_RGx,
        OpenCL_RA,
        OpenCL_RGB,
        OpenCL_RGBx,
        OpenCL_RGBA,
        OpenCL_ARGB,
        OpenCL_BGRA,

        // marker for invalid order
        UNSPECIFIED_CHANNEL_ORDER,
        INVALID_CHANNEL_ORDER
    };

    inline size_t GetChannelCount(const ImageChannelOrderVal& order)
    {
        switch( order )
        {
        case OpenCL_R:
        case OpenCL_A:
        case OpenCL_Rx:
        case OpenCL_INTENSITY:
        case OpenCL_LUMINANCE:
            return 1;

        case OpenCL_RG:
        case OpenCL_RA:
        case OpenCL_RGx:
            return 2;

        case OpenCL_RGB:
        case OpenCL_RGBx:
            return 3;

        case OpenCL_RGBA:
        case OpenCL_ARGB:
        case OpenCL_BGRA:
            return 4;

        default:
            throw Exception::InvalidArgument("GetChannelCount::Unknown ImageChannelOrderVal value");
        }
    }
    /// Data type container and its helper functions
    class ImageChannelOrderValWrapper
    {
    public:
        /// default ctor
        ImageChannelOrderValWrapper() 
            : m_value(UNSPECIFIED_CHANNEL_ORDER)
        {
            if (!m_isStaticInit) initStatic();
        };

        explicit ImageChannelOrderValWrapper(const ImageChannelOrderVal& value) : m_value(value) 
        {
            if (!m_isStaticInit) initStatic();
            // if there is no metadata for value
            CheckValueAndThrow(value);
        };

        ImageChannelOrderVal GetValue() const
        {
            return m_value;
        };

        void SetValue(ImageChannelOrderVal val)
        {
            // if there is no metadata for value
            CheckValueAndThrow(val);
            m_value = val;
        };

        /// @return number of channels
        std::size_t GetSize() const { return m_metaData[m_value].m_size; };
        /// @returns  Text representation of image channel order
        std::string ToString() const { return m_metaData[m_value].m_toString; };

        static ImageChannelOrderVal ValueOf(const std::string& str)
        {
            // init static members
            if (!m_isStaticInit) initStatic(); 
            // ImageChannelOrderVal first element must be 0 and last element must be INVALID_CHANNEL_ORDER
            for (int  i = 0; i < INVALID_CHANNEL_ORDER; i++)
            {
                ImageChannelOrderVal dt = (ImageChannelOrderVal) i;
                // should consider  comparing the strings converted to lower case to allow more flexibility
                if (m_metaData[dt].m_toString == str) {
                    return dt;
                }
            }
            throw Exception::InvalidArgument("NonSupported Image channel order " + str);
        };

        inline bool operator == (const ImageChannelOrderValWrapper& a) const
        {
            return (a.m_value == m_value);
        }

    private:

        ImageChannelOrderVal m_value;

        inline void CheckValueAndThrow(const ImageChannelOrderVal& in_value)
        {
            assert(m_isStaticInit);
            if (m_metaData.count(in_value) < 1)
            {
                throw Exception::InvalidArgument("Invalid arg. No metadata for this ImageChannelOrder");
            }
        }

        void static initStatic();

        class ImageChannelOrderMetadata
        {
        public:
            ImageChannelOrderMetadata()
                : m_size(0), m_toString(""){};

            ImageChannelOrderMetadata(std::size_t size, const std::string& toString) 
                : m_size(size), m_toString(toString) {};

            std::size_t m_size;
            std::string m_toString;
        };

        static std::map<ImageChannelOrderVal, ImageChannelOrderMetadata> m_metaData;
        static bool m_isStaticInit;
    };

} // namespace Validation

#endif // __IMAGE_CHANNEL_ORDER_H__

