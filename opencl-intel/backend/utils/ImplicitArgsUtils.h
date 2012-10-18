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

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  class CallbackContext;
  struct sWorkInfo;

  /// @brief  ImplicitArgsUtils class used to provide helper utilies for handling
  ///         implicit arguments.
  /// @Author Marina Yatsina
  class ImplicitArgsUtils {
  
  public:
    enum IMPLICIT_ARGS {
      IA_SLM_BUFFER,
      IA_WORK_GROUP_INFO,
      IA_WORK_GROUP_ID,
      IA_GLOBAL_BASE_ID,
      IA_CALLBACK_CONTEXT,
      IA_LOCAL_ID_BUFFER,
      IA_LOOP_ITER_COUNT,
      IA_BARRIER_BUFFER,
      IA_CURRENT_WORK_ITEM,
      IA_NUMBER
    };
    static const unsigned int m_numberOfImplicitArgs = IA_NUMBER;

    /// @brief Returns the implicit arguments properties
    /// @returns The implicit arguments properties
    //static unsigned int getNumArgs() {  return m_numberOfImplicitArgs; }
    
    /// @brief Returns the implicit argument properties of given argument index
    /// @param arg     The implicit argument index
    /// @returns The implicit argument properties
    static const ImplicitArgProperties& getImplicitArgProps(unsigned int arg);
    
    /// @brief Constructor
    ImplicitArgsUtils() {}
    /// @brief Destructor
    ~ImplicitArgsUtils() {}

    /// @brief Creates implicit arguments based on the implicit arguments properties
    /// @param pDest          A buffer that should hold the values of the implicit arguments
    void createImplicitArgs(char* pDest);
    
    /// @brief Sets values of implicit arguments for arguments that have same
    ///        values per executable
    /// @param implicitArgument     The implicit arguments arguments
    /// @param pExecutable          The executable
    /// @param pLocalMemoryBuffers  The local memory buffers, will be used to set the pLocalMem arg
    /// @param pWGStackFrame        The work group stack frame, used to set the local IDs and the special buffer
    /// @param uiWICount            The work item count, uset to set the number of iterations
    void setImplicitArgsPerExecutable(
                         void* pLocalMemoryBuffer,
                         const sWorkInfo* pWorkInfo,
                         const size_t* pGlobalBaseId,
                         const CallbackContext* pCallBackContext, 
                         bool bJitCreateWIids,
                         unsigned int packetWidth,
                         size_t* pWIids,
                         const size_t iterCounter,
                         char* pBarrierBuffer,
                         size_t* pCurrWI);
    
    /// @brief Sets values of implicit arguments for arguments that have same
    ///        values per work group
    /// @param pParams              The arguments values array
    void setImplicitArgsPerWG(const void* pParams);
  
  private:
    /// @brief Initialized the work item local IDs
    /// @param implicitArgument     The implicit arguments arguments
    void initWILocalIds(const sWorkInfo* pWorkInfo, const unsigned int packetWidth, size_t* pWIids);
  

    /// static list of implicit argument properties 
    static ImplicitArgProperties m_implicitArgProps[m_numberOfImplicitArgs];

    /// list of implicit arguments
    ImplicitArgument m_implicitArgs[m_numberOfImplicitArgs];
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __IMPLICIT_ARGS_UTILS_H__