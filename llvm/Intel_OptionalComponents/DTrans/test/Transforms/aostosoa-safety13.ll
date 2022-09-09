; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s

; This test is to verify the checks on uses of the pointer returned by the
; allocation call for types being transformed. In this case, the pointer is
; bitcast and used directly to access memory.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01 = type { i32, i32, i64 }
; CHECK: DTRANS-AOSTOSOA: Rejecting -- {{.*}}: struct.test01

@g_test01ptr = internal unnamed_addr global %struct.test01* zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01(i64 15, i8* null)
  ret i32 0
}

define void @test01(i64 %num, i8* %other) {
  %mem1 = call i8* @malloc(i64 160)

  %st_mem1 = bitcast i8* %mem1 to %struct.test01*
  store %struct.test01* %st_mem1, %struct.test01** @g_test01ptr

  ; Attempt to access the memory allocation directly via the pointer
  ; returned from malloc. This will pass the safety checks, but
  ; cannot be handled for AOS-to-SOA.
  %cast32 = bitcast i8* %mem1 to i32*
  %val1 = load i32, i32* %cast32

  ret void
}

declare i8* @malloc(i64)
