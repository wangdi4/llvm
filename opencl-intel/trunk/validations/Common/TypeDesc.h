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

File Name:  TypeDesc.h

\*****************************************************************************/
#ifndef __TYPE_DESC_H__
#define __TYPE_DESC_H__

#include "assert.h"
#include <map>
#include <vector>
#include "llvm/System/DataTypes.h"      // LLVM data types
#include "dxfloat.h"
#include "Exception.h"
#include "IMemoryObjectDesc.h"
#include <sstream>


namespace Validation
{
    /// Structure describing type of elements in Buffer
    /// TypeVal's first element must be 0 and last element must be INVALID_DATA_TYPE
    enum TypeVal
    {
        // Floating point types
        THALF=0, TFLOAT, TDOUBLE,
        // Signed integer types
        TCHAR, TSHORT, TINT, TLONG,
        // Unsigned integer types
        TUCHAR, TUSHORT, TUINT, TULONG,
        // Bool type
        TBOOL,
        // Void type
        TVOID,
        // Pointer type
        TPOINTER,
        // Aggregate types
        TARRAY, TVECTOR, TSTRUCT,

        UNSPECIFIED_TYPE,
        // marker for invalid data type
        INVALID_TYPE
    };

    /// template function to return enum TypeVal from T
    template<class T>
    TypeVal InstTypeVal();

    /// helper macro to instantiate InstTypeVal() function definition
#define DEF_INST_TYPEVAL(_TYPE,_TYPEVAL) template<> inline TypeVal InstTypeVal<_TYPE>() { return _TYPEVAL; }

    /// specialize functions
    DEF_INST_TYPEVAL(CFloat16, THALF) 
    DEF_INST_TYPEVAL(float, TFLOAT) 
    DEF_INST_TYPEVAL(double, TDOUBLE)
    DEF_INST_TYPEVAL(int8_t, TCHAR)
    DEF_INST_TYPEVAL(int16_t, TSHORT)
    DEF_INST_TYPEVAL(int32_t, TINT)
    DEF_INST_TYPEVAL(int64_t, TLONG)
    DEF_INST_TYPEVAL(uint8_t, TUCHAR)
    DEF_INST_TYPEVAL(uint16_t, TUSHORT)
    DEF_INST_TYPEVAL(uint32_t, TUINT)
    DEF_INST_TYPEVAL(uint64_t, TULONG)

    /// Data type container and its helper functions
    class TypeValWrapper
    {
    public:
        /// default ctor
        TypeValWrapper()
            : m_value(UNSPECIFIED_TYPE)
        {
            if (!m_isStaticInit) initStatic();
        }

        explicit TypeValWrapper(const TypeVal& val) : m_value(val)
        {
            if (!m_isStaticInit) initStatic();
            // if there is no metadata for value
            CheckValueAndThrow(val);
        }

        TypeVal GetValue() const
        {
            return m_value;
        }

        void SetValue(TypeVal val)
        {
            // if there is no metadata for value
            CheckValueAndThrow(val);
            m_value = val;
        }

        /// \return Size which objects of type m_value occupies in memory.
        std::size_t GetSizeInBytes() const { return m_metaData[m_value].m_size; }
        /// \return Set size which objects of type m_value occupies in memory.
        void SetSize(std::size_t size) { m_metaData[m_value].m_size = size; }
        /// \returns Whether m_value type contains floating point data.
        bool IsFloatingPoint() const { return m_metaData[m_value].m_isFloatingPoint; }
        std::string ToString() const { return m_metaData[m_value].m_toString; }

        /// \return true if type is vector, array or structure
        bool IsAggregate() const
        {
            return m_value == TVECTOR || m_value == TSTRUCT || m_value == TARRAY;
        }

        /// \return true if type is vector, array, pointer or structure
        bool IsComposite() const
        {
            return IsAggregate() || IsPointer();
        }

        bool IsPointer() const
        {
            return m_value == TPOINTER;
        }

        bool IsStruct() const
        {
            return m_value == TSTRUCT;
        }

