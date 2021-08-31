/*===================== begin_copyright_notice
==================================

INTEL CONFIDENTIAL
Copyright 2017
Intel Corporation All Rights Reserved.

The source code contained or described herein and all documents related to the
source code ("Material") are owned by Intel Corporation or its suppliers or
licensors. Title to the Material remains with Intel Corporation or its suppliers
and licensors. The Material contains trade secrets and proprietary and
confidential information of Intel or its suppliers and licensors. The Material
is protected by worldwide copyright and trade secret laws and treaty provisions.
No part of the Material may be used, copied, reproduced, modified, published,
uploaded, posted, transmitted, distributed, or disclosed in any way without
Intel's prior express written permission.

No license under any patent, copyright, trade secret or other intellectual
property right is granted to or conferred upon you by disclosure or delivery
of the Materials, either expressly, by implication, inducement, estoppel
or otherwise. Any license under such intellectual property rights must be
express and approved by Intel in writing.

======================= end_copyright_notice
==================================*/

#ifndef CMRTLIB_PROPRIETARY_SIMULATION_GSFWRITER_H_
#define CMRTLIB_PROPRIETARY_SIMULATION_GSFWRITER_H_

#include "genISAi_api.h"
#include "ze_common.hpp"
#include <cassert>
#include <fstream>

#define IMPLICIT_PER_THREAD_ARG_NUM 3

namespace __zert__ {

struct ZeKernel;
struct ZeBuffer;

namespace gsf {
class GsfWriter {
public:
  GsfWriter(genISAi_device_kind_t platform, unsigned int grf_size)
      : m_grfSize(grf_size), m_platform(platform) {}
  ~GsfWriter() = default;

  bool GenerateHeader(std::ofstream &gsf_stream, char const *ctx_name,
                      int32_t ccs_id, int32_t deviceTile_id,
                      bool isUMCompressionOn);

  bool GeneratePipelineSetting(std::ofstream &gsf_stream, char const *ctx_name,
                               int num_of_walkers,
                               gsf::MEMORY_OBJECT_CONTROL memObjCtrl,
                               bool isChainedKernels);

  bool SetWarlkerNum(int walkerNum, std::ofstream &gsf_stream,
                     char const *ctx_name);

  bool GenerateKernelPayload(uint32_t *grf_array, size_t length,
                             std::ofstream &gsf_stream);

  bool GenerateComputeWalker(std::ofstream &gsf_stream, ZeKernel *kernel,
                             std::vector<uint32_t> grid, char const *ctx_name,
                             unsigned int slm_size);

  bool SetComputeModeParams(const char *ctx_name, std::ostream &gsf_stream,
                            bool large_grf);

  void SetGrfSize(int grfSize) { m_grfSize = grfSize; }

  bool GenerateBindingTable(std::ostream &strm, char const *ctx_name);

  bool GenerateL3CacheSettingInGsf(MEMORY_OBJECT_CONTROL option,
                                   std::ostream &gsf_stream);

  bool GenerateSettingForGritsL1CachePolicy(MEMORY_OBJECT_CONTROL option,
                                            std::ostream &gsf_stream);

  bool GenerateBufferInformation(std::ostream &output_stream,
                                 const char *ctx_name, ZeBuffer *buf,
                                 genISAi_surface_state_t surface_state);

  bool GenerateStatelessBufferInformation(std::ostream &output_stream,
                                          const char *ctx_name, ZeBuffer *buf);

  bool GenerateStatelessStatefulBufferInformation(
      std::ostream &output_stream, const char *ctx_name, ZeBuffer *buf,
      genISAi_surface_state_t surface_state);

  bool GenGSFComputeFlush(std::ofstream &strm, char const *ctx_name,
                          int deviceTileID, bool flag, bool dataCacheFlush);

  bool GenGSFComputeTail(std::ofstream &strm, char const *ctx_name);

  bool genGSFOutSurf(std::ofstream &strm, ZeBuffer *buf);

  unsigned int getGRFSize() { return m_grfSize; }

  bool GenerateBtiInGsf(std::ofstream &strm, char const *ctx_name,
                        ZeBuffer *buf, unsigned int bti);

protected:
  unsigned int m_grfSize;
  genISAi_device_kind_t m_platform;

private:
  bool GenerateIndirectData(std::ofstream &gsf_stream, char const *ctx_name,
                            ZeKernel *kerne);

  bool GenerateL3MOCSHeap(std::ofstream &gsf_stream, char const *ctx_name,
                          const char *heap_name, std::string &l3Ctrl,
                          bool isChainedKernels);
};
} // namespace gsf
} // namespace __zert__

#endif // #ifndef CMRTLIB_PROPRIETARY_SIMULATION_GSFWRITER_H_
