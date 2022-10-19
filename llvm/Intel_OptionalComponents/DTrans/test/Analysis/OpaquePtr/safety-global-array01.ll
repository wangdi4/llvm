; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test of safety data for global array definitions.

%struct.test01 = type { i64, i64 }
@array_of_struct = internal global [2 x %struct.test01] zeroinitializer
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Global instance | Global array{{ *$}}
; CHECK: End LLVMType: %struct.test01


%struct.test02 = type { i64, i64 }
@array_of_structptr = internal global [2 x %struct.test02*] zeroinitializer, !intel_dtrans_type !2
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Global pointer{{ *$}}
; CHECK: End LLVMType: %struct.test02


%struct.test03 = type { i64, i64 }
@array_of_structptrptr = internal global [2 x %struct.test03**] zeroinitializer, !intel_dtrans_type !4
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: Global pointer{{ *$}}
; CHECK: End LLVMType: %struct.test03


%struct.test04 = type { i64, i64 }
@ptr_to_array_of_struct = internal global [2 x %struct.test04]* zeroinitializer, !intel_dtrans_type !6
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04
; CHECK: Safety data: Global pointer{{ *$}}
; CHECK: End LLVMType: %struct.test04


%struct.test05 = type { i64, i64 }
@ptr_to_array_of_structptr = internal global [2 x %struct.test05*]* zeroinitializer, !intel_dtrans_type !9
; DTrans does not treat global pointer as pointer carried because the pointer
; to the structure is not being directly instantiated.
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05
; CHECK: Safety data: No issues found{{ *$}}
; CHECK: End LLVMType: %struct.test05


%struct.test06 = type { i64, i64 }
@ptr_to_array_of_structptrptr = internal global [2 x %struct.test06**]* zeroinitializer, !intel_dtrans_type !12
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test06
; CHECK: Safety data: No issues found{{ *$}}
; CHECK: End LLVMType: %struct.test06


%struct.test07 = type { i64, i64 }
@array_of_array_of_struct = internal global [2 x [4 x %struct.test07]] zeroinitializer
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test07
; CHECK: Safety data: Global instance | Global array{{ *$}}
; CHECK: End LLVMType: %struct.test07


%struct.test08 = type { i64, i64 }
@array_of_array_of_structptr = internal global [2 x [4 x %struct.test08*]] zeroinitializer, !intel_dtrans_type !15
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test08
; CHECK: Safety data: Global pointer{{ *$}}
; CHECK: End LLVMType: %struct.test08


%struct.test09 = type { i64, i64 }
@array_of_array_of_structptrptr = internal global [2 x [4 x %struct.test09**]] zeroinitializer, !intel_dtrans_type !18
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test09
; CHECK: Safety data: Global pointer{{ *$}}
; CHECK: End LLVMType: %struct.test09


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
