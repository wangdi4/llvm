/*===================== begin_copyright_notice
==================================

INTEL CONFIDENTIAL
Copyright 2017-2020
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

#include "gsf_writer.hpp"
#include "ze_buffer.hpp"
#include "ze_kernel.hpp"
#include "ze_utils.hpp"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace __zert__ {

extern bool is_stateless_surface(ZeKernel *kernel, unsigned int pos,
                                 ZeBuffer *&buf);

namespace gsf {

// Grits issue: save_to_file() has size limitation due to 32 bits for size.
// Besides, memory buffer size should be 4B alignment, so set the largest size
// of single memory buffer as 0xfffffffc. For any stateless buffer with >= 4GB
// size, need to create mutiple smaller size buffer and make them continuous in
// memory region to achieve this.
#define CM_BUFFER_STATELESS_LARGE_SIZE 0xfffffffc // 4GB - 4B

static const uint32_t pvc_max_thread_size_per_group_256_grf_mode = 32;
static const uint32_t pvc_max_thread_size_per_group_128_grf_mode = 64;

bool GsfWriter::GenerateHeader(std::ofstream &gsf_stream, char const *ctx_name,
                               int32_t ccs_id, int32_t deviceTile_id,
                               bool isUMCompressionOn) {
  std::string device = "Device";
  gsf_stream << "#====================================================\n";
  gsf_stream << "# INCLUDES\n";
  gsf_stream << "#====================================================\n";
  gsf_stream << "require \"Gfx\"\n";
  gsf_stream << "include Gfx\n";

  std::string context_type_name("RenderContext");
  std::string enable_compute_engine;

  if (deviceTile_id >= 0) {
    device.append("[");
    device.append(std::to_string(deviceTile_id));
    device.append("]");
  }

  enable_compute_engine.assign(device);
  enable_compute_engine.append(".register.memory_interface");
  enable_compute_engine.append(".render_control_unit_mode(\n");
  enable_compute_engine.append(
      "  :computeEngineEnable => true, :cpu => true)\n");

  enable_compute_engine.append(device);
  enable_compute_engine.append(".register.memory_interface");
  enable_compute_engine.append(".cache_mode_subslice(\n");
  enable_compute_engine.append("  :instructionCachePrefetchEnable => true)\n");

  if (isUMCompressionOn) {
    enable_compute_engine.append(device);
    enable_compute_engine.append(
        ".register.memory_interface.dual_subslice_unified_memory_"
        "compression_settings(\n");
    enable_compute_engine.append("  :compressionFormat => :linearRaw,\n");
    enable_compute_engine.append(
        "  :addressPairingBit => :sixtyFourByteCachelineStride,\n");
    enable_compute_engine.append("  :compressionEnabled => true)\n");
    enable_compute_engine.append(device);
    enable_compute_engine.append(
        ".register.memory_interface.lni_unified_memory_compression_"
        "settings(\n");
    enable_compute_engine.append("  :compressionFormat => :linearRaw,\n");
    enable_compute_engine.append(
        "  :addressPairingBit => :sixtyFourByteCachelineStride,\n");
    enable_compute_engine.append("  :compressionEnabled => true)\n");
    enable_compute_engine.append(device);
    enable_compute_engine.append(".register.memory_interface.unified_"
                                 "memory_compression_settings(\n");
    enable_compute_engine.append("  :compressionFormat => :linearRaw,\n");
    enable_compute_engine.append(
        "  :addressPairingBit => :sixtyFourByteCachelineStride,\n");
    enable_compute_engine.append("  :compressionEnabled => true)\n");
  }

  context_type_name.assign("ComputeContext");
  gsf_stream << enable_compute_engine;
  gsf_stream << context_type_name << ".scheduler \"run list\"\n";
  gsf_stream << device << ".compute_engine_enable\n";
  gsf_stream << ctx_name << " = " << context_type_name << ".new";

  if (deviceTile_id >= 0) {
    gsf_stream << "(:memoryAllocationDeviceTileAffinity=>{:deviceTileIndex=>"
               << deviceTile_id;
    gsf_stream << ", :strength=>:strong}, :ring=>RingContext.new)\n";
  } else {
    gsf_stream << "\n";
  }

  gsf_stream << ctx_name << ".logical_streamer_index = " << ccs_id << "\n";

  gsf_stream << "Grits.disable_warning(\"AllFencesUsed\")\n";
  gsf_stream << "Grits.disable_error(\"DegradeToLinearOnFenceExhaustion\")\n";
  gsf_stream << "Grits.disable_warning(\"ExplicitMemoryRegionAddress\")\n";
  gsf_stream
      << "Grits.disable_warning(\"ExplicitMemoryRegionPPGTTAddress\")\n\n";

  return gsf_stream.good();
}

bool GsfWriter::GeneratePipelineSetting(std::ofstream &gsf_stream,
                                        char const *ctx_name,
                                        int num_of_walkers,
                                        gsf::MEMORY_OBJECT_CONTROL memObjCtrl,
                                        bool isChainedKernels) {
  gsf_stream << "#========================================================\n";
  gsf_stream << "# Configures Compute Front End state: CFE.\n";

  gsf_stream << "cfe = ComputeFrontEnd.new \n";
  SetWarlkerNum(num_of_walkers, gsf_stream, ctx_name);

  std::string l3Control;
  if (MEMORY_OBJECT_CONTROL_PVC_UNCACHEABLE == memObjCtrl) {
    l3Control.assign("uncacheable");
  } else {
    // L3 writeback for ATS/PVC/DG2
    l3Control.assign("writeback");
  }

  gsf_stream << "scratch_buffer_" << ctx_name << " = Surface.new(\n";
  gsf_stream << "    :format => \"RAW\",\n";
  gsf_stream << "    :baseSize => [2048, Grits.device_parameter(:threads)], \n";
  gsf_stream << "    :tiling => false, \n";
  gsf_stream << "    :isRenderTarget => false, \n";
  gsf_stream << "    :memoryObjectControl => MemoryObjectControl.new(\n";
  gsf_stream << "        :l3Control => { :cacheabilityControl => \""
             << l3Control << "\", \n";
  gsf_stream << "                        :globalGOEnable => false }, \n";
  gsf_stream
      << "        :llcControl => { :cacheabilityControl => \"writeback\", \n";
  gsf_stream << "                         :cacheInclusion => \"llcellc\", \n";
  gsf_stream << "                         :age => 3} \n";
  gsf_stream << "        ), \n";
  gsf_stream << "    :surfaceType => \"scratchbuffer\" \n";
  gsf_stream << "    ).resolve(" << ctx_name << ") \n";

  gsf_stream << "cfe.scratch_space_buffer = scratch_buffer_" << ctx_name
             << "\n";

  gsf_stream << ctx_name << ".compute.front_end = cfe \n";
  // per fulsim team's request to set this flag in case error detection cause
  // fulsim halt
  auto stepping = util::get_envvar("L0SIM_GRITS_STEPPING");
  if (!stepping.empty() && stepping == "a0") {
    gsf_stream << ctx_name << ".compute.systolic_mode_enable = true \n";
  } else {
    gsf_stream << ctx_name << ".compute.systolic_mode_enable = false \n";
  }

  // Enable L3 for Heap DSH/GSH/SSH/ISH via MOCS
  gsf_stream << ctx_name << "_lc = LogicalContext.new()\n";
  if ((GENISAI_DEVICE_KIND_PVC == m_platform) &&
      memObjCtrl > MEMORY_OBJECT_CONTROL_PVC_DEFAULT &&
      memObjCtrl < MEMORY_OBJECT_CONTROL_PVC_COUNT) {
    gsf_stream << ctx_name << "_lc.";
    switch (memObjCtrl) {
    case MEMORY_OBJECT_CONTROL_PVC_UNCACHEABLE:
    case MEMORY_OBJECT_CONTROL_PVC_L3WB_L1_UNCACHEABLE:
      gsf_stream << "l1_cache_policy = \"uncacheable\"\n";
      break;
    case MEMORY_OBJECT_CONTROL_PVC_L3WB_L1_WRITEBYPASS:
      gsf_stream << "l1_cache_policy = \"writebypass\"\n";
      break;
    case MEMORY_OBJECT_CONTROL_PVC_L3WB_L1_WRITEBACK:
      gsf_stream << "l1_cache_policy = \"writeback\"\n";
      break;
    case MEMORY_OBJECT_CONTROL_PVC_L3WB_L1_WRITETHROUGH:
      gsf_stream << "l1_cache_policy = \"writethrough\"\n";
      break;
    case MEMORY_OBJECT_CONTROL_PVC_L3WB_L1_WRITESTREAMING:
      gsf_stream << "l1_cache_policy = \"writestreaming\"\n";
      break;
    default:
      break;
    }
  }

  GenerateL3MOCSHeap(gsf_stream, ctx_name, "dynamic_indirect_state_region",
                     l3Control, isChainedKernels);
  GenerateL3MOCSHeap(gsf_stream, ctx_name, "general_indirect_state_region",
                     l3Control, false);
  GenerateL3MOCSHeap(gsf_stream, ctx_name, "surface_indirect_state_region",
                     l3Control, false);
  GenerateL3MOCSHeap(gsf_stream, ctx_name, "instructions_region", l3Control,
                     false);

  gsf_stream << ctx_name
             << "_lc.stateless_data_port_access_memory_object_control = "
             << "MemoryObjectControl.new(\n";
  gsf_stream << "    :l3Control => {\n";
  gsf_stream << "        :cacheabilityControl => \"" << l3Control << "\", \n";
  gsf_stream << "        :skipCachingEnable => false, \n";
  gsf_stream << "        :globalGOEnable => false }\n";
  gsf_stream << "    )\n";

  gsf_stream << ctx_name << ".use " << ctx_name << "_lc\n" << std::endl;

  return gsf_stream.good();
}

bool GsfWriter::SetWarlkerNum(int walkerNum, std::ofstream &gsf_stream,
                              char const * /*ctx_name*/) {
  gsf_stream << "#========================================================\n";
  gsf_stream << "cfe.number_of_walkers = " << walkerNum << "\n";

  return gsf_stream.good();
}

