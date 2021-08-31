// INTEL_COPYRIGHT_BEGIN
// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END
#pragma once

#include "level_zero/include/ze_api.h"
#include "ze_common.hpp"
#include <list>
#include <mutex>

struct _ze_driver_handle_t {};

namespace __zert__ {

struct ZeContext;
struct ZeDevice;
struct ZeBuffer;
struct ZeEventPool;

struct ZeDriver final : _ze_driver_handle_t {
public:
  static ZeDriver &getInstance();
  static void init();
  virtual ~ZeDriver();
  std::vector<ZeDevice *> const &devices() { return devices_; }

  ZeBuffer *makeZeBuffer(ZeDevice *);
  ZeBuffer *getZeBuffer(void const *) const;
  ze_result_t destroyZeBuffer(ZeBuffer *);

  ze_result_t createEventPool(ze_event_pool_desc_t const *,
                              std::vector<ZeDevice *> devices,
                              ze_event_pool_handle_t *phEventPool);
  ze_result_t destroyEventPool(ZeEventPool *pool);
  ze_result_t createContext(ze_context_desc_t const &desc,
                            ze_context_handle_t *handle);
  ze_result_t destroyContext(ZeContext *context);
  ze_result_t getProperties(ze_driver_properties_t *properties);
  ze_result_t getApiVersion(ze_api_version_t *version);
  ze_result_t getExtensionProperties(
      uint32_t *pCount, ze_driver_extension_properties_t *pExtensionProperties);
  ze_result_t
  getMemAllocProperties(const void *ptr,
                        ze_memory_allocation_properties_t *pMemAllocProperties,
                        ze_device_handle_t *phDevice);

  // TODO: Spec extensions
  const std::vector<std::pair<std::string, uint32_t>> extensionsSupported = {};

private:
  ZeDriver();
  std::vector<std::unique_ptr<ZeDevice>> devices_owned_;
  std::vector<ZeDevice *> devices_;
  std::list<std::unique_ptr<ZeBuffer>> buffers_;
  std::list<std::unique_ptr<ZeEventPool>> event_pool_;
  std::vector<std::unique_ptr<ZeContext>> contexts_;
  static bool initialized;
  std::mutex mutex_;
  uint32_t buffer_id_ = 0;
};

} // namespace __zert__
