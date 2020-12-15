// RUN: %clang_cc1 -fintel-compatibility -mllvm -enable-subscript-lowering=0 -emit-llvm -o - %s | FileCheck %s
// expected-no-diagnostics

void foo() {
  int i, s, a[10], b[10], n = 10;

  s = 0;
  i = 0;
  #pragma distribute_point
  while (i < n) {
    // CHECK: br label %{{.*}}, !llvm.loop !{{.+}}
    a[i] += 3;
    s += b[i];
  }
  i = 0;
  #pragma distribute_point
  do {
    // CHECK: br i1 %{{.*}}, !llvm.loop !{{.+}}
    a[i] += 4;
    s += b[i];
  } while (i < n);
  #pragma distribute_point
  for (i = 0; i < n; ++i) {
    // CHECK: br label %{{.*}}, !llvm.loop !{{.+}}
    a[i] += 4;
    s += b[i];
  }
  #pragma distribute_point
  for (auto j: a) {
    // CHECK: br label %{{.*}}, !llvm.loop !{{.+}}
    j += 4;
  }
  for (i = 0; i < n; ++i) {
    #pragma distribute_point
    //CHECK: [[TOK:%[0-9]+]] = call token {{.*}}region.entry() [ "DIR.PRAGMA.DISTRIBUTE_POINT"() ]
    //CHECK: region.exit(token [[TOK]]) [ "DIR.PRAGMA.END.DISTRIBUTE_POINT"() ]
    //CHECK: br label %{{.*}}, !llvm.loop !{{.+}}
    a[i] += 4;
    s += b[i];
  }
  for (i = 0; i < n; ++i) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[NO_DISTPT:.+]]
    a[i] += 4;
    s += b[i];
  }
}

// CHECK: ![[MD_DISTPT:[0-9]+]] = !{!"llvm.loop.distribute.enable"
// CHECK-NOT: ![[NO_DISTPT]] = {{.*}}![[MD_DISTPT]]