bool GsfWriter::GenerateKernelPayload(uint32_t *grf_array, size_t length_in_dw,
                                      std::ofstream &gsf_stream) {
  size_t arg_count_in_dw_per_grf = m_grfSize >> 2;
  gsf_stream << "#=========================\n";
  gsf_stream << "# Generates CURBE data.\n";
  gsf_stream << "curbe = []\n";
  gsf_stream << "curbe << [";

  // Insert argument data into CURBE
  for (size_t grf_count = 0; grf_count < length_in_dw; ++grf_count) {
    gsf_stream << grf_array[grf_count];

    if (grf_count < length_in_dw - 1) {
      if ((grf_count + 1) % arg_count_in_dw_per_grf != 0) {
        gsf_stream << ", ";
      } else {
        gsf_stream << "]\n";
        gsf_stream << "curbe << [";
      }
    }
  }
  gsf_stream << "]\n" << std::endl;
  return gsf_stream.good();
}

bool GsfWriter::SetComputeModeParams(const char *ctx_name,
                                     std::ostream &gsf_stream, bool large_grf) {
  // Disable force MultiGPU Atomics and disable force MultiGPU partial writes
  // in sim mode
  gsf_stream << ctx_name << ".compute.compute_mode(\n"
             << "  :forceMultiGPUAtomicsDisable => true,\n"
             << "  :forceMultiGPUPartialWritesDisable => true";

  if (large_grf) {
    gsf_stream << ",\n  :grfMode => :large";
  }

  gsf_stream << ")\n" << std::endl;

  return gsf_stream.good();
}

