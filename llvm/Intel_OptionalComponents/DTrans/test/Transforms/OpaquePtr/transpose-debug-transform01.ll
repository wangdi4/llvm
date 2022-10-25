; REQUIRES: asserts
; RUN: opt -opaque-pointers < %s -disable-output -dtrans-transpose -debug-only=dtrans-transpose-transform 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers < %s -disable-output -passes=dtrans-transpose -debug-only=dtrans-transpose-transform 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Check that no subscripts are transformed, because there are none.

; CHECK-LABEL: Transform candidate: test_var2
; CHECK-NOT: Before
; CHECK-NOT: After
; CHECK-LABEL: Transform candidate: test_var4
; CHECK-NOT: Before
; CHECK-NOT: After
; CHECK-LABEL: Transform candidate: test_var5
; CHECK-NOT: Before
; CHECK-NOT: After
; CHECK-LABEL: Transform candidate: test_var8
; CHECK-NOT: Before
; CHECK-NOT: After
; CHECK-LABEL: Transform candidate: test_var9
; CHECK-NOT: Before
; CHECK-NOT: After

@test_var1 = internal global [10 x i32] zeroinitializer
@test_var2 = internal global [9 x [9 x i64]] zeroinitializer
@test_var3 = internal global [9 x [9 x ptr]] zeroinitializer
@test_var4 = internal global [9 x [9 x [9 x i32]]] zeroinitializer
@test_var5 = internal global [9 x [8 x [9 x i32]]] zeroinitializer
@test_var6 = internal global [2 x [2 x i32]] [[2 x i32] [i32 0, i32 1], [2 x i32] [i32 2, i32 3]]
@test_var7 = global [9 x [9 x ptr]] zeroinitializer
@test_var8 = internal global [10 x [9 x float]] zeroinitializer
@test_var9 = internal global [10 x [9 x double]] zeroinitializer
@test_var10 = internal global [9 x [6 x [9 x i32]]] zeroinitializer

define void @test01() {
bb:
  %v1ptr = getelementptr [10 x i32], ptr @test_var1, i64 0, i32 0
  %v2ptr = getelementptr [9 x [9 x i64]], ptr @test_var2, i64 0, i32 0
  %v3ptr = getelementptr [9 x [9 x ptr]], ptr @test_var3, i64 0, i32 0
  %v4ptr = getelementptr [9 x [9 x [9 x i32]]], ptr @test_var4, i64 0, i32 0
  %v5ptr = getelementptr [9 x [8 x [9 x i32]]], ptr @test_var5, i64 0, i32 0
  %v6ptr = getelementptr [2 x [2 x i32]], ptr @test_var6, i64 0, i32 0
  %v7ptr = getelementptr [9 x [9 x ptr]], ptr @test_var7, i64 0, i32 0
  %v8ptr = getelementptr [10 x [9 x float]], ptr @test_var8, i64 0, i32 0
  %v9ptr = getelementptr [10 x [9 x double]], ptr @test_var9, i64 0, i32 0
  %v10ptr = getelementptr [9 x [6 x [9 x i32]]], ptr @test_var10, i64 0, i32 0
  ret void
}
