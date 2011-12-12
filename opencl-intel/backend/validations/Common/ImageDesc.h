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

    enum ImageTypeVal
    {
        OpenCL_MEM_OBJECT_IMAGE1D = 0,
        OpenCL_MEM_OBJECT_IMAGE1D_BUFFER, 
        OpenCL_MEM_OBJECT_IMAGE1D_ARRAY,
        OpenCL_MEM_OBJECT_IMAGE2D, 
        OpenCL_MEM_OBJECT_IMAGE2D_ARRAY,
        OpenCL_MEM_OBJECT_IMAGE3D,

        UNSPECIFIED_MEM_OBJECT_IMAGE,
        INVALID_MEM_OBJECT_IMAGE
    };

    // this struct is used to support binary reading from 1.1 images
    struct ImageSizeDesc_1_1 {
        // sizes
        uint64_t width;
        uint64_t height;
        uint64_t depth;
        // size of lines in bytes (AKA pitch, widthStep)
        uint64_t row;
        uint64_t slice;

        ImageSizeDesc_1_1()
            : width(0),
            height(0),
            depth(0),
            row(0),
            slice(0)
        {}
    };

    struct ImageSizeDesc {

        // sizes
        uint64_t width;
        uint64_t height;
        uint64_t depth;

        // size of lines in bytes (AKA pitch, widthStep)
        uint64_t row;
        uint64_t slice;
        uint64_t array_size;

        ImageSizeDesc()
            : width(0),
            height(0),
            depth(0),
            row(0),
            slice(0),
            array_size(0)
        {}

        void Init(ImageTypeVal imageType, const uint64_t& in_width, const uint64_t& in_height, const uint64_t& in_depth, 
            const uint64_t& in_row, const uint64_t& in_slice, const uint64_t& in_array_size)
        {
            switch(imageType) {
                case OpenCL_MEM_OBJECT_IMAGE1D :
                case OpenCL_MEM_OBJECT_IMAGE1D_BUFFER :
                    width = in_width;
                    row = in_row;
                    break;
                case OpenCL_MEM_OBJECT_IMAGE1D_ARRAY :
                    width = in_width;
                    row = in_row;
                    array_size = in_array_size;
                    break;                    
                case OpenCL_MEM_OBJECT_IMAGE2D :
                    width = in_width;
                    height = in_height;
                    row = in_row;
                    break;
                case OpenCL_MEM_OBJECT_IMAGE2D_ARRAY :
                    width = in_width;
                    height = in_height;
                    row = in_row;
                    array_size = in_array_size;
                    break;
                case OpenCL_MEM_OBJECT_IMAGE3D :
                    width = in_width;
                    height = in_height;
                    depth = in_depth;
                    row = in_row;
                    slice = in_slice;
                    break;
                default :
                    throw Exception::OutOfRange("Incorrect image type.");
             }
        }

        /// comparison
        /// !!! Compares only sizes,  row and slice are ignored
        inline bool operator == (const ImageSizeDesc& a) const
        {
            bool res = true;
            res &= (a.width == width);
            res &= (a.height == height);
            res &= (a.depth == depth);
            res &= (a.array_size == array_size);
            //res &= (a.row == row);
            //res &= (a.slice == slice);
            return res;
        }
        
        inline bool operator != (const ImageSizeDesc& a) const
        {
            return !(*this == a);
        }

    };

    ImageTypeVal GetImageTypeFromDimCount(uint32_t dim_count);

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
            m_imageType(),
            m_size(),
            m_isNEAT(false)
        {}

        /// @brief ctor of object
        /// @param in_numOfVectors - number of vectors in image
        /// @param in_vw - Vector Width i.e. Number of elements in vector
        ///                from enum VectorWidth. V1, ... V16
        /// @param in_dt - data type of elements in image
        /// @param in_isNEAT - this image contains NEAT intervals
        explicit ImageDesc(const ImageTypeVal in_imageType,
            const ImageSizeDesc in_sizes,
            const ImageChannelDataTypeVal in_dt,
            const ImageChannelOrderVal in_order,
            const bool in_isNEAT = false)
            : m_order(in_order),
            m_dataType(in_dt),
            m_imageType(in_imageType),
            m_size(in_sizes),
            m_num_mip_levels(0),
            m_num_samples(0)
        {
            SetNeat(in_isNEAT);

            // fixup row
            if( m_size.row == 0)
                m_size.row = m_size.width * GetElementSize();

            // fixup slice
            if(m_size.slice == 0) {
                if (m_imageType == OpenCL_MEM_OBJECT_IMAGE2D_ARRAY ||
                    m_imageType == OpenCL_MEM_OBJECT_IMAGE3D ) {
                    m_size.slice = m_size.row * m_size.height;
                }
                else if (m_imageType == OpenCL_MEM_OBJECT_IMAGE1D_ARRAY) {
                    m_size.slice = m_size.row;
                }
            }
        }

        /// get size description structure
        ImageSizeDesc GetSizesDesc() const {return m_size;}

        /// get channel order
        ImageChannelOrderVal GetImageChannelOrder() const {return m_order.GetValue();}
        
        /// get channel data type. in case of NEAT returns underlying data type
        ImageChannelDataTypeVal GetImageChannelDataType() const {return m_dataType.GetValue();}

        ImageTypeVal GetImageType() const {return m_imageType;}

        // function to get dimension count (OpenCL 1.1) from ImageTypeVal (OpenCL 1.2)
        uint32_t GetDimensionCount() const {
            if (m_imageType == OpenCL_MEM_OBJECT_IMAGE2D)
                return 2;
            else if (m_imageType == OpenCL_MEM_OBJECT_IMAGE3D)
                return 3;
            else
                return 0;
        }

        /// get image data size in bytes
        /// in case of NEAT returns correct number of bytes occupied by NEAT image
        inline size_t GetImageSizeInBytes() const {
            size_t res = 0;
            if(m_isNEAT)
            {
                switch(m_imageType) {
                    case OpenCL_MEM_OBJECT_IMAGE1D :
                    case OpenCL_MEM_OBJECT_IMAGE1D_BUFFER :
                        res = GetElementSize() * m_size.width;
                        break;
                    case OpenCL_MEM_OBJECT_IMAGE1D_ARRAY :
                        res = GetElementSize() * m_size.width * m_size.array_size;
                        break;
                    case OpenCL_MEM_OBJECT_IMAGE2D :
                        res = GetElementSize() * m_size.width * m_size.height;
                        break;
                    case OpenCL_MEM_OBJECT_IMAGE2D_ARRAY :
                        res =  GetElementSize() * m_size.width * m_size.height * m_size.array_size;
                        break;
                    case OpenCL_MEM_OBJECT_IMAGE3D :
                        res =  GetElementSize() * m_size.width * m_size.height * m_size.depth;
                        break;
                    default :
                        throw Exception::OutOfRange("Incorrect image type.");
                }               
            }
            else
            {
                switch(m_imageType) {
                    case OpenCL_MEM_OBJECT_IMAGE1D :
                    case OpenCL_MEM_OBJECT_IMAGE1D_BUFFER :
                        res = m_size.row;
                        break;
                    case OpenCL_MEM_OBJECT_IMAGE1D_ARRAY :
                        res = m_size.slice * m_size.array_size;
                        break;
                    case OpenCL_MEM_OBJECT_IMAGE2D :
                        res = m_size.row * m_size.height;
                        break;
                    case OpenCL_MEM_OBJECT_IMAGE2D_ARRAY :
                        res =  m_size.slice * m_size.array_size;
                        break;
                    case OpenCL_MEM_OBJECT_IMAGE3D :
                        res =  m_size.slice * m_size.depth;
                        break;
                    default :
                        throw Exception::OutOfRange("Incorrect image type.");
                }
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
                assert((m_dataType.GetValue() == OpenCL_FLOAT) && "ImageDesc with NEAT supports only FLOAT images");
                m_size = GetNEATImageSizeDesc(m_size, GetChannelCount(m_order.GetValue()));
            }
            m_isNEAT = in_IsNeat; 
        }

        /// assignment
        inline ImageDesc& operator = (const ImageDesc& a)
        {
            m_order =           a.m_order;
            m_dataType =        a.m_dataType;
            m_imageType =       a.m_imageType;
            m_size =            a.m_size;
            m_isNEAT =          a.m_isNEAT;
            m_num_mip_levels =    a.m_num_mip_levels;
            m_num_samples =       a.m_num_samples;
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
            res &= (a.m_imageType == m_imageType);
            res &= (a.m_size == m_size);
            res &= (a.m_isNEAT == m_isNEAT);
            res &= (a.m_num_mip_levels == m_num_mip_levels);
            res &= (a.m_num_samples == m_num_samples);
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
        inline ImageSizeDesc GetNEATImageSizeDesc(const ImageSizeDesc& in, const uint32_t &nchannels) const
        {
            ImageSizeDesc res;
            const uint64_t pitchNEAT = nchannels * sizeof(NEATValue) * in.width;
            const uint64_t sliceNEAT = pitchNEAT * in.height;

            res.Init(m_imageType, in.width, in.height, in.depth, pitchNEAT, sliceNEAT, in.array_size);

            return res;
        }
    private:
        ImageChannelOrderValWrapper m_order; ///< channels
        ImageChannelDataTypeValWrapper m_dataType; ///< data type
        ImageTypeVal m_imageType;
        ImageSizeDesc m_size;                   ///< size of image including pitch
        uint64_t m_num_mip_levels;              // reserved for future use
        uint64_t m_num_samples;                 // reserved for future use
        bool    m_isNEAT;                       ///< Is image contains NEAT structures
    };

} // End of Validation namespace
#endif // __IMAGE_DESC_H__