bool GsfWriter::GenerateL3MOCSHeap(std::ofstream &gsf_stream,
                                   char const *ctx_name, const char *heap_name,
                                   std::string &l3Ctrl, bool isChainedKernels) {
  gsf_stream << ctx_name << "_lc." << heap_name
             << "= IndirectStateRegion.new(\n";
  if (isChainedKernels) {
    gsf_stream << "    :region => 0x02000000,\n";
  }
  gsf_stream << "    :memoryObjectControl => MemoryObjectControl.new(\n";
  gsf_stream << "        :l3Control => {\n";
  gsf_stream << "            :cacheabilityControl => \"" << l3Ctrl << "\",\n";
  gsf_stream << "            :skipCachingEnable => false, \n";
  gsf_stream << "            :globalGOEnable => false\n";
  gsf_stream << "        }\n";
  gsf_stream << "    )\n";
  gsf_stream << ")\n";

  return gsf_stream.good();
}

bool GsfWriter::GenerateIndirectData(std::ofstream &gsf_stream,
                                     char const *ctx_name, ZeKernel *kernel) {
  gsf_stream << ctx_name << ".commit\n"
             << "curbe_data = curbe.flatten\n"
             << "argument_data_buffer_size_" << ctx_name
             << " = 4*curbe_data.size\n"
             << "argument_data_buffer_size_" << ctx_name
             << " = (argument_data_buffer_size_" << ctx_name << " + 63)/64*64\n"
             << "argument_data_buffer_" << ctx_name << " = MemoryBuffer.new(\n"
             << "  :size => argument_data_buffer_size_" << ctx_name << ",\n"
             << "  :alignment => 64,\n"
             << "  :memoryRegion =>" << ctx_name
             << ".logical.general_indirect_state_region)\n";
  gsf_stream << "argument_data_buffer_" << ctx_name
             << ".write(:data => curbe_data)\n"
             << std::endl;

  // write stateless buffer address into curbe
  unsigned int numArgs = 0;
  genISAi_kernel_get_all_kernel_params_count(kernel->genISAi_kernel(),
                                             &numArgs);

  std::vector<unsigned int> argSizes(numArgs, 0);
  std::vector<genISAi_kernel_param_type_t> argTypes(
      numArgs, GENISAI_KERNEL_PARAM_TYPE_UNKNOWN);
  std::vector<unsigned int> offsetInPayload(numArgs, 0);
  std::vector<unsigned int> argIndex(numArgs, 0);
  genISAi_kernel_get_all_kernel_params_info(
      kernel->genISAi_kernel(), numArgs, argSizes.data(), argTypes.data(),
      offsetInPayload.data(), argIndex.data());

  for (uint32_t argCount = 0; argCount < numArgs; argCount++) {
    auto arg_type = GENISAI_KERNEL_PARAM_TYPE_UNKNOWN;
    auto addr_mode = GENISAI_KERNEL_PARAM_ADDRESS_MODE_NONE;

    genISAi_kernel_get_kernel_param_type(kernel->genISAi_kernel(), argCount,
                                         &arg_type);
    genISAi_kernel_get_kernel_param_address_mode(kernel->genISAi_kernel(),
                                                 argCount, &addr_mode);

    ZeBuffer *buf = nullptr;
    if (is_stateless_surface(kernel, argCount, buf)) {
      // The param offset in kernel payload
      uint32_t offset = offsetInPayload[argCount];

      // The base Address offset
      assert(kernel->arguments_[argCount].size() == 8 &&
             "stateless kernel argument should be a 8 bytes address");
      uint64_t argumentAddr =
          *reinterpret_cast<uint64_t *>(kernel->arguments_[argCount].data());
      uint64_t BaseAddroffset =
          argumentAddr - reinterpret_cast<uint64_t>(buf->ptr_beg());

      gsf_stream << "argument_data_buffer"
                 << "_" << ctx_name
                 << ".write_memory_buffer_address(context:" << ctx_name
                 << ", offset:" << offset
                 << ", addressOffset:" << BaseAddroffset
                 << ", memoryBuffer:" << buf->getNameAsGritsObj();

      if (buf->size() > CM_BUFFER_STATELESS_LARGE_SIZE) {
        gsf_stream << "[0])\n";
      } else {
        gsf_stream << ")\n";
      }
    }
  }

  gsf_stream << std::endl;

  return gsf_stream.good();
}