        bool IsInteger() const
        {
            return (m_value > TDOUBLE) && (m_value < TBOOL);
        }

        static TypeVal ValueOf(const std::string& str)
        {
            // init static members
            if (!m_isStaticInit) initStatic(); 
            // TypeVal first element must be 0 and last element must be INVALID_TYPE
            for (int  i = 0; i < INVALID_TYPE; i++)
            {
                TypeVal dt = (TypeVal) i;
                // should consider  comparing the strings converted to lower case to allow more flexibility
                if (m_metaData[dt].m_toString == str) {
                    return dt;
                }
            }
            throw Exception::InvalidArgument("NonSupported Data Type " + str);
        }

    private:

        TypeVal m_value;

        inline void CheckValueAndThrow(const TypeVal& in_value)
        {
            assert(m_isStaticInit);
            if (m_metaData.count(in_value) < 1)
            {
                throw Exception::InvalidArgument("Invalid arg. No metadata for this TypeDesc");
            }
        }

        void static initStatic();

        class TypeMetadata
        {
        public:
            TypeMetadata()
                : m_size(0), m_isFloatingPoint(false), m_toString(""){}

            TypeMetadata(std::size_t size, bool isFloatingPoint, const std::string& toString) 
                : m_size(size), m_isFloatingPoint(isFloatingPoint), m_toString(toString) {}

            std::size_t m_size;
            bool m_isFloatingPoint;
            std::string m_toString;
        };

        static std::map<TypeVal, TypeMetadata> m_metaData;
        static bool m_isStaticInit;
    };

    /// \brief Description of data type of buffer elements.
    class TypeDesc : public IMemoryObjectDesc
    {
    public:
        TypeDesc() : m_numElements(0), m_offsetInStruct(0), m_type(UNSPECIFIED_TYPE), m_isNEAT(false), m_size(0)
        {}

        /// Constructor for aggregate data type description.
        /// \param in_type                      Type value.
        /// \param in_numOfElements             For array/vector data type - number of elements in array/vector.
        ///                                     For pointer - number of elements which could be accessible through the pointer, if it's known.
        /// \param in_numOfStructureElements    Number of structure members/fields.
        ///                                     Number of subtypes. It equals to 0 for basic data types.
        TypeDesc(TypeVal in_type, uint64_t in_numOfElements = 0, uint64_t in_numOfStructureElements = 0) : m_numElements(in_numOfElements), m_offsetInStruct(0), m_type(in_type), m_isNEAT(false), m_size(0)
        {
            SetUpSubTypes(in_numOfStructureElements);
        }

        /// Getter/Setter for the number of the types contained into the current type.
        // If type is basic type not containing other types (except pointer) then 0 will be returned.
        uint64_t GetNumOfSubTypes() const {return m_subTypes.size();}
        void SetNumOfSubTypes(uint64_t n) {SetUpSubTypes(n);}

        virtual void SetNeat(const bool inNEAT);
        virtual bool IsNEAT() const;
        
        virtual IMemoryObjectDesc * Clone() const
        { return new TypeDesc(*this); }
        
        /// @brief get Name of class
        virtual std::string GetName() const{return "TypeDesc"; } 


        /// Gets number of elements in array/vector or number of elements accessed by pointer.
        uint64_t GetNumberOfElements() const
        {
            if (m_type.GetValue() == TVECTOR || m_type.GetValue() == TARRAY || m_type.GetValue() == TPOINTER)
                return m_numElements;
            return 0;
        }

        /// Sets number of elements in array/vector or number of elements accessed by pointer.
        void SetNumberOfElements(uint64_t num)
        {
            if (num)
            {
                if (m_type.GetValue() == TVECTOR || m_type.GetValue() == TARRAY || m_type.GetValue() == TPOINTER)
                {
                    m_numElements = num;
                }
                else
                {
                    throw Exception::IllegalFunctionCall(std::string("Can't set number of elements for ") + m_type.ToString());
                }
            }
            else
                m_numElements = 0;
        }

