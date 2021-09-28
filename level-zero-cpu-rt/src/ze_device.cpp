// INTEL_COPYRIGHT_BEGIN
// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END

#include "ze_device.hpp"
#include "cm_printf_host.h"
#include "ze_cmdlist.hpp"
#include "ze_cmdqueue.hpp"
#include "ze_kernel.hpp"
#include "ze_module.hpp"
#include "ze_utils.hpp"
#include <cstring>

namespace __zert__ {

//-----------------------------------------------------------------------------

ZeDevice::ZeDevice(ZeDriver *driver, Kind kind, int32_t tile_count)
    : driver_(driver), kind_(kind), tile_count_(tile_count) {
  properties_.type = ZE_DEVICE_TYPE_CPU;
  properties_.vendorId = 0xDEADBEEFU;
  properties_.deviceId = 0xFEEDEAEFU;
  properties_.uuid = ze_device_uuid_t{};
  properties_.subdeviceId = 0;
  properties_.coreClockRate = 1000000;
  properties_.maxCommandQueuePriority = 1;
  properties_.numThreadsPerEU = 128;
  properties_.physicalEUSimdWidth = 16;
  properties_.numEUsPerSubslice = 8;
  properties_.numSubslicesPerSlice = 1;
  properties_.numSlices = 64;
  my_strcpy_s(properties_.name, sizeof(properties_.name), "unk");
}

ZeDevice::~ZeDevice() = default;

ze_result_t ZeDevice::getPlatform(cpu_device_kind_t &device) {
  auto device_str = "AVX512"; // TODO: select target arch basing on CPU detetct
#if 0
    if (device_str.empty())
    {
        ZESIMERR << "CPU device arch is not set";
        return ZE_RESULT_ERROR_UNKNOWN;
    }
#endif
  std::map<std::string, cpu_device_kind_t> device_map = {
      {"AVX512", CPU_DEVICE_ARCH_AVX512},
      {"AVX2", CPU_DEVICE_ARCH_AVX2},
      {"AVX", CPU_DEVICE_ARCH_AVX},
      {"SSE42", CPU_DEVICE_ARCH_SSE42}};

  auto it = device_map.find(device_str);
  if (device_map.end() == it) {
    ZESIMERR << "No CPU device arch =" << device_str;
    return ZE_RESULT_ERROR_UNKNOWN;
  } else {
    device = it->second;
    return ZE_RESULT_SUCCESS;
  }
}

ze_result_t ZeDevice::initialize() {
  if (tile_count_ > 1) {
    subdevices_.push_back(this);
  }
  for (int32_t i = 1; i < tile_count_; ++i) {
    auto subdev = this->newSubdevice();
    auto res = subdev->initialize();
    if (res != ZE_RESULT_SUCCESS) {
      ZESIMERR << "Failed to initialize subdevice " << i;
      return ZE_RESULT_ERROR_UNKNOWN;
    }
    subdevices_.push_back(subdev.get());
    subdevices_excluding_this_device_.push_back(std::move(subdev));
  }
  return ZE_RESULT_SUCCESS;
}

ze_result_t ZeDevice::createModule(ZeContext *context,
                                   ze_module_desc_t const *desc,
                                   ze_module_handle_t *module) {
  auto zeModule = std::make_unique<ZeModule>(context, this);

  ze_result_t ret = zeModule->initialize(desc);
  if (ZE_RESULT_SUCCESS != ret) {
    ZESIMERR << "Failed to initalize module";
    return ret;
  }

  *module = zeModule.get();
  this->modules_.push_back(std::move(zeModule));

  return ZE_RESULT_SUCCESS;
}

ze_result_t ZeDevice ::destroyModule(ze_module_handle_t module) {
  for (auto it = modules_.begin(); it != modules_.end(); ++it) {
    if (it->get() == module) {
      modules_.erase(it);
      return ZE_RESULT_SUCCESS;
    }
  }
  ZESIMERR << "Unknown module, cannot destroy";
  return ZE_RESULT_ERROR_INVALID_ARGUMENT;
}

ze_result_t ZeDevice::createCmdQueue(ZeContext *context,
                                     ze_command_queue_desc_t const *desc,
                                     ze_command_queue_handle_t *handle) {
  auto queue = std::make_unique<ZeCmdQueue>(context, this, *desc);

  *handle = queue.get();
  this->cmd_queues_.push_back(std::move(queue));

  return ZE_RESULT_SUCCESS;
}

ze_result_t ZeDevice::destroyCmdQueue(ze_command_queue_handle_t handle) {
  for (auto it = cmd_queues_.begin(); it != cmd_queues_.end(); ++it) {
    if (it->get() == handle) {
      cmd_queues_.erase(it);
      return ZE_RESULT_SUCCESS;
    }
  }
  ZESIMERR << "Unknown cmdqueue, cannot destroy";
  return ZE_RESULT_ERROR_INVALID_ARGUMENT;
}

ze_result_t ZeDevice::createCmdList(ZeContext *context,
                                    ze_command_list_desc_t const *desc,
                                    ze_command_list_handle_t *handle) {
  auto list = std::make_unique<ZeCmdList>(context, this, *desc);

  *handle = list.get();
  this->cmd_lists_.push_back(std::move(list));

  return ZE_RESULT_SUCCESS;
}

ze_result_t ZeDevice::createCmdList(ZeContext *context,
                                    ze_command_queue_desc_t const *desc,
                                    ze_command_list_handle_t *handle) {
  auto list = std::make_unique<ZeCmdList>(context, this, *desc);

  *handle = list.get();
  this->cmd_lists_.push_back(std::move(list));

  return ZE_RESULT_SUCCESS;
}

ze_result_t ZeDevice::destroyCmdList(ze_command_list_handle_t handle) {
  uint32_t pending_events =
      static_cast<ZeCmdList *>(handle)->get_pending_events();
  if (pending_events != 0) {
    ZESIMERR << "destroyCmdList: Not all submitted commands are completed, "
                "count= "
             << pending_events;
    return ZE_RESULT_ERROR_UNKNOWN;
  }
  for (auto it = cmd_lists_.begin(); it != cmd_lists_.end(); ++it) {
    if (it->get() == handle) {
      cmd_lists_.erase(it);
      return ZE_RESULT_SUCCESS;
    }
  }
  ZESIMERR << "Unknown cmdlist, cannot destroy";
  return ZE_RESULT_ERROR_INVALID_ARGUMENT;
}
ze_result_t ZeDevice::getComputeProperties(
    ze_device_compute_properties_t *pComputeProperties) {
  pComputeProperties->maxTotalGroupSize =
      128; // static_cast<uint32_t>(deviceInfo.maxWorkGroupSize);

  pComputeProperties->maxGroupSizeX =
      128; // static_cast<uint32_t>(deviceInfo.maxWorkItemSizes[0]);
  pComputeProperties->maxGroupSizeY =
      128; // static_cast<uint32_t>(deviceInfo.maxWorkItemSizes[1]);
  pComputeProperties->maxGroupSizeZ =
      128; // static_cast<uint32_t>(deviceInfo.maxWorkItemSizes[2]);

  pComputeProperties->maxGroupCountX =
      128; // std::numeric_limits<uint32_t>::max();
  pComputeProperties->maxGroupCountY =
      128; // std::numeric_limits<uint32_t>::max();
  pComputeProperties->maxGroupCountZ =
      128; // std::numeric_limits<uint32_t>::max();

  pComputeProperties->maxSharedLocalMemory =
      128; // static_cast<uint32_t>(deviceInfo.localMemSize);

  pComputeProperties->numSubGroupSizes =
      0; // static_cast<uint32_t>(deviceInfo.maxSubGroups.size());

  for (uint32_t i = 0; i < pComputeProperties->numSubGroupSizes; ++i) {
    pComputeProperties->subGroupSizes[i] =
        1; // static_cast<uint32_t>(deviceInfo.maxSubGroups[i]);
  }

  return ZE_RESULT_SUCCESS;
}
//-----------------------------------------------------------------------------

CPUdevice::CPUdevice(ZeDriver *driver, int32_t tile_count)
    : Base(driver, this_kind(), tile_count) {
  my_strcpy_s(properties_.name, sizeof(properties_.name), "CPU_DEVICE");
}

std::unique_ptr<ZeDevice> CPUdevice::newSubdevice() {
  return std::unique_ptr<ZeDevice>(new CPUdevice(this->driver_,
                                                 /*tile_count*/ 1));
}

CPUdevice::~CPUdevice() = default;

ze_result_t CPUdevice::initialize() {
  ze_result_t result = ZE_RESULT_SUCCESS;

  result = ZeDevice::initialize();
  if (ZE_RESULT_SUCCESS == result) {
    auto maxKernels =
        util::get_envvar("L0SIM_DEVICE_MAX_PARALLEL_KERNELS_LAUNCH");
    if (!maxKernels.empty()) {
      maxParallelKernelsLaunchPerDevice_ = std::atoi(maxKernels.c_str());
      fprintf(stderr,
              "genisa: max number of lauching kernels in parallel per "
              "device is %d\n",
              maxParallelKernelsLaunchPerDevice_);
    } else {
      fprintf(stderr,
              "genisa:`L0SIM_DEVICE_MAX_PARALLEL_KERNELS_LAUNCH` not set, "
              "max number of lauching kernels in parallel per device by "
              "default is %d\n",
              maxParallelKernelsLaunchPerDevice_);
    }

    semaphore_ = std::unique_ptr<Semaphore>(
        new Semaphore(maxParallelKernelsLaunchPerDevice_));
  }

  return result;
}

} // namespace __zert__