bool GsfWriter::GenerateComputeWalker(std::ofstream &gsf_stream,
                                      ZeKernel *kernel,
                                      std::vector<uint32_t> grid,
                                      char const *ctx_name,
                                      unsigned int slm_size) {
  GenerateIndirectData(gsf_stream, ctx_name, kernel);

  uint32_t threadSizePerGroup = kernel->getGroupSize()[0] *
                                kernel->getGroupSize()[1] *
                                kernel->getGroupSize()[2];

  // validate thread size on PVC
  if (GENISAI_DEVICE_KIND_PVC == m_platform) {
    uint32_t maxThreadSize;

    if (kernel->isLargeGRFMode()) {
      maxThreadSize = pvc_max_thread_size_per_group_256_grf_mode;
    } else {
      maxThreadSize = pvc_max_thread_size_per_group_128_grf_mode;
    }
    if (threadSizePerGroup > maxThreadSize) {
      fprintf(stderr, "Error: The thread size exceeds the max allowed!");
      return false;
    }
  }

  gsf_stream << ctx_name << "_ShaderKernel_"
             << " = ShaderKernel.new(\n";
  gsf_stream << "  :sourceFile => \"" << kernel->getGenBinaryFileName().c_str()
             << "\", \n";
  gsf_stream << "  :inline => false )\n\n";

  gsf_stream << ctx_name << ".walk ComputeWalker.new(\n";
  gsf_stream << "  :shaderKernel => " << ctx_name << "_ShaderKernel_"
             << ",\n";
  gsf_stream << "  :indirectBuffer => argument_data_buffer_" << ctx_name
             << ",\n";
  gsf_stream << "  :bindingTable => bindingTable_" << ctx_name << ",\n";

  gsf_stream << "  :samplerStateTable => SamplerStateTable.new(),\n";

  gsf_stream << "  :gpgpuThreadsInGroup => " << threadSizePerGroup << ",\n"
             << "  :threadGroupIDDimensions => [" << grid[0] << ", " << grid[1]
             << ", " << grid[2] << "],\n"
             << "  :threadGroupIDStarts => [0, 0, 0],\n"
             << "  :simdSize => 32,\n"
             << "  :executionMask => " << 0xffffffff << ",\n"
             << "  :roundingMode => \"nearestEven\",\n";

  if (GENISAI_DEVICE_KIND_PVC == m_platform) {
    uint32_t nbarrier_cnt = kernel->GetNBarrierCnt();
    if (nbarrier_cnt > 0) {
      gsf_stream << "  :numBarriers => " << nbarrier_cnt << ",\n";
    }
  } else {
    gsf_stream << "  :barrierEnable => true,\n";
  }

  gsf_stream << "  :sharedLocalMemorySize => " << slm_size << ")\n"
             << std::endl;

  return gsf_stream.good();
}

