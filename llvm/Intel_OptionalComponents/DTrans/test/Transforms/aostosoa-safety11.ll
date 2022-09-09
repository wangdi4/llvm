; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-heur-override=struct.test01 -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-heur-override=struct.test01 -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s

; This test verifies that the heuristic override flag does not trigger
; transformation of a type that does not met the safety criteria.

%struct.test01 = type { i32, i64, i32 }
%struct.test01other = type { i32, i32, i32, i32 }

; This structure is defined here to have a structure that passes the safety
; tests. There needs to be at least one candidate structure that meets all
; the safety checks to reach the code where heuristic overriding takes
; place.
%struct.test02 = type { i32, i64, i32 }

define i32 @main(i32 %argc, i8** %argv) {
  %local1 = alloca %struct.test01*
  %local2 = alloca %struct.test02*

  ; Introduce a bad bitcast.
  %local1_val = load %struct.test01*, %struct.test01** %local1
  %bad_cast = bitcast  %struct.test01* %local1_val to %struct.test01other*
  %gep = getelementptr %struct.test01other, %struct.test01other* %bad_cast, i64 0, i32 0

  ret i32 0
}

; CHECK-NOT: DTRANS-AOSTOSOA: Passed qualification tests: struct.test01
; CHECK-NOT: DTRANS-AOSTOSOA: Passed qualification tests: struct.test02
