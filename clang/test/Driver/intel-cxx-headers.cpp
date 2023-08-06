// Various tests that verify that the driver will properly fixup the directory
// structure for cases where there are multiple gcc installations and one of
// the installs does not have a full C++ installation.

// REQUIRES: system-linux

// RUN: %clangxx --gcc-toolchain=%S/Inputs/intel_linux_cxx_tree -### %s 2>&1 \
// RUN:  | FileCheck %s --check-prefix=CXX10
// CXX10: clang{{.*}} "{{.*}}/include/c++/10"
// CXX10: ld{{.*}} "{{.*}}lib/gcc/x86_64-linux-gnu/10"

// RUN: not %clangxx --gcc-toolchain=%S/Inputs/intel_linux_tree/usr -### %s 2>&1 \
// RUN:  | FileCheck %s --check-prefix=NO_CXX_ERROR
// NO_CXX_ERROR: error: C++ header location not resolved; check installed C++ dependencies

// RUN: %clang --gcc-toolchain=%S/Inputs/intel_linux_cxx_tree -### %s 2>&1 \
// RUN:  | FileCheck %s --check-prefix=C11
// C11: ld{{.*}} "{{.*}}lib{{(64)*}}/gcc/x86_64-linux-gnu/11"
