// RUN: %clang_cc1 -fintel-compatibility %s -emit-llvm -o - | FileCheck %s
int foo() {
  char buf[5] = {0,0,0};
// CHECK: call void @llvm.memset.p0i8.i64(i8* %{{.+}}, i8 0, i64 5, i32 1, i1 false)
  memset(buf, 10, 5);
// CHECK: call i8* @memset(i8* %{{.+}}, i32 10, i64 5)
  char dst_buf[3] = {1,1,1};
  memmove(dst_buf, buf, 3);
// CHECK: call i8* @memmove(i8* %{{.+}}, i8* %{{.+}}, i64 3)
  memcpy(dst_buf, buf, 3);
// CHECK: call i8* @memcpy(i8* %{{.+}}, i8* %{{.+}}, i64 3)
  return 0;
}
