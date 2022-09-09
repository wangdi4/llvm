; UNSUPPORTED: enable-opaque-pointers

; With the old pass manager run, we need to run the aos-to-soa pass followed by
; the dynclone pass in order to trigger the analysis of the modified structures
; to occur, because it is being lazily recomputed when invalidated by a
; transformation.
; RUN: opt < %s -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume -dtrans-dynclone -dtrans-print-types 2>&1 | FileCheck %s

; New pass manager will detect that analysis should be run again when the
; aos-to-soa pass makes modifications.
; RUN: opt < %s -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes='dtrans-aostosoa,require<dtransanalysis>' -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume -dtrans-print-types 2>&1 | FileCheck %s

; This test checks the safety data on the new data types created by the
; AOS-to-SOA transformation. Specifically, this is to check the safety data
; on structure types that are referenced by the type being transformed.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01 = type { i32, i32, i32, %struct.test01*, %struct.test01dep*, i8* }
%struct.test01dep = type { i16, %struct.test01*, %struct.test01* }
@g_test01ptr = internal unnamed_addr global %struct.test01* zeroinitializer


define i32 @main(i32 %argc, i8** %argv) {
  %alloc01 = call i8* @calloc(i64 10, i64 480)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*

  %alloc02 = call i8* @calloc(i64 2, i64 24)
  %struct02_mem = bitcast i8* %alloc02 to %struct.test01dep*
  %faddr = getelementptr %struct.test01dep, %struct.test01dep* %struct02_mem, i64 0, i32 1
  store %struct.test01* %struct01_mem, %struct.test01** %faddr

  call void @test1(%struct.test01dep * %struct02_mem)
  ret i32 0
}

; During the AOS-to-SOA processing, this function creates a bitcast pointer of
; a field address to convert it into a 32-bit pointer. These bitcasts must be
; removed by the transform to prevent the resultant dependent structure from
; being marked as bad bitcast.
define void @test1(%struct.test01dep* %in) {
  %faddr = getelementptr %struct.test01dep, %struct.test01dep* %in, i64 0, i32 1
  %faddr_p64 = bitcast %struct.test01** %faddr to i64*
  %val = load i64, i64* %faddr_p64
  store i64 %val, i64* %faddr_p64
  ret void
}

declare i8* @calloc(i64, i64)

; CHECK-LABEL: LLVMType: %struct.test01 = type { i32, i32, i32, %struct.test01*, %struct.test01dep*, i8* }
; CHECK: Safety data: Global pointer

; CHECK-LABEL: LLVMType: %struct.test01dep = type { i16, %struct.test01*, %struct.test01* }
; CHECK: Safety data: No issues found

; CHECK-LABEL: LLVMType: %__SOADT_struct.test01dep = type { i16, i32, i32 }
; CHECK: Safety data:
; CHECK-NOT: Bad casting
; CHECK-NOT: Unsafe pointer store

; CHECK-LABEL: LLVMType: %__SOA_struct.test01 = type { i32*, i32*, i32*, i32*, %__SOADT_struct.test01dep**, i8** }
; CHECK: Safety data: Global instance
