; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test of safety data for global array definitions.

%struct.test01 = type { i64, i64 }
@array_of_struct = internal global [2 x %struct.test01] zeroinitializer
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: Global instance | Global array{{ *$}}


%struct.test02 = type { i64, i64 }
@array_of_structptr = internal global [2 x %struct.test02*] zeroinitializer, !intel_dtrans_type !2
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Global pointer{{ *$}}


%struct.test03 = type { i64, i64 }
@array_of_structptrptr = internal global [2 x %struct.test03**] zeroinitializer, !intel_dtrans_type !4
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: Safety data: Global pointer{{ *$}}


%struct.test04 = type { i64, i64 }
@ptr_to_array_of_struct = internal global [2 x %struct.test04]* zeroinitializer, !intel_dtrans_type !6
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test04
; CHECK: Safety data: Global pointer{{ *$}}


%struct.test05 = type { i64, i64 }
@ptr_to_array_of_structptr = internal global [2 x %struct.test05*]* zeroinitializer, !intel_dtrans_type !9
; DTrans does not treat global pointer as pointer carried because the pointer
; to the structure is not being directly instantiated.
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test05
; CHECK: Safety data: No issues found{{ *$}}

%struct.test06 = type { i64, i64 }
@ptr_to_array_of_structptrptr = internal global [2 x %struct.test06**]* zeroinitializer, !intel_dtrans_type !12
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test06
; CHECK: Safety data: No issues found{{ *$}}


%struct.test07 = type { i64, i64 }
@array_of_array_of_struct = internal global [2 x [4 x %struct.test07]] zeroinitializer
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test07
; CHECK: Safety data: Global instance | Global array{{ *$}}


%struct.test08 = type { i64, i64 }
@array_of_array_of_structptr = internal global [2 x [4 x %struct.test08*]] zeroinitializer, !intel_dtrans_type !15
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test08
; CHECK: Safety data: Global pointer{{ *$}}


%struct.test09 = type { i64, i64 }
@array_of_array_of_structptrptr = internal global [2 x [4 x %struct.test09**]] zeroinitializer, !intel_dtrans_type !18
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test09
; CHECK: Safety data: Global pointer{{ *$}}


!1 = !{i64 0, i32 0}  ; i64
!2 = !{!"A", i32 2, !3}  ; [2 x %struct.test02*]
!3 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!4 = !{!"A", i32 2, !5}  ; [2 x %struct.test03**]
!5 = !{%struct.test03 zeroinitializer, i32 2}  ; %struct.test03**
!6 = !{!7, i32 1}  ; [2 x %struct.test04]*
!7 = !{!"A", i32 2, !8}  ; [2 x %struct.test04]
!8 = !{%struct.test04 zeroinitializer, i32 0}  ; %struct.test04
!9 = !{!10, i32 1}  ; [2 x %struct.test05*]*
!10 = !{!"A", i32 2, !11}  ; [2 x %struct.test05*]
!11 = !{%struct.test05 zeroinitializer, i32 1}  ; %struct.test05*
!12 = !{!13, i32 1}  ; [2 x %struct.test06**]*
!13 = !{!"A", i32 2, !14}  ; [2 x %struct.test06**]
!14 = !{%struct.test06 zeroinitializer, i32 2}  ; %struct.test06**
!15 = !{!"A", i32 2, !16}  ; [2 x [4 x %struct.test08*]]
!16 = !{!"A", i32 4, !17}  ; [4 x %struct.test08*]
!17 = !{%struct.test08 zeroinitializer, i32 1}  ; %struct.test08*
!18 = !{!"A", i32 2, !19}  ; [2 x [4 x %struct.test09**]]
!19 = !{!"A", i32 4, !20}  ; [4 x %struct.test09**]
!20 = !{%struct.test09 zeroinitializer, i32 2}  ; %struct.test09**
!21 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!22 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!23 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!24 = !{!"S", %struct.test04 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!25 = !{!"S", %struct.test05 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!26 = !{!"S", %struct.test06 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!27 = !{!"S", %struct.test07 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!28 = !{!"S", %struct.test08 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!29 = !{!"S", %struct.test09 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }

!intel.dtrans.types = !{!21, !22, !23, !24, !25, !26, !27, !28, !29}