bool GsfWriter::GenerateBindingTable(std::ostream &strm, char const *ctx_name) {
  strm << "#====================================================" << std::endl;
  strm << "# BINDING TABLE" << std::endl;
  strm << "# Set up the binding table" << std::endl;
  strm << "#====================================================\n"
       << std::endl;
  strm << "bindingTable"
       << "_" << ctx_name << " = BindingTable.new(nil)\n"
       << std::endl;

  return strm.good();
}

bool GsfWriter::GenerateL3CacheSettingInGsf(MEMORY_OBJECT_CONTROL option,
                                            std::ostream &gsf_stream) {
  std::string l3Ctrl;
  if (MEMORY_OBJECT_CONTROL_PVC_UNCACHEABLE == option) {
    l3Ctrl.assign("uncacheable");
  } else {
    l3Ctrl.assign("writeback");
  }

  gsf_stream << "\t:memoryObjectControl => MemoryObjectControl.new(\n";
  gsf_stream << "\t    :l3Control => {:cacheabilityControl => \"" << l3Ctrl
             << "\", \n";
  gsf_stream << "\t                   :globalGOEnable => false } \n";
  gsf_stream << "\t)," << std::endl;

  return gsf_stream.good();
}

bool GsfWriter::GenerateSettingForGritsL1CachePolicy(
    MEMORY_OBJECT_CONTROL option, std::ostream &gsf_stream) {
  bool result = true;

  switch (option) {
  case MEMORY_OBJECT_CONTROL_PVC_UNCACHEABLE:
  case MEMORY_OBJECT_CONTROL_PVC_L3WB_L1_UNCACHEABLE:
    gsf_stream << "\t:l1CachePolicy => \"uncacheable\", \n";
    break;
  case MEMORY_OBJECT_CONTROL_PVC_L3WB_L1_WRITEBYPASS:
    gsf_stream << "\t:l1CachePolicy => \"writebypass\", \n";
    break;
  case MEMORY_OBJECT_CONTROL_PVC_L3WB_L1_WRITEBACK:
    gsf_stream << "\t:l1CachePolicy => \"writeback\", \n";
    break;
  case MEMORY_OBJECT_CONTROL_PVC_L3WB_L1_WRITETHROUGH:
    gsf_stream << "\t:l1CachePolicy => \"writethrough\", \n";
    break;
  case MEMORY_OBJECT_CONTROL_PVC_L3WB_L1_WRITESTREAMING:
    gsf_stream << "\t:l1CachePolicy => \"writestreaming\", \n";
    break;
  default:
    result = false;
    break;
  }
  return result;
}

