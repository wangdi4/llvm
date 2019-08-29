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

#include "ImagesALU.h"
#include "cl_types.h"
#include "NEATALU.h"
#include "Utils.h"

namespace Validation
{
    const double NEATALU::DIV_ERROR = 2.5;
    const double NEATALU::MIX_ERROR = 1.5; // rounding error of sub, mul and add
    const double NEATALU::NORMALIZE_ERROR = 2.5; // 2f + 0.5f  error in rsqrt + error in multiply
    const double NEATALU::SQRT_ERROR_DOUBLE = 0.5;  // sqrt is correctly rounded for doubles

    const long double NEATALU::pi = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899L;
    const long double NEATALU::two_pi = 6.2831853071795864769252867665590057683943387987502116L; //2*3.14159265358979323846264338327950288419716939937510582097494459230781640628620899L;
    const long double NEATALU::pi_2 = 1.5707963267948966192313216916397514420985846996875529L; //3.14159265358979323846264338327950288419716939937510582097494459230781640628620899L/2.0L;
    const long double NEATALU::div180by_pi = 57.295779513082320876798154814105170332405472466564321L; //180.L/3.14159265358979323846264338327950288419716939937510582097494459230781640628620899L;
    const long double NEATALU::divpiby_180 = 0.0174532925199432957692369076848861271344287188854172L; // 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899L/180.L;

  NEATALU::NEATALU()
  {
  }

  template<>
  double NEATALU::sqrtSetUlps<float>(void) {
      return NEATALU::SQRT_ERROR;
  }
  template<>
  double NEATALU::sqrtSetUlps<double>(void) {
      return NEATALU::SQRT_ERROR_DOUBLE;
  }

  template<>
  double NEATALU::divSetUlps<float>(void) {
      return NEATALU::DIV_ERROR;
  }
  template<>
  double NEATALU::divSetUlps<double>(void) {
      return NEATALU::DIV_ERROR_DOUBLE;
  }

  template <>
  NEATValue NEATALU::InternalDiv<float> ( const NEATValue& a, const NEATValue& b, double ulps)
  {
      // Check if both arguments may have any value
      if(a.IsAny() && b.IsAny())
          // Then result is unknown
          return NEATValue(NEATValue::UNKNOWN);

      if (a.IsNaN<float>() || b.IsNaN<float>())
          return NEATValue::NaN<float>();

      // Case when both could be any was tested. Other special statuses lead to unknown result
      if(CheckAUU(a) || (CheckAUU(b)))
      {
          return NEATValue(NEATValue::UNKNOWN);
      }

      // if a and b are not finite (i.e. NaN or Inf), the result is NaN
      if (!b.IsAcc() && (b.Includes((float)0.0) || b.Includes((float)(-0.0))) )
      {
          // if b (in a/b) has more than one allowed value
          //  and one of the values allowed is +0.0 or -0.0,
          //  then we cannot predict the result (it may be finite and may be inf)
          return NEATValue(NEATValue::UNKNOWN);
      }

      if(Utils::IsInf<float>(ulps))
          return NEATValue(NEATValue::ANY);

      const int RES_COUNT = 8;
      double val[RES_COUNT];

      val[0] = RefALU::div((double)*a.GetMin<float>(),(double)*b.GetMin<float>());
      val[1] = RefALU::div((double)*a.GetMin<float>(),(double)*b.GetMax<float>());
      val[2] = RefALU::div((double)*a.GetMax<float>(),(double)*b.GetMin<float>());
      val[3] = RefALU::div((double)*a.GetMax<float>(),(double)*b.GetMax<float>());
      val[4] = RefALU::mul((double)*a.GetMin<float>(), RefALU::div((double)1.0L,(double)*b.GetMin<float>()) );
      val[5] = RefALU::mul((double)*a.GetMin<float>(), RefALU::div((double)1.0L,(double)*b.GetMax<float>()) );
      val[6] = RefALU::mul((double)*a.GetMax<float>(), RefALU::div((double)1.0L,(double)*b.GetMin<float>()) );
      val[7] = RefALU::mul((double)*a.GetMax<float>(), RefALU::div((double)1.0L,(double)*b.GetMax<float>()) );

      return NEATALU::CreateNEATValue<double>(val, RES_COUNT, IntervalError<float>(ulps));
  }

