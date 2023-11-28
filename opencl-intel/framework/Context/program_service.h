// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
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

#include "build_event.h"
#include "cl_shared_ptr.h"
#include "cl_types.h"
#include "fe_compiler.h"
#include "program.h"
#include "task_executor.h"

namespace Intel {
namespace OpenCL {
namespace Framework {

typedef void(CL_CALLBACK *pfnNotifyBuildDone)(cl_program, void *);

class FissionableDevice;
class FrontEndCompiler;
class Context;

/*******************************************************************************
 * Class name:   ProgramService
 *
 * Description:  represents a program service
 ******************************************************************************/
class ProgramService {
public:
  /*****************************************************************************
   * Function:     ~ProgramService
   * Description:  The Program Service class destructor
   ****************************************************************************/
  ~ProgramService();

  /*****************************************************************************
   * Function:     SetContext
   * Description:  set the Context of this ProgramService
   * Arguments:    pContext a SharedPtr<Context> pointing to the Context to be
   *               set
   ****************************************************************************/
  void SetContext(Context *pContext) { m_pContext = pContext; }

  /*****************************************************************************
   * Function:     CompileProgram
   * Description:  Compile program from a set of source and headers
   * Arguments:    program - The program to be compiled
   *               num_devices - the number of devices in device_list
   *               device_list - a list of devices on which to compile the
   *                             program
   *               num_input_headers - the number of headers in input_headers
   *               input_headers - a list of programs to be used as input
   *               headers header_include_names - a list of headers names
   *                                              corresponding to input_headers
   *               options - compile options
   *               pfn_notify - the notify function from the user
   *               user_data - user data to be passed to pfn_notify
   * Return value: CL_SUCCESS - The operation succeeded
   ****************************************************************************/
  cl_err_code
  CompileProgram(const SharedPtr<Program> &program, cl_uint num_devices,
                 const cl_device_id *device_list, cl_uint num_input_headers,
                 SharedPtr<Program> *input_headers,
                 const char **header_include_names, const char *options,
                 pfnNotifyBuildDone pfn_notify, void *user_data);

  /*****************************************************************************
   * Function:     LinkProgram
   * Description:  Link program from a set of binaries
   * Arguments:    program - The output linked program
   *               num_devices - the number of devices in device_list
   *               device_list - a list of devices on which to compile the
   *               program num_input_programs - the number of binaries in
   *                                            input_programs
   *               input_programs - a list of programs to be linked
   *               options - link options
   *               pfn_notify - the notify function from the user
   *               user_data - user data to be passed to pfn_notify
   * Return value: CL_SUCCESS - The operation succeeded
   ****************************************************************************/
  cl_err_code LinkProgram(const SharedPtr<Program> &program,
                          cl_uint num_devices, const cl_device_id *device_list,
                          cl_uint num_input_programs,
                          SharedPtr<Program> *input_programs,
                          const char *options, pfnNotifyBuildDone pfn_notify,
                          void *user_data);

  /*****************************************************************************
   * Function:     BuildProgram
   * Description:  build (compile and link) program
   * Arguments:    program - The program to be built
   *               num_devices - the number of devices in device_list
   *               device_list - a list of devices on which to compile the
   *                             program
   *               options - build options
   *               pfn_notify - the notify function from the user
   *               user_data - user data to be passed to pfn_notify
   * Return value: CL_SUCCESS - The operation succeeded
   ****************************************************************************/
  cl_err_code BuildProgram(SharedPtr<Program> &program, cl_uint num_devices,
                           const cl_device_id *device_list, const char *options,
                           pfnNotifyBuildDone pfn_notify, void *user_data);