bool GsfWriter::GenerateBufferInformation(
    std::ostream &output_stream, const char *ctx_name, ZeBuffer *buf,
    genISAi_surface_state_t surface_state) {
  // // Padded to a multiple of 4.
  unsigned int padded_size = (buf->size() + 3) & ~3;
  uint64_t offset = (uint8_t *)surface_state.base_address - buf->ptr_beg();

  std::string bufferName =
      "buffer_" + std::to_string(buf->getBufferId()) + "_" + ctx_name;
  buf->setNameAsGritsObj(bufferName);

  output_stream << "#============1D surface===============\n";
  output_stream << buf->getNameAsGritsObj() << " = Surface.new(\n";
  output_stream << "  :image => \"" << buf->getInputBufferName().c_str()
                << "\",\n";
  output_stream << "  :format => \"RAW\",\n";
  output_stream << "  :baseSize => " << padded_size << ",\n";
  if (offset > 0) {
    output_stream << "  :baseOffset => " << offset << ",\n";
  }

  output_stream << "  :tiling => false,\n";
  output_stream << "  :isRenderTarget => false,\n";

  // Todo: need add interface in buffer to set cache policy, here just
  // hard code it as writeback for both L1/L3.
  GenerateL3CacheSettingInGsf(gsf::MEMORY_OBJECT_CONTROL_PVC_L3WB_L1_WRITEBACK,
                              output_stream);
  GenerateSettingForGritsL1CachePolicy(
      gsf::MEMORY_OBJECT_CONTROL_PVC_L3WB_L1_WRITEBACK, output_stream);
  output_stream << "  :surfaceType => \"buffer\"\n";
  output_stream << ")" << std::endl;

  output_stream << buf->getNameAsGritsObj();
  output_stream << ".resolve(" << ctx_name << ")\n" << std::endl;

  return output_stream.good();
}

