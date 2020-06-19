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
@array_of_structptr = internal global [2 x %struct.test02*] zeroinitializer, !dtrans_type !2
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Global pointer{{ *$}}


%struct.test03 = type { i64, i64 }
@array_of_structptrptr = internal global [2 x %struct.test03**] zeroinitializer, !dtrans_type !5
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: Safety data: Global pointer{{ *$}}


%struct.test04 = type { i64, i64 }
@ptr_to_array_of_struct = internal global [2 x %struct.test04]* zeroinitializer, !dtrans_type !13
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test04
; CHECK: Safety data: Global pointer{{ *$}}


%struct.test05 = type { i64, i64 }
@ptr_to_array_of_structptr = internal global [2 x %struct.test05*]* zeroinitializer, !dtrans_type !16
; DTrans does not treat global pointer as pointer carried because the pointer
; to the structure is not being directly instantiated.
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test05
; CHECK: Safety data: No issues found{{ *$}}

%struct.test06 = type { i64, i64 }
@ptr_to_array_of_structptrptr = internal global [2 x %struct.test06**]* zeroinitializer, !dtrans_type !20
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test06
; CHECK: Safety data: No issues found{{ *$}}


%struct.test07 = type { i64, i64 }
@array_of_array_of_struct = internal global [2 x [4 x %struct.test07]] zeroinitializer
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test07
; CHECK: Safety data: Global instance | Global array{{ *$}}


%struct.test08 = type { i64, i64 }
@array_of_array_of_structptr = internal global [2 x [4 x %struct.test08*]] zeroinitializer, !dtrans_type !24
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test08
; CHECK: Safety data: Global pointer{{ *$}}


%struct.test09 = type { i64, i64 }
@array_of_array_of_structptrptr = internal global [2 x [4 x %struct.test09**]] zeroinitializer, !dtrans_type !28
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test09
; CHECK: Safety data: Global pointer{{ *$}}


!1 = !{i64 0, i32 0}  ; i64
!2 = !{!"A", i32 2, !3}  ; [2 x %struct.test02*]
!3 = !{!4, i32 1}  ; %struct.test02*
!4 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!5 = !{!"A", i32 2, !6}  ; [2 x %struct.test03**]
!6 = !{!7, i32 2}  ; %struct.test03**
!7 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!8 = !{!9, i32 1}  ; [8 x i64]*
!9 = !{!"A", i32 8, !1}  ; [8 x i64]
!10 = !{!11, i32 1}  ; [4 x i32*]*
!11 = !{!"A", i32 4, !12}  ; [4 x i32*]
!12 = !{i32 0, i32 1}  ; i32*
!13 = !{!14, i32 1}  ; [2 x %struct.test04]*
!14 = !{!"A", i32 2, !15}  ; [2 x %struct.test04]
!15 = !{!"R", %struct.test04 zeroinitializer, i32 0}  ; %struct.test04
!16 = !{!17, i32 1}  ; [2 x %struct.test05*]*
!17 = !{!"A", i32 2, !18}  ; [2 x %struct.test05*]
!18 = !{!19, i32 1}  ; %struct.test05*
!19 = !{!"R", %struct.test05 zeroinitializer, i32 0}  ; %struct.test05
!20 = !{!21, i32 1}  ; [2 x %struct.test06**]*
!21 = !{!"A", i32 2, !22}  ; [2 x %struct.test06**]
!22 = !{!23, i32 2}  ; %struct.test06**
!23 = !{!"R", %struct.test06 zeroinitializer, i32 0}  ; %struct.test06
!24 = !{!"A", i32 2, !25}  ; [2 x [4 x %struct.test08*]]
!25 = !{!"A", i32 4, !26}  ; [4 x %struct.test08*]
!26 = !{!27, i32 1}  ; %struct.test08*
!27 = !{!"R", %struct.test08 zeroinitializer, i32 0}  ; %struct.test08
!28 = !{!"A", i32 2, !29}  ; [2 x [4 x %struct.test09**]]
!29 = !{!"A", i32 4, !30}  ; [4 x %struct.test09**]
!30 = !{!31, i32 2}  ; %struct.test09**
!31 = !{!"R", %struct.test09 zeroinitializer, i32 0}  ; %struct.test09
!32 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!33 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!34 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!35 = !{!"S", %struct.test04 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!36 = !{!"S", %struct.test05 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!37 = !{!"S", %struct.test06 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!38 = !{!"S", %struct.test07 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!39 = !{!"S", %struct.test08 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!40 = !{!"S", %struct.test09 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }

!dtrans_types = !{!32, !33, !34, !35, !36, !37, !38, !39, !40}
