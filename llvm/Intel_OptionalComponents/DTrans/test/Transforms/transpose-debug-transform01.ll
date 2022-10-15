; REQUIRES: asserts
; RUN: opt < %s -disable-output -dtrans-transpose -debug-only=dtrans-transpose-transform 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -passes=dtrans-transpose -debug-only=dtrans-transpose-transform 2>&1 | FileCheck %s

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

; Too few dimensions to be a candidate
@test_var1 = internal global [10 x i32] zeroinitializer

; Should be a candidate
@test_var2 = internal global [9 x [9 x i64]] zeroinitializer

; Element type is pointer. Should not be a candidate
@test_var3 = internal global [9 x [9 x i64*]] zeroinitializer

; Try more dimensions. Should be a candidate
@test_var4 = internal global [9 x [9 x [9 x i32]]] zeroinitializer

; Not all dimensions match. Should be a candidate
@test_var5 = internal global [9 x [8 x [9 x i32]]] zeroinitializer

; Non-zero initialization. Should not be a candidate.
@test_var6 = internal global [2 x [2 x i32]]
 [[2 x i32] [i32 0, i32 1], [2 x i32] [i32 2, i32 3]]

; Non-internal linkage. Should not be a candidate
@test_var7 = global [9 x [9 x i64*]] zeroinitializer

; Base type is float. Should be a candidate
@test_var8 = internal global [10 x [9 x float]] zeroinitializer

; Base type is double. Should be a candidate
@test_var9 = internal global [10 x [9 x double]] zeroinitializer

; Not all dimensions large enough. Should not be a candidate
@test_var10 = internal global [9 x [6 x [9 x i32]]] zeroinitializer

 define void @test01() {
   %v1ptr = getelementptr [10 x i32], [10 x i32]* @test_var1, i64 0, i32 0
   %v2ptr = getelementptr [9 x [9 x i64]], [9 x [9 x i64]]* @test_var2, i64 0, i32 0
   %v3ptr = getelementptr  [9 x [9 x i64*]],  [9 x [9 x i64*]]* @test_var3, i64 0, i32 0
   %v4ptr = getelementptr [9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @test_var4, i64 0, i32 0
   %v5ptr = getelementptr [9 x [8 x [9 x i32]]], [9 x [8 x [9 x i32]]]* @test_var5, i64 0, i32 0
   %v6ptr = getelementptr [2 x [2 x i32]], [2 x [2 x i32]]* @test_var6, i64 0, i32 0
   %v7ptr = getelementptr  [9 x [9 x i64*]],  [9 x [9 x i64*]]* @test_var7, i64 0, i32 0
   %v8ptr = getelementptr  [10 x [9 x float]],  [10 x [9 x float]]* @test_var8, i64 0, i32 0
   %v9ptr = getelementptr  [10 x [9 x double]],  [10 x [9 x double]]* @test_var9, i64 0, i32 0
   %v10ptr = getelementptr [9 x [6 x [9 x i32]]], [9 x [6 x [9 x i32]]]* @test_var10, i64 0, i32 0
   ret void
 }

