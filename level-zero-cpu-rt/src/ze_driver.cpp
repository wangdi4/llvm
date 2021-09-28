// INTEL_COPYRIGHT_BEGIN
// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END
//
#include "ze_driver.hpp"
#include "ze_basic_config.hpp"
#include "ze_buffer.hpp"
#include "ze_context.hpp"
#include "ze_device.hpp"
#include "ze_event.hpp"
#include "ze_utils.hpp"
#include <algorithm>
#include <map>
#include <set>
#include <sstream>

namespace __zert__ {

//-----------------------------------------------------------------------------
ZeDriver::ZeDriver() {
  std::map<std::string, std::unique_ptr<ZeDevice>> devices_map;
  devices_map.emplace("CPU_DEVICE",
                      std::unique_ptr<ZeDevice>(new CPUdevice(this, 1)));
  // comma separate list of devices
  std::string devices_str = util::get_envvar("L0_SELECT_DEVICES");
  if (devices_str.empty()) {
    devices_str = "CPU_DEVICE"; // no spaces (!)
  }

  // parse device string into separate devices
  std::set<std::string> device_str_set;
  std::stringstream ss(devices_str);
  std::vector<std::unique_ptr<ZeDevice>> devices_list;
  while (ss.good()) {
    std::string device_str;
    getline(ss, device_str, ',');
    if (device_str_set.count(device_str) == 0) {
      device_str_set.insert(device_str);
      devices_list.push_back(std::move(devices_map.at(device_str)));
    }
  }
  for (auto &d : devices_list) {
    if (ZE_RESULT_SUCCESS == d->initialize()) {
      devices_.push_back(d.get());
      devices_owned_.push_back(std::move(d));
    };
  }
}

ZeDriver &ZeDriver::getInstance() {
  static ZeDriver instance;
  if (!ZeDriver::initialized) {
    ZESIMERR << "ZeDriver not initialized";
  }
  return instance;
}

bool ZeDriver::initialized = false;
void ZeDriver::init() {
  // don't actually do anything
  // the instance would be generated in getInstance()
  ZeDriver::initialized = true;
}

ZeDriver::~ZeDriver() = default;

//-----------------------------------------------------------------------------

ZeBuffer *ZeDriver::makeZeBuffer(ZeDevice *device) {
  std::unique_lock<std::mutex> lk(mutex_);
  buffers_.push_back(std::make_unique<ZeBuffer>(this, device, buffer_id_++));
  return buffers_.back().get();
}

//-----------------------------------------------------------------------------

ZeBuffer *ZeDriver::getZeBuffer(void const *ptr) const {
  for (auto const &b : buffers_) {
    if (b->ptr_beg() <= ptr && ptr < b->ptr_end()) {
      return b.get();
    }
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
ze_result_t ZeDriver::getProperties(ze_driver_properties_t *properties) {
  uint32_t versionMajor =
      static_cast<uint32_t>(strtoul(L0_PROJECT_VERSION_MAJOR, NULL, 10));
  uint32_t versionMinor =
      static_cast<uint32_t>(strtoul(L0_PROJECT_VERSION_MINOR, NULL, 10));
  uint32_t versionBuild =
      static_cast<uint32_t>(strtoul(NEO_VERSION_BUILD, NULL, 10));
  uint64_t uuidTimestamp = 0u; // cdai2: Get compilation time:

  properties->driverVersion = ((versionMajor << 24) & 0xFF000000) |
                              ((versionMinor << 16) & 0x00FF0000) |
                              (versionBuild & 0x0000FFFF);

  uint64_t uniqueId =
      (properties->driverVersion) | (uuidTimestamp & 0xFFFFFFFF00000000);
  util::memcpy_s(properties->uuid.id, sizeof(uniqueId), &uniqueId,
                 sizeof(uniqueId));

  return ZE_RESULT_SUCCESS;
}
//-----------------------------------------------------------------------------
ze_result_t ZeDriver::getApiVersion(ze_api_version_t *version) {
  *version = ZE_API_VERSION_1_0;
  return ZE_RESULT_SUCCESS;
}
//-----------------------------------------------------------------------------
ze_result_t ZeDriver::getExtensionProperties(
    uint32_t *pCount, ze_driver_extension_properties_t *pExtensionProperties) {

  if (nullptr == pExtensionProperties) {
    *pCount = static_cast<uint32_t>(this->extensionsSupported.size());
    return ZE_RESULT_SUCCESS;
  }

  *pCount = std::min(static_cast<uint32_t>(this->extensionsSupported.size()),
                     *pCount);

  for (uint32_t i = 0; i < *pCount; i++) {
    auto extension = this->extensionsSupported[i];
    util::strncpy_s(pExtensionProperties[i].name, ZE_MAX_EXTENSION_NAME,
                    extension.first.c_str(), extension.first.length() + 1);
    pExtensionProperties[i].version = extension.second;
  }

  return ZE_RESULT_SUCCESS;
}

//-----------------------------------------------------------------------------
ze_result_t ZeDriver::destroyZeBuffer(ZeBuffer *buf) {
  for (auto it = buffers_.begin(); it != buffers_.end(); ++it) {
    if (it->get() == buf) {
      buffers_.erase(it);
      return ZE_RESULT_SUCCESS;
    }
  }
  ZESIMERR << "Unknown zebuffer, cannot destroy";
  return ZE_RESULT_ERROR_INVALID_ARGUMENT;
}

//-----------------------------------------------------------------------------

ze_result_t ZeDriver::createEventPool(ze_event_pool_desc_t const *desc,
                                      std::vector<ZeDevice *> devices,
                                      ze_event_pool_handle_t *phEventPool) {
  ze_result_t result;
  auto pool = ZeEventPool::create(*desc, this, devices, result);
  *phEventPool = nullptr;
  if (!pool) {
    ZESIMERR << "Failed to create ZeEventPool";
    return result;
  }
  *phEventPool = pool.get();
  event_pool_.push_back(std::move(pool));
  assert(ZE_RESULT_SUCCESS == result);
  return result;
}

//-----------------------------------------------------------------------------

ze_result_t ZeDriver::destroyEventPool(ZeEventPool *pool) {
  for (auto it = event_pool_.begin(); it != event_pool_.end(); ++it) {
    if (it->get() == pool) {
      event_pool_.erase(it);
      return ZE_RESULT_SUCCESS;
    }
  }
  ZESIMERR << "Event pool not found in driver..";
  return ZE_RESULT_ERROR_UNKNOWN;
}

//-----------------------------------------------------------------------------

ze_result_t ZeDriver::createContext(ze_context_desc_t const &desc,
                                    ze_context_handle_t *handle) {
  auto context = std::make_unique<ZeContext>(this, desc);
  *handle = context.get();
  contexts_.push_back(std::move(context));
  return ZE_RESULT_SUCCESS;
}

//-----------------------------------------------------------------------------

ze_result_t ZeDriver::destroyContext(ZeContext *context) {
  for (auto it = contexts_.begin(); it != contexts_.end(); ++it) {
    if (it->get() == context) {
      contexts_.erase(it);
      return ZE_RESULT_SUCCESS;
    }
  }
  ZESIMERR << "Unknown zecontext, cannot destroy";
  return ZE_RESULT_ERROR_INVALID_ARGUMENT;
}

//-----------------------------------------------------------------------------

ze_result_t ZeDriver::getMemAllocProperties(
    const void *ptr, ze_memory_allocation_properties_t *pMemAllocProperties,
    ze_device_handle_t *phDevice) {
  ZeBuffer *buffer = this->getZeBuffer(ptr);

  if (buffer == nullptr) {
    pMemAllocProperties->type = ZE_MEMORY_TYPE_UNKNOWN;
    return ZE_RESULT_SUCCESS;
  }

  // Currently only support allocating device memory, need to do corresponding
  // change here if more memory types are supported, i.e. host, shared.
  pMemAllocProperties->type = ZE_MEMORY_TYPE_DEVICE;
  pMemAllocProperties->id = reinterpret_cast<uint64_t>(buffer->ptr_beg());

  if (phDevice != nullptr) {
    *phDevice = buffer->device();
  }

  if (pMemAllocProperties->pNext) {
    return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
  }

  return ZE_RESULT_SUCCESS;
}

//-----------------------------------------------------------------------------

} // namespace __zert__