        /// Getter/Setter for the type value.
        TypeVal GetType() const {return m_type.GetValue();}
        void SetType(TypeVal in_type) {m_type.SetValue(in_type); SetUpSubTypes(0);}

        /// Getter/Setter for the id-th sub-type of aggregate data type.
        TypeDesc GetSubTypeDesc(uint64_t id) const
        {
            if (!IsComposite())
            {
                throw Exception::IllegalFunctionCall("GetSubTypeDesc: " + m_type.ToString() + std::string(" has no subtypes!"));
            }
            if (id >= m_subTypes.size())
            {
                throw Exception::InvalidArgument(std::string("Can't set sub-type description. Invalid sub-type index for ") + m_type.ToString() + std::string(" data type!"));
            }
            return m_subTypes[id];
        }

        void SetSubTypeDesc(uint64_t id, const TypeDesc& in_typeDesc)
        {
            if (!IsComposite())
            {
                throw Exception::IllegalFunctionCall("SetSubTypeDesc: " + m_type.ToString() + std::string(" has no subtypes!"));
            }
            if (id >= m_subTypes.size())
            {
                throw Exception::InvalidArgument(std::string("Can't set sub-type description. Invalid sub-type index for ") + m_type.ToString() + std::string(" data type!"));
            }
            m_subTypes[id] = in_typeDesc;
        }

        /// Sets memory size of objects of aggregate data type.
        // It's primarily needed for 'struct' data types which assumes paddings and alignment.
        void SetTypeAllocSize(std::size_t in_size) {m_size = in_size;}

        /// If current data types is used as 'struct' member, this method returns member offset in bytes.
        uint64_t GetOffsetInStruct() const {return m_offsetInStruct;}
        /// If current data types is used as 'struct' member, this method sets member offset in bytes.
        void SetOffsetInStruct (uint64_t offset) {m_offsetInStruct = offset;}

        /// Returns object size of m_type data type in bytes.
        std::size_t GetSizeInBytes() const;

        /// Returns string representation of data type (not including sub types!).
        std::string TypeToString() const
        {
            return m_type.ToString();
        }

        /// \returns Whether m_type type contains floating point data.
        bool IsFloatingPoint() const
        {
            bool ret = m_type.IsFloatingPoint();
            for (std::size_t i = 0; i < m_subTypes.size(); ++i)
            {
                ret |= m_subTypes[i].IsFloatingPoint();
            }
            return ret;
        }

        /// \return true if type is vector, array or structure
        bool IsAggregate() const
        {
            return m_type.IsAggregate();
        }

        /// \return true if type is vector, array, pointer or structure
        bool IsComposite() const
        {
            return IsAggregate() || IsPointer();
        }

        bool IsPointer() const
        {
            return m_type.IsPointer();
        }

        bool IsStruct() const
        {
            return m_type.IsStruct();
        }

        bool IsInteger() const
        {
            return m_type.IsInteger();
        }

        bool operator ==(const TypeDesc& a) const;
        bool operator !=(const TypeDesc& a) const
        {
            return !(*this == a);
        }
    private:
        void SetUpSubTypes(std::size_t in_numOfStructureSubTypes);
        // Number of elements in:.
        // array data type - number of elements in array.
        // vector data type - number of elements in vector.
        // if data type is pointer - number of elements which could be accessible through the pointer, if it's known. Default value = 1.
        uint64_t m_numElements;
        // offset of type inside struct type.
        uint64_t m_offsetInStruct;
        TypeValWrapper m_type;
        bool m_isNEAT;
        uint64_t m_size;    // Type size in memory (including alignment).
        // Contains description of data types for:
        // 1. Members of struct.
        // 2. Elements of array. Since all elements have the same type, the size of m_subTypes must be = 1.
        // 3. Elements of vector. Since all elements have the same type, the size of m_subTypes must be = 1.
        // 4. value which pointer refers to. Since all elements have the same type, the size of m_subTypes must be = 1.
        std::vector<TypeDesc> m_subTypes;
    };
} // namespace Validation

#endif // __TYPE_DESC_H__

