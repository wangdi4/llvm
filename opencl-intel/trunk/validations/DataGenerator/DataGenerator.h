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

File Name:  DataGenerator.h

\*****************************************************************************/
#ifndef __DATA_GENERATOR_H__
#define __DATA_GENERATOR_H__


#include <time.h>
#include <list>
#include <limits>
#include <iostream>
#include <llvm/System/DataTypes.h>
#include <BufferContainerList.h>
#include <IDataReader.h>
#include <Exception.h>
#include <dxfloat.h>

namespace Validation
{
    /// @brief Used to restrict function to work with the set of types
    /// To use this trick, declare variable of type IsDGType in the function
    /// using function template argument type. Type definition is specified
    /// only for 11 numerical types, listed below
    /// This trick causes Unreferenced local variable warning that can be
    /// disabled using UNUSED_ARGUMENT macro defined above
    template <typename T>
    class IsDGType;

    /// Definitions for numerical types.
    template <> class IsDGType<uint8_t> {};
    template <> class IsDGType<uint16_t> {};
    template <> class IsDGType<uint32_t> {};
    template <> class IsDGType<uint64_t> {};
    template <> class IsDGType<int8_t> {};
    template <> class IsDGType<int16_t> {};
    template <> class IsDGType<int32_t> {};
    template <> class IsDGType<int64_t> {};
    template <> class IsDGType<float> {};
    template <> class IsDGType<double> {};
    template <> class IsDGType<CFloat16> {};

    /// Data Generator class
    ///
    class DataGenerator : public IDataReader
    {
    public:

        /// Fill buffer method signal variable
        enum FillMethod
        {
            FILL_VALUE = 0, /// const value is used to fill buffer
            FILL_RANDOM, /// random value of full range of specified data type
                         /// is used to fill buffer
            FILL_RANDOM_FROM_RANGE, /// random value of limited range of
                         /// specified data type is used to fill buffer
            FILL_RANDOM_FLOAT_WITH_SPECIAL_VALUES, /// generate special
                              /// values with pre-defined probabilities
            INVALID_FILL_METHOD
        };

        union FillDataType
        {
            int8_t   dI8;
            uint8_t  dU8;
            int16_t  dI16;
            uint16_t dU16;
            int32_t  dI32;
            uint32_t dU32;
            int64_t  dI64;
            uint64_t dU64;
            uint16_t dF16;
            float    dF32;
            double   dF64;
        };

        /// Buffer Fill method description
        struct BufferFillMethodDesc{
            /// method of generating data
            FillMethod fillMethod;

            /// if fillMethod is FILL_VALUE then use fillValue to fill buffer
            FillDataType fillValue;

            /// if fillMethod is FILL_RANDOM then lowValue and highValue are
            /// minimum and maximum possible values of specified data type,
            /// i.e. lowValue=0 highValue=255 for unsigned 8-bit integer

            /// if fillMethod is FILL_RANDOM_FROM_RANGE then lowValue and
            /// highValue are used as interval boundaries lowValue should be
            /// lower than highValue
            FillDataType lowValue;
            FillDataType highValue;

            /// if fillMethod is FILL_RANDOM_FLOAT_WITH_SPECIAL_VALUES
            /// then with specialValuesProb probability special values are
            /// generated,Probability (0.f;1.f) of generating special values:
            /// NaNs, Denormals, +Inf, -Inf
            float specialValuesProb;
        };

        /// Pair of structures describing buffer format and filling method
        typedef std::pair<BufferDesc, BufferFillMethodDesc>
                                      BufferDescAndFillMethodPair;

        /// Vector of pairs of structures describing buffer format and filling
        /// method. Each pair describes one buffer
        typedef std::vector<BufferDescAndFillMethodPair>
                            BufferContainerFillMethod;

        /// Vector of vectors BufferContainerFillMethod.
        /// It is an input for DataGenerator ctor
        typedef std::vector<BufferContainerFillMethod>
                            BufferContainerListFillMethod;

        /// Ctor with seed value
        DataGenerator(const BufferContainerListFillMethod& in_method,
                      const uint64_t& in_seed ) :
        m_BufferContainerListFillMethod(in_method), m_Seed(in_seed)
        {
            InitGenerator();
        }

