; RUN: opt < %s -S -dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s
; RUN: opt < %s -S -passes=dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s


; This test verifies that shrinking the peeling index to 32-bits gets inhibited
; if there is a byte-flattened GEP form of a field access in a dependent global
; variable.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
%struct.test01 = type { i64, i64 }
%struct.test01dep = type { i64, %struct.test01*, %struct.test01**, i64 }

@g_test01depptr = internal unnamed_addr global %struct.test01dep zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  %alloc01 = call i8* @calloc(i64 10, i64 16)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*
  store %struct.test01* %struct01_mem, %struct.test01** getelementptr (%struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 1)

  ; Access of field of the dependent structure using a byte flattened GEP form
  ; that is a constant operator of the global variable. The computing of the
  ; new offset is not currently supported for the constant operator by the
  ; transformation code, so will prevent the index shrinking portion of the
  ; transformation, while allowing the AOS-to-SOA conversion to happen on
  ; %struct.test01 since that does not change the size of the dependent
  ; structure.
  store i64 0, i64*
    bitcast (i8*
      getelementptr (i8, i8*
        bitcast (%struct.test01dep* @g_test01depptr to i8*),
      i64 24)
  to i64*)

  ret i32 0
}

; Peeling should occur, but the index type should be 64-bits
; CHECK: %__SOADT_struct.test01dep = type { i64, i64, i64*, i64 }

declare i8* @calloc(i64, i64)
