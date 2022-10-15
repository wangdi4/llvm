; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -dtrans-outofboundsok=false -dtrans-aostosoa -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume -intel-libirc-allowed 2>&1 | FileCheck %s
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -dtrans-outofboundsok=false -passes=dtrans-aostosoa -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume -intel-libirc-allowed 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that passing the address of a field within a dependent
; data structure, but not taking the address of pointer to type being transformed
; does not inhibit the AOS-to-SOA transformation when using the rule for
; -dtrans-outofboundsok=false.

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

; CHECK: %__SOA_struct.test01 = type { i64*, i64* }

declare i8* @calloc(i64, i64) #0
declare i8* @strcpy(i8*, i8*)

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }
