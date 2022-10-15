; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases of converting a pointer to an integer for use in a subtract
; instruction that are NOT safe.

; Subtracting two pointers that came from field addresses is computing the
; layout of an aggregate, and is not safe. This will cause %struct.test01a to be
; marked with "Bad pointer manipulation". %struct.test01b is not 'bad pointer
; manipulation' even though those are the types involved in the subtraction
; because they are ptr-to-ptr types. However, %struct.test01c is "Bad pointer
; manipulation" because it is nested with the structure that the address offsets
; were computed, and therefore this type of pointer subtraction could result in
; its size being deduced by pointer arithmetic of the field addresses.
%struct.test01a = type { %struct.test01b*, %struct.test01c, %struct.test01b*, %struct.test01b* }
%struct.test01b = type { i32, i32 }
%struct.test01c = type { i64 }
define void @test01(%struct.test01a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !6 {
  %pField0 = getelementptr %struct.test01a, %struct.test01a* %pStruct, i64 0, i32 0
  %pField2 = getelementptr %struct.test01a, %struct.test01a* %pStruct, i64 0, i32 2
  %tmp1 = ptrtoint %struct.test01b** %pField0 to i64
  %tmp2 = ptrtoint %struct.test01b** %pField2 to i64
  %offset = sub i64 %tmp2, %tmp1
  %div = sdiv i64 %offset, 8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Bad pointer manipulation | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01b

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01c
; CHECK: Safety data: Bad pointer manipulation | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01c


; Subtraction with one value being an field element and the other not is not
; permitted even if both pointers are the same type.
%struct.test02a = type { i32, %struct.test02b }
%struct.test02b = type { i32, i32 }
define void @test02(%struct.test02a* "intel_dtrans_func_index"="1" %pStructA, %struct.test02b* "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !10 {
  %pField = getelementptr %struct.test02a, %struct.test02a* %pStructA, i64 0, i32 1
  %tmp1 = ptrtoint %struct.test02b* %pField to i64
  %tmp2 = ptrtoint %struct.test02b* %pStructB to i64
  %offset = sub i64 %tmp2, %tmp1
  %div = sdiv i64 %offset, 8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02a
; CHECK: Safety data: Bad pointer manipulation | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02b
; CHECK: Safety data: Bad pointer manipulation | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02b


; Subtracting two pointers of different types is not permitted
%struct.test03a = type { i32, i32 }
%struct.test03b = type { i64 }
define void @test03(%struct.test03a* "intel_dtrans_func_index"="1" %pStructA, %struct.test03b* "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !13 {
  %tmp1 = ptrtoint %struct.test03a* %pStructA to i64
  %tmp2 = ptrtoint %struct.test03b* %pStructB to i64
  %offset = sub i64 %tmp2, %tmp1
  %div = sdiv i64 %offset, 8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03a
; CHECK: Safety data: Bad pointer manipulation{{ *$}}
; CHECK: End LLVMType: %struct.test03a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03b
; CHECK: Safety data: Bad pointer manipulation{{ *$}}
; CHECK: End LLVMType: %struct.test03b


; Subtracting a scalar from a pointer is not permitted
%struct.test04 = type { i32, i32 }
define void @test04(%struct.test04* "intel_dtrans_func_index"="1" %pStruct, i64 %other) !intel.dtrans.func.type !15 {
  %tmp1 = ptrtoint %struct.test04* %pStruct to i64
  %offset = sub i64 %tmp1, %other
  %div = sdiv i64 %offset, 8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04
; CHECK: Safety data: Bad pointer manipulation{{ *$}}
; CHECK: End LLVMType: %struct.test04


; Subtracting a scalar from a pointer is not permitted
%struct.test05 = type { i32, i32 }
define void @test05(%struct.test05* "intel_dtrans_func_index"="1" %pStruct, i64 %other) !intel.dtrans.func.type !17 {
  %tmp1 = ptrtoint %struct.test05* %pStruct to i64
  %offset = sub i64 %other, %tmp1
  %div = sdiv i64 %offset, 8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05
; CHECK: Safety data: Bad pointer manipulation{{ *$}}
; CHECK: End LLVMType: %struct.test05


; Ambiguous types are not permitted
%struct.test06a = type { i32, i32 }
%struct.test06b = type { i64 }
define void @test06(%struct.test06a* "intel_dtrans_func_index"="1" %pStruct1, %struct.test06a* "intel_dtrans_func_index"="2" %pStruct2) !intel.dtrans.func.type !19 {
  %pStruct1.as.pB = bitcast %struct.test06a* %pStruct1 to %struct.test06b*
  %pStruct2.as.pB = bitcast %struct.test06a* %pStruct1 to %struct.test06b*
  %use1 = getelementptr %struct.test06b, %struct.test06b* %pStruct1.as.pB, i64 0, i32 0
  %use2 = getelementptr %struct.test06b, %struct.test06b* %pStruct2.as.pB, i64 0, i32 0
  %tmp1 = ptrtoint %struct.test06b* %pStruct1.as.pB to i64
  %tmp2 = ptrtoint %struct.test06b* %pStruct2.as.pB to i64
  %offset = sub i64 %tmp2, %tmp1
  %div = sdiv i64 %offset, 8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test06a
; CHECK: Safety data: Bad casting | Bad pointer manipulation | Ambiguous GEP{{ *$}}
; CHECK: End LLVMType: %struct.test06a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test06b
; CHECK: Safety data: Bad casting | Bad pointer manipulation | Ambiguous GEP{{ *$}}
; CHECK: End LLVMType: %struct.test06b


; Subtracting two pointers of the same type, but not using it for a divide.
; This prevents DTrans from handling it
%struct.test07 = type { i64, i64 }
define void @test07(%struct.test07* "intel_dtrans_func_index"="1" %pStruct1, %struct.test07* "intel_dtrans_func_index"="2" %pStruct2, i64* "intel_dtrans_func_index"="3" %distance) !intel.dtrans.func.type !22 {
  %t1 = ptrtoint %struct.test07* %pStruct1 to i64
  %t2 = ptrtoint %struct.test07* %pStruct2 to i64
  %offset = sub i64 %t2, %t1
  store i64 %offset, i64* %distance
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test07
; CHECK: Safety data: Bad pointer manipulation{{ *$}}
; CHECK: End LLVMType: %struct.test07


; Subtracting two pointers but dividing by a value that is not the recognized
; size of the aggregate is not permitted
%struct.test08 = type { i64, i64 }
define void @test08(%struct.test08* "intel_dtrans_func_index"="1" %pStruct1, %struct.test08* "intel_dtrans_func_index"="2" %pStruct2, i64 %n) !intel.dtrans.func.type !24 {
  %t1 = ptrtoint %struct.test08* %pStruct1 to i64
  %t2 = ptrtoint %struct.test08* %pStruct2 to i64
  %offset = sub i64 %t2, %t1
  %div = sdiv i64 %offset, %n
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test08
; CHECK: Safety data: Bad pointer manipulation{{ *$}}
; CHECK: End LLVMType: %struct.test08


!1 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!2 = !{%struct.test01c zeroinitializer, i32 0}  ; %struct.test01c
!3 = !{i32 0, i32 0}  ; i32
!4 = !{i64 0, i32 0}  ; i64
!5 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!6 = distinct !{!5}
!7 = !{%struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!8 = !{%struct.test02a zeroinitializer, i32 1}  ; %struct.test02a*
!9 = !{%struct.test02b zeroinitializer, i32 1}  ; %struct.test02b*
!10 = distinct !{!8, !9}
!11 = !{%struct.test03a zeroinitializer, i32 1}  ; %struct.test03a*
!12 = !{%struct.test03b zeroinitializer, i32 1}  ; %struct.test03b*
!13 = distinct !{!11, !12}
!14 = !{%struct.test04 zeroinitializer, i32 1}  ; %struct.test04*
!15 = distinct !{!14}
!16 = !{%struct.test05 zeroinitializer, i32 1}  ; %struct.test05*
!17 = distinct !{!16}
!18 = !{%struct.test06a zeroinitializer, i32 1}  ; %struct.test06a*
!19 = distinct !{!18, !18}
!20 = !{%struct.test07 zeroinitializer, i32 1}  ; %struct.test07*
!21 = !{i64 0, i32 1}  ; i64*
!22 = distinct !{!20, !20, !21}
!23 = !{%struct.test08 zeroinitializer, i32 1}  ; %struct.test08*
!24 = distinct !{!23, !23}
!25 = !{!"S", %struct.test01a zeroinitializer, i32 4, !1, !2, !1, !1} ; { %struct.test01b*, %struct.test01c, %struct.test01b*, %struct.test01b* }
!26 = !{!"S", %struct.test01b zeroinitializer, i32 2, !3, !3} ; { i32, i32 }
!27 = !{!"S", %struct.test01c zeroinitializer, i32 1, !4} ; { i64 }
!28 = !{!"S", %struct.test02a zeroinitializer, i32 2, !3, !7} ; { i32, %struct.test02b }
!29 = !{!"S", %struct.test02b zeroinitializer, i32 2, !3, !3} ; { i32, i32 }
!30 = !{!"S", %struct.test03a zeroinitializer, i32 2, !3, !3} ; { i32, i32 }
!31 = !{!"S", %struct.test03b zeroinitializer, i32 1, !4} ; { i64 }
!32 = !{!"S", %struct.test04 zeroinitializer, i32 2, !3, !3} ; { i32, i32 }
!33 = !{!"S", %struct.test05 zeroinitializer, i32 2, !3, !3} ; { i32, i32 }
!34 = !{!"S", %struct.test06a zeroinitializer, i32 2, !3, !3} ; { i32, i32 }
!35 = !{!"S", %struct.test06b zeroinitializer, i32 1, !4} ; { i64 }
!36 = !{!"S", %struct.test07 zeroinitializer, i32 2, !4, !4} ; { i64, i64 }
!37 = !{!"S", %struct.test08 zeroinitializer, i32 2, !4, !4} ; { i64, i64 }

!intel.dtrans.types = !{!25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37}
