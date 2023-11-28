// Use of -fplugin is not guaranteed to work.
// RUN: %clangxx --target=x86_64-unknown-linux-gnu -c -fplugin=dummyarg \
// RUN:          %s -### 2>&1 \
// RUN:  | FileCheck %s
// CHECK: Use of '-fplugin=dummyarg' is not guaranteed to work and may lead to unexpected results.
// CHECK: clang{{.*}} "-load" "dummyarg"

// RUN: %clangxx --target=x86_64-unknown-linux-gnu -c -fplugin=dummyarg \
// RUN:          -Wno-plugin-behavior %s -### 2>&1 \
// RUN:  | FileCheck -check-prefix NO_DIAG %s
// NO_DIAG-NOT: Use of '-fplugin=dummyarg' is not guaranteed to work and may lead to unexpected results.
