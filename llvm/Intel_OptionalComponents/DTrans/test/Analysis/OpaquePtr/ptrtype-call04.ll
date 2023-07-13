; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery for "call" instructions for cases where the
; argument is not used by the callee. In these cases there is no need to
; collect the callee expected argument type as a usage type for the caller.


; Test with bitcast argument
%struct.test01a = type { i32, i32 }
%struct.test01b = type { i64 }
define internal void @test01(ptr "intel_dtrans_func_index"="1" %pp) !intel.dtrans.func.type !4 {
  %pStruct = load ptr, ptr %pp
  %pStruct.as.1b = bitcast ptr %pStruct to ptr
  call void @test01callee(ptr %pStruct.as.1b)
  ret void
}
define void @test01callee(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !6 {
  ret void
}
; CHECK-LABEL: internal void @test01
; CHECK:  %pStruct = load ptr, ptr %pp
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:    %struct.test01a*{{ *$}}
; CHECK-NEXT:  No element pointees.


; Test with bitcast function call
%struct.test02a = type { i32, i32 }
%struct.test02b = type { i64 }
define internal void @test02(ptr "intel_dtrans_func_index"="1" %pp) !intel.dtrans.func.type !8 {
  %pStruct = load ptr, ptr %pp
  call void bitcast (ptr @test02callee
                       to ptr)(ptr %pStruct)
  ret void
}
define void @test02callee(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !10 {
  ret void
}
; CHECK-LABEL: internal void @test02
; CHECK:  %pStruct = load ptr, ptr %pp
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:    %struct.test02a*{{ *$}}
; CHECK-NEXT:  No element pointees.


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test01a zeroinitializer, i32 2}  ; %struct.test01a**
!4 = distinct !{!3}
!5 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!6 = distinct !{!5}
!7 = !{%struct.test02a zeroinitializer, i32 2}  ; %struct.test02a**
!8 = distinct !{!7}
!9 = !{%struct.test02b zeroinitializer, i32 1}  ; %struct.test02b*
!10 = distinct !{!9}
!11 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!12 = !{!"S", %struct.test01b zeroinitializer, i32 1, !2} ; { i64 }
!13 = !{!"S", %struct.test02a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!14 = !{!"S", %struct.test02b zeroinitializer, i32 1, !2} ; { i64 }

!intel.dtrans.types = !{!11, !12, !13, !14}
