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

File Name:  IComparisonResults.h

\*****************************************************************************/

#ifndef __ICOMPARISONRESULT_H__
#define __ICOMPARISONRESULT_H__

#include "BufferContainerList.h"
#include "IMemoryObjectDesc.h"
#include "NEATValue.h"
#include <sstream>
#include "StatisticsCollector.h"

namespace Validation {

    enum MismatchType {REF_BACKEND, REF_NEAT, BACKEND_NEAT};

    ///////////////////////
    /// IComparisonResults code
    ///
    /// reports all errors/warnings/mismatched intervals
    class IComparisonResults {
    public:
        /// @brief Adds mismatch information
        /// @param  [in]  index                 Indexes of mismatched value in containers(BufferContainer index, Buffer index, Vector index, element index)
        /// @param  [in]  bufferDesc            Description of the buffer containing the mismatched element
        /// @param  [in]  referenceDataPtr      Reference element
        /// @param  [in]  actualDataPtr         Actual element
        /// @param  [in]  neat                  Neat interval for element ('Accurate' for integer elements)
        ///

        struct MismatchedVal;

        virtual void AddMismatch(const MismatchedVal& in_Val) = 0;

        virtual MismatchedVal GetMismatch(size_t index) = 0;

        virtual size_t GetMismatchCount() = 0;

        virtual void Clear() = 0;

        /// @brief returns pointer to object that contains statistics data
        virtual StatisticsCollector* GetStatistics() = 0;

        /// structure describing location of comparison error
        /// in BufferContainerList/BufferContainer/Buffer/Vector/Element hierarchy
        struct Index
        {
            typedef std::vector<uint32_t> StackTy;
            void pushIdx(uint32_t idx)
            {
                m_stack.push_back(idx);
            }
            void popIdx()
            {
                assert(!m_stack.empty());
                m_stack.pop_back();
            }
            uint32_t GetBufId() const
            {
                return m_stack.at(BUF_IDX);
            }
            std::string ToString() const
            {
                std::stringstream ss;
                StackTy::const_iterator it = m_stack.begin();
                ss<<"( ";
                if(!m_stack.empty())
                {
                    ss<<*it;
                    it++;
                }
                for(; it != m_stack.end(); it++)
                {
                    ss<<"; ";
                    ss<<*it;
                }
                ss<<" )";
                return ss.str();
            }
            Index cpyIndexStack()
            {
                Index toRet;
                toRet.m_stack = this->m_stack;
                return toRet;
            }
            StackTy m_stack;
        private:
            static const uint32_t BUF_IDX = 1;
        };


        struct NEATValueContainer
        {
            NEATValueContainer()
            {
                memset(&val, 0, sizeof(NEATValue));
                isValid = false;
            }

            template<typename T>
            NEATValueContainer(T in_val)
            {
                SetVal<T>(in_val);
            }

            template<typename T>
            void SetVal(const NEATValue& in_val)
            {
                val = in_val;
                m_str = in_val.ToString<T>();
                isValid = true;
            }

            NEATValue GetVal()
            {
                return val;
            }

            std::string ToString() const
            {
                return m_str;
            }

            bool IsValid() const
            {
                return isValid;
            }
        private:
            bool isValid;
            NEATValue val;
            std::string m_str;
        };

        struct ValueContainer
        {
            ValueContainer()
            {
                memset(val, 0, sizeof(char)*MaxBytes);
                isValid = false;
            }

            template<typename T>
            ValueContainer(T in_val)
            {
                SetVal<T>(in_val);
            }

            void SetVal(const void* data, int size)
            {
                if(size != 0)
                {
                    memcpy(val, data, size);
                    isValid = true;
                    m_str = "No type";
                } else
                {
                    throw Exception::InvalidArgument("");
                }
            }

            template<typename T>
            void SetVal(const T& in_val)
            {
                std::stringstream ss;
                memcpy(val, &in_val, sizeof(T));
                ss << in_val;
                m_str = ss.str();
                isValid = true;
            }


            template<typename T>
            T GetVal()
            {
                T toReturn;
                memcpy(&toReturn, val, sizeof(T));
                return toReturn;
            }

            std::string ToString() const
            {
                return m_str;
            }

            bool IsValid() const
            {
                return isValid;
            }
        private:
            bool isValid;
            static const uint32_t MaxBytes = sizeof(long double);
            char val[MaxBytes];
            std::string m_str;
        };


        /// Mismatch information will be stored as the list of the following structures
        struct MismatchedVal
        {
            MismatchedVal() : pDesc(NULL) {}

            template<typename T>
            MismatchedVal(const Index& in_index, const IMemoryObjectDesc *in_pDesc, const T* in_referenceDataPtr,
                const T* in_actualDataPtr, const NEATValue* in_pNEAT)
            {
                pDesc = in_pDesc;
                if(in_actualDataPtr != NULL)
                    act.SetVal<T>(*in_actualDataPtr);
                if(in_referenceDataPtr != NULL)
                    ref.SetVal<T>(*in_referenceDataPtr);
                if(in_pNEAT != NULL)
                    neat.SetVal<T>(*in_pNEAT);
                idx = in_index;
            }

            bool HasReference() { return ref.IsValid(); }
            bool HasActual()    { return act.IsValid(); }
            bool HasNEAT()      { return neat.IsValid(); }

            template<typename T>
            void SetReference(T val)
            {
                ref.SetVal(val);
            }

            template<typename T>
            void SetActual(T val)
            {
                act.SetVal(val);
            }

            template<typename T>
            void SetNEAT(NEATValue val)
            {
                neat.SetVal<T>(val);
            }

