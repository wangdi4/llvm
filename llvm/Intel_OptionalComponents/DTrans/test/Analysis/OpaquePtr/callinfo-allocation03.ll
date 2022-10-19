; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-callinfo -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-callinfo -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test creation of CallInfo objects for a memory allocation involving
; an array of structures.

; Test with malloc result cast to pointer to an array.
%struct.test01 = type { i32, i32, i32 }
define void @test01(i64 %n) {
  %call = tail call i8* @malloc(i64 240)
  %p = bitcast i8* %call to [6 x %struct.test01]*
  %head = getelementptr [6 x %struct.test01], [6 x %struct.test01]* %p, i64 0, i32 0
  %addr0 = getelementptr %struct.test01, %struct.test01* %head, i64 0, i32 0
  store i32 0, i32* %addr0
  ret void
}
; CHECK: Function: test01
; CHECK: AllocCallInfo:
; CHECK:   Kind: Malloc
; CHECK:   Aliased types:
; CHECK:     Type: [6 x %struct.test01]

declare !intel.dtrans.func.type !3 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 1}  ; i8*
!3 = distinct !{!2}
!4 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

!intel.dtrans.types = !{!4}
