// RUN: env ICXCFG=%S/Inputs/intel.cfg \
// RUN: %clang -### --intel --config %S/Inputs/intel-dummy.cfg -c %s 2>&1 \
// RUN:  | FileCheck %s -check-prefixes=NO_INTEL_CONFIG,CONFIG
// CONFIG: "-D" "INTEL_DUMMY_CFG"
// NO_INTEL_CONFIG-NOT: "-D" "INTEL_CFG"

/// Test CFG environment variable behaviors
// RUN: env ICXCFG=%S/Inputs/intel.cfg \
// RUN: %clang -### --intel -c %s 2>&1 \
// RUN:  | FileCheck %s -check-prefix=INTEL_CONFIG
// RUN: env ICXCLCFG=%S/Inputs/intel.cfg \
// RUN: %clang_cl -### --intel -c %s 2>&1 \
// RUN:  | FileCheck %s -check-prefix=INTEL_CONFIG
// RUN: env ICPXCFG=%S/Inputs/intel-icpx.cfg \
// RUN: %clangxx -### --intel -c %s 2>&1 \
// RUN:  | FileCheck %s -check-prefix=INTEL_ICPX_CONFIG
// RUN: env DPCPPCFG=%S/Inputs/intel-dpcpp.cfg \
// RUN: %clangxx -### --dpcpp -c %s 2>&1 \
// RUN:  | FileCheck %s -check-prefix=INTEL_DPCPP_CONFIG
// INTEL_CONFIG: "-D" "INTEL_CFG"
// INTEL_ICPX_CONFIG: "-D" "INTEL_ICPX_CFG"
// INTEL_DPCPP_CONFIG: "-D" "INTEL_DPCPP_CFG"
