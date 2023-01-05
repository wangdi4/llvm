// INTEL CONFIDENTIAL
//
// Copyright 2006-2022 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once

#include "cl_framework.h"
#include <Logger.h>
#include <cl_dynamic_lib.h>
#include <cl_object.h>
#include <frontend_api.h>

namespace Intel {
namespace OpenCL {
namespace Framework {

/*******************************************************************************
 * Class name:   FECompiler
 *
 * Description:  front-end compiler class
 * Author:       Uri Levy
 * Date:         March 2008
 ******************************************************************************/
class FrontEndCompiler : public OCLObject<_cl_object> {

private:
  /*****************************************************************************
   * Function:     FrontEndCompiler
   * Description:  Private copy constructor to avoid wrong assignment (Klocwork)
   * Arguments:
   * Author:       Guy Benyei
   * Date:         June 2012
   ****************************************************************************/
  FrontEndCompiler(const FrontEndCompiler &)
      : OCLObject<_cl_object>(nullptr, ""){};

  /*****************************************************************************
   * Function:     FECompiler
   * Description:  The Frontend compiler class constructor
   * Arguments:
   * Author:       Uri Levy
   * Date:         March 2008
   ****************************************************************************/
  FrontEndCompiler();

public:
  PREPARE_SHARED_PTR(FrontEndCompiler)

  static SharedPtr<FrontEndCompiler> Allocate() {
    return SharedPtr<FrontEndCompiler>(new FrontEndCompiler());
  }

  /*****************************************************************************
   * Function:     Initialize
   * Description:  Initialize the front-end compiler
   * Arguments:
   * Return value: CL_SUCCESS - The initialization operation succeeded
   * Author:       Uri Levy
   * Date:         March 2008
   ****************************************************************************/
  cl_err_code Initialize(const void *pDeviceInfo, size_t stDevInfoSize);

  /*****************************************************************************
   * Function:     FreeResources
   * Description:  Frees the front-end compiler resources
   * Arguments:
   * Return value: CL_SUCCESS - The operation succeeded
   * Author:       Doron Singer
   * Date:         March 2008
   ****************************************************************************/
  void FreeResources();

  /*****************************************************************************
   * Function:     CompileProgram
   * Description:  Compile source code and headers and return binary data
   * Arguments:    szProgramSource - the main program source string
   *               uiNumInputHeaders - the number of input headers in
   *                                   pszInputHeaders
   *               pszInputHeaders - an array of input headers strings
   *               pszInputHeadersNames - array of headers names corresponding
   *                                      to pszInputHeaders
   *               szOptions - compile options string
   *               bFpgaEmulator -
   * Output:       ppBinary - the compiled binary container
   *               puiBinarySize - pBinary size in bytes
   *               pszCompileLog - compile log string
   * Return value: CL_SUCCESS - The operation succeeded
   * Author:       Sagi Shahar
   * Date:         January 2012
   ****************************************************************************/
  cl_err_code CompileProgram(const char *szProgramSource,
                             unsigned int uiNumInputHeaders,
                             const char **pszInputHeaders,
                             const char **pszInputHeadersNames,
                             const char *szOptions, bool bFpgaEmulator,
                             OUT char **ppBinary, OUT size_t *puiBinarySize,
                             OUT char **pszCompileLog) const;

  /*****************************************************************************
   * Function:     ParseSpirv
   * Description:  Convert SPIRV binary to LLVM.
   * Arguments:    szProgramBinary - the main program binary.
   *               szOptions - compile options string.
   * Output:       ppBinary - the compiled binary container.
   *               puiBinarySize - pBinary size in bytes.
   *               pszCompileLog - compile log string.
   * Return value: CL_SUCCESS - The operation succeeded.
   * Author: Vlad Romanov
   * Date:         January 2016
   ****************************************************************************/
  cl_err_code ParseSpirv(const char *szProgramBinary,
                         unsigned int uiProgramBinarySize,
                         const char *szOptions, size_t uiSpecConstCount,
                         const uint32_t *puiSpecConstIds,
                         const uint64_t *pszSpecConstValues,
                         OUT char **ppBinary, OUT size_t *puiBinarySize,
                         OUT char **pszCompileLog) const;

  /*****************************************************************************
   * Function:     MaterializeSPIR
   * Description:  Converts SPIR 1.2 binary to LLVM IR form compatible with
   *               OpenCL compiler.
   * Arguments:    szProgramBinary - SPIR 1.2 program binary.
   *               uiProgramBinarySize - SPIR 1.2 probram binary size
   * Output:       ppBinary - materialized binary container.
   *               puiBinarySize - pBinary size in bytes.
   *               pszCompileLog - compile log string.
   * Return value: CL_SUCCESS - The operation succeeded.
   * **************************************************************************/
  cl_err_code MaterializeSPIR(const char *szProgramBinary,
                              unsigned int uiProgramBinarySize,
                              OUT char **ppBinary, OUT size_t *puiBinarySize,
                              OUT char **pszCompileLog) const;

