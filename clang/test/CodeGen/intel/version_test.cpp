// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -O0 -emit-llvm %s -o - \
// RUN:  -debug-info-kind=limited -dwarf-version=4 \
// RUN:  | FileCheck %s

// CHECK: constant {{.*}} c"{{[1-9][0-9]*[\.][0-9]+[\.][0-9]+}}
// CHECK-SAME: (icx {{[0-9\.]+}})

// CHECK-LABEL: foo
int foo()
{
  const char *c = __clang_version__;
  return c[0];
}

// CHECK: !llvm.ident = !{[[I:![0-9]+]]}
// CHECK: !DICompileUnit{{.*}}producer: "clang based
// CHECK: [[I]] = {{.*}}icx
