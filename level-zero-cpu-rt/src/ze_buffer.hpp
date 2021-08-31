// INTEL_COPYRIGHT_BEGIN
// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END
#pragma once

#include "ze_device.hpp"
#include "ze_driver.hpp"

namespace __zert__ {

struct ZeBuffer final {
public:
  ZeBuffer(ZeDriver *, ZeDevice *, uint32_t id);
  virtual ~ZeBuffer();

  ze_result_t allocate(ze_device_mem_alloc_desc_t desc, size_t size,
                       size_t alignment);
  size_t size() const { return data_.size(); }
  uint8_t *ptr_beg() { return data_.data(); }
  uint8_t const *ptr_beg() const { return data_.data(); }
  uint8_t const *ptr_end() const { return data_.data() + data_.size(); }

  static ze_result_t copy(ZeDevice *device, ZeBuffer *srcbuf, ZeBuffer *dstbuf,
                          void *dst, void const *src, size_t size);
  ZeDriver *driver() { return driver_; }
  ZeDevice *device() { return device_; }

  void setInputBufferName(std::string name) { buf_in_file_name_ = name; }
  void setOutputBufferName(std::string name) { buf_out_file_name_ = name; }
  std::string getInputBufferName() { return buf_in_file_name_; }
  std::string getOutputBufferName() { return buf_out_file_name_; }

  void setNameAsGritsObj(std::string name) { grits_obj_name = name; }

  std::string getNameAsGritsObj() { return grits_obj_name; }

  void setStateless(bool b) { is_stateless = b; }
  bool isStateless() { return is_stateless; }
  uint32_t getBufferId() { return id_; }
  int32_t getExecId() { return execute_id; }
  void setExecId(int32_t id) { execute_id = id; }

private:
  ZeDriver *driver_;
  ZeDevice *device_;
  uint32_t id_;
  uint32_t ordinal_;
  size_t alignment_;

  std::vector<uint8_t> data_;

  std::string buf_in_file_name_;
  std::string buf_out_file_name_;
  std::string grits_obj_name;
  bool is_stateless = false;
  int32_t execute_id = -1;
};

} // namespace __zert__
