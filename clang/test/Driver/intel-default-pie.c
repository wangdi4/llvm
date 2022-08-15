// UNSUPPORTED: system-windows

/// Check for dynamic setting of -pie based on gcc defaults
// RUN: env PATH=%S/Inputs/intel/bin:$PATH \
// RUN: %clangxx --intel -target x86_64-unknown-linux-gnu -### %s 2>&1 \
// RUN:  | FileCheck %s
// CHECK: "{{.*}}crtbeginS.o"
