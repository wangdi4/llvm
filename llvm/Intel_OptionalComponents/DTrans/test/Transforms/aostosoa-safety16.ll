; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s

; This test verifies that AOS-to-SOA does not transform a structure which has
; the first element directly accessed via a bitcast of pointer to the structure.
; This could be handled as safe, but would require recognizing and transforming
; the load instruction that is not based on a GEP address, which is currently
; not supported.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01 = type { i32, i32, i32, %struct.test01*, %struct.test01*, i8* }
; CHECK: DTRANS-AOSTOSOA: Rejecting -- {{.*}}: struct.test01

@g_test01ptr = internal unnamed_addr global %struct.test01* zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  %alloc01 = call i8* @calloc(i64 10, i64 480)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*

  %tmp = call i32 @test01(i64 1)
  ret i32 0
}

define i32 @test01(i64 %idx1) {
  %base = load %struct.test01*, %struct.test01** @g_test01ptr

  ; The following access to the structure field is not supported by AOS-to-SOA.
  %x_addr = bitcast %struct.test01* %base to i32*
  %res = load i32, i32* %x_addr
  ret i32 %res
}

declare i8* @calloc(i64, i64)
