// INTEL CONFIDENTIAL
//
// Copyright 2018 Intel Corporation.
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

#include "cl_sys_info.h"

#include <driverversion.h>
#include <hwloc.h>

#include "llvm/Support/Threading.h"

// OCL product version string will change with each commit, it will thwart
// build-same check. So we put it in a specific section and then ignore this
// section when doing build-same check.
#ifdef _WIN32
#pragma section(".oclver", read)
__declspec(allocate(".oclver"))
#else
__attribute__((section(".oclver")))
#endif
    const char OCL_VER_EXT[] = VERSIONSTRING_WITH_EXT;
static hwloc_topology_t topology = nullptr;

////////////////////////////////////////////////////////////////////////////////
// return the product version:
// Arguments - year, LLVM version, month, digit (0) - output version numbers
///////////////////////////////////////////////////////////////////////////////
const char *Intel::OpenCL::Utils::GetModuleProductVersion() {
  return OCL_VER_EXT;
}

///////////////////////////////////////////////////////////////////////////////////////////
// load hardware infomation
////////////////////////////////////////////////////////////////////
void Intel::OpenCL::Utils::InitHwloc() {
  static llvm::once_flag OnceFlag;
  llvm::call_once(OnceFlag, [&]() {
    // initialize topology for hwloc
    [[maybe_unused]] int err = hwloc_topology_init(&topology);
    assert(!err && "hwloc_topology_init failed");

    // filter everything out
    err = hwloc_topology_set_all_types_filter(topology,
                                              HWLOC_TYPE_FILTER_KEEP_NONE);
    assert(!err && "hwloc_topology_set_all_types_filter failed");

    // filter GROUP back in
    err = hwloc_topology_set_type_filter(topology, HWLOC_OBJ_GROUP,
                                         HWLOC_TYPE_FILTER_KEEP_STRUCTURE);
    assert(!err && "Enable HWLOC_OBJ_GROUP failed");

    // filter Package back in
    err = hwloc_topology_set_type_filter(topology, HWLOC_OBJ_PACKAGE,
                                         HWLOC_TYPE_FILTER_KEEP_ALL);
    assert(!err && "Enable HWLOC_OBJ_PACKAGE failed");

    // filter core back in
    err = hwloc_topology_set_type_filter(topology, HWLOC_OBJ_CORE,
                                         HWLOC_TYPE_FILTER_KEEP_ALL);
    assert(!err && "Enable HWLOC_OBJ_CORE failed");

    // disable distances, memory attributes and CPU kinds
    err =
        hwloc_topology_set_flags(topology, HWLOC_TOPOLOGY_FLAG_NO_DISTANCES |
                                               HWLOC_TOPOLOGY_FLAG_NO_MEMATTRS |
                                               HWLOC_TOPOLOGY_FLAG_NO_CPUKINDS);
    assert(!err && "hwloc_topology_set_flags failed");

    // disable componentes, do not check return value
    // because function failed when hwloc do not check the component
    hwloc_topology_set_components(
        topology, HWLOC_TOPOLOGY_COMPONENTS_FLAG_BLACKLIST, "xml");
    hwloc_topology_set_components(
        topology, HWLOC_TOPOLOGY_COMPONENTS_FLAG_BLACKLIST, "synthetic");

    // load hardware information
    err = hwloc_topology_load(topology);
    assert(!err && "hwloc_topology_load failed");
  });
}

///////////////////////////////////////////////////////////////////////////////////////////
// destroy topology
////////////////////////////////////////////////////////////////////
void Intel::OpenCL::Utils::DestroyHwloc() {
  if (topology) {
    hwloc_topology_destroy(topology);
    topology = nullptr;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////
// return the number of physical cpus (sockets) configured.
////////////////////////////////////////////////////////////////////
unsigned int Intel::OpenCL::Utils::GetNumberOfCpuSockets() {
  static unsigned int numCpuSockets = 0;
  static llvm::once_flag OnceFlag;
  llvm::call_once(OnceFlag, [&]() {
    InitHwloc();
    int depth = hwloc_get_type_depth(topology, HWLOC_OBJ_PACKAGE);
    if (depth == HWLOC_TYPE_DEPTH_UNKNOWN) {
      assert(false && "Failed to detect the number of sockets!");
    } else
      numCpuSockets = hwloc_get_nbobjs_by_depth(topology, depth);
  });
  assert(numCpuSockets != 0 && "Number of sockets should not be 0");
  return numCpuSockets;
}

///////////////////////////////////////////////////////////////////////////////////////////
// return whether cpu is using hyper-threading
////////////////////////////////////////////////////////////////////
bool Intel::OpenCL::Utils::IsHyperThreadingEnabled() {
  static bool hyperThreadingEnabled = false;
  static llvm::once_flag OnceFlag;
  llvm::call_once(OnceFlag, [&]() {
    InitHwloc();
    unsigned int cpucores = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_CORE);
    assert(cpucores != 0 && "Number of cores should not be 0");
    unsigned int PUs = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_PU);
    assert(PUs != 0 && "Number of PUs should not be 0");

    // hyper-threading/SMT is Enabled if the number of PU is larger than core
    hyperThreadingEnabled = (PUs == 2 * cpucores);
  });
  return hyperThreadingEnabled;
}

///////////////////////////////////////////////////////////////////////////////////////////
// return the number of NUMA nodes on the system
////////////////////////////////////////////////////////////////////
unsigned long Intel::OpenCL::Utils::GetMaxNumaNode() {
  static int numNodes = 0;
  static llvm::once_flag OnceFlag;
  llvm::call_once(OnceFlag, [&]() {
    InitHwloc();
    numNodes = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_NUMANODE);
  });
  assert(numNodes > 0 && "Failed to get number of NUMA nodes");
  return numNodes;
}

///////////////////////////////////////////////////////////////////////////////////////////
// return an index representing the processors in a given NUMA node
////////////////////////////////////////////////////////////////////
bool Intel::OpenCL::Utils::GetProcessorIndexFromNumaNode(
    unsigned long node, std::vector<cl_uint> &index) {
  InitHwloc();
  // If no object exists, NULL is returned
  hwloc_obj_t obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_NUMANODE, node);
  if (NULL == obj)
    return false;
  index.clear();
  int prev = hwloc_bitmap_next(obj->cpuset, -1);
  while (-1 != prev) {
    index.push_back(prev);
    prev = hwloc_bitmap_next(obj->cpuset, prev);
  }
  return true;
}

std::unordered_map<int, int> Intel::OpenCL::Utils::GetProcessorToSocketMap() {
  unsigned numSockets = GetNumberOfCpuSockets();
  std::unordered_map<int, int> CoreIdToPhysicalId;
  for (unsigned node = 0; node < numSockets; node++) {
    // If no object exists, NULL is returned
    hwloc_obj_t obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_PACKAGE, node);
    assert(obj && "Get Socket Object Failed!");
    int prev = hwloc_bitmap_next(obj->cpuset, -1);
    while (-1 != prev) {
      CoreIdToPhysicalId[prev] = node;
      prev = hwloc_bitmap_next(obj->cpuset, prev);
    }
  }
  return CoreIdToPhysicalId;
}
