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

File Name:  ImageDesc.h

\*****************************************************************************/
#ifndef __IMAGE_DESC_H__
#define __IMAGE_DESC_H__

#include "IMemoryObjectDesc.h"
#include "ImageChannelOrder.h"
#include "ImageChannelDataType.h"

namespace Validation
{
    struct ImageSizes {

        // sizes
        uint64_t width;
        uint64_t height;
        uint64_t depth;

        // size of lines in bytes (AKA pitch, widthStep)
        uint64_t row;
        uint64_t slice;

        ImageSizes()
            : width(0),
            height(0),
            depth(0),
            row(0),
            slice(0)
        {}
        // 2D size constructor
        ImageSizes(const uint64_t& in_width, const uint64_t& in_height, const uint64_t& in_row)
            : width(in_width), height(in_height), depth(0), row(in_row), slice(0)
        {}

        // 3D size constructor
        ImageSizes(const uint64_t& in_width, const uint64_t& in_height, const uint64_t& in_depth, 
            const uint64_t& in_row, const uint64_t& in_slice)
            : width(in_width), height(in_height), depth(in_depth), row(in_row), slice(in_slice)
        {}

        /// comparison
        /// !!! Compares only sizes,  row and slice are ignored
        inline bool operator == (const ImageSizes& a) const
        {
            bool res = true;
            res &= (a.width == width);
            res &= (a.height == height);
            res &= (a.depth == depth);
            //res &= (a.row == row);
            //res &= (a.slice == slice);
            return res;
        }
        
        inline bool operator != (const ImageSizes& a) const
        {
            return (*this != a);
        }

    };

    /// @brief Image description structure.
    /// Describes the data which is stored into an image.
    class ImageDesc : public IMemoryObjectDesc
    {
    public:
        /// @brief default ctor. 
        /// Fills object with INVALID data which should be later filled with correct values
        /// Default ctor is enabled for reading/writing object to file
        ImageDesc()
            : m_order(),
            m_dataType(),
            m_numOfDimensions(0),
            m_size(),
            m_isNEAT(false)
        {}

        /// @brief ctor of object
        /// @param in_numOfVectors - number of vectors in image
        /// @param in_vw - Vector Width i.e. Number of elements in vector
        ///                from enum VectorWidth. V1, ... V16
        /// @param in_dt - data type of elements in image
        /// @param in_isNEAT - this image contains NEAT intervals
        explicit ImageDesc(const size_t in_numOfDimensions,
            const ImageSizes in_sizes,
            const ImageChannelDataTypeVal in_dt,
            const ImageChannelOrderVal in_order,
            const bool in_isNEAT = false)
            : m_order(in_order),
            m_dataType(in_dt),
            m_numOfDimensions(in_numOfDimensions),
            m_size(in_sizes)
        {
            SetNeat(in_isNEAT);
        }

        /// get number of dimensions of image
        size_t GetNumOfDimensions() const {return m_numOfDimensions;}

        /// get size of image
        ImageSizes GetSizes() const {return m_size;}

        /// get channel order
        ImageChannelOrderVal GetImageChannelOrder() const {return m_order.GetValue();}
        
        /// get channel data type. in case of NEAT returns underlying data type
        ImageChannelDataTypeVal GetImageChannelDataType() const {return m_dataType.GetValue();}
        
        /// get image data size in bytes
        /// in case of NEAT returns correct number of bytes occupied by NEAT image
        inline size_t GetImageSizeInBytes() const {
            size_t res = 0;
            if(m_isNEAT)
            {
                assert( 2 == m_numOfDimensions);
                res = GetElementSize() * m_size.width * m_size.height;
            }
            else
            {
                if (2 == m_numOfDimensions)
                    res = m_size.row * m_size.height;
                else if (3 == m_numOfDimensions)
                    res =  m_size.slice * m_size.depth;
                else
                    throw Exception::OutOfRange("Incorrect number of image dimensions.");
            }
            return res;
        }

        /// obtain pixel size in bytes
        inline size_t GetElementSize() const {
            size_t res = 0;
            if(m_isNEAT)
            {
                res = m_order.GetSize() * sizeof(NEATValue);
            }
            else
            {
                res = CalcPixelSizeInBytes(m_dataType.GetValue(), m_order.GetValue());
            }
            return res;
        }

        /// If image contains NEAT
        virtual bool IsNEAT() const
        {
            return m_isNEAT;
        }

        /// Set Neat flag
        virtual void SetNeat(const bool in_IsNeat) { 
            // OpenCL images which can be written are 2D images (OpenCL 1.1). 
            // So NEAT tracks write only 2D images with Float pixel data type
            // other images are considered as accurate. NEAT assumes their pixel values are accurate 
            // and obtains them from Interpreter Context
            if(in_IsNeat)
            {
                assert((m_numOfDimensions == 2) && "ImageDesc with NEAT supports only 2D images");
                assert((m_dataType.GetValue() == OpenCL_FLOAT) && "ImageDesc with NEAT supports only FLOAT images");
                m_size = CalcSizeNEAT_2D(m_size, GetChannelCount(m_order.GetValue()));
            }
            m_isNEAT = in_IsNeat; 
        }

        /// assignment
        inline ImageDesc& operator = (const ImageDesc& a)
        {
            m_order =           a.m_order;
            m_dataType =        a.m_dataType;
            m_numOfDimensions = a.m_numOfDimensions;
            m_size =            a.m_size;
            m_isNEAT =          a.m_isNEAT;
            return *this;
        }

        /// clone ImageDesc
        virtual IMemoryObjectDesc * Clone() const
        {
            return new ImageDesc(*this);
        }

