/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

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

    static void initImplicitArgProps(unsigned int uiSizeT);
    
    /// @brief Constructor
    ImplicitArgsUtils() {}
    /// @brief Destructor
    ~ImplicitArgsUtils() {}

#ifndef __APPLE__
    /// @brief Creates implicit arguments based on the implicit arguments properties
    /// @param pDest          A buffer that should hold the values of the implicit arguments
    void createImplicitArgs(char* pDest);
    
    /// @brief Sets values of implicit arguments for arguments that have same
    ///        values per executable
    /// @param pWorkInfo        The work group information parameter
    /// @param pGlobalBaseId    The global base id parameter
    /// @param pCallBackContext The callback context parameter
    /// @param bJitCreateWIids  The indiectaor for JIT creating WI ids parameter
    /// @param packetWidth      The packet width for vectorized JIT parameter
    /// @param pWIids           The work item ids buffer parameter
    /// @param iterCounter      The number of iterations parameter
    /// @param pBarrierBuffer   The barrier buffer parameter
    void setImplicitArgsPerExecutable(
                         const sWorkInfo* pWorkInfo,
                         const size_t* pGlobalBaseId,
                         const CallbackContext* pCallBackContext, 
                         bool bJitCreateWIids,
                         unsigned int packetWidth,
                         size_t* pWIids,
                         const size_t iterCounter,
                         char* pBarrierBuffer);
    
    /// @brief Sets values of implicit arguments for arguments that have same
    ///        values per work group
    /// @param pParams              The arguments values array
    void setImplicitArgsPerWG(const void* pParams);
  
  private:
    /// @brief Initialized the work item local IDs
    /// @param implicitArgument     The implicit arguments arguments
    void initWILocalIds(const sWorkInfo* pWorkInfo, const unsigned int packetWidth, size_t* pWIids);
#endif //#ifndef __APPLE__

  private:
    /// static list of implicit argument properties 
    static ImplicitArgProperties m_implicitArgProps[m_numberOfImplicitArgs];
  
    /// list of implicit arguments
    ImplicitArgument m_implicitArgs[m_numberOfImplicitArgs];
    static bool m_initialized;
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __IMPLICIT_ARGS_UTILS_H__