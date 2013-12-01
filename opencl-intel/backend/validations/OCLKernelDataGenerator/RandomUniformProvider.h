/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name: RandomUniformProvider.h

\*****************************************************************************/

#ifndef __RANDOM_UNIFORM_PROVIDER_H__
#define __RANDOM_UNIFORM_PROVIDER_H__


#include"DataType.h"
#include"dxfloat.h"
#include"FloatOperations.h"

#include<cfloat>

namespace Validation
{
    /// uniform random generator class is container of random generators 
    /// Random generators generate scalar values of supported data types
    /// They share the same initial seed produces uniformly distributed values
    /// within their interval
    class RandomUniformProvider
    {
    public:
        /// ctor with seed. should initialize
        explicit RandomUniformProvider(const uint64_t seed = 0);
        // obtain seed
        uint64_t seed() const{
            return m_seed;
        }
        /// get random next sample. 
        /// @return uint32_t value from whole uint32_t set
        uint32_t sample_u32_unscaled() const;
        /// get random next sample. 
        /// @return uint64_t value from whole uint64_t set
        uint64_t sample_u64(uint64_t min=0, uint64_t max=UINT64_MAX) const;
        /// get random next sample. 
        /// @return unscaled uint64_t value from whole uint64_t set
        uint64_t sample_u64_unscaled() const;
        // floating point 
        // from -FLT_MAX and FLT_MAX
        float sample_f32(float min=-FLT_MAX,float max=FLT_MAX) const;
        // from -DBL_MAX and DBL_MAX
        double sample_f64(double min=-DBL_MAX, double max=DBL_MAX) const;

        // floating point
        // from -FLT_MAX and FLT_MAX
        float sample_f32_binary() const;
        // from -DBL_MAX and DBL_MAX
        double sample_f64_binary() const;
    private:
        // hide copy ctor to disable passing as function argument by value
        RandomUniformProvider(const RandomUniformProvider & UniformQuasiGenerator );
        /// internal generator's variables
        mutable uint32_t m_v11, m_v12, m_v13, m_v2, m_flag;
        uint64_t m_seed;
        double Generator() const;
        void InitGenerator();
    };

    template<typename T>
    struct RandomGeneratorInterfaceProvider;

    template <> struct RandomGeneratorInterfaceProvider<CFloat16>
    {
        static CFloat16 sample(const RandomUniformProvider& rnd)
        {
            return (CFloat16)(rnd.sample_f32());
        }
    };
    template <> struct RandomGeneratorInterfaceProvider<float>
    {
        static float sample(const RandomUniformProvider& rnd)
        {
            return rnd.sample_f32();
        }
    };
    template <> struct RandomGeneratorInterfaceProvider<double>
    {
        static double sample(const RandomUniformProvider& rnd)
        {
            return rnd.sample_f64();
        }
    };
    template <typename T> struct RandomGeneratorInterfaceProvider
    {
        static T sample(const RandomUniformProvider& rnd)
        {
            IsIntegerType<T> _notUsed;
            UNUSED_ARGUMENT(_notUsed);
            return rnd.sample_u64_unscaled();
        }
    };

} // End of Validation namespace
#endif //__RANDOM_UNIFORM_PROVIDER_H__
