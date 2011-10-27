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

File Name:  Comparator.h

\*****************************************************************************/
#ifndef __COMPARATOR_H__
#define __COMPARATOR_H__

#define DEBUG_TYPE "Comparator"
#include <vector>
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "BufferContainerList.h"
#include "Buffer.h"
#include "Image.h"
#include "IComparisonResults.h"
#include "Exception.h"
#include "DataType.h"
#include "FloatOperations.h"
#include "cl_device_api.h"
#include "cl_dev_backend_api.h"
#include "StatisticsCollector.h"

using namespace std;

namespace Validation
{

    /// Comparison status
    enum COMP_RESULT {
        PASSED = 0, /// Successful comparison status
        NOT_PASSED  /// Comparison failed. Some values mismatched
    };

    // !!! default value for tolerance in accurate mode for comparison of floating point values
    static const double DEFAULT_ULP_TOLERANCE = 0.0;

    /// @ brief Used for comparison of actual output with reference output.
    /// It can also validate actual output according to NEAT data.
    /// Comparison is performed using compare function call.
    class Comparator
    {
    public:
        
        /// @brief Ctor
        /// Comparator
        Comparator() : m_ULP_TOLERANCE(DEFAULT_ULP_TOLERANCE)
        {}

        ///
        /// @brief Compares actual output with reference using optionally NEAT information.
        /// Comparison type will be chosen depending on the input. Reference or NEAT pointers 
        /// passed to Compare can be NULL, but not both. If Reference is NULL then NEAT
        /// structures will be used to validate actual output. Another option is to pass only
        /// reference and leave NEAT to be NULL. In this case precise comparison will be performed.
        /// You can also specify both NEAT and Reference values. Then firstly NEAT structures will
        /// be used to validate output. Afterwards reference values will be validated to fit within
        /// NEAT intervals and corresponding warnings will be reported. Validation of reference values
        /// will not influence output and test is supposed to be passed even if reference doesn't fit 
        /// within NEAT intervals.
        ///
        /// @param  [in]  res       This object can be used in case of failing test
        ///                         All mismatch information is reported here.
        /// @param  [in]  Actual    Actual output values
        /// @param  [in]  pRef      Reference output values. If NULL specified then pNEAT should be provided.
        /// @param  [in]  pNEAT     NEAT intervals. If NULL specified then pRef should be provided.
        /// @return     PASSED if comparison succeeded. NOT_PASSED if comparison failed.
        /// @throws     InvalidArgument
        ///
        COMP_RESULT Compare(IComparisonResults& res,
            const std::vector<bool>* ignoreList,
            const IBufferContainerList& Actual,
            const IBufferContainerList* Reference = 0,
            const IBufferContainerList* NEAT = 0);

        /// @brief tolerance in ULPs used in accurate comparison of floating point types
        inline void SetULPTolerance(const double& in_Tolerance)
        {
            assert(in_Tolerance >= 0.0);
            m_ULP_TOLERANCE = in_Tolerance;
        }
        
        /// @brief tolerance in ULPs used in accurate comparison of floating point types
        inline double GetULPTolerance() const
        {
            return m_ULP_TOLERANCE;
        }

    protected:
         /// @brief Performs comparison of buffers
         /// @param [in] p       Actual output buffer
         /// @param [in] Ref     Reference output buffer
         /// @param [in] NEAT    NEAT info buffer
         /// @return     Comparison status
         COMP_RESULT CompareBuffers(const Buffer* pAct, const Buffer* pRef, const Buffer* pNEAT);

         /// @brief Performs comparison of images
         COMP_RESULT CompareImages(const Image* pImg, const Image* pImgRef, const Image* pImgNEAT);
         
         /// @brief Performs comparison of N sequential elements in memory
         /// TypeDesc can be aggregate
         /// this function will recurse itself in case of aggregate typeDesc
         COMP_RESULT CompareNElements(const int8_t* pAct, const int8_t* pRef, 
             const NEATValue* pNEAT, 
             const TypeDesc& typeDesc, const std::size_t& N);

         /// @brief Performs comparison of N scalar sequential elements in memory
         template<typename T>
         COMP_RESULT CompareNScalarElements(const T* pAct, const T* pRef, 
             const NEATValue* pNEAT, 
             const  std::size_t& N);

         /// @brief calls either NEAT interval comparison if pNEAT is not NULL
         //  calls accurate comparison if NEAT data not present
         template<typename T>
         COMP_RESULT CompareScalarValueNEATConditional(const T* pAct, const T* pRef, 
             const NEATValue* pNEAT);

