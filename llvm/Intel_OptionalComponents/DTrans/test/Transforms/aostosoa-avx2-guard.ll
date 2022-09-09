; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes='dtrans-aostosoa,require<dtransanalysis>' 2>&1 | FileCheck %s

; Verify that AOS-to-SOA is still triggered when Intel AVX2 is not enabled.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01 = type { i32, i32, i32, %struct.test01*, %struct.test02*, i8* }
%struct.test02 = type { i16, %struct.test01*, %struct.test01* }

; CHECK-DAG:%__SOA_struct.test02 = type { i16*, %__SOADT_struct.test01**, %__SOADT_struct.test01** }
; CHECK-DAG:%__SOADT_struct.test01 = type { i32, i32, i32, %__SOADT_struct.test01*, i32, i8* }

@g_test01ptr = internal unnamed_addr global %struct.test01* zeroinitializer


define i32 @main(i32 %argc, i8** %argv) {
  %alloc01 = call i8* @calloc(i64 10, i64 480)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*

  %alloc02 = call i8* @calloc(i64 2, i64 24)
  %struct02_mem = bitcast i8* %alloc02 to %struct.test02*
  %faddr = getelementptr %struct.test02, %struct.test02* %struct02_mem, i64 0, i32 1
  store %struct.test01* %struct01_mem, %struct.test01** %faddr

  call void @test1(%struct.test02 * %struct02_mem)
  ret i32 0
}

define void @test1(%struct.test02* %in) {
  %faddr = getelementptr %struct.test02, %struct.test02* %in, i64 0, i32 1
  %faddr_p64 = bitcast %struct.test01** %faddr to i64*
  %val = load i64, i64* %faddr_p64
  store i64 %val, i64* %faddr_p64
  ret void
}

declare i8* @calloc(i64, i64)
