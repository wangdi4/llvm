// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
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

#include "device_program.h"
#include "kernel.h"
#include <Logger.h>
#include <atomic>
#include <cl_object.h>
#include <cl_objects_map.h>
#include <cl_synch_objects.h>
#include <cl_types.h>
#include <map>
#include <memory>
#include <mutex>
#include <observer.h>
#include <vector>

namespace Intel {
namespace OpenCL {
namespace Framework {
class Context;

class Program : public OCLObject<_cl_program_int> {

public:
  Program(SharedPtr<Context> pContext);

  PREPARE_SHARED_PTR(Program)

  Program(SharedPtr<Context> pContext, ocl_entry_points *pOclEntryPoints);

  // return the context to which the program belongs
  const SharedPtr<Context> &GetContext() const { return m_pContext; }

  // create new kernel object
  virtual cl_err_code CreateKernel(const char *pscKernelName,
                                   SharedPtr<Kernel> *ppKernel);

  // create all kernels from the program object
  virtual cl_err_code CreateAllKernels(cl_uint uiNumKernels,
                                       cl_kernel *pclKernels,
                                       cl_uint *puiNumKernelsRet);

  // get the kernels associated to the program
  virtual cl_err_code GetKernels(cl_uint uiNumKernels,
                                 SharedPtr<Kernel> *ppKernels,
                                 cl_uint *puiNumKernelsRet);

  // remove kernel from program
  virtual cl_err_code RemoveKernel(cl_kernel clKernel);

  // Query details about the build
  cl_err_code GetBuildInfo(cl_device_id clDevice,
                           cl_program_build_info clParamName,
                           size_t szParamValueSize, void *pParamValue,
                           size_t *pszParamValueSizeRet);

  // Implement the common queries. Specific queries like binaries or source go
  // to implementing classes
  virtual cl_err_code GetInfo(cl_int param_name, size_t param_value_size,
                              void *param_value,
                              size_t *param_value_size_ret) const override;

  // Returns a read only pointer to internal binary, used for
  // build stages by program service
  const char *GetBinaryInternal(cl_device_id clDevice);

  // Returns internal binary size, used for build stages by
  // program service
  size_t GetBinarySizeInternal(cl_device_id clDevice);

  // Returns internal binary type, used for build stages by
  // program service
  cl_program_binary_type GetBinaryTypeInternal(cl_device_id clDevice);

  // Sets the binary type for a specific device, used for build
  // stages by program service
  cl_err_code SetBinaryTypeInternal(cl_device_id clDevice,
                                    cl_program_binary_type clBinaryType);

  // Sets the binary for a specific device, used for build stages
  // by program service
  cl_err_code SetBinaryInternal(cl_device_id clDevice, size_t uiBinarySize,
                                const void *pBinary,
                                cl_program_binary_type clBinaryType);

  // Clears the current build log, called in the beginning of each
  // build sequence
  cl_err_code ClearBuildLogInternal(cl_device_id clDevice);

  // Set the program build log for a specific device, used for build stages by
  // program service
  cl_err_code SetBuildLogInternal(cl_device_id clDevice,
                                  const char *szBuildLog);

  // Returns a read only pointer to internal source, used for build stages by
  // program service
  virtual const char *GetSourceInternal() { return nullptr; };

  // set the program's build options
  // Creates a copy of the input
  // Returns CL_SUCCESS if nothing unexpected happened
  cl_err_code SetBuildOptionsInternal(cl_device_id clDevice,
                                      const char *szBuildOptions);

  // get the latest program's build options for a specific device
  const char *GetBuildOptionsInternal(cl_device_id clDevice);

  // Set the program state
  cl_err_code SetStateInternal(cl_device_id clDevice,
                               EDeviceProgramState state);

  // Get the program state
  EDeviceProgramState GetStateInternal(cl_device_id clDevice);

  // Sets device handle, used for build stages by program service
  cl_err_code SetDeviceHandleInternal(cl_device_id clDevice,
                                      cl_dev_program programHandle);

