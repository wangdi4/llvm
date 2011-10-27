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

File Name:  NEATALU.cpp


\*****************************************************************************/
#include "ImagesALU.h"
#include "NEATALU.h"

namespace Validation
{
    const double NEATALU::DIV_ERROR = 2.5;
    const double NEATALU::MIX_ERROR = 1.5; // rounding error of sub, mul and add
    const double NEATALU::NORMALIZE_ERROR = 2.5; // 2f + 0.5f  error in rsqrt + error in multiply
    const double NEATALU::NATIVE_TAN_ERROR = 2.16; // Measured by SVML team!!!

    const long double NEATALU::pi = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899L;
    const long double NEATALU::two_pi = 2*3.14159265358979323846264338327950288419716939937510582097494459230781640628620899L;
    const long double NEATALU::pi_2 = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899L/2.0L;
    const long double NEATALU::div180by_pi = 180.L/3.14159265358979323846264338327950288419716939937510582097494459230781640628620899L;

  NEATALU::NEATALU()
  {
  }

  NEATVector NEATALU::processVector(const NEATVector& vec1, const NEATVector& vec2, const NEATValue& val, NEATScalarTernaryOp f)
  {
      NEATVector toReturn(vec1.GetWidth());
      if(vec1.GetSize() != vec2.GetSize())
          throw Exception::InvalidArgument("Vectors with different sizes were passed to function");
      for(uint32_t i = 0; i<vec1.GetSize(); i++)
      {
          toReturn[i] = f(vec1[i], vec2[i], val);
      }
      return toReturn;
  }

  NEATVector NEATALU::processVector(const NEATVector& vec1, const NEATVector& vec2, const NEATVector& vec3, NEATScalarTernaryOp f)
  {
      NEATVector toReturn(vec1.GetWidth());
      if(vec1.GetSize() != vec2.GetSize())
          throw Exception::InvalidArgument("Vectors with different sizes were passed to function");
      if(vec1.GetSize() != vec3.GetSize())
          throw Exception::InvalidArgument("Vectors with different sizes were passed to function");

      for(uint32_t i = 0; i<vec1.GetSize(); i++)
      {
          toReturn[i] = f(vec1[i], vec2[i], vec3[i]);
      }
      return toReturn;
  }

  NEATVector NEATALU::processVector(const NEATVector& vec1, const NEATValue& val2, const NEATValue& val3, NEATScalarTernaryOp f)
  {
      NEATVector toReturn(vec1.GetWidth());
      for(uint32_t i = 0; i<vec1.GetSize(); i++)
      {
          toReturn[i] = f(vec1[i], val2, val3);
      }
      return toReturn;
  }

  NEATVector NEATALU::processVector(const NEATVector& vec, const NEATValue& val, NEATScalarBinaryOp f)
  {
      NEATVector toReturn(vec.GetWidth());
      for(uint32_t i = 0; i < vec.GetSize(); ++i)
      {
          toReturn[i] = f(vec[i], val);
      }
      return toReturn;
  }

  NEATVector NEATALU::processVector(const NEATValue& val, const NEATVector& vec, NEATScalarBinaryOp f)
  {
      NEATVector toReturn(vec.GetWidth());
      for(uint32_t i = 0; i < vec.GetSize(); ++i)
      {
          toReturn[i] = f(val, vec[i]);
      }
      return toReturn;
  }


  NEATVector NEATALU::processVector(const NEATVector& vec1, const NEATVector& vec2, NEATScalarBinaryOp f)
  {
      NEATVector toReturn(vec1.GetWidth());
      if(vec1.GetSize() != vec2.GetSize())
          throw Exception::InvalidArgument("Vectors with different sizes were passed to function");
      for(uint32_t i = 0; i<vec1.GetSize(); i++)
      {
          toReturn[i] = f(vec1[i], vec2[i]);
      }
      return toReturn;
  }

  NEATVector NEATALU::processVector(const NEATVector& vec, NEATScalarUnaryOp f)
  {
      NEATVector toReturn(vec.GetWidth());
      for(uint32_t i = 0; i<vec.GetSize(); i++)
      {
          toReturn[i] = f(vec[i]);
      }
      return toReturn;
  }

  NEATValue NEATALU::select (const bool& cond, const NEATValue& a, const NEATValue& b) {
    NEATValue res;

    if(cond)
      res = a;
    else
      res = b;

    return res;
  }

  NEATVector NEATALU::select (const bool& cond, const NEATVector& a, const NEATVector& b) {
    NEATVector vec1 = a;
    NEATVector vec2 = b;

    if(cond)
      return vec1;
    else
      return vec2;
  }

  NEATVector NEATALU::select (const std::vector<bool>& cond, const NEATVector& a, const NEATVector& b) {
    NEATVector vec1 = a;
    NEATVector vec2 = b;
    unsigned condSize = (unsigned)cond.size();

    if( (condSize != (unsigned)vec1.GetSize()) || (condSize != (unsigned)vec2.GetSize()))
      throw Exception::InvalidArgument("[NEATALU::select] wrong vector size\n");

    NEATVector res(vec1.GetWidth());

    for(unsigned int i=0;i<condSize;i++)
      res[i] = cond[i] ? vec1[i] : vec2[i];

    return res;
  }

