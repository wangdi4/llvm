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

File Name:  ImplicitArgsUtils.h

\*****************************************************************************/

#ifndef __IMPLICIT_ARGS_UTILS_H__
#define __IMPLICIT_ARGS_UTILS_H__

#include "ImplicitArgProperties.h"
#include "ImplicitArgument.h"
#include "Binary.h"
#include "Executable.h"

#include <vector>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  /// @brief  ImplicitArgsUtils class used to provide helper utilies for handling
  ///         implicit arguments.
  /// @Author Marina Yatsina
  class ImplicitArgsUtils {
  
  public:
    /// @brief Returns the implicit arguments properties
    /// @returns The implicit arguments properties
    static unsigned int getNumArgs() {  return m_implicitArgProps.size(); }
    
    /// @brief Returns the implicit arguments properties
    /// @returns The implicit arguments properties
    static const std::vector<ImplicitArgProperties>& getImplicitArgProps() 
                          { return m_implicitArgProps; }
    
    /// @brief Creates implicit arguments based on the implicit arguments properties
    /// @param pDest          A buffer that should hold the values of the implicit arguments
    /// @param implicitArgs   Implicit arguments pointing to offsets in the buffer that
    ///                       will allow setting the arguments' values directly to the right 
    ///                       places in the buffer
    static void createImplicitArgs(char* pDest, std::vector<ImplicitArgument>& /* OUT */  implicitArgs);
    
    /// @brief Sets values of implicit arguments for arguments that have same
    ///        values per binary
    /// @param implicitArgument     The implicit arguments arguments
    /// @param pBinary              The binary
    static void setImplicitArgsPerBinary(
                         std::vector<ImplicitArgument>& implicitArgument, 
                         const Binary* pBinary);
    
    /// @brief Sets values of implicit arguments for arguments that have same
    ///        values per executable
    /// @param implicitArgument     The implicit arguments arguments
    /// @param pExecutable          The executable
    /// @param pLocalMemoryBuffers  The local memory buffers, will be used to set the pLocalMem arg
    /// @param pWGStackFrame        The work group stack frame, used to set the local IDs and the special buffer
    /// @param uiWICount            The work item count, uset to set the number of iterations
    static void setImplicitArgsPerExecutable(
                         std::vector<ImplicitArgument>& implicitArgument, 
                         const Executable* pExecutable, 
                         void* *pLocalMemoryBuffers, 
                         void* pWGStackFrame, 
                         unsigned int uiWICount);
    
    /// @brief Sets values of implicit arguments for arguments that have same
    ///        values per work group
    /// @param implicitArgument     The implicit arguments arguments
    /// @param pParams              The arguments values array
    static void setImplicitArgsPerWG(
                         std::vector<ImplicitArgument>& implicitArgument, 
                         void* pParams);
  
  private:
     /// @brief Initialized the implicit arguments properties
     /// @returns The implicit arguments properties
     static std::vector<ImplicitArgProperties> initArgPropsVector();
     
     /// @brief Initialized the work item local IDs
     static void initWILocalIds(const Executable* pExecutable, size_t* pWIids);
  
  private:
    // TODO : make it small vector?
    static const std::vector<ImplicitArgProperties> m_implicitArgProps;    
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __IMPLICIT_ARGS_UTILS_H__