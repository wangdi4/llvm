// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -O0 -emit-llvm %s -o - \
// RUN:  -debug-info-kind=limited -dwarf-version=4 \
// RUN:  | FileCheck %s

// CHECK: constant {{.*}} c"{{[1-9][0-9]*[\.][0-9]+[\.][0-9]+}}
// CHECK-SAME: (icx {{.*}})

// CHECK-LABEL: foo
int foo()
{
  constexpr int intel_llvm = __INTEL_LLVM_COMPILER;
  constexpr int intel_clang = __INTEL_CLANG_COMPILER;
  static_assert(intel_llvm>0, "INTEL LLVM COMPILER version is incorrect");
  static_assert(intel_clang>0, "INTEL CLANG COMPILER version is incorrect");
  const char *c = __clang_version__;
  return c[0];
}

// CHECK: !llvm.ident = !{[[I:![0-9]+]]}
// CHECK: !DICompileUnit{{.*}}producer: "clang based
// CHECK: [[I]] = {{.*}}DPC++
