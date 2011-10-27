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

File Name:  IMemoryObjectDesc.h

\*****************************************************************************/
#ifndef __I_MEMORY_OBJECT_DESC_H__
#define __I_MEMORY_OBJECT_DESC_H__

namespace Validation
{
    class IMemoryObjectDesc
    {
    public:
        /// @brief is NEAT object
        virtual bool IsNEAT() const = 0;
        /// @brief Set NEAT property
        virtual void SetNeat(const bool in_IsNeat) = 0;
        /// @brief create clone object
        virtual IMemoryObjectDesc * Clone() const = 0;
        /// @brief get Name of class
        virtual std::string GetName() const = 0;
    };

    // helper class to track IMemoryObjectDesc * objects
    template<class T>
    class IMemoryObjectDescWrapper{
        /// pointer to object with IMemoryObjectDesc interface
        T* m_p;
    public:
        IMemoryObjectDescWrapper()
            :m_p(NULL){
        }
        IMemoryObjectDescWrapper(const IMemoryObjectDescWrapper& r){
            m_p = (r.m_p)?r.m_p->Clone():NULL;
        }
        IMemoryObjectDescWrapper(const T * pr){
            m_p = pr ? pr->Clone():NULL;
        }
        ~IMemoryObjectDescWrapper()
        {
            if(m_p) delete m_p;
        }
        IMemoryObjectDescWrapper& operator =(const IMemoryObjectDescWrapper&r){
            if(m_p)
                delete m_p;
            m_p = (r.m_p) ? r.m_p->Clone() : NULL;
            return *this;
        }
        IMemoryObjectDescWrapper& operator =(const T*pr){
            if(m_p)
                delete m_p;
            m_p = (pr)?pr->Clone():NULL;
            return *this;
        }
        T* operator ->(){
            return m_p;
        }
        
        T *get() const 
        {   // return wrapped pointer
            return (m_p);
        }
    };

    
    typedef IMemoryObjectDescWrapper<IMemoryObjectDesc> IMemoryObjectDescPtr;


} // End of Validation namespace
#endif // __I_MEMORY_OBJECT_DESC_H__
