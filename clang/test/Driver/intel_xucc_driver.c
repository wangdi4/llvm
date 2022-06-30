#if INTEL_FEATURE_XUCC
// REQUIRES: intel_feature_xucc
// Check if the target-cpu is x86-64, if the x86 specific options can be recognized, and use the integrated assembler(llvm-mc) as default.
// RUN: %clang -target x86_64_xucc-unknown-linux-gnu -mavx512f -c -### %s 2>&1 | FileCheck %s
// CHECK: "-target-cpu" "x86-64" "-target-feature" "+avx512f"
// CHECK-NOT: "-fno-integrated-as"

// RUN: touch %t.o
// RUN: %clang -target x86_64-unknown-linux --dyld-prefix /foo -### %t.o 2>&1 | FileCheck --check-prefix=CHECK-LD %s
// CHECK-LD: "-dynamic-linker" "/foo{{(/usr/x86_64-unknown-linux)?}}/lib{{(64)?}}/ld-linux-x86-64.so.2"

#endif // INTEL_FEATURE_XUCC
