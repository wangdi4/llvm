/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  TypeAlignment.h

\*****************************************************************************/

#ifndef __TYPE_ALIGNMRENT_H__
#define __TYPE_ALIGNMRENT_H__

#include "cl_device_api.h"
#include "cpu_dev_limits.h"

#include <vector>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

	/// @brief  TypeAlignment class used to provide alignment and size information
	///         for cl_kernel_argument and general alignment utilities 
	///         (aligning offsets and pointers)
  /// @Author Marina Yatsina
	class TypeAlignment
	{
	public:
	  
	  /// @brief Returns the size of the given argument
    /// @param arg         The argument for which to return its size
    /// @returns The size of the given argument
	  static size_t getSize(const cl_kernel_argument& arg);
	  
	  /// @brief Returns the alignment of the given argument
    /// @param arg         The argument for which to return its alignment
    /// @returns The alignment of the given argument
	  static size_t getAlignment(const cl_kernel_argument& arg);
	  
	  /// @brief Returns offest aligned based on the given alignment
    /// @param alignment    The alignment
    /// @param offset       The offset to align
    /// @returns The offest aligned based on the given alignment
	  static size_t align(size_t alignment, size_t offset);
	  
	  /// @brief Returns pointer aligned based on the given alignment
    /// @param alignment    The alignment
    /// @param pointer       The pointer to align
    /// @returns The pointer aligned based on the given alignment
	  static char* align(size_t alignment, char* ptr);
	  
	public:
	  // Represents the maximum alignment
	  static const size_t MAX_ALIGNMENT = CPU_DEV_MAXIMUM_ALIGN;
	};
	
}}}

#endif // __TYPE_ALIGNMRENT_H__
