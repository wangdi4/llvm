/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ExplicitLocalMemArgument.h

\*****************************************************************************/

#ifndef __EXPLICIT_LOCAL_MEM_ARGUMENT_H__
#define __EXPLICIT_LOCAL_MEM_ARGUMENT_H__

#include "ExplicitArgument.h"
#include "cpu_dev_limits.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  class ExplicitLocalMemArgument : public ExplicitArgument {
  
  public:
  
    /// @brief Constructor
    /// @param arg            OpenCL argument
    /// @param bufferPtr      A pointer to the local memory buffer
    /// @param offset         Implict argument's offset from the beginning 
    ///                       of the arguments buffer
    ExplicitLocalMemArgument(const cl_kernel_argument& arg, size_t bufferSize, size_t offset)
    // During construction we do not have a valid pValue
    : ExplicitArgument(NULL, arg), m_bufferSize(ADJUST_SIZE_TO_MAXIMUM_ALIGN(bufferSize)), m_offset(offset) { }
    
    /// @brief Sets the pointer to the local memory buffer
    /// @param pParmasBase     A pointer to the base parameters buffers
    /// @param bufferPtr       A pointer to the local memory buffer
    void setBufferPtr(char* pParmasBase, void* bufferPtr) { 
      setValuePtr(pParmasBase + m_offset);
      *((void**)m_pValue) = bufferPtr; 
      }
      
    /// @brief Ovveriding implementation - there is no logic to this function anymore
    //         don't want to set value if m_pValue points to NULL
    ///        setBufferPtr replace this functionality
    /// @brief Sets the value of this argument
    /// @param pValue       The src from which to copy the value 
    virtual void setValue(char* pValue) {}
    
    /// @brief Returns the local buffer's size aligned to MAX_ALIGNMENT
    /// @returns  local buffer's size aligned to MAX_ALIGNMENT
    size_t getBufferSize() const { return m_bufferSize; }
    
    /// @brief Returns this implicit argument's offset from the beginning 
    ///        of the arguments buffer
    /// @returns  This implicit argument's offset from the beginning 
    ///           of the arguments buffer
    size_t getOffset() const { return m_offset; }
    
  private:
    /// @brief Sets implict argument's value destination pointer
    /// @param pValue       Implict argument's value destination pointer
    void setValuePtr(char* pValue) 
              { m_pValue = TypeAlignment::align(m_alignment, pValue); }
    
  private:
    /// @brief local buffer's size aligned to MAX_ALIGNMENT
    size_t m_bufferSize;
    
    /// @brief offset from the beginning of the arguments buffer
    size_t m_offset;
  
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __EXPLICIT_LOCAL_MEM_ARGUMENT_H__
