; RUN: opt < %s -S -dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s
; RUN: opt < %s -S -passes=dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s


; This test verifies that shrinking the peeling index to 32-bits gets inhibited
; if the safety conditions for a dependent type are found that will prevent
; modifying the size of the dependent type.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
%struct.test01 = type { i64, i64 }
%struct.test01dep = type { i64, %struct.test01*, %struct.test01**, i64 }

define i32 @main(i32 %argc, i8** %argv) {
  %alloc01 = call i8* @calloc(i64 10, i64 16)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*

  ; Allocate the dependent type with a value that is not a multiple of
  ; the structure size. This will inhibit using a 32-bit peeling index
  ; because the transformation will not be able to modify the allocation
  ; size of the dependent structure.
  %alloc02 = call i8* @calloc(i64 1, i64 36)
  %strut02_mem = bitcast i8* %alloc02 to %struct.test01dep*

  ret i32 0
}

; Peeling should occur, but the index type should be 64-bits
; CHECK: %__SOADT_struct.test01dep = type { i64, i64, i64*, i64 }

declare i8* @calloc(i64, i64)