    NEATValue NEATALU::extractelement ( const NEATVector& vec, const uint32_t& idx )
    {
        NEATValue val(NEATValue::UNWRITTEN);
        NEATVector srcVec = vec; // just to have const for vec

        if(srcVec.GetSize() > idx) {
            val = srcVec[idx];
        }

        return val;
    }

    NEATVector NEATALU::insertelement ( NEATVector vec, const NEATValue elt, const uint32_t& idx )
    {
        NEATVector dstVec = vec;
        if(dstVec.GetSize() > idx) {
            dstVec[idx] = elt;
        }

        return dstVec;
    }


    NEATVector NEATALU::shufflevector ( const NEATVector& vec1, const NEATVector& vec2,
                                      const std::vector<uint32_t>& mask)
    {
        NEATVector src1 = vec1;
        NEATVector src2 = vec2;
        unsigned src1Size = (unsigned)src1.GetSize();
        unsigned src2Size = (unsigned)src2.GetSize();
        unsigned maskSize = (unsigned)mask.size();

        // possible vector sizes: V1, V2, V3, V4, V8, V16
        VectorWidth dstSize;
        switch(maskSize){
            case 1: dstSize = V1;
                break;
            case 2: dstSize = V2;
                break;
            case 3: dstSize = V3;
                break;
            case 4: dstSize = V4;
                break;
            case 8: dstSize = V8;
                break;
            case 16: dstSize = V16;
                break;
            default:
                throw Exception::InvalidArgument(
                "[NEATALU::shufflevector] Wrong mask size");
                break;
        }

        NEATVector dstVec(dstSize);

        for( unsigned i=0; i<maskSize; i++) {
            dstVec[i] = NEATValue(NEATValue::UNWRITTEN);
            unsigned j = mask[i];
            if(j < src1Size) {
                dstVec[i] = src1[j];
            } else {
                if(j < src1Size + src2Size) {
                    dstVec[i] = src2[j-src1Size];
                } else {
                    throw Exception::InvalidArgument(
                    "[NEATALU::shufflevector] Wrong element selector value ");
                }
            }
        }

        return dstVec;
    }

    Validation::NEATVector NEATALU::read_imagef_src_noneat( void *imageData, Conformance::image_descriptor *imageInfo, 
        float x, float y, float z, Conformance::image_sampler_data *imageSampler )
    {
/*
         This function uses the same approach that is done in Conformance test images_kernel_read_write
         However this Conformance approach has some faults. 
         In most cases (linear filtering, address_repeat etc) conformance uses as error "magic number" MAKE_HEX_FLOAT(0x1.0p-7f, 0x1L, -7)
         In linear filtering Conformance does not take into account error introduced by number of multiplication/additions.
         For integer image data types it considers error produced by conversions from integer to float data type.
         In NEAT implementation we may revisit this "magic number" approach in future to be more strict.

         Current implementation of NEAT computes maximum relative error based on image_format. 
         Then it multiplies it by maximum pixel value used in sampling.
         Computes maximum absolute error for the image format.
         Selects maximum between error obtained with relative and maximum absolute error
         Obtained value is maximum allowed error. 
         This value is used to construct NEAT interval by adding/subtracting from accurate Pixel value obtained in sampling
*/
        Conformance::FloatPixel accPix;
        NEATVector retVec(V4);

        int haveDenorms;
        const int is3D = !(imageInfo->depth == 0);
        
        Conformance::FloatPixel maxPixel= Conformance::sample_image_pixel_float( imageData, 
            imageInfo,
            x, y, z,
            imageSampler, 
            accPix.p, 
            false, // verbose
            &haveDenorms );

        // Get the maximum relative error for this format
        const float maxErr = get_max_relative_error( imageInfo->format, imageSampler, 
            is3D, CL_FILTER_LINEAR == imageSampler->filter_mode );

        // Get the maximum absolute error for this format
        const double formatAbsoluteError = get_max_absolute_error(imageInfo->format, imageSampler); 

        const float maxErrAbs[4] = {
            MAX( maxErr * maxPixel.p[0], formatAbsoluteError ),
            MAX( maxErr * maxPixel.p[1], formatAbsoluteError ),
            MAX( maxErr * maxPixel.p[2], formatAbsoluteError ), 
            MAX( maxErr * maxPixel.p[3], formatAbsoluteError )
        };

        /// Initialize color with default value
        retVec[0].SetAccurateVal<float>(0.0f);
        retVec[1].SetAccurateVal<float>(0.0f);
        retVec[2].SetAccurateVal<float>(0.0f);
        retVec[3].SetAccurateVal<float>(1.0f);

        switch( imageInfo->format->image_channel_order )
        {
        case CL_A:
            retVec[3].SetIntervalVal<float>(accPix.p[3] - maxErrAbs[3], accPix.p[3] + maxErrAbs[3]);
            break;
        case CL_R:
        case CL_Rx:
            retVec[0].SetIntervalVal<float>(accPix.p[0] - maxErrAbs[0], accPix.p[0] + maxErrAbs[0]);
            break;
        case CL_RA:
            retVec[0].SetIntervalVal<float>(accPix.p[0] - maxErrAbs[0], accPix.p[0] + maxErrAbs[0]);
            retVec[3].SetIntervalVal<float>(accPix.p[3] - maxErrAbs[3], accPix.p[3] + maxErrAbs[3]);
            break;
        case CL_RG:
        case CL_RGx:
            retVec[0].SetIntervalVal<float>(accPix.p[0] - maxErrAbs[0], accPix.p[0] + maxErrAbs[0]);
            retVec[1].SetIntervalVal<float>(accPix.p[1] - maxErrAbs[1], accPix.p[1] + maxErrAbs[1]);
            break;
        case CL_RGB:
        case CL_RGBx:
        case CL_LUMINANCE:
            retVec[0].SetIntervalVal<float>(accPix.p[0] - maxErrAbs[0], accPix.p[0] + maxErrAbs[0]);
            retVec[1].SetIntervalVal<float>(accPix.p[1] - maxErrAbs[1], accPix.p[1] + maxErrAbs[1]);
            retVec[2].SetIntervalVal<float>(accPix.p[2] - maxErrAbs[2], accPix.p[2] + maxErrAbs[2]);
            break;
        case CL_RGBA:
        case CL_ARGB:
        case CL_BGRA:
        case CL_INTENSITY:
            retVec[0].SetIntervalVal<float>(accPix.p[0] - maxErrAbs[0], accPix.p[0] + maxErrAbs[0]);
            retVec[1].SetIntervalVal<float>(accPix.p[1] - maxErrAbs[1], accPix.p[1] + maxErrAbs[1]);
            retVec[2].SetIntervalVal<float>(accPix.p[2] - maxErrAbs[2], accPix.p[2] + maxErrAbs[2]);
            retVec[3].SetIntervalVal<float>(accPix.p[3] - maxErrAbs[3], accPix.p[3] + maxErrAbs[3]);
            break;
        default:
            throw Exception::InvalidArgument("ImagesALU::read_image_pixel_float Invalid format");
            break;
        }

        return retVec;
    }