bool GsfWriter::GenerateStatelessBufferInformation(std::ostream &output_stream,
                                                   const char *ctx_name,
                                                   ZeBuffer *buf) {
  // Padded to a multiple of 4.
  size_t padded_size = (buf->size() + 3) & ~3;
  std::string inputFileName = buf->getInputBufferName();

  std::string bufferName =
      "buffer_" + std::to_string(buf->getBufferId()) + "_" + ctx_name;
  buf->setNameAsGritsObj(bufferName);

  bool isLargeSize =
      (buf->size() > CM_BUFFER_STATELESS_LARGE_SIZE) ? true : false;
  if (isLargeSize) {
    assert(0 && "Not tested");
    // Workaround: MemoryBuffer.save_to_file() limits the size to be less
    // than 4GB.
    // So need to multiple smaller memory buffers for a large size. Grtis
    // nees couple of months to fix it. After done, this workaournd can be
    // removed.

    std::string memRegionName =
        "memRegion_" + std::to_string(buf->getBufferId()) + "_" + ctx_name;

    size_t numMemoryBuffers =
        (padded_size % CM_BUFFER_STATELESS_LARGE_SIZE)
            ? (padded_size / CM_BUFFER_STATELESS_LARGE_SIZE + 1)
            : (padded_size / CM_BUFFER_STATELESS_LARGE_SIZE);

    output_stream << "#============1D stateless surface===============\n";
    output_stream << memRegionName << " = MemoryRegion.new(\n";
    output_stream << "  :size => " << padded_size << ",\n";
    output_stream << "  :initialGTT => " << ctx_name << "\n";
    output_stream << ")\n\n";
    output_stream << buf->getNameAsGritsObj();
    output_stream << " = []\n";
    output_stream << numMemoryBuffers << ".times do |i|\n";
    output_stream << "    if i != " << numMemoryBuffers - 1 << std::endl;
    output_stream << "        size = " << CM_BUFFER_STATELESS_LARGE_SIZE
                  << std::endl;
    output_stream << "    else" << std::endl;
    output_stream << "        size = "
                  << padded_size -
                         CM_BUFFER_STATELESS_LARGE_SIZE * (numMemoryBuffers - 1)
                  << std::endl;
    output_stream << "    end\n";
    output_stream << "    memoryBuffer = MemoryBuffer.new(\n";
    output_stream << "      :size => size,\n";
    output_stream << "      :tiling => false,\n";
    output_stream << "      :memoryRegion => " << memRegionName << ",\n";
    output_stream << "      :memoryRegionOffset => i * "
                  << CM_BUFFER_STATELESS_LARGE_SIZE << ",\n";
    output_stream << "      :initialGTT => " << ctx_name << std::endl;
    output_stream << "    )\n";
    output_stream << "    memoryBuffer.initialize_from_file(\n";
    std::string curImageName =
        inputFileName.insert(inputFileName.length() - 4, ".#{i}");
    output_stream << "      :fileName => \"" << curImageName.c_str() << "\",\n";
    output_stream << "      :format => \"RAW\"\n";
    output_stream << "    )\n";
    output_stream << "    ";
    output_stream << buf->getNameAsGritsObj();
    output_stream << " << memoryBuffer\n";
    output_stream << "end\n\n";
  } else {
    output_stream << "#============1D stateless surface===============\n";
    output_stream << buf->getNameAsGritsObj();
    output_stream << " = MemoryBuffer.new(\n";
    output_stream << "  :size => " << padded_size << ",\n";
    output_stream << "  :tiling => false,\n";
    output_stream << "  :initialGTT => " << ctx_name << "\n";
    output_stream << ")\n";
    output_stream << buf->getNameAsGritsObj();
    output_stream << ".initialize_from_file(\n";
    output_stream << "  :fileName => \"" << inputFileName.c_str() << "\", \n";
    output_stream << "  :format => \"RAW\" \n";
    output_stream << ")\n\n";
  }

  return output_stream.good();
}