         /// @brief  Adds mismatch information to comparison results
         /// @param  pRef pointer to reference value
         /// @param  pAct pointer to actual value
         /// @param  pNeat pointer to NEAT Value
         template<typename T>
         inline void ReportMismatch(const T* pAct, const T* pRef, 
             const NEATValue* pNeat)
         {
             IComparisonResults::Index ind = m_indexStack.cpyIndexStack();
             IComparisonResults::MismatchedVal val(ind, m_pMemObjDescCurrent.get(), pRef, pAct, pNeat);
             m_pComparisonResults->AddMismatch(val);
         }

         /// @brief compares floating point scalar with NEAT interval
         template<typename T>
         COMP_RESULT CompareScalarNEAT(const T& Act, const NEATValue& NEAT);

         /// @brief compares scalar accurate or with specified ULP accuracy for 
         /// floats
         template<typename T>
         COMP_RESULT CompareScalarAccurate(const T& Act, const T& Ref);


         /// @brief compare integer value accurate
         template<typename T>
         inline COMP_RESULT CompareScalarAccurateInteger(const T& Act, const T& Ref)
         {
             IsIntegerType<T> _notUsed;
             UNUSED_ARGUMENT(_notUsed);
             return (Utils::eq<T>(Act, Ref)) ? PASSED : NOT_PASSED;
         }

         /// @brief detect and check special values
         /// @return true if special values were detected: Inf, Nans
         /// @return false otherwise
         template<typename T>
         inline bool DetectAndCheckInfsNaNs(const T& Act, const T& Ref, COMP_RESULT& res)
         {
             IsFloatType<T> _notUsed;
             UNUSED_ARGUMENT(_notUsed);
             bool ret = true;

             if(Utils::IsInf<T>(Act) || Utils::IsInf<T>(Ref))
             {  // infinity detected
                 // both are + INF
                 if(Utils::IsPInf<T>(Act) && Utils::IsPInf<T>(Ref))
                     res = PASSED;
                 // both are - INF
                 else if(Utils::IsNInf<T>(Act) && Utils::IsNInf<T>(Ref))
                     res = PASSED;
                 else
                     res = NOT_PASSED;
             }
             else if(Utils::IsNaN<T>(Act) || Utils::IsNaN<T>(Ref))
             {   // check NaNs
                 // if one of the values is NaN
                 // if both are NaNs then passed else fails
                 if(Utils::IsNaN<T>(Act) && Utils::IsNaN<T>(Ref))
                     res = PASSED;
                 else 
                     res =  NOT_PASSED;
             }
             else 
             {   // no special values were detected
                 ret = false;
             }
             return ret;
         }

         // compare value accurate with floating point
         template<typename T>
         inline COMP_RESULT  CompareScalarAccurateFloatingPoint(const T& Act, const T& Ref)
         {
             IsFloatType<T> _notUsed;
             UNUSED_ARGUMENT(_notUsed);

             COMP_RESULT res = NOT_PASSED;
             // if special values were detected and handled
             if(DetectAndCheckInfsNaNs<T>(Act, Ref, res))
                 return res;
             // tolerance in comparator
             res = (Utils::eq_tol<T>(Act, Ref, m_ULP_TOLERANCE) == true) ? PASSED : NOT_PASSED;
             return res;
         }

         // check value fits into interval
         template<typename T>
         inline COMP_RESULT CheckInterval(const T& intervalMin, const T& intervalMax, const T& val)
         {
             if(Utils::le(intervalMin,val) && Utils::le(val, intervalMax))
                 return PASSED;
             else
                 return NOT_PASSED;
         }

         /// @brief  Compares data using NEAT intervals
         /// @param [in] actData Actual data 
         /// @param [in] in_vd   NEAT
         /// @return     Comparison status
         template<typename T>
         COMP_RESULT CompareNEAT(const T& actData, const NEATValue& in_vd)
         {
             IsFloatType<T> _notUsed;
             UNUSED_ARGUMENT(_notUsed);

             // handle special status of NEAT
             NEATValue::Status st = in_vd.GetStatus();
             if(NEATValue::ANY == st)
             {
                 return PASSED;
             }

             if(NEATValue::UNKNOWN == st)
             {
                 DEBUG(llvm::dbgs()<<"[COMPARATOR WARNING] Value with UNKNOWN status found!!\n");
                 return PASSED;
             }

             if(NEATValue::UNWRITTEN == st)
             {
                 DEBUG(llvm::dbgs()<<"[COMPARATOR WARNING] Value with UNWRITTEN status found!!\n");
                 return PASSED;
             }

             const T actualOut = actData;
             const T maxVal = *(in_vd.GetMax<T>());
             const T minVal = *(in_vd.GetMin<T>());
             
             if(Utils::IsNaN(maxVal) || Utils::IsNaN(minVal))
             {
                 DEBUG(llvm::dbgs()<<"[COMPARATOR WARNING] NEAT intervals contain NaN.\n");
                 return PASSED;
             }
             /// Check infinities
             if((Utils::IsNInf(minVal)) && (Utils::IsPInf(maxVal)))
             {
                 DEBUG(llvm::dbgs()<<"[COMPARATOR WARNING] NEAT interval is infinite.\n");
                 return PASSED;
             }
             if(maxVal < minVal)
             {
                 DEBUG(llvm::dbgs()<<"[COMPARATOR WARNING] Neat interval is invalid.\n");
                 return NOT_PASSED;
             }
             if(in_vd.GetStatus() == NEATValue::ACCURATE)
             {  // in accurate case use accurate comparison
                 return CompareScalarAccurate<T>(actData, *(in_vd.GetAcc<T>()));
             }

             COMP_RESULT checkResult = CheckInterval<T>(minVal, maxVal, actualOut);
             return checkResult;
         }

