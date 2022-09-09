; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s


; This test verifies the initialization of dependent types is handled when the
; size of the index type is changed to be 32-bits instead of the size of a
; pointer.

; The only cases were an initializer of a dependent type should occur is when
; a the pointer is being set to null or the field is a pointer to pointer,
; because global instances are not supported on the types being transformed.
; This tests the pointer-to-pointer case.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
%struct.test01 = type { i64, i64 }
%struct.test01dep = type { i32, %struct.test01** }

@g_test01ptr = internal unnamed_addr global %struct.test01* zeroinitializer
@g_test01depptr = internal unnamed_addr global %struct.test01dep { i32 1, %struct.test01** @g_test01ptr }
; CHECK: @g_test01ptr = internal unnamed_addr global i32 0
; CHECK: @g_test01depptr = internal unnamed_addr global %__SOADT_struct.test01dep { i32 1, i32* @g_test01ptr }

define i32 @main(i32 %argc, i8** %argv) {
  %alloc01 = call i8* @calloc(i64 10, i64 16)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*

  ret i32 0
}

declare i8* @calloc(i64, i64)
