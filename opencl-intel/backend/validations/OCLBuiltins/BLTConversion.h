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

File Name:  BLTConversion.h

\*****************************************************************************/
#ifndef BLT_CONVERSION_H
#define BLT_CONVERSION_H

#include <vector>
#include <llvm/DerivedTypes.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include "Helpers.h"
#include "IBLTMapFiller.h"
#include "RefALU.h"
#include "Conformance/test_common/rounding_mode.h"
#include <limits.h>

#ifndef SATURATE
#define SATURATE true
#endif

#ifndef DONT_SATURATE
#define DONT_SATURATE !SATURATE
#endif
using namespace Conformance;

namespace Validation {
namespace OCLBuiltins {

    // This class adds references to the implementations of OpenCL built-in functions from 6.2 section.
    class ConversionMapFiller : public IBLTMapFiller
    {
    public:
        void addOpenCLBuiltins(std::map<std::string, PBLTFunc>& funcNames);
    };

	// template func to convert from type T to either APInt in case of integers of
	// convert to float and double in case of floating point
	template<typename T>
	typename retType<T>::type AsT(const T& R){return APInt(sizeof(T) * 8, R, isSignedType<T>());}

	template<>
	float AsT(const float& R){return R;}

	template<>
	double AsT(const double& R){return R;}
	
	template<typename TDst, typename TSrc, int n, bool saturate>
	llvm::GenericValue lle_X_convert(llvm::FunctionType *FT,
									 const std::vector<llvm::GenericValue> &Args)
	{
		IsScalarType<TDst> _x; UNUSED_ARGUMENT(_x);
		IsScalarType<TSrc> _y; UNUSED_ARGUMENT(_y);
		llvm::GenericValue R;

		// if it is vector use AggregateVal
		if( n > 1)
			R.AggregateVal.resize(n);
		llvm::GenericValue arg0 = Args[0];
		for (uint32_t i = 0; i < n; ++i)
		{
			TSrc val = (TSrc) getVal<TSrc,n>(arg0, i);

			if(saturate)
				val = std::max(std::min((TSrc)std::numeric_limits<TDst>::max(), val), (TSrc)std::numeric_limits<TDst>::min());
			TDst nval = (TDst)val;

			getRef<TDst,n>(R,i) = AsT<TDst>(nval);
		}
		return R;
	}

	template<typename IntType, int n, bool saturate>
	llvm::GenericValue lle_X_convert_float2int_rte(llvm::FunctionType *FT,
	   const std::vector<llvm::GenericValue> &Args)
	{
        IsIntegerType<IntType>();
        IsScalarType<IntType> _x; UNUSED_ARGUMENT(_x);

        llvm::GenericValue R;

        // if it is vector use AggregateVal
        if(n > 1)
            R.AggregateVal.resize(n);

        llvm::GenericValue arg0 = Args[0];
        for (uint32_t i = 0; i < n; ++i)
        {
            float val = (getVal<float,n>(arg0, i));

            // clamp
            if(saturate)
                val = std::max(std::min((float)std::numeric_limits<IntType>::max(), val), (float)std::numeric_limits<IntType>::min());
            IntType nval = (IntType) (val + 0.5f);

            // make the result even if fractional part == 0.5
            if((val - (int)val) == 0.5f)
                nval = (int)(val / 2 + 0.5f) * 2;

            getRef<IntType,n>(R,i) = AsT<IntType>(nval);
        }
        return R;
	}
	// template function to convert from LLVM Typename to Conformance Type from rounding_mode.h
	template<typename T>
	Conformance::Type LLVMTypeToConformanceType();

	#define DEF_FUNC_LLVM_TO_CONF_TYPE(_T1, _T2) \
		template<> Conformance::Type LLVMTypeToConformanceType<_T2>(){\
		return _T1;}

	DEF_FUNC_LLVM_TO_CONF_TYPE(kuchar, uint8_t)
	DEF_FUNC_LLVM_TO_CONF_TYPE(kchar, int8_t)
	DEF_FUNC_LLVM_TO_CONF_TYPE(kushort, uint16_t)
	DEF_FUNC_LLVM_TO_CONF_TYPE(kshort, int16_t)
	DEF_FUNC_LLVM_TO_CONF_TYPE(kuint, uint32_t)
	DEF_FUNC_LLVM_TO_CONF_TYPE(kint, int32_t)
	DEF_FUNC_LLVM_TO_CONF_TYPE(kfloat, float)
	DEF_FUNC_LLVM_TO_CONF_TYPE(kdouble, double)
	DEF_FUNC_LLVM_TO_CONF_TYPE(kulong, uint64_t)
	DEF_FUNC_LLVM_TO_CONF_TYPE(klong, int64_t)

	
} // namespace OCLBuiltins
} // namespace Validation

#endif // BLT_CONVERSION_H