        /// Ctor with default seed value
        DataGenerator(const BufferContainerListFillMethod& in_method) :
        m_BufferContainerListFillMethod(in_method), m_Seed(time(NULL)+rand())
        {
            InitGenerator();
        }

        /// @brief Interface function to generate data and put it into buffers.
        /// This function uses parameters defined in
        /// m_BufferContainerListFillMethod member and seed from m_Seed member
        /// for random generator init.
        /// Creates and allocates BufferContainers.
        /// Creates and allocates Buffers.
        /// Generates data with method defined in BufferFillMethodDesc
        /// structure inside BufferContainerListFillMethod structure
        /// @param  [inout]  p pointer to interface for BufferContainerList
        virtual void Read(IContainer *p);

        /// @brief Static function to set buffer parameters according to
        /// BufferDesc and BufferFillMethodDesc
        /// @param  [inout] bcfm refernece to BufferContainerFillMethod vector.
        /// Each element of  bcfm vector subscribes one data beffer to be
        /// filled by generated data.
        /// Data buffer consists of  "buffLen" vectors, each vector of
        /// "vecWidth" width. Each alement of vector is "dataType" type.
        /// @param [in] dataType data type of buffer, should be
        /// in range (F16;U64). See Validation::DataTypeVal
        /// @param [in] vecWidth the width of vector, should be
        /// in range (V1;V16). See Validation::VectorWidth
        /// @param [in] buffLen the length of buffer counted in vectors of
        /// vecWidth width.
        /// @param [in] fillMethod method to generate data, see
        /// Validation::DataGenerator::FillMethod
        /// @param [in] value. The value to fill buffer. Makes sense only if
        /// fillMethod==FILL_VALUE
        /// @param [in] lowValue. The bottom limit to fill buffer.
        /// Makes sense only if fillMethod==FILL_RANDOM_FROM_RANGE
        /// @param [in] highValue. The top limit to fill buffer.
        /// Makes sense only if fillMethod==FILL_RANDOM_FROM_RANGE
        /// @param [in] specialValuesProb. The probability of generaion special
        /// values in the float or double buffer, should be in range (0.f;1.f),
        /// sakes sense only if
        /// fillMethod==FILL_RANDOM_FLOAT_WITH_SPECIAL_VALUES
        template <typename T> static void SetBufferContainerFillMethod(
                                          BufferContainerFillMethod& bcfm,
                                          DataTypeValWrapper dataType,
                                          VectorWidthWrapper vecWidth,
                                          const uint32_t buffLen,
                                          FillMethod fillMethod,
                                          T value, T lowValue, T highValue,
                                          float specialValuesProb)
        {
            /// Constraint template to work with numerical types only.
            /// Otherwise compiler will report an error
            IsDGType<T> _notUsed;
            UNUSED_ARGUMENT(_notUsed);

            BufferDesc desc(buffLen, vecWidth.GetValue(), dataType.GetValue());
            BufferFillMethodDesc bfmd =
                                 {fillMethod,
                                  SetFillDataType(dataType.GetValue(),value),
                                  SetFillDataType(dataType.GetValue(),lowValue),
                                  SetFillDataType(dataType.GetValue(),highValue),
                                  specialValuesProb};
            BufferDescAndFillMethodPair bdafmp (desc,bfmd);

            bcfm.push_back(bdafmp);
        }


        /// @brief Static function to set buffer parameters with
        /// fillMethod==FILL_RANDOM
        /// @param  [inout] bcfm refernece to BufferContainerFillMethod vector.
        /// Each element of  bcfm vector subscribes one data beffer to be
        /// filled by generated data.
        /// Data buffer consists of  "buffLen" vectors, each vector of
        /// "vecWidth" width. Each alement of vector is "dataType" type.
        /// @param [in] dataType data type of buffer, should be
        /// in range (F16;U64). See Validation::DataTypeVal
        /// @param [in] vecWidth the width of vector, should be
        /// in range (V1;V16). See Validation::VectorWidth
        /// @param [in] buffLen the length of buffer counted in vectors of
        /// vecWidth width.
        static void SetRandomMethod(BufferContainerFillMethod& bcfm,
                                    DataTypeValWrapper dataType,
                                    VectorWidthWrapper vecWidth,
                                    const uint32_t buffLen);

