; REQUIRES: asserts
; This test verifies initial candidate selection for the transpose
; transformation.

; RUN: opt < %s -disable-output -dtrans-transpose -debug-only=dtrans-transpose 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -passes=dtrans-transpose -debug-only=dtrans-transpose 2>&1 | FileCheck %s

; Too few dimensions to be a candidate
@test_var1 = internal global [10 x i32] zeroinitializer

; Should be a candidate
@test_var2 = internal global [9 x [9 x i64]] zeroinitializer

; Element type is pointer, should not be a candidate
@test_var3 = internal global [9 x [9 x i64*]] zeroinitializer

; Try more dimensions. Should be a candidate
@test_var4 = internal global [9 x [9 x [9 x i32]]] zeroinitializer

; Not all dimensions match, should not be a candidate
@test_var5 = internal global [9 x [6 x [9 x i32]]] zeroinitializer

; non-zero initialization. should not be a candidate.
@test_var6 = internal global [2 x [2 x i32]]
 [[2 x i32] [i32 0, i32 1], [2 x i32] [i32 2, i32 3]]

; non-internal linkage. should not be a candidate
@test_var7 = global [9 x [9 x i64*]] zeroinitializer

 define void @test01() {
   %v1ptr = getelementptr [10 x i32], [10 x i32]* @test_var1, i64 0, i32 0
   %v2ptr = getelementptr [9 x [9 x i64]], [9 x [9 x i64]]* @test_var2, i64 0, i32 0
   %v3ptr = getelementptr  [9 x [9 x i64*]],  [9 x [9 x i64*]]* @test_var3, i64 0, i32 0
   %v4ptr = getelementptr [9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @test_var4, i64 0, i32 0
   %v5ptr = getelementptr [9 x [6 x [9 x i32]]], [9 x [6 x [9 x i32]]]* @test_var5, i64 0, i32 0
   %v6ptr = getelementptr [2 x [2 x i32]], [2 x [2 x i32]]* @test_var6, i64 0, i32 0
   %v7ptr = getelementptr  [9 x [9 x i64*]],  [9 x [9 x i64*]]* @test_var7, i64 0, i32 0
   ret void
 }

; CHECK-NOT: Adding candidate: @test_var1
; CHECK: Adding candidate: @test_var2
; CHECK-NOT: Adding candidate:@test_var3
; CHECK: Adding candidate: @test_var4
; CHECK-NOT: Adding candidate: @test_var5
; CHECK-NOT: Adding candidate: @test_var6
; CHECK-NOT: Adding candidate: @test_var7
