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

/*
 *
 * File cpu_device.h
 * declares C++ interface between the device and the Open CL frame work.
 *
 */
#pragma once

#include "backend_wrapper.h"
#include "cl_device_api.h"
#include "cl_dynamic_lib.h"
#include "cpu_config.h"
#include "cpu_dev_limits.h"
#include "cpu_logger.h"
#include "task_dispatcher.h"
#include "task_executor.h"
#ifdef __USE_TBB_SCALABLE_ALLOCATOR__
#include "tbb/scalable_allocator.h"
#endif
#include <atomic>
#include <cl_synch_objects.h>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace Intel {
namespace OpenCL {
namespace CPUDevice {

extern const char *CPU_STRING;
extern const char *VENDOR_STRING;

class ProgramService;
class MemoryAllocator;

class CPUDevice : public IOCLDeviceAgent,
                  public IOCLDeviceFECompilerDescription,
                  public IAffinityChangeObserver {
protected:
  static CPUDeviceConfig m_CPUDeviceConfig;
  static CPUDevice *CPUDeviceInstance;
  std::atomic_int m_refCount;

  ProgramService *m_pProgramService;
  MemoryAllocator *m_pMemoryAllocator;
  TaskDispatcher *m_pTaskDispatcher;
  IOCLFrameworkCallbacks *m_pFrameworkCallBacks;
  cl_uint m_uiCpuId;
  IOCLDevLogDescriptor *m_pLogDescriptor;
  cl_int m_iLogHandle;
  cl_dev_cmd_list m_defaultCommandList;
  OpenCLBackendWrapper m_backendWrapper;

  // ID of (logical) CPU on which master thread is running
  unsigned int m_uiMasterHWId;
  // Whether master thread will be pinned. Currently we only pin master thread
  // if DPCPP_CPU_CU_AFFINITY env is correctly set.
  bool m_pinMaster;

  bool m_disableMasterJoin; // Check whether master join is disabled or not
  unsigned long m_numCores; // Architectural data on the underlying HW
  unsigned int
      *m_pComputeUnitMap; // A mapping between an OpenCL-defined core ID (1 is
                          // first CPU on second socket) and OS-defined core ID
  // Maps OS thread ID to core ID.
  std::unordered_map<threadid_t, int> m_threadToCore;
  std::vector<threadid_t>
      m_pCoreToThread; // Maps OpenCL core ID to OS thread id which is pinned to
                       // the core, which can then be used access the scoreboard
                       // above
  std::vector<bool>
      m_pCoreInUse; // Keeps track over used compute units to prevent overlap

#ifdef __HARD_TRAPPING__
  bool m_bUseTrapping; // Use worker thread trapping when device fission with
                       // core affinity is used
#endif
  std::mutex m_ComputeUnitScoreboardMutex;

#ifdef __USE_TBB_SCALABLE_ALLOCATOR__
  tbb::scalable_allocator<DeviceNDRange> m_deviceNDRangeAllocator;
  tbb::scalable_allocator<char> m_deviceNDRangeContextAllocator;
#endif

  static volatile bool m_bDeviceIsRunning;

  virtual ~CPUDevice();

  // Called once on init to cache information about the underlying architecture
  cl_dev_err_code QueryHWInfo();

  // Calculate m_pComputeUnitMap based on DPCPP_CPU_CU_AFFINITY env
  void calculateComputeUnitMap();

  // The functions below are called when a set of cores is about to be "trapped"
  // into a sub-device, or released from such The acquire fails if one of the
  // compute units requested is a part of another sub-device. These are
  // currently called on create/release command list (= command queue)
  bool AcquireComputeUnits(unsigned int *which, unsigned int how_many);
  void ReleaseComputeUnits(unsigned int *which, unsigned int how_many);

  /** Affinity observer interface
   * @param tid - Thread id
   * @param core_index - Thread position (index) in tbb arena.
   * @param relocate - Whether the previous thread pinned at core_index should
   *                   be relocated to previous core of current thread.
   * @param need_mutex - Whether mutex is needed.
   */
  void NotifyAffinity(threadid_t tid, unsigned int core_index, bool relocate,
                      bool need_mutex) override;

  // Check if master thread will be pinned.
  bool IsPinMasterAllowed() const override { return m_pinMaster; };

  // Translate an "absolute" core (CPU core) to a core index
  // Needed to allow the user to limit the cores the CPU device will run on
  bool CoreToCoreIndex(unsigned int *core);

  CPUDevice(cl_uint devId, IOCLFrameworkCallbacks *devCallbacks,
            IOCLDevLogDescriptor *logDesc);

public:
  // NOTE: this function is not thread-safe by itself and caller is
  // responsible for performing synchronization
  static CPUDevice *clDevGetInstance(cl_uint uiDevId,
                                     IOCLFrameworkCallbacks *devCallbacks,
                                     IOCLDevLogDescriptor *logDesc,
                                     cl_dev_err_code &rc);

  cl_dev_err_code Init();

  static void WaitUntilShutdown();

  static cl_dev_err_code clDevGetDeviceInfo(unsigned int IN dev_id,
                                            cl_device_info IN param,
                                            size_t IN val_size,
                                            void *OUT paramVal,
                                            size_t *OUT param_val_size_ret);

  static cl_ulong clDevGetDeviceTimer();

  static cl_dev_err_code
  clDevGetAvailableDeviceList(size_t IN deviceListSize,
                              unsigned int *OUT deviceIdsList,
                              size_t *OUT deviceIdsListSizeRet);

  // Device Fission support

  cl_dev_err_code clDevPartition(
      cl_dev_partition_prop IN props, cl_uint IN num_requested_subdevices,
      cl_dev_subdevice_id IN parent_device_id, cl_uint *INOUT num_subdevices,
      void *IN param, cl_dev_subdevice_id *OUT subdevice_ids) override;
  cl_dev_err_code
  clDevReleaseSubdevice(cl_dev_subdevice_id IN subdevice_id) override;

  // Device entry points
  cl_dev_err_code clDevSetDefaultCommandList(cl_dev_cmd_list IN list) override;
  cl_dev_err_code clDevCreateCommandList(cl_dev_cmd_list_props IN props,
                                         cl_dev_subdevice_id IN subdevice_id,
                                         cl_dev_cmd_list *OUT list) override;
  void *clDevGetCommandListPtr(cl_dev_cmd_list IN list) override;
  cl_dev_err_code clDevFlushCommandList(cl_dev_cmd_list IN list) override;
  cl_dev_err_code clDevReleaseCommandList(cl_dev_cmd_list IN list) override;
  cl_dev_err_code clDevCommandListExecute(cl_dev_cmd_list IN list,
                                          cl_dev_cmd_desc *IN *cmds,
                                          cl_uint IN count) override;
  cl_dev_err_code
  clDevCommandListWaitCompletion(cl_dev_cmd_list IN list,
                                 cl_dev_cmd_desc *IN cmdToWait) override;
  cl_dev_err_code clDevCommandListCancel(cl_dev_cmd_list IN list) override;
  cl_dev_err_code clDevGetSupportedImageFormats(
      cl_mem_flags IN flags, cl_mem_object_type IN imageType,
      cl_uint IN numEntries, cl_image_format *OUT formats,
      cl_uint *OUT numEntriesRet) const override;
  cl_dev_err_code
  clDevGetMemoryAllocProperties(cl_mem_object_type IN memObjType,
                                cl_dev_alloc_prop *OUT pAllocProp) override;
  cl_dev_err_code
  clDevCreateMemoryObject(cl_dev_subdevice_id IN node_id, cl_mem_flags IN flags,
                          const cl_image_format *IN format, size_t IN dim_count,
                          const size_t *IN dim_size,
                          IOCLDevRTMemObjectService *IN pRTMemObjService,
                          IOCLDevMemoryObject *OUT *memObj) override;
  cl_dev_err_code clDevCheckProgramBinary(size_t IN binSize,
                                          const void *IN bin) override;
  cl_dev_err_code clDevCreateProgram(size_t IN binSize, const void *IN bin,
                                     cl_dev_binary_prop IN prop,
                                     cl_dev_program *OUT prog) override;
  cl_dev_err_code
  clDevCreateBuiltInKernelProgram(const char *IN szBuiltInNames,
                                  cl_dev_program *OUT prog) override;
  cl_dev_err_code
  clDevCreateLibraryKernelProgram(cl_dev_program *OUT Prog,
                                  const char **OUT KernelNames) override;

  cl_dev_err_code clDevBuildProgram(cl_dev_program IN prog,
                                    const char *IN options,
                                    cl_build_status *OUT buildStatus) override;
  cl_dev_err_code clDevFinalizeProgram(cl_dev_program IN prog) override;
  cl_dev_err_code clDevReleaseProgram(cl_dev_program IN prog) override;
  cl_dev_err_code clDevUnloadCompiler() override;
  cl_dev_err_code clDevGetProgramBinary(cl_dev_program IN prog, size_t IN size,
                                        void *OUT binary,
                                        size_t *OUT sizeRet) override;
  cl_dev_err_code clDevGetBuildLog(cl_dev_program IN prog, size_t IN size,
                                   char *OUT log,
                                   size_t *OUT size_ret) override;
  cl_dev_err_code clDevGetSupportedBinaries(size_t IN count,
                                            cl_prog_binary_desc *OUT types,
                                            size_t *OUT sizeRet) override;
  cl_dev_err_code clDevGetKernelId(cl_dev_program IN prog, const char *IN name,
                                   cl_dev_kernel *OUT kernelId) override;
  cl_dev_err_code clDevGetProgramKernels(cl_dev_program IN prog,
                                         cl_uint IN numKernels,
                                         cl_dev_kernel *OUT kernels,
                                         cl_uint *OUT numKernelsRet) override;
  cl_dev_err_code clDevGetGlobalVariableTotalSize(cl_dev_program IN prog,
                                                  size_t *OUT size) override;
  cl_dev_err_code clDevGetKernelInfo(cl_dev_kernel IN kernel,
                                     cl_dev_kernel_info IN param,
                                     size_t IN input_value_size,
                                     const void *IN input_value,
                                     size_t IN value_size, void *OUT value,
                                     size_t *OUT value_size_ret) override;
  cl_ulong clDevGetPerformanceCounter() override;
  cl_dev_err_code clDevSetLogger(IOCLDevLogDescriptor *) override;

  cl_dev_err_code
  clDevGetFunctionPointerFor(cl_dev_program IN prog, const char *IN func_name,
                             cl_ulong *OUT func_pointer_ret) const override;

  // Retrieves sizes/pointers of all global variables in a built program
  void clDevGetGlobalVariablePointers(cl_dev_program IN prog,
                                      const cl_prog_gv OUT **gvPtrs,
                                      size_t OUT *gvCount) const override;

  // Retrieves mapping between OpenCL-defined core ID and OS-defined core ID
  // count is the number of elements in the map array.
  void clDevGetComputeUnitMap(const unsigned OUT **computeUnitMap,
                              size_t OUT *count) const override;

  // NOTE: this function is not thread-safe by itself and caller is
  // responsible for performing synchronization
  void clDevCloseDevice(void) override;
  cl_dev_err_code
  clDevReleaseCommand(cl_dev_cmd_desc *IN cmdToRelease) override;

  const IOCLDeviceFECompilerDescription *
  clDevGetFECompilerDecription() const override {
    return this;
  };
  IOCLDevRawMemoryAllocator *clDevGetRawMemoryAllocator() override {
    return nullptr;
  };

  // IOCLDeviceFECompilerDescription
  const char *clDevFEModuleName() const override;
  const void *clDevFEDeviceInfo() const override;
  size_t clDevFEDeviceInfoSize() const override;

  int EnqueueMarker(queue_t queue, cl_uint uiNumEventsInWaitList,
                    const clk_event_t *pEventWaitList, clk_event_t *pEventRet);

  int RetainEvent(clk_event_t event);

  int ReleaseEvent(clk_event_t event);

  clk_event_t CreateUserEvent(int *piErrcodeRet);

  int SetEventStatus(clk_event_t event, int iStatus);

  void CaptureEventProfilingInfo(clk_event_t event, clk_profiling_info name,
                                 volatile void *pValue);

  queue_t GetDefaultQueueForDevice() const;

  unsigned int GetNumComputeUnits() const;

  void Acquire() { ++m_refCount; }

private:
  CPUDevice() = delete;
  CPUDevice(const CPUDevice &) = delete;
  CPUDevice(const CPUDevice &&) = delete;
};

} // namespace CPUDevice
} // namespace OpenCL
} // namespace Intel
