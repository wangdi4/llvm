// Test to ensure the opt level is passed down to the ThinLTO backend.
// REQUIRES: x86-registered-target

// RUN: %clang_cc1 -O2 -o %t.o -flto=thin -triple x86_64-unknown-linux-gnu -emit-llvm-bc %s
// RUN: llvm-lto -thinlto -o %t %t.o

// INTEL_CUSTOMIZATION
// LoopVectorizer is not enabled by default.
// RUN: %clang_cc1 -fno-legacy-pass-manager -triple x86_64-unknown-linux-gnu -emit-obj -O2 -mllvm -enable-lv -o %t2.o -x ir %t.o -fthinlto-index=%t.thinlto.bc -fdebug-pass-manager 2>&1 | FileCheck %s --check-prefix=O2
// END INTEL_CUSTOMIZATION
// O2: Running pass: LoopVectorizePass

// RUN: %clang_cc1 -O0 -o %t.o -flto=thin -triple x86_64-unknown-linux-gnu -emit-llvm-bc %s
// RUN: llvm-lto -thinlto -o %t %t.o

// INTEL_CUSTOMIZATION
// RUN: %clang_cc1 -fno-legacy-pass-manager -triple x86_64-unknown-linux-gnu -emit-obj -O0 -mllvm -enable-lv -o %t2.o -x ir %t.o -fthinlto-index=%t.thinlto.bc -fdebug-pass-manager 2>&1 | FileCheck %s --check-prefix=O0
// END INTEL_CUSTOMIZATION
// O0-NOT: Running pass: LoopVectorizePass

void foo(void) {
}
