; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type recovery for "call" instructions for cases where the
; argument is not used by the callee. In these cases there is no need to
; collect the callee expected argument type as a usage type for the caller.

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; Test with bitcast argument
%struct.test01a = type { i32, i32 }
%struct.test01b = type { i64 }
define internal void @test01(%struct.test01a** %pp) !dtrans_type !3 {
  %pStruct = load %struct.test01a*, %struct.test01a** %pp
  %pStruct.as.1b = bitcast %struct.test01a* %pStruct to %struct.test01b*
  call void @test01callee(%struct.test01b* %pStruct.as.1b)
  ret void
}
define void @test01callee(%struct.test01b* %in) !dtrans_type !7 {
  ret void
}
; CHECK-LABEL: internal void @test01
; CHECK-CUR:  %pStruct = load %struct.test01a*, %struct.test01a** %pp
; CHECK-FUT:  %pStruct = load p0, p0 %pp
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:    %struct.test01a*{{ *$}}
; CHECK-NEXT:  No element pointees.


; Test with bitcast function call
%struct.test02a = type { i32, i32 }
%struct.test02b = type { i64 }
define internal void @test02(%struct.test02a** %pp) !dtrans_type !10 {
  %pStruct = load %struct.test02a*, %struct.test02a** %pp
  call void bitcast (void (%struct.test02b*)* @test02callee
                       to void (%struct.test02a*)*)(%struct.test02a* %pStruct)
  ret void
}
define void @test02callee(%struct.test02b* %in) !dtrans_type !13 {
  ret void
}
; CHECK-LABEL: internal void @test02
; CHECK-CUR:  %pStruct = load %struct.test02a*, %struct.test02a** %pp
; CHECK-FUT:  %pStruct = load p0, p0 %pp
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:    %struct.test02a*{{ *$}}
; CHECK-NEXT:  No element pointees.


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{!"F", i1 false, i32 1, !4, !5}  ; void (%struct.test01a**)
!4 = !{!"void", i32 0}  ; void
!5 = !{!6, i32 2}  ; %struct.test01a**
!6 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!7 = !{!"F", i1 false, i32 1, !4, !8}  ; void (%struct.test01b*)
!8 = !{!9, i32 1}  ; %struct.test01b*
!9 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!10 = !{!"F", i1 false, i32 1, !4, !11}  ; void (%struct.test02a**)
!11 = !{!12, i32 2}  ; %struct.test02a**
!12 = !{!"R", %struct.test02a zeroinitializer, i32 0}  ; %struct.test02a
!13 = !{!"F", i1 false, i32 1, !4, !14}  ; void (%struct.test02b*)
!14 = !{!15, i32 1}  ; %struct.test02b*
!15 = !{!"R", %struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!16 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!17 = !{!"S", %struct.test01b zeroinitializer, i32 1, !2} ; { i64 }
!18 = !{!"S", %struct.test02a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!19 = !{!"S", %struct.test02b zeroinitializer, i32 1, !2} ; { i64 }

!dtrans_types = !{!16, !17, !18, !19}
