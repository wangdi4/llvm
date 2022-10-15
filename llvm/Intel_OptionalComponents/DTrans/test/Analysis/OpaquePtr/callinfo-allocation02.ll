; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-callinfo -disable-output %s 2>&1 | FileCheck %s

; Test creation of CallInfo objects for a memory allocation that gets
; used as more than one type.

; Test with malloc result cast to more than one type.
%struct.test01a = type { i32, i32, i32 }
%struct.test01b = type { i16, i16, i16, i16, i16, i16 }
define void @test01() {
  %p = tail call i8* @malloc(i64 132)
  %s1 = bitcast i8* %p to %struct.test01a*
  %s2 = bitcast i8* %p to %struct.test01b*

  ; We need to use the bitcast types for the safety analyzer to resolve them.
  %addr.s1.0 = getelementptr %struct.test01a, %struct.test01a* %s1, i64 0, i32 0
  store i32 1, i32* %addr.s1.0
  %addr.s2.0 = getelementptr %struct.test01b, %struct.test01b* %s2, i64 0, i32 0
  %half = load i16, i16* %addr.s2.0
  ret void
}
; CHECK-LABEL: Function: test01
; CHECK: AllocCallInfo:
; CHECK:   Kind: Malloc
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test01a = type { i32, i32, i32 }
; CHECK:     Type: %struct.test01b = type { i16, i16, i16, i16, i16, i16 }

declare !intel.dtrans.func.type !4 "intel_dtrans_func_index"="1" i8* @malloc(i64)  #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!intel.dtrans.types = !{!5, !6}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i16 0, i32 0}  ; i16
!3 = !{i8 0, i32 1}  ; i8*
!4 = distinct !{!3}
!5 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!6 = !{!"S", %struct.test01b zeroinitializer, i32 6, !2, !2, !2, !2, !2, !2} ; { i16, i16, i16, i16, i16, i16 }
