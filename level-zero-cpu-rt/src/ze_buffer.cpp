// INTEL_COPYRIGHT_BEGIN
// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END

#include "ze_buffer.hpp"
#include "ze_utils.hpp"
#include <cstring>

namespace __zert__ {

//-----------------------------------------------------------------------------

ZeBuffer::ZeBuffer(ZeDriver *driver, ZeDevice *device, uint32_t id)
    : driver_(driver), device_(device), id_(id) {}
ZeBuffer::~ZeBuffer() = default;

//-----------------------------------------------------------------------------

ze_result_t ZeBuffer::allocate(ze_device_mem_alloc_desc_t desc, size_t size,
                               size_t alignment) {
  if (desc.ordinal != 0) {
    ZESIMERR << "desc.ordinal must be 0, got " << desc.ordinal;
    return ZE_RESULT_ERROR_INVALID_ENUMERATION;
  }
  if (alignment == 0) {
    alignment = 4096; // use a default alignment 4k if input is 0
  }

  this->ordinal_ = desc.ordinal;
  this->alignment_ = alignment;
  this->data_.resize(size);
  return ZE_RESULT_SUCCESS;
}

//-----------------------------------------------------------------------------

ze_result_t ZeBuffer::copy(ZeDevice *device, ZeBuffer *srcbuf, ZeBuffer *dstbuf,
                           void *dst, void const *src, size_t size) {
  (void)srcbuf;
  (void)dstbuf;
  (void)device;
  assert(device->kind() == ZeDevice::kCPU);
  std::memcpy(dst, src, size);

  return ZE_RESULT_SUCCESS;
}

//-----------------------------------------------------------------------------

} // namespace __zert__