  /*****************************************************************************
   * Function:     SetSpecializationConstant
   * Description:  Set the value of a specialization constant
   * Arguments:    program[inout] - pointer to the program created from a
   *                                 SPIR-V module.
   *               spec_const_id[in] - id of the specialization constant whose
   *                                   value will be set.
   *               spec_const_Size[in] - size in bytes of the data pointed to by
   *                                     spec_value.
   *               spec[in] - pointer to the memory location that contains the
   *                          value of the specialization constant.
   * Return Value: CL_INVALID_PROGRAM if program is not a valid program object
   *                                  created from a module in an intermediate
   *                                  format (e.g. SPIR-V).
   *               CL_INVALID_SPEC_ID if spec_id is not a valid specialization
   *                                  constant ID
   *               CL_INVALID_VALUE   if spec_size does not match the size of
   *                                  the specialization constant in the module,
   *                                  or if spec_value is NULL.
   * **************************************************************************/
  cl_err_code SetSpecializationConstant(const SharedPtr<Program> &program,
                                        cl_uint spec_const_id,
                                        size_t spec_const_size,
                                        const void *spec);

protected:
  Context *m_pContext; // since ProgramService is aggregated by Context, this
                       // can be a regular pointer
};

class BuildTask : public Intel::OpenCL::Framework::BuildEvent,
                  public Intel::OpenCL::TaskExecutor::ITask {
public:
  PREPARE_SHARED_PTR(BuildTask)

  virtual long Release() override;

  virtual void DoneWithDependencies(const SharedPtr<OclEvent> &pEvent) override;

  bool Launch();

  virtual void SetComplete(cl_int returnCode) override;

  virtual void Cleanup(bool = false) override { delete this; }

  virtual Intel::OpenCL::TaskExecutor::TASK_PRIORITY
  GetPriority() const override {
    return Intel::OpenCL::TaskExecutor::TASK_PRIORITY_MEDIUM;
  }

  virtual Intel::OpenCL::TaskExecutor::IThreadLibTaskGroup *
  GetNDRangeChildrenTaskGroup() override {
    return NULL;
  }

  bool SetAsSyncPoint() override {
    assert(0 && "Should not be called");
    return false;
  }
  bool IsCompleted() const override {
    assert(0 && "Should not be called");
    return true;
  }
  bool CompleteAndCheckSyncPoint() override { return false; }

protected:
  BuildTask(_cl_context_int *context, const SharedPtr<Program> &pProg,
            const ConstSharedPtr<FrontEndCompiler> &pFECompiler);

  ~BuildTask();

  SharedPtr<Program> m_pProg;
  ConstSharedPtr<FrontEndCompiler> m_pFECompiler;
};

class CompileTask : public BuildTask {
public:
  PREPARE_SHARED_PTR(CompileTask)

  static SharedPtr<CompileTask>
  Allocate(_cl_context_int *context, const SharedPtr<Program> &pProg,
           const ConstSharedPtr<FrontEndCompiler> &pFECompiler,
           DeviceProgram *pDeviceProgram, unsigned int uiNumHeaders,
           const char **pszHeaders, const char **pszHeadersNames,
           const char *szOptions) {
    return new CompileTask(context, pProg, pFECompiler, pDeviceProgram,
                           uiNumHeaders, pszHeaders, pszHeadersNames,
                           szOptions);
  }

  virtual bool Execute() override;
  virtual void Cancel() override;

protected:
  CompileTask(_cl_context_int *context, const SharedPtr<Program> &pProg,
              const ConstSharedPtr<FrontEndCompiler> &pFECompiler,
              DeviceProgram *pDeviceProgram, unsigned int uiNumHeaders,
              const char **pszHeaders, const char **pszHeadersNames,
              const char *szOptions);

  ~CompileTask();

  DeviceProgram *m_pDeviceProgram;

  unsigned int m_uiNumHeaders;
  const char **m_pszHeaders;
  const char **m_pszHeadersNames;
  std::string m_sOptions;

  static std::mutex m_compileMtx;
};

class LinkTask : public BuildTask {
public:
  PREPARE_SHARED_PTR(LinkTask)

  static SharedPtr<LinkTask>
  Allocate(_cl_context_int *context, const SharedPtr<Program> &pProg,
           const ConstSharedPtr<FrontEndCompiler> &pFECompiler,
           DeviceProgram *pDeviceProgram, SharedPtr<Program> *ppBinaries,
           unsigned int uiNumBinaries, const char *szOptions) {
    return new LinkTask(context, pProg, pFECompiler, pDeviceProgram, ppBinaries,
                        uiNumBinaries, szOptions);
  }