  /*****************************************************************************
   * Function:     LinkProgram
   * Description:  Compile source code and return binary data
   * Arguments:    ppBinaries - array of binary containers to be link together
   *               uiNumInputBinaries - num containers in ppBinaries
   *               puiBinariesSizes - sizes of the containers in ppBinaries
   *               szOptions - link options string
   * Output:       pBinary - the linked binary container
   *               uiBinarySize - pBinary size in bytes
   *               szCompileLog - link log string
   * Return value: CL_SUCCESS - The operation succeeded
   * Author:       Sagi Shahar
   * Date:         January 2012
   ****************************************************************************/
  cl_err_code LinkProgram(const void **ppBinaries,
                          unsigned int uiNumInputBinaries,
                          const size_t *puiBinariesSizes, const char *szOptions,
                          OUT char **ppBinary, OUT size_t *puiBinarySize,
                          OUT std::vector<char> &linkLog, OUT bool *pbIsLibrary,
                          OUT char **ppKernelNames) const;

  /*****************************************************************************
   * Function:     GetKernelArgInfo
   * Description:  Get the kernel arguments info
   * Arguments:    pBin - the program's binary including the header
   *               uiBinarySize - size of the binary
   *               szKernelName - the name of the kernel for which we query the
   *                              arg info
   * Output:       ppArgInfo - a struct containing all the arguments info
   * Return value: CL_SUCCESS - The operation succeeded
   *               CL_KERNEL_ARG_INFO_NOT_AVAILABLE if binary was built without
   *                                                -cl-kernel-arg-info
   *               CL_OUT_OF_HOST_MEMORY for out of host memory
   * Author:       Sagi Shahar
   * Date:         March 2012
   ****************************************************************************/
  cl_err_code GetKernelArgInfo(const void *pBin, size_t uiBinarySize,
                               const char *szKernelName,
                               ClangFE::IOCLFEKernelArgInfo **ppArgInfo) const;

  /*****************************************************************************
   * Function:     CheckCompileOptions
   * Description:  Check if the compile options are legal
   * Arguments:    szOptions - a string representing the compile options
   * Output:       szUnrecognizedOptions - a new string containing the
   *                                       unrecognized options separated by
   *                                       spaces
   * Return value: true - the compile options are legal false otherwise
   * Author:       Sagi Shahar
   * Date: May 2012
   ****************************************************************************/
  bool CheckCompileOptions(const char *szOptions, char *szUnrecognizedOptions,
                           size_t uiUnrecognizedOptionsSize) const;

  /*****************************************************************************
   * Function:     CheckLinkOptions
   * Description:  Check if the link options are legal
   * Arguments:    szOptions - a string representing the link options
   * Output:       szUnrecognizedOptions - a new string containing the
   *                                       unrecognized options separated by
   *                                       spaces
   * Return value: true - the link options are legal false otherwise
   * Author:       Sagi Shahar
   * Date: May 2012
   ****************************************************************************/
  bool CheckLinkOptions(const char *szOptions, char *szUnrecognizedOptions,
                        size_t uiUnrecongnizedOptionsSize) const;

  /*****************************************************************************
   * Function:    GetSpecConstInfo
   * Description: Get information about specialization constants in the program.
   * Arguments: szProgramBinary - the program binary in SPIR-V format
   *            uiProgramSize - size of the binary
   * Output:    SpecConstInfo - id of constants available for specialization and
   *                            their size in bytes
   * Author:       Alexey Sotkin
   * Date: January 2020
   ****************************************************************************/
  using SpecConstInfoTy = std::pair<uint32_t, uint32_t>;
  void GetSpecConstInfo(const char *szProgramBinary, size_t uiProgramSize,
                        std::vector<SpecConstInfoTy> &SpecConstInfo) const;

  // OclObject implementation
  cl_err_code GetInfo(cl_int /*iParamName*/, size_t /*szParamValueSize*/,
                      void * /*pParamValue*/,
                      size_t * /*pszParamValueSizeRet*/) const override {
    return CL_INVALID_OPERATION;
  }

protected:
  /*****************************************************************************
   * Function:     ~FECompiler
   * Description:  The Frontend compiler class destructor
   * Arguments:
   * Author:       Uri Levy
   * Date:         March 2008
   ****************************************************************************/
  virtual ~FrontEndCompiler();

  Intel::OpenCL::FECompilerAPI::IOCLFECompiler *m_pFECompiler;

  DECLARE_LOGGER_CLIENT;

private:
  FrontEndCompiler &operator=(const FrontEndCompiler &);
  cl_err_code ProcessResults(cl_err_code Error, IOCLFEBinaryResult *Result,
                             char **Binary, size_t *BinarySize,
                             char **CompileLog) const;
};
} // namespace Framework
} // namespace OpenCL
} // namespace Intel