        /// @brief get Name of class
        virtual std::string GetName() const { return GetImageDescName(); }

        /// @brief Static Name of class
        static std::string GetImageDescName() { return "ImageDesc"; }

        /// Obtain DataType as string
        inline std::string DataTypeToString() const
        {
            return m_dataType.ToString();
        }

        /// Obtain image channel order as string
        inline std::string OrderToString() const
        {
            return m_order.ToString();
        }


        /// if ImageDesc has floating point values
        inline bool IsFloatingPoint() const
        {
            return m_dataType.IsFloatingPoint();
        }

        /// is image descriptors equal
        inline bool operator == (const ImageDesc& a) const
        {
            bool res = true;
            res &= (a.m_order == m_order);
            res &= (a.m_dataType == m_dataType);
            res &= (a.m_numOfDimensions == m_numOfDimensions);
            res &= (a.m_size == m_size);
            res &= (a.m_isNEAT == m_isNEAT);
            return res;
        }
        /// is image descriptors not equal
        inline bool operator != (const ImageDesc& a) const
        {
            return (!(*this == a));
        }

        /// helper template function to get pixel size 
        /// @param T - enum ImageChannelDataTypeVal
        /// @param nchannels - number of channels
        template<ImageChannelDataTypeVal T>
        static inline std::size_t GetPixelSizeInBytesT(const std::size_t& nchannels)
        {
            // use templated <> type to convert ImageChannelDataTypeVal to C type
            typedef typename Validation::ImageChannelDataTypeValToCType<T>::type pixtype;
            return nchannels * sizeof(pixtype); 
        }

        /// @brief get pixel size in bytes
        /// @param dt - channel datatype
        /// @param order - channel order
        static inline std::size_t CalcPixelSizeInBytes(const ImageChannelDataTypeVal& dt, 
            const ImageChannelOrderVal& order)
        {
            const std::size_t nchannels = GetChannelCount(order);
            std::size_t ret = 0;
            switch(dt){
               case OpenCL_SNORM_INT8 : ret = GetPixelSizeInBytesT<OpenCL_SNORM_INT8>(nchannels); break;
               case OpenCL_SNORM_INT16 : ret = GetPixelSizeInBytesT<OpenCL_SNORM_INT16>(nchannels); break;
               case OpenCL_UNORM_INT8 : ret = GetPixelSizeInBytesT<OpenCL_UNORM_INT8>(nchannels); break;
               case OpenCL_UNORM_INT16 : ret = GetPixelSizeInBytesT<OpenCL_UNORM_INT16>(nchannels); break;
               case OpenCL_UNORM_SHORT_565 :
                   { 
                       // packed into uint16_t
                       typedef ImageChannelDataTypeValToCType<OpenCL_UNORM_SHORT_565>::type pixtype;
                       ret = sizeof(pixtype); 
                       break;
                   }
               case OpenCL_UNORM_SHORT_555 :
                   { 
                       // packed into uint16_t
                       typedef ImageChannelDataTypeValToCType<OpenCL_UNORM_SHORT_555>::type pixtype;
                       ret = sizeof(pixtype); 
                       break;
                   }
               case OpenCL_UNORM_INT_101010 :
                   { 
                       // packed into uint32_t
                       typedef ImageChannelDataTypeValToCType<OpenCL_UNORM_INT_101010>::type pixtype;
                       ret = sizeof(pixtype); 
                       break;
                   }
               case OpenCL_SIGNED_INT8 : ret = GetPixelSizeInBytesT<OpenCL_SIGNED_INT8>(nchannels); break;
               case OpenCL_SIGNED_INT16 : ret = GetPixelSizeInBytesT<OpenCL_SIGNED_INT16>(nchannels); break;
               case OpenCL_SIGNED_INT32 : ret = GetPixelSizeInBytesT<OpenCL_SIGNED_INT32>(nchannels); break;
               case OpenCL_UNSIGNED_INT8 : ret = GetPixelSizeInBytesT<OpenCL_UNSIGNED_INT8>(nchannels); break;
               case OpenCL_UNSIGNED_INT16 : ret = GetPixelSizeInBytesT<OpenCL_UNSIGNED_INT16>(nchannels); break;
               case OpenCL_UNSIGNED_INT32 : ret = GetPixelSizeInBytesT<OpenCL_UNSIGNED_INT32>(nchannels); break;
               case OpenCL_HALF_FLOAT : ret = GetPixelSizeInBytesT<OpenCL_HALF_FLOAT>(nchannels); break;
               case OpenCL_FLOAT : ret = GetPixelSizeInBytesT<OpenCL_FLOAT>(nchannels); break;
               default:
                   throw Exception::InvalidArgument("GetPixelSizeInBytes::Unknown image pixel format");
            }
            return ret;
        }

    protected:
        // image size in bytes for NEAT
        inline ImageSizes CalcSizeNEAT_2D(const ImageSizes& in, const uint32_t &nchannels) const
        {
            const uint64_t pitchNEAT = in.width * nchannels * sizeof(NEATValue);
            return ImageSizes(in.width, in.height, pitchNEAT);
        }
    private:
        ImageChannelOrderValWrapper m_order; ///< channels
        ImageChannelDataTypeValWrapper m_dataType; ///< data type 
        size_t m_numOfDimensions;               ///< 2D or 3D image
        ImageSizes m_size;                      ///< size of image including pitch
        bool    m_isNEAT;                       ///< Is image contains NEAT structures
    };

} // End of Validation namespace
#endif // __IMAGE_DESC_H__
