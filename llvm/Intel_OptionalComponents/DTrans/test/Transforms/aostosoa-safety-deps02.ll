; REQUIRES: asserts
; RUN: opt < %s -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -dtrans-outofboundsok=true -dtrans-aostosoa -debug-only=dtrans-aostosoa -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -dtrans-outofboundsok=true -passes=dtrans-aostosoa -debug-only=dtrans-aostosoa -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s

; This test verifies that when a dependent type has the "field address
; taken" safety bit cascaded to the candidate type (due to the pointer
; carried safety with -dtrans-outofboundsok=true), the candidate gets
; rejected for the AOS-to-SOA transformation.

%struct.test01 = type { i64, i64 }
%struct.test01dep = type { [200 x i8], %struct.test01*, %struct.test01** }
@g_test01depptr = internal unnamed_addr global %struct.test01dep zeroinitializer
@g_blank = internal unnamed_addr global [200 x i8] zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  ; Allocate the type to be transformed
  %alloc01 = call i8* @calloc(i64 10, i64 16)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*

  %dep_addr = getelementptr %struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 1
  store %struct.test01* %struct01_mem, %struct.test01** %dep_addr

  %arr_begin1 = getelementptr %struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 0, i64 0
  %arr_begin2 = getelementptr [200 x i8], [200 x i8]* @g_blank, i64 0, i64 0
  %res = call i8* @strcpy(i8* %arr_begin1, i8* %arr_begin2)

  ret i32 0
}

; CHECK: DTRANS-AOSTOSOA: Rejecting -- Unsupported safety data: struct.test01

declare i8* @calloc(i64, i64)
declare i8* @strcpy(i8*, i8*)
