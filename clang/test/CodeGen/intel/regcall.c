// RUN: %clang_cc1 %s -triple %itanium_abi_triple -emit-llvm -fms-extensions -fintel-compatibility -o - | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-windows-msvc -emit-llvm %s -fms-extensions -fintel-compatibility -o - | FileCheck %s

/* Function declarations. */
__declspec(noinline) double __regcall foo(int bar);
double __attribute__((regcall)) foo2(int bar);

int main(int argc, char *argv[]){
  return foo (argc) - foo2 (argc);
}

// CHECK-LABEL: define {{.*}}main
// CHECK: call x86_regcallcc {{.*}}foo
// CHECK: call x86_regcallcc {{.*}}foo2

__declspec(noinline) double __regcall foo(int bar){
  return (double)bar;
}

// CHECK-LABEL: define x86_regcallcc{{.*}}foo

double __attribute__((regcall)) foo2(int bar){
  return bar;
}

// CHECK-LABEL: define x86_regcallcc{{.*}}foo2
