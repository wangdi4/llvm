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

#include "pipe.h"
#include "CL/cl_fpga_ext.h"
#include "framework_proxy.h"
#include <algorithm>

using namespace Intel::OpenCL::Framework;

#define SET_ERROR(errPtr, error)                                               \
  if (errPtr) {                                                                \
    *(errPtr) = (error);                                                       \
  }

cl_err_code Pipe::Initialize(cl_mem_flags flags, cl_uint uiPacketSize,
                             cl_uint uiMaxPackets, void *pHostPtr) {
  m_uiPacketSize = uiPacketSize;
  m_uiMaxPackets = uiMaxPackets;

  if (((flags & CL_MEM_HOST_WRITE_ONLY) && (flags & CL_MEM_WRITE_ONLY)) ||
      ((flags & CL_MEM_HOST_READ_ONLY) && (flags & CL_MEM_READ_ONLY))) {
    return CL_INVALID_VALUE;
  }

  const size_t szDim = CalcPipeSize(uiPacketSize, uiMaxPackets);
  cl_err_code err = CL_SUCCESS;
  if (!pHostPtr) {
    err = GenericMemObject::Initialize(flags, nullptr, 1, &szDim, nullptr,
                                       nullptr, 0);
  } else {
    err = GenericMemObject::Initialize(flags | CL_MEM_USE_HOST_PTR, nullptr, 1,
                                       &szDim, nullptr, pHostPtr, 0);
  }
  if (CL_FAILED(err)) {
    return err;
  }

  IOCLDevBackingStore *pBS;
  err = GetBackingStore(CL_DEV_BS_GET_ALWAYS, &pBS);
  assert(CL_SUCCEEDED(err) && "GetBackingStore failed");

  if (this->GetContext()->IsFPGAEmulator()) {
    int mode = FrameworkProxy::Instance()
                   ->GetOCLConfig()
                   ->GetChannelDepthEmulationMode();
    __pipe_init_fpga(pBS->GetRawData(), uiPacketSize, uiMaxPackets, mode);
    m_mapBuffer.reserve(uiPacketSize * uiMaxPackets);
  } else {
    pipe_control_intel_t *pipeCtrl = (pipe_control_intel_t *)pBS->GetRawData();
    memset(pipeCtrl, 0, INTEL_PIPE_HEADER_RESERVED_SPACE);
    // one extra packet is required by the pipe implementation
    pipeCtrl->pipe_max_packets_plus_one = uiMaxPackets + 1;
  }

  return CL_SUCCESS;
}

cl_int Pipe::GetPipeInfo(cl_pipe_info paramName, size_t szParamValueSize,
                         void *pParamValue, size_t *pszParamValueSizeRet) {
  size_t szSize;
  const void *pValue;

  switch (paramName) {
  case CL_PIPE_PACKET_SIZE:
    szSize = sizeof(cl_uint);
    pValue = &m_uiPacketSize;
    break;
  case CL_PIPE_MAX_PACKETS:
    szSize = sizeof(cl_uint);
    pValue = &m_uiMaxPackets;
    break;
  case CL_PIPE_PROPERTIES:
    // OCL3.0 doesn't define any optional properties for pipe,
    // so we just return param_value_size_ret equal to 0 here
    szSize = 0;
    break;
  default:
    return CL_INVALID_VALUE;
  }

  if (nullptr != pParamValue && szParamValueSize < szSize) {
    LOG_ERROR(TEXT("szParamValueSize (=%zu) < szSize (=%zu)"), szParamValueSize,
              szSize);
    return CL_INVALID_VALUE;
  }

  if (nullptr != pszParamValueSizeRet)
    *pszParamValueSizeRet = szSize;

  if (nullptr != pParamValue && szSize > 0 && pValue)
    MEMCPY_S(pParamValue, szParamValueSize, pValue, szSize);

  return CL_SUCCESS;
}