bool GsfWriter::GenerateStatelessStatefulBufferInformation(
    std::ostream &output_stream, const char *ctx_name, ZeBuffer *buf,
    genISAi_surface_state_t surface_state) {
  // Padded to a multiple of 4.
  size_t padded_size = (buf->size() + 3) & ~3;
  std::string inputFileName = buf->getInputBufferName();
  uint64_t offset = (uint8_t *)surface_state.base_address - buf->ptr_beg();

  std::string memoryBufferName =
      "memoryBuffer_" + std::to_string(buf->getBufferId()) + "_" + ctx_name;
  buf->setNameAsGritsObj(memoryBufferName);

  output_stream
      << "#============1D stateless/stateful surface===============\n";
  output_stream << memoryBufferName;
  output_stream << " = MemoryBuffer.new(\n";
  output_stream << "  :size => " << padded_size << ",\n";
  output_stream << "  :tiling => false,\n";
  output_stream << "  :initialGTT => " << ctx_name << "\n";
  output_stream << ")\n";

  std::string bufferName =
      "buffer_" + std::to_string(buf->getBufferId()) + "_" + ctx_name;
  output_stream << bufferName << " = Surface.new(\n";
  output_stream << "  :image => \"" << buf->getInputBufferName().c_str()
                << "\",\n";
  output_stream << "  :format => \"RAW\",\n";
  output_stream << "  :baseSize => " << padded_size << ",\n";
  if (offset > 0) {
    output_stream << "  :baseOffset => " << offset << ",\n";
  }
  output_stream << "  :tiling => false,\n";
  output_stream << "  :isRenderTarget => false,\n";
  output_stream << "  :memoryBuffer => " << memoryBufferName << ",\n ";

  // Todo: need add interface in buffer to set cache policy, here just
  // hard code it as writeback for both L1/L3
  GenerateL3CacheSettingInGsf(gsf::MEMORY_OBJECT_CONTROL_PVC_L3WB_L1_WRITEBACK,
                              output_stream);
  GenerateSettingForGritsL1CachePolicy(
      gsf::MEMORY_OBJECT_CONTROL_PVC_L3WB_L1_WRITEBACK, output_stream);
  output_stream << "  :surfaceType => \"buffer\"\n";
  output_stream << ")" << std::endl;

  output_stream << bufferName;
  output_stream << ".resolve(" << ctx_name << ")\n" << std::endl;

  return output_stream.good();
}

bool GsfWriter::GenGSFComputeFlush(std::ofstream &strm, char const *ctx_name,
                                   int deviceTileID, bool flag,
                                   bool dataCacheFlush) {
  if (flag == true) {
    strm << ctx_name << ".force_resend(:frontEnd)" << std::endl;
  }

  std::string device = "Device";
  if (deviceTileID >= 0) {
    device.append("[");
    device.append(std::to_string(deviceTileID));
    device.append("]");
  }

  if (dataCacheFlush == true) {
    strm << device << ".pipe_control( :context=>" << ctx_name
         << ", :dataCacheFlush => true )" << std::endl;
  } else {
    strm << device << ".pipe_control( :context=>" << ctx_name
         << ", :dataCacheFlush => false )" << std::endl;
  }

  return strm.good();
}

bool GsfWriter::GenGSFComputeTail(std::ofstream &strm, char const *ctx_name) {
  strm << "Device.dispatch( :context => " << ctx_name << ", :sync => 'poll' )"
       << std::endl;

  return strm.good();
}

bool GsfWriter::genGSFOutSurf(std::ofstream &strm, ZeBuffer *buf) {
  if (buf->isStateless() && buf->size() > CM_BUFFER_STATELESS_LARGE_SIZE) {
    assert(0 && "Not tested");
    auto bufSize = buf->size();
    size_t numMemoryBuffers =
        (bufSize % CM_BUFFER_STATELESS_LARGE_SIZE)
            ? (bufSize / CM_BUFFER_STATELESS_LARGE_SIZE + 1)
            : (bufSize / CM_BUFFER_STATELESS_LARGE_SIZE);
    std::string outSurfName = buf->getOutputBufferName();
    outSurfName.insert(outSurfName.length() - 4, ".#{i}");
    strm << numMemoryBuffers << ".times {|i| ";
    strm << buf->getNameAsGritsObj();
    strm << "[i]"
         << ".save_to_file(\"" << outSurfName.c_str() << "\")}" << std::endl;
  } else {
    strm << buf->getNameAsGritsObj() << ".save_to_file(\""
         << buf->getOutputBufferName().c_str() << "\")" << std::endl;
  }
  return strm.good();
}

bool GsfWriter::GenerateBtiInGsf(std::ofstream &strm, char const *ctx_name,
                                 ZeBuffer *buf, unsigned int bti) {
  std::string bufferName =
      "buffer_" + std::to_string(buf->getBufferId()) + "_" + ctx_name;
  strm << "bindingTable_" << ctx_name << ".insert(";
  strm << bufferName << ", " << bti << ")\n" << std::endl;

  return strm.good();
}
} // namespace gsf
} // namespace __zert__
