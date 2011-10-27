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

File Name:  DataType.h

\*****************************************************************************/
#ifndef __DATATYPE_H__
#define __DATATYPE_H__

#include "assert.h"
#include <map>
#include "llvm/System/DataTypes.h"      // LLVM data types
#include "Exception.h"


namespace Validation
{
    /// Structure describing type of elements in Buffer
    /// DataTypeVal's first element must be 0 and last element must be INVALID_DATA_TYPE
    enum DataTypeVal
    {
        // Floating point types
        F16=0, F32, F64,
        // Signed integer types
        I8, I16, I32, I64,
        // Unsigned integer types
        U8, U16, U32, U64,
        // marker for invalid datatype
        INVALID_DATA_TYPE,
        UNSPECIFIED_DATA_TYPE
    };

    /// Data type container and its helper functions
    class DataTypeValWrapper
    {
    public:
        /// default ctor
        DataTypeValWrapper()
            : m_value(INVALID_DATA_TYPE)
        {
            if (!m_isStaticInit) initStatic();
        }

        explicit DataTypeValWrapper(const DataTypeVal& value) : m_value(value)
        {
            if (!m_isStaticInit) initStatic();
            // if there is no metadata for value
            CheckValueAndThrow(value);
        }

        DataTypeVal GetValue() const
        {
            return m_value;
        }

        void SetValue(DataTypeVal val)
        {
            // if there is no metadata for value
            CheckValueAndThrow(val);
            m_value = val;
        }

        std::size_t GetSize() const { return m_metaData[m_value].m_size; }
        bool IsFloatingPoint() const { return m_metaData[m_value].m_isFloatingPoint; }
        std::string ToString() const { return m_metaData[m_value].m_toString; }

        static DataTypeVal ValueOf(const std::string& str)
        {
            // init static members
            if (!m_isStaticInit) initStatic(); 
            // DataTypeVal first element must be 0 and last element must be INVALID_DATA_TYPE
            for (int  i = 0; i < INVALID_DATA_TYPE; i++)
            {
                DataTypeVal dt = (DataTypeVal) i;
                // should consider  comparing the strings converted to lower case to allow more flexibility
                if (m_metaData[dt].m_toString == str) {
                    return dt;
                }
            }
            throw Exception::InvalidArgument("NonSupported Data Type " + str);
        }

    private:

        DataTypeVal m_value;

        inline void CheckValueAndThrow(const DataTypeVal& in_value)
        {
            assert(m_isStaticInit);
            if (m_metaData.count(in_value) < 1)
            {
                throw Exception::InvalidArgument("Invalid arg. No metadata for this DataType");
            }
        }

        void static initStatic();

        class DataTypeMetadata
        {
        public:
            DataTypeMetadata()
                : m_size(0), m_isFloatingPoint(false), m_toString(""){}

            DataTypeMetadata(std::size_t size, bool isFloatingPoint, const std::string& toString) 
                : m_size(size), m_isFloatingPoint(isFloatingPoint), m_toString(toString) {}

            std::size_t m_size;
            bool m_isFloatingPoint;
            std::string m_toString;
        };

        static std::map<DataTypeVal, DataTypeMetadata> m_metaData;
        static bool m_isStaticInit;
    };

} // namespace Validation

#endif // __DATATYPE_H__