void *Pipe::Map(cl_mem_flags flags, size_t requestedSize,
                size_t *pMappedSizeRet, cl_err_code *pError) {
  // Flags are reserved for future use
  if (flags != 0) {
    SET_ERROR(pError, CL_INVALID_VALUE);
    return nullptr;
  }

  if (!(GetFlags() & CL_MEM_HOST_READ_ONLY) &&
      !(GetFlags() & CL_MEM_HOST_WRITE_ONLY)) {
    SET_ERROR(pError, CL_INVALID_MEM_OBJECT);
    return nullptr;
  }

  // Reject invalid sizes:
  //   - size of 0 does not makes sense.
  //   - sizes aliquant to the packet size.
  if (requestedSize == 0 || requestedSize % m_uiPacketSize) {
    SET_ERROR(pError, CL_INVALID_VALUE);
    return nullptr;
  }

  if (!pMappedSizeRet) {
    SET_ERROR(pError, CL_INVALID_VALUE);
    return nullptr;
  }

  // Ensure that we don't reallocate the map buffer, otherwise pointers get
  // invalidated.
  size_t freeCapacity = m_mapBuffer.capacity() - m_mapBuffer.size();
  if (!freeCapacity) {
    SET_ERROR(pError, CL_OUT_OF_RESOURCES);
    return nullptr;
  }

  MapSegment seg;
  seg.size = std::min(requestedSize, freeCapacity);

  // Round mapped size down to be an integral of packet size, otherwise it
  // cannot be fully unmapped.
  seg.size = (seg.size / m_uiPacketSize) * m_uiPacketSize;
  assert(seg.size > 0 && "Segment size must be nonzero");
  seg.sizeToUnmap = seg.size;

  m_mapBuffer.resize(m_mapBuffer.size() + seg.size);
  seg.ptr = &*(m_mapBuffer.end() - seg.size);

  m_mapSegments.push_back(seg);

  *pMappedSizeRet = seg.size;
  SET_ERROR(pError, CL_SUCCESS);

  if (GetFlags() & CL_MEM_HOST_READ_ONLY) {
    MapRead(seg);
  } else {
    MapWrite(seg);
  }
  return seg.ptr;
}

cl_err_code Pipe::Unmap(void *pMappedMem, size_t sizeToUnmap,
                        size_t *pUnmappedSizeRet) {
  if (m_mapSegments.size() == 0 || sizeToUnmap == 0) {
    return CL_INVALID_VALUE;
  }

  auto seg = std::find_if(
      m_mapSegments.begin(), m_mapSegments.end(),
      [pMappedMem](MapSegment &val) { return val.ptr == pMappedMem; });
  if (seg == m_mapSegments.end()) {
    return CL_INVALID_VALUE;
  }

  // Requested_size must be an integer multiple of the packet size.
  if (sizeToUnmap % m_uiPacketSize) {
    return CL_INVALID_VALUE;
  }

  size_t unmappedSize = sizeToUnmap;

  if (seg->size < unmappedSize) {
    return CL_INVALID_VALUE;
  }

  seg->sizeToUnmap -= unmappedSize;

  if (pUnmappedSizeRet) {
    *pUnmappedSizeRet = unmappedSize;
  } else {
    return CL_INVALID_VALUE;
  }

  if (seg->sizeToUnmap > 0) {
    return CL_SUCCESS;
  }

  if (seg != m_mapSegments.begin()) {
    // Segment is not the first one - it should wait for the leading
    // segments to unmap
    return CL_SUCCESS;
  }

  // Now clean up all fully-unmapped segments starting from the beginning.
  while (!m_mapSegments.empty() && m_mapSegments.front().sizeToUnmap == 0) {
    if (GetFlags() & CL_MEM_HOST_READ_ONLY) {
      UnmapRead(m_mapSegments.front());
    } else {
      UnmapWrite(m_mapSegments.front());
    }

    m_mapSegments.pop_front();
  }

  if (GetFlags() & CL_MEM_HOST_READ_ONLY) {
    FlushRead();
  } else {
    FlushWrite();
  }

  if (m_mapSegments.empty()) {
    m_mapBuffer.resize(0);
  }

  return CL_SUCCESS;
}

void Pipe::MapRead(MapSegment seg) {
  assert(seg.size % m_uiPacketSize == 0 &&
         "Segment size must have integral number of packets");

  void *pPipe = GetBackingStoreData();

  size_t numPackets = seg.size / m_uiPacketSize;
  for (size_t i = 0; i < numPackets; ++i) {
    while (__read_pipe_2_fpga(pPipe, seg.ptr + i * m_uiPacketSize,
                              /*size=*/m_uiPacketSize,
                              /*align=*/m_uiPacketSize)) {
      FlushRead();
    }
  }
}

