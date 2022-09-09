; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s


; This test verifies the initialization of dependent types is handled when the
; size of the index type is changed to be 32-bits instead of the size of a
; pointer. In this case, the type of the dependent variable is changed, but it
; should still maintain zeroinitializer.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
%struct.test01 = type { i64, i64 }
%struct.test01dep = type { i32, %struct.test01* }
; CHECK: %__SOADT_struct.test01dep = type { i32, i32 }

@g_test01depptr = internal unnamed_addr global %struct.test01dep zeroinitializer
; CHECK: @g_test01depptr = internal unnamed_addr global %__SOADT_struct.test01dep zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  %alloc01 = call i8* @calloc(i64 10, i64 16)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*
  store i8* %alloc01, i8** bitcast (%struct.test01** getelementptr (%struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 1) to i8**)

  ret i32 0
}

declare i8* @calloc(i64, i64)