        /// @brief Static function to set buffer parameters with
        /// fillMethod==FILL_RANDOM_FLOAT_WITH_SPECIAL_VALUES
        /// @param  [inout] bcfm refernece to BufferContainerFillMethod vector.
        /// Each element of  bcfm vector subscribes one data beffer to be
        /// filled by generated data.
        /// Data buffer consists of  "buffLen" vectors, each vector of
        /// "vecWidth" width. Each alement of vector is "dataType" type.
        /// @param [in] dataType data type of buffer, should be
        /// in range (F16;U64). See Validation::DataTypeVal
        /// @param [in] vecWidth the width of vector, should be
        /// in range (V1;V16). See Validation::VectorWidth
        /// @param [in] buffLen the length of buffer counted in vectors of
        /// vecWidth width.
        /// @param [in] specialValuesProb. The probability of generaion special
        /// values in the float or double buffer, should be in range (0.f;1.f)
         static void SetSpecialValuesMethod(BufferContainerFillMethod& bcfm,
                                           DataTypeValWrapper dataType,
                                           VectorWidthWrapper vecWidth,
                                           const uint32_t buffLen,
                                           float specialValuesProb);

        /// @brief Static function to set buffer parameters with
        /// fillMethod==FILL_VALUE
        /// @param  [inout] bcfm refernece to BufferContainerFillMethod vector.
        /// Each element of  bcfm vector subscribes one data beffer to be
        /// filled by generated data.
        /// Data buffer consists of  "buffLen" vectors, each vector of
        /// "vecWidth" width. Each alement of vector is "dataType" type.
        /// @param [in] dataType data type of buffer, should be
        /// in range (F16;U64). See Validation::DataTypeVal
        /// @param [in] vecWidth the width of vector, should be
        /// in range (V1;V16). See Validation::VectorWidth
        /// @param [in] buffLen the length of buffer counted in vectors of
        /// vecWidth width.
        /// @param [in] value. The value to fill buffer.
        template <typename T> static void SetValueMethod(
                                          BufferContainerFillMethod& bcfm,
                                          DataTypeValWrapper dataType,
                                          VectorWidthWrapper vecWidth,
                                          const uint32_t buffLen, T value)
        {
            /// Constraint template to work with numerical types only.
            /// Otherwise compiler will report an error
            IsDGType<T> _notUsed;
            UNUSED_ARGUMENT(_notUsed);

            SetBufferContainerFillMethod<T>(bcfm, dataType, vecWidth,
                                            buffLen, FILL_VALUE, value,
                                            0.0f, 0.0f, 0.0f);
        }

        /// @brief Static function to set buffer parameters with
        /// fillMethod==FILL_RANDOM_FROM_RANGE
        /// @param  [inout] bcfm refernece to BufferContainerFillMethod vector.
        /// Each element of  bcfm vector subscribes one data beffer to be
        /// filled by generated data.
        /// Data buffer consists of  "buffLen" vectors, each vector of
        /// "vecWidth" width. Each alement of vector is "dataType" type.
        /// @param [in] dataType data type of buffer, should be
        /// in range (F16;U64). See Validation::DataTypeVal
        /// @param [in] vecWidth the width of vector, should be
        /// in range (V1;V16). See Validation::VectorWidth
        /// @param [in] buffLen the length of buffer counted in vectors of
        /// vecWidth width.
        /// @param [in] lowValue. The bottom limit to fill buffer.
        /// @param [in] highValue. The top limit to fill buffer.
        template <typename T> static void SetRandomFromRangeMethod(
                                          BufferContainerFillMethod& bcfm,
                                          DataTypeValWrapper dataType,
                                          VectorWidthWrapper vecWidth,
                                          const uint32_t buffLen,
                                          T lowValue, T highValue)
        {
            /// Constraint template to work with numerical types only.
            /// Otherwise compiler will report an error
            IsDGType<T> _notUsed;
            UNUSED_ARGUMENT(_notUsed);

            SetBufferContainerFillMethod<T>(bcfm, dataType, vecWidth, buffLen,
                                            FILL_RANDOM_FROM_RANGE,
                                            0.0f, lowValue, highValue, 0.0f);
        }

        /// @brief Static function to add input bcfm vector to the bclfm vector
        /// @param [in] bcfm. Vector of data buffers to be filled
        /// by DataGenerator
        /// @param [out] bclfm. Vector of vectors of data buffers to be filled
        /// by DataGenerator.
        /// bclfm is an input for DataGenerator ctor
        static void SetBufferContainerListFillMethod(
                                          BufferContainerListFillMethod& bclfm,
                                          BufferContainerFillMethod& bcfm)
        {
            bclfm.push_back(bcfm);
        }