void Pipe::MapWrite(MapSegment /*seg*/) {
  // MapWrite is no-op: we just allocate memory where packets would go.
  return;
}

void Pipe::UnmapRead(MapSegment /*seg*/) {
  // UnmapRead is a no-op: we don't have to do anything after we
  // read.
}

void Pipe::UnmapWrite(MapSegment seg) {
  assert(seg.size % m_uiPacketSize == 0 &&
         "Segment size must have integral number of packets");

  void *pPipe = GetBackingStoreData();

  size_t numPackets = seg.size / m_uiPacketSize;
  for (size_t i = 0; i < numPackets; ++i) {
    while (__write_pipe_2_fpga(pPipe, seg.ptr + i * m_uiPacketSize,
                               /*size=*/m_uiPacketSize,
                               /*align=*/m_uiPacketSize)) {
      FlushWrite();
    }
  }
}
cl_err_code Pipe::ReadPacket(void *pDst) {
  if (!pDst) {
    return CL_INVALID_VALUE;
  }

  if (!(GetFlags() & CL_MEM_HOST_READ_ONLY)) {
    return CL_INVALID_MEM_OBJECT;
  }

  if (!m_mapSegments.empty()) {
    // Map is active, thus we cannot directly write to keep ordering b/w
    // concurrent read/map operations.
    size_t mappedSize = 0;
    cl_err_code error = CL_SUCCESS;
    void *mapPtr = Map(0, m_uiPacketSize, &mappedSize, &error);
    if (CL_FAILED(error)) {
      assert(error == CL_OUT_OF_RESOURCES && "Unexpected Pipe::Map error.");
      return CL_PIPE_EMPTY;
    }

    memcpy(pDst, mapPtr, m_uiPacketSize);

    size_t unmappedSize = 0;
    error = Unmap(mapPtr, m_uiPacketSize, &unmappedSize);
    assert(error == CL_SUCCESS &&
           "Cannot fail unmap unless arguments are incorrect.");
    assert((unmappedSize == mappedSize) && (mappedSize == m_uiPacketSize) &&
           "Inconsistent mapped/unmapped size.");
    return CL_SUCCESS;
  }

  void *pPipe = GetBackingStoreData();
  if (__read_pipe_2_fpga(pPipe, pDst, /*size=*/m_uiPacketSize,
                         /*align=*/m_uiPacketSize)) {
    return CL_PIPE_EMPTY;
  }
  FlushRead();
  return CL_SUCCESS;
}

cl_err_code Pipe::WritePacket(const void *pSrc) {
  if (!pSrc) {
    return CL_INVALID_VALUE;
  }

  if (!(GetFlags() & CL_MEM_HOST_WRITE_ONLY)) {
    return CL_INVALID_MEM_OBJECT;
  }

  if (!m_mapSegments.empty()) {
    // Map is active, thus we cannot directly write to keep ordering b/w
    // concurrent write/map operations.
    size_t mappedSize = 0;
    cl_err_code error = CL_SUCCESS;
    void *mapPtr = Map(0, m_uiPacketSize, &mappedSize, &error);
    if (CL_FAILED(error)) {
      assert(error == CL_OUT_OF_RESOURCES && "Unexpected Pipe::Map error.");
      return CL_PIPE_FULL;
    }

    memcpy(mapPtr, pSrc, m_uiPacketSize);

    size_t unmappedSize = 0;
    error = Unmap(mapPtr, m_uiPacketSize, &unmappedSize);
    assert(error == CL_SUCCESS &&
           "Cannot fail unmap unless arguments are incorrect.");
    assert((unmappedSize == mappedSize) && (mappedSize == m_uiPacketSize) &&
           "Inconsistent mapped/unmapped size.");
    return CL_SUCCESS;
  }

  void *pPipe = GetBackingStoreData();
  if (__write_pipe_2_fpga(pPipe, pSrc, /*size=*/m_uiPacketSize,
                          /*align=*/m_uiPacketSize)) {
    return CL_PIPE_FULL;
  }
  FlushWrite();
  return CL_SUCCESS;
}

void Pipe::FlushRead() {
  void *pPipe = GetBackingStoreData();
  __flush_read_pipe(pPipe);
}

void Pipe::FlushWrite() {
  void *pPipe = GetBackingStoreData();
  __flush_write_pipe(pPipe);
}
