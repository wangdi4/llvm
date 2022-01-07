/// Test --intel-config behaviors
// RUN: %clang -### --intel-config %S/Inputs/intel.cfg -c %s 2>&1 \
// RUN:  | FileCheck %s -check-prefix=INTEL_CONFIG
// RUN: %clang_cl -### --intel-config %S/Inputs/intel.cfg -c %s 2>&1 \
// RUN:  | FileCheck %s -check-prefix=INTEL_CONFIG
// RUN: %clangxx -### --intel-config %S/Inputs/intel.cfg -c %s 2>&1 \
// RUN:  | FileCheck %s -check-prefix=INTEL_CONFIG
// INTEL_CONFIG: "-D" "INTEL_CFG"

// RUN: %clang -### --intel-config %S/Inputs/intel.cfg --config %S/Inputs/intel-dummy.cfg -c %s 2>&1 \
// RUN:  | FileCheck %s -check-prefixes=NO_INTEL_CONFIG,CONFIG
// CONFIG: "-D" "INTEL_DUMMY_CFG"
// NO_INTEL_CONFIG-NOT: "-D" "INTEL_CFG"
