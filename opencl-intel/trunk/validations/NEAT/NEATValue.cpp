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

File Name:  NEATValue.cpp


\*****************************************************************************/
#include "NEATValue.h"

namespace Validation 
{

    /// assignment o-r
    NEATValue& NEATValue::operator=(const NEATValue& p)
    {
        if (this != &p)
        {  // make sure not same object
            m_Status = p.m_Status;
            //m_Type   = p.m_Type;
            memcpy(m_min, p.m_min, MaxBytes);
            memcpy(m_max, p.m_max, MaxBytes);
        }
        return *this;
    }

    void NEATValue::SetStatus(Status in_Status)
    {
        m_Status = in_Status;
    }

    const NEATValue::Status& NEATValue::GetStatus() const
    {
        return m_Status;
    }

    bool NEATValue::IsUnknown() const
    {
        return (m_Status == UNKNOWN);
    }

    /// Checks whether variable can have any value.
    /// both "unwritten" status and "any" status applicable
    bool NEATValue::IsAny() const
    {
        return (m_Status == NEATValue::ANY);
    }

    bool NEATValue::IsAcc() const
    {
        return (m_Status == ACCURATE);
    }
    bool NEATValue::IsUnwritten() const
    {
        return (m_Status == UNWRITTEN);
    }
    
    std::istream& operator >> (std::istream& is,  NEATValue& k)
    {
        std::string StatusStr;
        is >> StatusStr;
        if("ACCURATE" == StatusStr )
        {
            k.m_Status = NEATValue::ACCURATE;
            for(uint32_t i=0;i<NEATValue::MaxBytes;++i)
            {
                int32_t _t;
                is >> _t;
                k.m_min[i] = static_cast<uint8_t>(_t);
            }        
        }
        else if("UNKNOWN" == StatusStr )
        {
            k.m_Status = NEATValue::UNKNOWN; 
        }
        else if("UNWRITTEN" == StatusStr )
        {
            k.m_Status = NEATValue::UNWRITTEN;
        }
        else if("ANY" == StatusStr )
        {
            k.m_Status = NEATValue::ANY;      
        }
        else if( "INTERVAL" == StatusStr )
        {
            k.m_Status = NEATValue::INTERVAL;      
            for(uint32_t i=0;i<NEATValue::MaxBytes;++i)
            {
                int32_t _t;
                is >> _t;
                k.m_min[i] = static_cast<uint8_t>(_t);
            }        
            for(uint32_t i=0;i<NEATValue::MaxBytes;++i)
            {
                int32_t _t;
                is >> _t;
                k.m_max[i] = static_cast<uint8_t>(_t);
            }        
        }
        else
        {
            throw Exception::IOError("Invalid input NEAT state variable");
        }
        return is;
    }

    std::ostream& operator << (std::ostream& os,  const NEATValue& k)
    {
        switch (k.m_Status) 
        {
                    case NEATValue::ACCURATE:
                        os << "ACCURATE" << " ";
                        for(uint32_t i=0;i<NEATValue::MaxBytes;++i)
                            os << (int32_t) k.m_min[i] << " ";
                        break;
                    case NEATValue::UNKNOWN:
                        os << "UNKNOWN";
                        break;
                    case NEATValue::UNWRITTEN:
                        os << "UNWRITTEN";
                        break;
                    case NEATValue::ANY:
                        os << "ANY";
                        break;
                    case NEATValue::INTERVAL:
                        os << "INTERVAL" << " "; 
                        for(uint32_t i=0;i<NEATValue::MaxBytes;++i)
                            os << (int32_t) k.m_min[i] << " ";
                        for(uint32_t i=0;i<NEATValue::MaxBytes;++i)
                            os << (int32_t) k.m_max[i] << " ";
                        break;
                    default:
                        throw Exception::InvalidArgument("");
        }
        return os;
    }
}
