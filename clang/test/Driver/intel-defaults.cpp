// loopopt settings
// RUN: %clang -### -fsycl -fsycl-targets=spir64-unknown-unknown -fsycl-device-only --intel -c %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT %s
// RUN: %clang_cl -### -fsycl -fsycl-targets=spir64-unknown-unknown -fsycl-device-only --intel -c %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT %s
// CHECK-INTEL-LOOPOPT-NOT: "-mllvm" "-loopopt={{.*}}"
// CHECK-INTEL-LOOPOPT-NOT: "-floopopt-pipeline={{.*}}"

// RUN: %clang -### -fsycl -fsycl-targets=spir64-unknown-unknown -fsycl-device-only --intel -mllvm -loopopt=1 -xAVX2 -c %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT1 %s
// RUN: %clang_cl -### -fsycl -fsycl-targets=spir64-unknown-unknown -fsycl-device-only --intel -mllvm -loopopt=1 -QxAVX2 -c %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT1 %s
// CHECK-INTEL-LOOPOPT1: "-mllvm" "-loopopt=1"
// CHECK-INTEL-LOOPOPT1-NOT: "-floopopt-pipeline={{.*}}"