        uint64_t GetSeed(void)
        {
            return m_Seed;
        }
    protected:
        /// method to fill m_BufferContainerListFillMethod list
        void FillBuffer(const BufferFillMethodDesc& bfmd, IMemoryObject* buff);

    private:
        /// methods to fill buffer container list
        const BufferContainerListFillMethod& m_BufferContainerListFillMethod;
        /// seed to use in pseudorandom generator
        uint64_t m_Seed;
        /// internal generator's variables
        uint32_t m_v11, m_v12, m_v13, m_v2, m_flag;
        /// (-1,1] pseudorandom generator implementing RANDU algorithm
        /// G.Marsaglia, A.Zaman. Computer in Physics, v8, #1, 1994
        double Generator(void);
        void InitGenerator(void);

        /// internal functions to make 16bit, 32bit and 64bit denormal value with
        /// ranodmly generated mantissa
        void MakeDenormValue(CFloat16* );
        void MakeDenormValue(float* );
        void MakeDenormValue(double* );

        /// x and y should be the same variable: MyIsnan(a,a);
        /// It is done to avoid code like "return x != x" being optimized to
        /// "return false" by some "smart" compiler.
        /// Functions similar _isnan() are not supported by some compilers.
        template <typename T> bool MyIsnan(T x,T y)
        {
            /// Constraint template to work with numerical types only.
            /// Otherwise compiler will report an error
            IsDGType<T> _notUsed;
            UNUSED_ARGUMENT(_notUsed);

            return x != y;
        }

        /// internal function to fill array arr of length len by pseudorandom
        /// values from range (min,max) special case 16bit float
        void FillRandom( CFloat16* , int32_t , CFloat16 , CFloat16 );
        /// other data types
        template <typename T> void FillRandom( T* arr, int32_t len,
                                               T max, T min )
        {
            /// Constraint template to work with numerical types only.
            /// Otherwise compiler will report an error
            IsDGType<T> _notUsed;
            UNUSED_ARGUMENT(_notUsed);

            if (MyIsnan(max,max) || MyIsnan(min,min))
            {
                throw Exception::InvalidArgument(
                          "[DataGenerator::FillRandom] bad data range limits");
            }
            if(max <= min)
            {
                throw Exception::InvalidArgument(
                          "[DataGenerator::FillRandom] highValue is lower or\
                          equal to lowValue");
            }

            if((max+min) < max || (max-min) > max)
            {
                /// for instance, for int64_t if max = 9223372036854775807 and
                /// min =-9223372036854775807, max+min=0 and max-min=-2,
                /// then mpy = (max-min)/2 =1 it is wrong and we should
                /// calculate max/2 and min/2 first, so
                /// we have correct mpy = max/2+min/2 = 9223372036854775806

                /// also we can't use double values for this calculation due to
                /// rounding error
                /// example: max = 9223372036854775806,
                /// min = 9223372036854775706,
                /// max-min = 100, but
                /// double a = double(max) - double(min) = 0
                /// due to rounding int64_t value to double value

                T add = max/(T)2.0+min/(T)2.0;
                double mpy = (double)(max/(T)2.0-min/(T)2.0);
                for( int32_t i = 0; i < len; i++ )
                {
                    arr[i] = (T)(mpy*Generator()) + add;
                    /// to avoid issue like max= -17019 vs arr[i]=-17018 due to
                    /// rounding error form double precision Generator()
                    if(arr[i] > max) arr[i] = max;
                    if(arr[i] < min) arr[i] = min;
                }
            } else {
                /// we can't use formula mpy = max/2-min/2 as it was above,
                /// because, for example, if max = 9 and min = 3, mpy = 5,
                /// but must be mpy = (max-min)/2 = 6

                double add = (double)(max+min)/2.0;
                double mpy = (double)(max-min)/2.0;
                for( int32_t i = 0; i < len; i++ )
                {
                    arr[i] = (T)(mpy*Generator()+add);
                    /// to avoid issue like max= -17019 vs arr[i]=-17018 due to
                    /// rounding error form double precision Generator()
                    if(arr[i] > max) arr[i] = max;
                    if(arr[i] < min) arr[i] = min;
                }
            }
        }

