; REQUIRES: asserts
; RUN: opt < %s -disable-output -dtrans-aostosoa -debug-only=dtrans-aostosoa -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -passes=dtrans-aostosoa -debug-only=dtrans-aostosoa -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s

; This test verifies that the AOS-to-SOA transformation of a
; type gets inhibited if a safety check for a dependent type
; does not allow for changing the pointer to the structure to
; a peeling index.
;
; The safety check is straightforward, so it is not necessary to check every
; safety condition individually.

%struct.test01 = type { i64, i64 }
%struct.test01dep = type { i32, %struct.test01*, %struct.test01** }
@g_test01depptr = internal unnamed_addr global %struct.test01dep zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  ; Allocate the type to be transformed
  %alloc01 = call i8* @calloc(i64 10, i64 16)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*

  %dep_addr = getelementptr %struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 1
  store %struct.test01* %struct01_mem, %struct.test01** %dep_addr

  ; Use the volatile attribute on a load of the dependent structure to cause
  ; a safety condition to be set on the dependent structure type.
  ; This will turn off the AOS-to-SOA transformation on struct.test01 even
  ; though that structure met the required conditions.
  %vl = load volatile %struct.test01*, %struct.test01** %dep_addr
  ret i32 0
}

; CHECK: Disqualifying type: struct.test01 based on safety conditions of dependent type: struct.test01dep

declare i8* @calloc(i64, i64)
