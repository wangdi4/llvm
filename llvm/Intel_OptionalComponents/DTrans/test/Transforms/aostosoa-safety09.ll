; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -disable-output -dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s
; RUN: opt < %s -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -disable-output -passes=dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s

; This test verifies that if one of the funtions leading to the allocation
; could be used for something other than a direct function call, the type
; is disqualified.

%struct.test01 = type { i32, i64, i32 }
@func_ptr = global void (i64)* @test01wrapper

define void @test01(i64 %in) {
  %p1 = call i8* @calloc(i64 %in, i64 16)
  %p1_test = bitcast i8* %p1 to %struct.test01*
  ret void
}

define void @test01wrapper(i64 %in) {
  call void @test01(i64 %in)
  ret void
}

define void @indirect_invoker() {
  %func_addr = load void (i64)*, void (i64)** @func_ptr
  tail call void %func_addr(i64 16)
  ret void
}

; CHECK: DTRANS-AOSTOSOA: Rejecting -- Multiple call paths: struct.test01


declare i8* @calloc(i64, i64)