        /// internal function to fill array arr of length len by value a
        template <typename T> void FillConst( T* arr, int32_t len, const T a )
        {
            /// Constraint template to work with numerical types only.
            /// Otherwise compiler will report an error
            IsDGType<T> _notUsed;
            UNUSED_ARGUMENT(_notUsed);

            for( int32_t i = 0; i < len; i++ )
            {
                arr[i] = a;
            }
        }

        /// internal function to fill array arr of length len by pseudorandom
        /// float or double values arr should include special values like NaN,
        /// +INF,-INF with probabity specialValuesProb
        /// so, if len==100 and specialValuesProb==0.6, about 60 elements of
        /// arr should be special values, and the rest elements (about 40)
        /// are pseudorandomly generated floating point values
        void FillRandomSpecial( CFloat16* arr, int32_t len,
                                float specialValuesProb );

        template <typename T> void FillRandomSpecial( T* arr, int32_t len,
                                                      float specialValuesProb )
        {
            /// Constraint template to work with numerical types only.
            /// Otherwise compiler will report an error
            IsDGType<T> _notUsed;
            UNUSED_ARGUMENT(_notUsed);

            const int32_t specValLen = 6;
            T specVal[specValLen];
            float prob;

            specVal[0] = std::numeric_limits<T>::quiet_NaN();
            specVal[1] = std::numeric_limits<T>::infinity();
            specVal[2] = -std::numeric_limits<T>::infinity();
            specVal[3] = (T)(+0.0);
            specVal[4] = (T)(-0.0);
            MakeDenormValue(&specVal[5]);

            T max = std::numeric_limits<T>::max();
            T min = (0-std::numeric_limits<T>::max());

            FillRandom((T*)arr,len,max,min);
            if( specialValuesProb > 0.0f)
            {
                for(int32_t j=0; j<len; j++)
                {
                    prob = (float)(0.5*Generator()+0.5);
                    // to make (0;1] from (-1;1]
                    if( prob <= specialValuesProb)
                    {
                        uint32_t i = (uint32_t)((((double)specValLen)/2.0)*
                            Generator()+((double)specValLen)/2.0);
                        arr[j] = specVal[i%specValLen];
                        // just to be sure i less than specValLen
                    }
                }
            }
        }

        /// internal function to save value as FillDataType type
        template<typename T> static FillDataType SetFillDataType(
                                                 DataTypeVal dataType,T value)
        {
            /// Constraint template to work with numerical types only.
            /// Otherwise compiler will report an error
            IsDGType<T> _notUsed;
            UNUSED_ARGUMENT(_notUsed);

            FillDataType result;
            switch(dataType)
            {
                case I8:
                    result.dI8 = (int8_t)value;
                break;
                case U8:
                    result.dU8 = (uint8_t)value;
                    break;
                case I16:
                    result.dI16 = (int16_t)value;
                    break;
                case U16:
                    result.dU16 = (uint16_t)value;
                    break;
                case I32:
                    result.dI32 = (int32_t)value;
                    break;
                case U32:
                    result.dU32 = (uint32_t)value;
                    break;
                case I64:
                    result.dI64 = (int64_t)value;
                    break;
                case U64:
                    result.dU64 = (uint64_t)value;
                    break;
                case F16:   /// CFloat16 represents half floats,
                            /// but uint16_t is used for storing.
                    if(sizeof(value) != sizeof(uint16_t))
                    {
                        throw Exception::InvalidArgument(
                            "[DataGenerator::SetFillDataType] the size\
                            of F16 is not valid");
                    } else {
                        uint16_t * a = (uint16_t*)&value;
                        result.dF16 = *a;
                    }
                    break;
                case F32:
                    result.dF32 = (float)value;
                    break;
                case F64:
                    result.dF64 = (double)value;
                    break;
                default:
                    throw Exception::InvalidArgument("[DataGenerator::\
                                    SetFillDataType] Unsupported data format");
                    break;
            }
            return result;
        }

            /// hide copy ctors to avoid copying. should create new
            /// DataGenerator object for new parameters
            DataGenerator(const DataGenerator& in)
                : IDataReader(), m_BufferContainerListFillMethod(
                              in.m_BufferContainerListFillMethod) {}

    };
}

#endif // __DATA_GENERATOR_H__