    private:
        Validation::IComparisonResults* m_pComparisonResults;
        /// general index into BuffferContainerList/BufferContainer/{Buffer, Image}/Element
        /// used during execution of comparison
        IComparisonResults::Index        m_indexStack;
        /// general Memory object descriptor used during execution of comparison
        IMemoryObjectDescPtr m_pMemObjDescCurrent;
        /// flag if Comparator has reference 
        bool m_haveReference;
        /// flag if Comparator has NEAT
        bool m_haveNEAT;
        /// flag if current Memory obj supports NEAT
        bool m_IsNEATSupportedMemObj;
        /// tolerance in ULPs used in accurate comparison of floating point types
        double m_ULP_TOLERANCE;

    };

/// specialization for Cfloat16 upconvert to float and call float
template<>
inline COMP_RESULT  Comparator::CompareScalarAccurateFloatingPoint<CFloat16>(const CFloat16& Act, const CFloat16& Ref)
{
    return CompareScalarAccurateFloatingPoint<float>((float) Act, (float) Ref);
}


    // define CompareValueAccurate() specialization for integer types
#define DEF_COMPARE_SCALAR_ACCURATE_INTEGER(__TYPE) \
    template<>\
    inline COMP_RESULT Comparator::CompareScalarAccurate<__TYPE>(const __TYPE & Act, const __TYPE & Ref)\
         {    return CompareScalarAccurateInteger<__TYPE>(Act, Ref);   }

        DEF_COMPARE_SCALAR_ACCURATE_INTEGER(int8_t)
        DEF_COMPARE_SCALAR_ACCURATE_INTEGER(int16_t)
        DEF_COMPARE_SCALAR_ACCURATE_INTEGER(int32_t)
        DEF_COMPARE_SCALAR_ACCURATE_INTEGER(int64_t)
        DEF_COMPARE_SCALAR_ACCURATE_INTEGER(uint8_t)
        DEF_COMPARE_SCALAR_ACCURATE_INTEGER(uint16_t)
        DEF_COMPARE_SCALAR_ACCURATE_INTEGER(uint32_t)
        DEF_COMPARE_SCALAR_ACCURATE_INTEGER(uint64_t)

        // define CompareValueAccurate() specialization for floating point types
#define DEF_COMPARE_SCALAR_ACCURATE_FLOATING(__TYPE) \
    template<> \
    inline COMP_RESULT Comparator::CompareScalarAccurate<__TYPE>(const __TYPE & Act, const __TYPE & Ref)\
    {    return CompareScalarAccurateFloatingPoint(Act, Ref);   }

        DEF_COMPARE_SCALAR_ACCURATE_FLOATING(CFloat16)
        DEF_COMPARE_SCALAR_ACCURATE_FLOATING(float)
        DEF_COMPARE_SCALAR_ACCURATE_FLOATING(double)


        // default implementation of NEAT comparator throws error
        template<typename T>
        inline COMP_RESULT Comparator::CompareScalarNEAT( const T& Act, const NEATValue& NEAT )
        {
            throw Exception::InvalidArgument("CompareScalarNEAT instantiated for non floating point type");
        }

        // macro for specialization of CompareScalarNEAT function for floating point types
#define DEF_COMPARE_SCALAR_NEAT_IMPL(__TYPE) template<>                \
    inline COMP_RESULT Comparator::CompareScalarNEAT<__TYPE>(          \
    const __TYPE& Act, const NEATValue& NEAT ) {                        \
    return CompareNEAT<__TYPE>(Act, NEAT);}

        DEF_COMPARE_SCALAR_NEAT_IMPL(CFloat16)
        DEF_COMPARE_SCALAR_NEAT_IMPL(float)
        DEF_COMPARE_SCALAR_NEAT_IMPL(double)

}

#endif // __COMPARATOR_H__
