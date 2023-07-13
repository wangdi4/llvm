; REQUIRES: asserts
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results -debug-only=dtrans-pta-verbose < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test of byte-flattened GEP analysis for cases that are marked as potentially
; unsafe due to the use of negative offset amounts. These cases are simplified
; forms seen in various benchmarks where a cast is seen to allow access of a
; location prior to the known structure type. The analyzer should mark them
; with the  'unknown byte flattened gep' flag for use by the safety analyzer
; and collect the types that the safety analyzer will need to set a safety
; bit on.


; None of the elements should be added as byte-flattened GEPs.
; CHECK-NOT: Adding BF-GEP access


; This case comes from a case involving deletion of a dynamic array of the
; form:
;   delete [] dyn_arry
; With additional analysis, it may be possible to treat the result of the
; GEP as just i8*.
%struct.test01elem = type { i32, i32 }
%struct.test01 = type { i32, ptr }
define void @test01(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !4 {
  %dyn_array_addr = getelementptr %struct.test01, ptr %in, i64 0, i32 1
  %dyn_array = load ptr, ptr %dyn_array_addr
  %bc = bitcast ptr %dyn_array to ptr
  %count_addr.i8 = getelementptr i8, ptr %bc, i64 -8
  %count_addr = bitcast ptr %count_addr.i8 to ptr
  %count = load i64, ptr %count_addr
  ret void
}
; CHECK-LABEL: void @test01(
; CHECK:  %count_addr.i8 = getelementptr i8, ptr %bc, i64 -8
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-SAME: <UNKNOWN BYTE FLATTENED GEP>
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i8*{{ *$}}
; CHECK-NEXT:      No element pointees.


; In this case, the pointer is used as the same type following the GEP/Bitcast
; as it was before the initial bitcast. With some additional analysis it
; should be possible to avoid setting the 'unknown byte flattened gep' flag on
; it, if necessary.
%struct.test02 = type { i32, i32 }
define void @test02(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !6 {
  %bc1 = bitcast ptr %p to ptr
  %addr = getelementptr i8, ptr %bc1, i64 -8
  %bc2 = bitcast ptr %addr to ptr
  %use = getelementptr %struct.test02, ptr %bc2, i64 0, i32 1
  %val = load i32, ptr %use
  ret void
}
; CHECK-LABEL: void @test02(
; CHECK: %addr = getelementptr i8, ptr %bc1, i64 -8
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-SAME: <UNKNOWN BYTE FLATTENED GEP>
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i8*{{ *$}}
; CHECK-NEXT:      No element pointees.


; This case comes from a case where the user code is explicitly doing some
; pointer arithmetic to cast one type to another.
%struct.test03nodebase = type { ptr }
%struct.test03nodeimpl = type { %struct.test03nodebase, i16 }
%struct.test03elementbase = type  { ptr }
%struct.test03elementimpl = type { %struct.test03elementbase, %struct.test03nodeimpl, i32 }
define void @test03(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !15 {
  %bc1 = bitcast ptr %p to ptr
  %addr = getelementptr inbounds i8, ptr %bc1, i64 -8
  %bc2 = bitcast ptr %addr to ptr
  %use = getelementptr %struct.test03elementimpl, ptr %bc2, i64 0, i32 2
  %val = load i32, ptr %use
  ret void
}
; CHECK-LABEL: void @test03(
; CHECK: %addr = getelementptr inbounds i8, ptr %bc1, i64 -8
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-SAME: <UNKNOWN BYTE FLATTENED GEP>
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i8*{{ *$}}
; CHECK-NEXT:      No element pointees.



!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01elem zeroinitializer, i32 1}  ; %struct.test01elem*
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = distinct !{!3}
!5 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!6 = distinct !{!5}
!7 = !{!"F", i1 true, i32 0, !1}  ; i32 (...)
!8 = !{!7, i32 2}  ; i32 (...)**
!9 = !{%struct.test03nodebase zeroinitializer, i32 0}  ; %struct.test03nodebase
!10 = !{i16 0, i32 0}  ; i16
!11 = !{%struct.test03nodebase zeroinitializer, i32 1}  ; %struct.test03nodebase*
!12 = !{%struct.test03elementbase zeroinitializer, i32 0}  ; %struct.test03elementbase
!13 = !{%struct.test03nodeimpl zeroinitializer, i32 0}  ; %struct.test03nodeimpl
!14 = !{%struct.test03nodeimpl zeroinitializer, i32 1}  ; %struct.test03nodeimpl*
!15 = distinct !{!14}
!16 = !{!"S", %struct.test01elem zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!17 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i32, %struct.test01elem* }
!18 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!19 = !{!"S", %struct.test03nodebase zeroinitializer, i32 1, !8} ; { i32 (...)** }
!20 = !{!"S", %struct.test03nodeimpl zeroinitializer, i32 2, !9, !10} ; { %struct.test03nodebase, i16 }
!21 = !{!"S", %struct.test03elementbase zeroinitializer, i32 1, !11} ; { %struct.test03nodebase* }
!22 = !{!"S", %struct.test03elementimpl zeroinitializer, i32 3, !12, !13, !1} ; { %struct.test03elementbase, %struct.test03nodeimpl, i32 }

!intel.dtrans.types = !{!16, !17, !18, !19, !20, !21, !22}