    template<>
    NEATVector NEATALU::vload_half<3, true>(size_t offset, const uint16_t* p) {
        NEATVector result(VectorWidthWrapper::ValueOf(3));
        for (unsigned int i = 0; i < 3; ++i)
            result[i].SetAccurateVal(float(CFloat16(*(p+offset*4+i))));
        return result;
    }

    template<>
    NEATValue NEATALU::convert_double<float>(NEATType<float>::type* src) {
        if (src->IsAcc()) return NEATValue((const double)*src->GetAcc<float>());
        if (src->IsAny() || src->IsUnwritten() || src->IsUnknown()) return NEATValue(src->GetStatus());
        if (!src->IsInterval()) assert(0 && "unknown NEAT value status");
        return NEATValue((const double)*src->GetMin<float>(), (const double)*src->GetMax<float>());
    }

#define CONVERT_DOUBLE(n)                                           \
    template<>                                                      \
    NEATVector NEATALU::convert_double<float, n>(NEATType<float>::type* src) {  \
        NEATVector result(VectorWidthWrapper::ValueOf(n));          \
        for (unsigned int i = 0; i < n; ++i) {                      \
            result[i] = convert_double<float>(src+i);               \
        }                                                           \
        return result;                                              \
    }

    CONVERT_DOUBLE(2)
    CONVERT_DOUBLE(3)
    CONVERT_DOUBLE(4)
    CONVERT_DOUBLE(8)
    CONVERT_DOUBLE(16)

    template<>
    NEATValue NEATALU::convert_float<double>(NEATType<double>::type* src) {
        if (src->IsAcc()) return NEATValue((const float)*src->GetAcc<double>());
        if (src->IsAny() || src->IsUnwritten() || src->IsUnknown()) return NEATValue(src->GetStatus());
        if (!src->IsInterval()) assert(0 && "unknown NEAT value status");
        return NEATValue((const float)*src->GetMin<double>(), (const float)*src->GetMax<double>());
    }

#define CONVERT_FLOAT(n)                                           \
    template<>                                                      \
    NEATVector NEATALU::convert_float<double, n>(NEATType<double>::type* src) {  \
        NEATVector result(VectorWidthWrapper::ValueOf(n));          \
        for (unsigned int i = 0; i < n; ++i) {                      \
            result[i] = convert_float<double>(src+i);               \
        }                                                           \
        return result;                                              \
    }

    CONVERT_FLOAT(2)
    CONVERT_FLOAT(3)
    CONVERT_FLOAT(4)
    CONVERT_FLOAT(8)
    CONVERT_FLOAT(16)

}

