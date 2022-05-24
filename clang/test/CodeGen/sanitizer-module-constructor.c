<<<<<<< HEAD
// INTEL_CUSTOMIZATION
// RUN: %clang_cc1 -fexperimental-new-pass-manager -triple x86_64-linux-gnu -fsanitize=address -emit-llvm -O3 -fdebug-pass-manager -o - %s 2>&1 | FileCheck %s
// RUN: %clang_cc1 -fexperimental-new-pass-manager -triple x86_64-linux-gnu -fsanitize=thread -emit-llvm -O3 -fdebug-pass-manager -o - %s 2>&1 | FileCheck %s
// RUN: %clang_cc1 -fexperimental-new-pass-manager -triple x86_64-linux-gnu -fsanitize=memory -emit-llvm -O3 -fdebug-pass-manager -o - %s 2>&1 | FileCheck %s
// end INTEL_CUSTOMIZATION
=======
// RUN: %clang_cc1 -triple x86_64-linux-gnu -fsanitize=address -O3 -emit-llvm -fdebug-pass-manager -o - %s 2>&1 | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-linux-gnu -fsanitize=thread -O3 -emit-llvm -fdebug-pass-manager -o - %s 2>&1 | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-linux-gnu -fsanitize=memory -O3 -emit-llvm -fdebug-pass-manager -o - %s 2>&1 | FileCheck %s
>>>>>>> 4f89ff3fc71b0b2adfeb74b900e9a2a90ef80174

// This is regression test for PR42877

typedef struct a *b;
struct a {
  int c;
};
int d;
b e;
static void f(b g) {
  for (d = g->c;;)
    ;
}
void h(void) { f(e); }

// CHECK: Running pass: {{.*}}SanitizerPass
// CHECK-NOT: Running pass: LoopSimplifyPass on {{.*}}san.module_ctor
// CHECK: Running analysis: DominatorTreeAnalysis on {{.*}}san.module_ctor
