// -fast -xOPT settings
// RUN: %clang -### --intel -c -fast -xSSE4.2 %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-FAST-X %s
// RUN: %clang_cl -### --intel -c -fast -QxSSE4.2 %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-FAST-X %s
// CHECK-INTEL-FAST-X: "-flto=full" "-flto-unit"
// CHECK-INTEL-FAST-X: "-target-cpu" "corei7"
// CHECK-INTEL-FAST-X: "-O3"
// CHECK-INTEL-FAST-X: "-mllvm" "-loopopt"

// -fast -xOPT should not generate duplicate entries
// RUN: %clang_cl -### --intel -fast -QxSSE4.2 %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-FAST-NODUP %s
// CHECK-INTEL-FAST-NODUP: "-mllvm:-mcpu=corei7"
// CHECK-INTEL-FAST-NODUP-NOT: "-mllvm:-enable-npm-multiversioning" "-mllvm:-enable-npm-multiversioning"