  virtual bool Execute() override;
  virtual void Cancel() override;

protected:
  LinkTask(_cl_context_int *context, const SharedPtr<Program> &pProg,
           const ConstSharedPtr<FrontEndCompiler> &pFECompiler,
           DeviceProgram *pDeviceProgram, SharedPtr<Program> *ppBinaries,
           unsigned int uiNumBinaries, const char *szOptions);

  ~LinkTask();

  DeviceProgram *m_pDeviceProgram;

  SharedPtr<Program> *m_ppPrograms;
  unsigned int m_uiNumPrograms;
  std::string m_sOptions;

  static std::mutex m_linkMtx;
};

class DeviceBuildTask : public BuildTask {
public:
  PREPARE_SHARED_PTR(DeviceBuildTask)

  static SharedPtr<DeviceBuildTask> Allocate(_cl_context_int *context,
                                             const SharedPtr<Program> &pProg,
                                             DeviceProgram *pDeviceProgram,
                                             const char *szOptions) {
    return new DeviceBuildTask(context, pProg, pDeviceProgram, szOptions);
  }

  virtual bool Execute() override;
  virtual void Cancel() override;

protected:
  DeviceBuildTask(_cl_context_int *context, const SharedPtr<Program> &pProg,
                  DeviceProgram *pDeviceProgram, const char *szOptions);

  ~DeviceBuildTask();

  DeviceProgram *m_pDeviceProgram;
  std::string m_sOptions;

  static std::mutex m_deviceBuildMtx;
};

class PostBuildTask : public BuildTask {
public:
  PREPARE_SHARED_PTR(PostBuildTask)

  static SharedPtr<PostBuildTask>
  Allocate(_cl_context_int *context, const SharedPtr<Program> &pProg,
           cl_uint num_devices, DeviceProgram **ppDevicePrograms,
           unsigned int uiNumHeaders, SharedPtr<Program> *ppHeaders,
           char **pszHeadersNames, unsigned int uiNumBinaries,
           SharedPtr<Program> *ppBinaries, pfnNotifyBuildDone pfn_notify,
           void *user_data) {
    return new PostBuildTask(context, pProg, num_devices, ppDevicePrograms,
                             uiNumHeaders, ppHeaders, pszHeadersNames,
                             uiNumBinaries, ppBinaries, pfn_notify, user_data);
  }

  PostBuildTask(const PostBuildTask &) = delete;
  PostBuildTask &operator=(const PostBuildTask &) = delete;

  virtual bool Execute() override;
  virtual void Cancel() override;

protected:
  PostBuildTask(_cl_context_int *context, const SharedPtr<Program> &pProg,
                cl_uint num_devices, DeviceProgram **ppDevicePrograms,
                unsigned int uiNumHeaders, SharedPtr<Program> *ppHeaders,
                char **pszHeadersNames, unsigned int uiNumBinaries,
                SharedPtr<Program> *ppBinaries, pfnNotifyBuildDone pfn_notify,
                void *user_data);

  ~PostBuildTask();

  cl_uint m_num_devices;
  DeviceProgram **m_ppDevicePrograms;

  unsigned int m_uiNumHeaders;
  SharedPtr<Program> *m_ppHeaders;
  char **m_pszHeadersNames;

  unsigned int m_uiNumBinaries;
  SharedPtr<Program> *m_ppBinaries;

  pfnNotifyBuildDone m_pfn_notify;
  void *m_user_data;
};

class CreateAutorunKernelsTask : public BuildTask {
public:
  PREPARE_SHARED_PTR(CreateAutorunKernelsTask)

  static SharedPtr<CreateAutorunKernelsTask>
  Allocate(_cl_context_int *context, const SharedPtr<Program> &pProg) {
    return new CreateAutorunKernelsTask(context, pProg);
  }

  virtual bool Execute() override;
  virtual void Cancel() override;

protected:
  CreateAutorunKernelsTask(_cl_context_int *context,
                           const SharedPtr<Program> &pProg);

  ~CreateAutorunKernelsTask();
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
