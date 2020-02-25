// RUN: %clang_cc1 -fhls -emit-llvm -o - %s | FileCheck %s

// "plain" loop_fuse
// CHECK: define{{.*}}foo0
void foo0(float* A, float* B, float* C) {
  //CHECK: [[TOK0:%[0-9]+]] = call token{{.*}}region.entry()
  //CHECK-SAME: [ "DIR.PRAGMA.FUSE"() ]
  //CHECK: region.exit(token [[TOK0]]) [ "DIR.PRAGMA.END.FUSE"() ]
  #pragma loop_fuse
  {
    for (int i = 0; i < 1024; ++i)
      C[i] += A[i];
    for (int j = 0; j < 1024; ++j)
      C[j] += B[j];
  }
}

// loop_fuse with depth specified
// CHECK: define{{.*}}foo1
void foo1(float* A, float* B, float* C) {
  //CHECK: [[TOK0:%[0-9]+]] = call token{{.*}}region.entry()
  //CHECK-SAME: [ "DIR.PRAGMA.FUSE"(),
  //CHECK-SAME: "QUAL.PRAGMA.FUSE.DISTANCE"(i32 2) ]
  //CHECK: region.exit(token [[TOK0]]) [ "DIR.PRAGMA.END.FUSE"() ]
  #pragma loop_fuse depth(2)
  {
    for (int i = 0; i < 1024; ++i)
      C[i] += A[i];
    for (int j = 0; j < 1024; ++j)
      C[j] += B[j];
  }
}

// loop_fuse with independent specified
// CHECK: define{{.*}}foo2
void foo2(float* A, float* B, float* C) {
  //CHECK: [[TOK0:%[0-9]+]] = call token{{.*}}region.entry()
  //CHECK-SAME: [ "DIR.PRAGMA.FUSE"(),
  //CHECK-SAME: "QUAL.PRAGMA.FUSE.INDEPENDENT"() ]
  //CHECK: region.exit(token [[TOK0]]) [ "DIR.PRAGMA.END.FUSE"() ]
  #pragma loop_fuse independent
  {
    for (int i = 0; i < 1024; ++i)
      C[i] += A[i];
    for (int j = 0; j < 1024; ++j)
      C[j] += B[j];
  }
}

// loop_fuse with both depth and independent specified
// CHECK: define{{.*}}foo2a
void foo2a(float* A, float* B, float* C) {
  //CHECK: [[TOK0:%[0-9]+]] = call token{{.*}}region.entry()
  //CHECK-SAME: [ "DIR.PRAGMA.FUSE"(),
  //CHECK-SAME: "QUAL.PRAGMA.FUSE.DISTANCE"(i32 2),
  //CHECK-SAME: "QUAL.PRAGMA.FUSE.INDEPENDENT"() ]
  //CHECK: region.exit(token [[TOK0]]) [ "DIR.PRAGMA.END.FUSE"() ]
  #pragma loop_fuse depth(2) independent
  {
    for (int i = 0; i < 1024; ++i)
      C[i] += A[i];
    for (int j = 0; j < 1024; ++j)
      C[j] += B[j];
  }
}
