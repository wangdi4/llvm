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

File Name:  ImplicitArgProperties.h

\*****************************************************************************/

#ifndef __IMPLICIT_ARG_PROPERTIES_H__
#define __IMPLICIT_ARG_PROPERTIES_H__

#include <string>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  /// @brief  ImplicitArgProperties class used to describe each implicit argument
  /// @Author Marina Yatsina

  class ImplicitArgProperties
  {
    public:
      
      /// @brief  Constructor, use this constructor this if the arguments size
      ///         and alignment are the same
      /// @param name        The implicit argument name
      /// @param size        The implicit argument size
      ImplicitArgProperties(const std::string& name, size_t size)
       : m_name(name), m_size(size), m_alignment(size) { }
       
      /// @brief  Constructor, use this constructor this if the arguments size
      ///         and alignment are NOT the same
      /// @param name        The implicit argument name
      /// @param size        The implicit argument size
      /// @param alignment   The implicit argument alignment
      ImplicitArgProperties(const std::string& name, size_t size, size_t alignment)
       : m_name(name), m_size(size), m_alignment(alignment) { }

      /// @brief Getter
      /// @returns The implicit argument's name
      std::string getName() const { return m_name; }
      
      /// @brief Getter
      /// @returns The implicit argument's size
      size_t getSize() const { return m_size; }
      
      /// @brief Getter
      /// @returns The implicit argument's alignment
      size_t getAlignment()const { return m_alignment; }
      
    private:
      /// @brief Implicit argument's name
      std::string m_name;
      
      /// @brief Implicit argument's size
      size_t m_size; 
      
      /// @brief Implicit argument's alignment
      size_t m_alignment; 
  };


}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __IMPLICIT_ARG_PROPERTIES_H__
