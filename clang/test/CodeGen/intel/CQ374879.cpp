// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -fintel-compatibility -O0 -emit-llvm %s -o - | FileCheck %s

// CHECK: [[m_init:@.+]] = private unnamed_addr constant [2 x i32] [i32 2, i32 3], align 4
// CHECK: [[s_init:@.+]] = private unnamed_addr constant %struct.anon { i32 4, i32 5 }, align 4
int main ()
{
  int n{1};
// CHECK: store i32 1, i32* {{%.+}}
  int m[2]{2, 3};
// CHECK: [[m:%.+]] = bitcast [2 x i32]* {{%.+}} to i8*
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* [[m]], i8* bitcast ([2 x i32]* [[m_init]] to i8*), i64 8, i32 4, i1 false)
  struct {int a, b;} s{4,5};
// CHECK: [[s:%.+]] = bitcast %struct.anon* {{%.+}} to i8*
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* [[s]], i8* bitcast (%struct.anon* [[s_init]] to i8*), i64 8, i32 4, i1 false)
  int *p = new int[2]{6,7};
// CHECK: [[call:%.+]] = call i8* {{@.+}}(i64 8)
// CHECK: [[t2:%.+]] = bitcast i8* [[call]] to i32*
// CHECK: store i32 6, i32* [[t2]], align 4
// CHECK: [[t3:%.+]] = getelementptr inbounds i32, i32* [[t2]], i64 1
// CHECK: store i32 7, i32* [[t3]], align 4
// CHECK: store i32* [[t2]], i32** {{%.+}}, align 8
  return (n != 1) | ((m[0] != 2) << 1) | ((m[1] != 3) << 2) | ((s.a != 4) << 3) |
         ((s.b != 5) << 4) | ((p[0] != 6) << 5) | ((p[1] != 7) << 6);
}


