// UNSUPPORTED: system-windows

/// Check for dynamic setting of -pie based on gcc defaults
// RUN: env PATH=%S/Inputs/intel/bin:$PATH \
// RUN: %clangxx --intel -target x86_64-unknown-linux-gnu -### %s 2>&1 \
// RUN:  | FileCheck %s
// CHECK: "{{.*}}Scrt1.o"
// CHECK: "{{.*}}crtbeginS.o"
// CHECK: "{{.*}}crtendS.o"

// RUN: env PATH=%S/Inputs/intel/bin:$PATH \
// RUN: %clangxx --intel -target x86_64-unknown-linux-gnu -static -### %s 2>&1 \
// RUN:  | FileCheck -check-prefix=STATIC %s
// STATIC: "-static"
// STATIC-NOT: "{{.*}}Scrt1.o"
// STATIC-NOT: "{{.*}}crtbeginS.o"
// STATIC-NOT: "{{.*}}crtendS.o"