  // division for doubles has zero ulps and performs in doubles with no extending the
  // output interval by using x*1/x operation, just to have division only as it is
  // done in conformance
  template <>
  NEATValue NEATALU::InternalDiv<double> ( const NEATValue& a, const NEATValue& b, double ulps)
  {
      // Check if both arguments may have any value
      if(a.IsAny() && b.IsAny())
          // Then result is unknown
          return NEATValue(NEATValue::UNKNOWN);

      if (a.IsNaN<double>() || b.IsNaN<double>())
          return NEATValue::NaN<double>();

      // Case when both could be any was tested. Other special statuses lead to unknown result
      if(CheckAUU(a) || (CheckAUU(b)))
      {
          return NEATValue(NEATValue::UNKNOWN);
      }

      // if a and b are not finite (i.e. NaN or Inf), the result is NaN
      if (!b.IsAcc() && (b.Includes((double)0.0) || b.Includes((double)(-0.0))) )
      {
          // if b (in a/b) has more than one allowed value
          //  and one of the values allowed is +0.0 or -0.0,
          //  then we cannot predict the result (it may be finite and may be inf)
          return NEATValue(NEATValue::UNKNOWN);
      }

      const int RES_COUNT = 4;
      long double val[RES_COUNT];

      val[0] = (long double)RefALU::div(*a.GetMin<double>(),*b.GetMin<double>());
      val[1] = (long double)RefALU::div(*a.GetMin<double>(),*b.GetMax<double>());
      val[2] = (long double)RefALU::div(*a.GetMax<double>(),*b.GetMin<double>());
      val[3] = (long double)RefALU::div(*a.GetMax<double>(),*b.GetMax<double>());

      return NEATALU::CreateNEATValue<long double>(val, RES_COUNT, IntervalError<double>(ulps));
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

        NEATVector dstVec(VectorWidthWrapper::ValueOf(maskSize));

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

    NEATVector NEATALU::shuffle ( const NEATVector& vec1,
                                  const std::vector<uint32_t>& maskArrIn) {

        unsigned maskArrLength = (unsigned)maskArrIn.size();
        NEATVector dstVec(VectorWidthWrapper::ValueOf(maskArrLength));

        unsigned vec1Length = (unsigned)vec1.GetSize();
        if((unsigned)maskArrIn.size() == 1) {
            throw Exception::InvalidArgument("[NEATALU::shuffle] Wrong mask size");
        }

        unsigned mask = (1 <<shuffleGetNumMaskBits(vec1Length)) - 1;

        for( unsigned i=0; i<maskArrLength; i++) {
            unsigned j = (unsigned)maskArrIn[i] & mask;
            dstVec[i] = vec1[j];
        }
        return dstVec;
    }

    NEATVector NEATALU::shuffle2 ( const NEATVector& vec1, const NEATVector& vec2,
                                  const std::vector<uint32_t>& maskArrIn) {

        unsigned maskArrLength = (unsigned)maskArrIn.size();
        NEATVector dstVec(VectorWidthWrapper::ValueOf(maskArrLength));

        unsigned vec1Length = (unsigned)vec1.GetSize(); // the size of vec1 and vec2 are the same - checked by NEAT plug-in
        if((unsigned)maskArrIn.size() == 1) {
            throw Exception::InvalidArgument("[NEATALU::shuffle2] Wrong mask size");
        }

        unsigned mask = (1 << (shuffleGetNumMaskBits(vec1Length) + 1) ) - 1;

        for( unsigned i=0; i<maskArrLength; i++) {
            unsigned j = (unsigned)maskArrIn[i] & mask;
            if(j < vec1Length) {
                dstVec[i] = vec1[j];
            } else {
                dstVec[i] = vec2[j-vec1Length];
            }
        }
        return dstVec;
    }

    NEATValue NEATALU::atomic_xchg ( NEATValue * p, const NEATValue& val ) {
        NEATValue old = *p;

        *p = val;

        return old;
    }

    Validation::NEATVector NEATALU::read_imagef_src_noneat( void *imageData, Conformance::image_descriptor *imageInfo,
        float x, float y, float z, Conformance::image_sampler_data *imageSampler )
    {
/*
         This function uses the same approach that is done in Conformance test images_kernel_read_write
         However this Conformance approach has some faults.
         In most cases (linear filtering, address_repeat etc.) conformance uses as error "magic number" MAKE_HEX_FLOAT(0x1.0p-7f, 0x1L, -7)
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

        const double maxErrAbs[4] = {
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
        case CLK_A:
            retVec[3].SetIntervalVal<float>(accPix.p[3] - maxErrAbs[3], accPix.p[3] + maxErrAbs[3]);
            break;
        case CLK_R:
        case CLK_DEPTH:
            retVec[0].SetIntervalVal<float>(accPix.p[0] - maxErrAbs[0], accPix.p[0] + maxErrAbs[0]);
            break;
        case CLK_RA:
            retVec[0].SetIntervalVal<float>(accPix.p[0] - maxErrAbs[0], accPix.p[0] + maxErrAbs[0]);
            retVec[3].SetIntervalVal<float>(accPix.p[3] - maxErrAbs[3], accPix.p[3] + maxErrAbs[3]);
            break;
        case CLK_RG:
            retVec[0].SetIntervalVal<float>(accPix.p[0] - maxErrAbs[0], accPix.p[0] + maxErrAbs[0]);
            retVec[1].SetIntervalVal<float>(accPix.p[1] - maxErrAbs[1], accPix.p[1] + maxErrAbs[1]);
            break;
        case CLK_RGB:
        case CLK_LUMINANCE:
            retVec[0].SetIntervalVal<float>(accPix.p[0] - maxErrAbs[0], accPix.p[0] + maxErrAbs[0]);
            retVec[1].SetIntervalVal<float>(accPix.p[1] - maxErrAbs[1], accPix.p[1] + maxErrAbs[1]);
            retVec[2].SetIntervalVal<float>(accPix.p[2] - maxErrAbs[2], accPix.p[2] + maxErrAbs[2]);
            break;
        case CLK_RGBA:
        case CLK_sRGBA:
        case CLK_sBGRA:
        case CLK_ARGB:
        case CLK_BGRA:
        case CLK_INTENSITY:
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
        if (src->IsAcc()) return NEATValue((double)*src->GetAcc<float>());
        if (src->IsAny() || src->IsUnwritten() || src->IsUnknown()) return NEATValue(src->GetStatus());
        if (!src->IsInterval()) assert(0 && "unknown NEAT value status");
        return NEATValue((double)*src->GetMin<float>(), (double)*src->GetMax<float>());
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
        if (src->IsAcc()) return NEATValue((float)*src->GetAcc<double>());
        if (src->IsAny() || src->IsUnwritten() || src->IsUnknown()) return NEATValue(src->GetStatus());
        if (!src->IsInterval()) assert(0 && "unknown NEAT value status");
        return NEATValue((float)*src->GetMin<double>(), (float)*src->GetMax<double>());
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