  // Get the number of associated devices
  cl_uint GetNumDevices();

  // Fill in ppDeviceID with associated devices, assuming ppDeviceID has at
  // least the number of cells returned from GetNumDevices
  cl_err_code GetDevices(cl_device_id *pDeviceID);

  // Get the number of kernels
  cl_uint GetNumKernels();

  // Returns true if the object can be safely worked on for the specified device
  // and false otherwise
  bool Acquire(cl_device_id clDevice);

  // Notifies that we're done working with this object
  void Unacquire(cl_device_id clDevice);

  typedef std::map<cl_device_id, DeviceProgram *> tDeviceProgramMap;

  // Create map from cl_device_id that exist in the context and have or its
  // parent (recursively) have built program (DeviceProgram*), to its related
  // DeviceProgram*
  void SetContextDevicesToProgramMappingInternal();

  // Find the DeviceProgram related to devID and set in pOutID the ID of this
  // DeviceProgram device. If not found return false, otherwise return true
  bool GetMyRelatedProgramDeviceIDInternal(const cl_device_id devID,
                                           cl_int *pOutID);

  // Retrive device program for specific device
  DeviceProgram *GetDeviceProgram(cl_device_id clDeviceId);

  // Retrive an array of all device programs
  std::vector<std::unique_ptr<DeviceProgram>> &GetProgramsForAllDevices() {
    return m_ppDevicePrograms;
  }

  virtual cl_err_code
  GetAutorunKernels(std::vector<SharedPtr<Kernel>> &kernels);

  virtual cl_err_code CreateAutorunKernels(cl_uint uiNumKernels,
                                           cl_kernel *pclKernels,
                                           cl_uint *puiNumKernelsRet);

  bool TestAndSetAutorunKernelsLaunched() {
    return m_afAutorunKernelsLaunched.test_and_set();
  }

  cl_err_code GetDeviceFunctionPointer(cl_device_id device,
                                       const char *func_name,
                                       cl_ulong *func_pointer_ret);

  // Retrieve size and pointer of a global variable given by name.
  cl_err_code GetDeviceGlobalVariablePointer(cl_device_id device,
                                             const char *gv_name,
                                             size_t *gv_size_ret,
                                             void **gv_pointer_ret);

  // Free USM wrappers for global variable pointers
  void FreeUSMForGVPointers();

  /// Clear m_isFinalized in the case that program is built again.
  void ClearFinalizedFlag();

  std::vector<std::string> &getKernelsWithArgsInfo() {
    return m_KernelsWithArgsInfo;
  }

  bool getIsSpir() { return BinaryIsSpir; }

  void setIsSpir() { BinaryIsSpir = true; }

protected:
  virtual ~Program();

  DeviceProgram *InternalGetDeviceProgram(cl_device_id clDeviceId);

  // Allocate USM wrappers for global variable pointers
  cl_err_code AllocUSMForGVPointers();

  /// Finalize program, e.g. run global ctor, load program dll for windows
  /// native debugger, and retrieve global variable address.
  bool Finalize();

  SharedPtr<Context> m_pContext;
  std::vector<std::unique_ptr<DeviceProgram>> m_ppDevicePrograms;
  cl_uint m_szNumAssociatedDevices;

  OCLObjectsMap<_cl_kernel_int> m_pKernels; // associated
  // assiciated autorun kernels
  OCLObjectsMap<_cl_kernel_int> m_pAutorunKernels;

  tDeviceProgramMap m_deviceToProgram;

  std::atomic_flag m_afAutorunKernelsLaunched;

  std::vector<std::string> m_KernelsWithArgsInfo;

private:
  // Mutex for m_deviceToProgram
  mutable std::mutex m_deviceProgramMapMutex;

  volatile bool m_isFinalized;
  std::mutex m_finalizeMutex;

  bool BinaryIsSpir = false;
};
} // namespace Framework
} // namespace OpenCL
} // namespace Intel