            std::string ToString() const
            {
                std::stringstream ss;
                if((ref.IsValid()) && neat.IsValid() && !act.IsValid())
                    ss<<"Reference outside NEAT: ";
                else if(act.IsValid() && neat.IsValid() && !ref.IsValid())
                    ss<<"Actual outside NEAT:    ";
                else
                    ss << "Mismatched values:    ";
                ss<<" index = "<< idx.ToString();
                if(ref.IsValid())
                    ss<<"; Reference = "<<ref.ToString();
                if(act.IsValid())
                    ss<<"; Actual = "<<act.ToString();
                if(neat.IsValid())
                    ss<<"; NEAT = "<<neat.ToString();
                return ss.str();
            }

            /// Template for integer types
            /// T should be integer
            template<typename T>
            double ComputeDiff(ValueContainer ref, ValueContainer act) const
            {
                if((!ref.IsValid()) || (!act.IsValid()))
                    throw Exception::InvalidArgument("Input value container was invalid during comparator mismatch reporting");
                double res = (T)abs((long)(ref.GetVal<T>()- act.GetVal<T>()));
                return res;
            }

            long double ComputeDeviation() const;

            int GetBufIdx() const
            {
                return idx.GetBufId();
            }

            const IMemoryObjectDesc *GetDesc() const
            {
                return pDesc.get();
            }

            TypeVal GetTypeVal() const
            {
                TypeVal ret;
                if("BufferDesc" == GetDesc()->GetName())
                {
                    const BufferDesc* pBufDesc = static_cast<const BufferDesc*>(GetDesc());
                    const TypeDesc typeDesc = pBufDesc->GetElementDescription();
                    const TypeVal typeVal = typeDesc.GetType();
                    
                    if(typeDesc.IsAggregate())
                    {   // aggregate type
                        if(typeVal == TVECTOR || typeVal == TARRAY){
                            ret = typeDesc.GetSubTypeDesc(0).GetType();
                        }
                        else
                        {
                            throw Exception::NotImplemented("this Aggregate type is not implemented");
                        }
                    }
                    else ret = typeVal;  // scalar type
                }
                else if(ImageDesc::GetImageDescName() == GetDesc()->GetName())
                {
                    const ImageDesc* pImgDesc = static_cast<const ImageDesc*>(GetDesc());
                    ret = ImageChannelDataTypeToTypeVal(pImgDesc->GetImageChannelDataType());
                }
                else throw Exception::InvalidArgument("Unsupported descriptor");

                return ret;
            }

        private:
            /// type of mismatch
            MismatchType type;
            /// description of memory object
            IMemoryObjectDescPtr pDesc;
            /// @brief Container with value and its string representation
            ValueContainer ref;
            /// @brief Container with value and its string representation
            ValueContainer act;
            /// @brief Container with NEATValue and its string representation
            NEATValueContainer neat;
            /// @brief  Indexes of mismatched value in containers
            Index idx;
        };
    };
            template<>
            inline void IComparisonResults::ValueContainer::SetVal(const int8_t& in_val)
            {
                std::stringstream ss;
                memcpy(val, &in_val, sizeof(in_val));
                ss << (int)in_val;
                m_str = ss.str();
                isValid = true;
            }

            template<>
            inline void IComparisonResults::ValueContainer::SetVal(const uint8_t& in_val)
            {
                std::stringstream ss;
                memcpy(val, &in_val, sizeof(in_val));
                ss << (int)in_val;
                m_str = ss.str();
                isValid = true;
            }

            template<>
            inline double IComparisonResults::MismatchedVal::ComputeDiff<float>(ValueContainer ref, ValueContainer act) const
            {
                double res = Utils::ulpsDiffSamePrecision(ref.GetVal<float>(), act.GetVal<float>());
                return res;
            }

            template<>
            inline double IComparisonResults::MismatchedVal::ComputeDiff<double>(ValueContainer ref, ValueContainer act) const
            {
                double res = Utils::ulpsDiffSamePrecision(ref.GetVal<double>(), act.GetVal<double>());
                return res;
            }

            template<>
            inline double IComparisonResults::MismatchedVal::ComputeDiff<CFloat16>(ValueContainer ref, ValueContainer act) const
            {
                float res = Utils::ulpsDiffSamePrecision((float)ref.GetVal<CFloat16>(), (float)act.GetVal<CFloat16>());
                return res;
            }

            inline long double IComparisonResults::MismatchedVal::ComputeDeviation() const
            {
                long double res = 0;
                TypeVal ty = GetTypeVal();

                switch(ty)
                {
                case THALF:
                    res = ComputeDiff<CFloat16>(ref, act);
                    break;
                case TFLOAT:
                    res = ComputeDiff<float>(ref, act);
                    break;
                case TDOUBLE:
                    res = ComputeDiff<double>(ref, act);
                    break;
                case TCHAR:
                    res = ComputeDiff<int8_t>(ref, act);
                    break;
                case TSHORT:
                    res = ComputeDiff<int16_t>(ref, act);
                    break;
                case TINT:
                    res = ComputeDiff<int32_t>(ref, act);
                    break;
                case TLONG:
                    res = ComputeDiff<int64_t>(ref, act);
                    break;
                case TUCHAR:
                    res = ComputeDiff<uint8_t>(ref, act);
                    break;
                case TUSHORT:
                    res = ComputeDiff<uint16_t>(ref, act);
                    break;
                case TUINT:
                    res = ComputeDiff<uint32_t>(ref, act);
                    break;
                case TULONG:
                    res = ComputeDiff<int64_t>(ref, act);
                    break;
                default:
                    throw Exception::IllegalFunctionCall("Can't add mismatch for type");
                    break;
                }
                return abs(res);
            }

}
#endif // __ICOMPARISONRESULT_H__

