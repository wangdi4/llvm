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

#include "GenericMemObj.h"
#include "PipeCommon.h"
#include "cl_shared_ptr.hpp"
#include "framework_proxy.h"

namespace Intel {
namespace OpenCL {
namespace Framework {

/**
 * This class represents a pipe memory object
 */
class Pipe : public GenericMemObject {

  PREPARE_SHARED_PTR(Pipe)

  static SharedPtr<Pipe> Allocate(const SharedPtr<Context> &pContext,
                                  cl_mem_object_type clObjType) {
    return new Pipe(pContext, clObjType);
  }

  /**
   *  For FPGA Emulator
   */
  long Release(void) override {
    if (this->GetContext()->IsFPGAEmulator()) {
      void *pPipe = GetBackingStoreData();
      __pipe_release_fpga(pPipe);
    }
    return 0;
  }

  /**
   * @param uiPacketSize the size in byte of this Pipe's packet
   * @param uiMaxPackets the maximum number of packets this Pipe can hold
   * @return the pipe's size
   */
  static size_t CalcPipeSize(cl_uint uiPacketSize, cl_uint uiMaxPackets) {
    const OCLConfig *pOclConfig = FrameworkProxy::Instance()->GetOCLConfig();
    if (FPGA_EMU_DEVICE == pOclConfig->GetDeviceMode()) {
      int mode = pOclConfig->GetChannelDepthEmulationMode();
      return __pipe_get_total_size_fpga(uiPacketSize, uiMaxPackets, mode);
    }
    return pipe_get_total_size(uiPacketSize, uiMaxPackets);
  }

  using GenericMemObject::Initialize;
  /**
   * Initialize this Pipe
   * @param uiPacketSize the size in byte of this Pipe's packet
   * @param uiMaxPackets the maximum number of packets this Pipe can hold
   * @param pHostPtr optional pointer coming from CRT
   * @return CL_SUCCESS in case of success or error code in case of failure
   */
  cl_err_code Initialize(cl_mem_flags flags, cl_uint uiPacketSize,
                         cl_uint uiMaxPackets, void *pHostPtr);

  /**
   * @param paramName specifies the information to query
   * @param szParamValueSize is used to specify the size in bytes of memory
   * pointed to by paramName
   * @param pParamValue is a pointer to memory where the appropriate result
   * being queried is returned. If pParamValue is NULL, it is ignored.
   * @param pszParamValueSizeRet returns the actual size in bytes of data being
   * queried by pParamValue. If pszParamValueSizeRet is NULL, it is ignored.
   * @return CL_SUCCESS in case of success or error code in case of failure
   */
  cl_int GetPipeInfo(cl_pipe_info paramName, size_t szParamValueSize,
                     void *pParamValue, size_t *pszParamValueSizeRet);

  /** Check if a pipe object was created with CL_MEM_HOST_READ_ONLY or
   *  CL_MEM_HOST_WRITE_ONLY, so it can be used for FPGA host-side pipes
   *  extension.
   */
  bool IsHostAccessible() {
    return (GetFlags() & CL_MEM_HOST_WRITE_ONLY) ||
           (GetFlags() & CL_MEM_HOST_READ_ONLY);
  }

  /**
   * Map pipe object for read or write (depending on a \p flags).
   * @param flags is for future use, and should currently be specified as 0
   * @param requestedSize  is a number of bytes to read or write
   * @param pMappedSizeRet is an actual size of returned memory buffer
   * @returns a valid pointer for memory map or nullptr
   */
  void *Map(cl_mem_flags flags, size_t requestedSize, size_t *pMappedSizeRet,
            cl_err_code *pError);

  /**
   * Unmap a memory returned by a Map call.
   * @param pMappedMem is a memory to unmap
   * @param sizeToUnmap is a number of bytes to unmap
   * @param pUnmappedSizeRet is a number of bytes that actually was
   *    unmapped. Runtime can unmap less than requested for performance
   *    reasons.
   */
  cl_err_code Unmap(void *pMappedMem, size_t sizeToUnmap,
                    size_t *pUnmappedSizeRet);

  /**
   * Read a single packet into \p pDst.
   */
  cl_err_code ReadPacket(void *pDst);

  /**
   * Write a single packet into \p pDst.
   */
  cl_err_code WritePacket(const void *pSrc);

private:
  Pipe(const SharedPtr<Context> &pContext, cl_mem_object_type clObjType)
      : GenericMemObject(pContext, clObjType) {
    assert(CL_MEM_OBJECT_PIPE == clObjType &&
           "CL_MEM_OBJECT_PIPE != clObjType");
  }

  struct MapSegment {
    char *ptr;
    size_t size;
    size_t sizeToUnmap;
  };

  void MapRead(MapSegment seg);
  void MapWrite(MapSegment seg);
  void UnmapRead(MapSegment seg);
  void UnmapWrite(MapSegment seg);
  void FlushRead();
  void FlushWrite();

  cl_uint m_uiPacketSize = 0;
  cl_uint m_uiMaxPackets = 0;

  std::vector<char> m_mapBuffer;
  std::deque<MapSegment> m_mapSegments;
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
