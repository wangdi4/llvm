// Copyright (c) 2021 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#include <algorithm>
#include <iterator>
#include <sstream>
#include <unordered_map>

#include "CL_BASE.h"
#include "TestsHelpClasses.h"

extern cl_device_type gDeviceType;

class CheckOpenCLCFeatures : public CL_base {
protected:
  virtual void SetUp() {
    cl_int err = clGetPlatformIDs(1, &m_platform, NULL);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, NULL);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");
  }

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
};

// Reference OpenCL C features. Update this list if supported features names or
// version changes.
const static std::unordered_map<const char *, cl_version> featsRefCPU = {
    {"__opencl_c_3d_image_writes",                 CL_MAKE_VERSION(3, 0, 0)},
    {"__opencl_c_atomic_order_acq_rel",            CL_MAKE_VERSION(3, 0, 0)},
    {"__opencl_c_atomic_order_seq_cst",            CL_MAKE_VERSION(3, 0, 0)},
    {"__opencl_c_atomic_scope_device",             CL_MAKE_VERSION(3, 0, 0)},
    {"__opencl_c_atomic_scope_all_devices",        CL_MAKE_VERSION(3, 0, 0)},
    {"__opencl_c_device_enqueue",                  CL_MAKE_VERSION(3, 0, 0)},
    {"__opencl_c_generic_address_space",           CL_MAKE_VERSION(3, 0, 0)},
    {"__opencl_c_fp64",                            CL_MAKE_VERSION(3, 0, 0)},
    {"__opencl_c_images",                          CL_MAKE_VERSION(3, 0, 0)},
    {"__opencl_c_int64",                           CL_MAKE_VERSION(3, 0, 0)},
    {"__opencl_c_pipes",                           CL_MAKE_VERSION(3, 0, 0)},
    {"__opencl_c_program_scope_global_variables",  CL_MAKE_VERSION(3, 0, 0)},
    {"__opencl_c_read_write_images",               CL_MAKE_VERSION(3, 0, 0)},
    {"__opencl_c_subgroups",                       CL_MAKE_VERSION(3, 0, 0)},
    {"__opencl_c_work_group_collective_functions", CL_MAKE_VERSION(3, 0, 0)}};

static inline std::string makeVersonString(cl_version version) {
  std::ostringstream ss;
  ss << "v";
  ss << CL_VERSION_MAJOR(version);
  ss << ".";
  ss << CL_VERSION_MINOR(version);
  ss << ".";
  ss << CL_VERSION_PATCH(version);
  return ss.str();
}

static bool operator==(const cl_name_version &lhs, const cl_name_version &rhs) {
  return strcmp(lhs.name, rhs.name) == 0 && lhs.version == rhs.version;
}

TEST_F(CheckOpenCLCFeatures, CpuDevice) {
  size_t retSize;
  size_t numOfExts;
  cl_int err = clGetDeviceInfo(m_device, CL_DEVICE_OPENCL_C_FEATURES, 0,
                               nullptr, &retSize);

  // CL_DEVICE_OPENCL_C_FEATURES is missing before OpenCL 3.0
  if (m_version < OPENCL_VERSION::OPENCL_VERSION_3_0) {
    ASSERT_OCL_EQ(err, CL_INVALID_VALUE, clGetDeviceInfo);
    return;
  }

  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo");

  numOfExts = retSize / sizeof(cl_name_version);
  std::vector<cl_name_version> featsCPU(numOfExts);
  err = clGetDeviceInfo(m_device, CL_DEVICE_OPENCL_C_FEATURES, retSize,
                        featsCPU.data(), nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo");

  // Check each queried features is present in reference features
  std::unordered_map<const char *, cl_version> featsRef(featsRefCPU);
  for (auto feat : featsCPU) {
    ASSERT_EQ(featsRef.count(feat.name), 1)
        << ("Expect " + std::string(feat.name) + "(" +
            makeVersonString(feat.version) +
            ") exists once in reference OpenCL C features");
    if (feat.version == featsRef[feat.name])
      featsRef.erase(feat.name);
  }

  // Check featsRef is empty.
  if (!featsRef.empty()) {
    std::ostringstream ss;
    for (auto feat : featsRef)
      ss << feat.first << "(" << makeVersonString(feat.second) << "), ";

    FAIL() << ("Reference OpenCL C features " + ss.str() +
               "are not in features queried from clGetDeviceInfo");
  }
}
