; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases of load instructions that should be identified as "safe"
; These cases cover the "safe" forms of the tests in
; safety-load-mismatched-elem[01-13].ll

%struct.test01 = type { i32, i32, i32 }
define i32 @test01(%struct.test01* %pStruct) !dtrans_type !2 {
  %pField = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 1
  %vField = load i32, i32* %pField
  ret i32 %vField
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Read{{ *$}}
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: No issues found

%struct.test02 = type { i32*, i32*, i32* }
define void @test02(%struct.test02* %pStruct) !dtrans_type !7 {
  %pField = getelementptr %struct.test02, %struct.test02* %pStruct, i64 0, i32 1
  %vField = load i32*, i32** %pField
  %use = load i32, i32* %vField
  ret void
}
; TODO: Field types will be 'p0' when opaque pointers are enabled
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: 0)Field LLVM Type: i32*
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32*
; CHECK: Field info: Read UnusedValue{{ *$}}
; CHECK: 2)Field LLVM Type: i32*
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: No issues found


%struct.test03a = type { %struct.test03b*, %struct.test03b*, %struct.test03b* }
%struct.test03b = type { i32, i32, i32 }
define i32 @test03(%struct.test03a* %pStruct) !dtrans_type !12 {
  %pField = getelementptr %struct.test03a, %struct.test03a* %pStruct, i64 0, i32 1
  %vField = load %struct.test03b*, %struct.test03b** %pField
  %pFieldB = getelementptr %struct.test03b, %struct.test03b* %vField, i64 0, i32 2
  %vFieldB = load i32, i32* %pFieldB
  ret i32 %vFieldB
}
; TODO: Field types will be 'p0' when opaque pointers are enabled
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03a
; CHECK: 0)Field LLVM Type: %struct.test03b*
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: %struct.test03b*
; CHECK: Field info: Read{{ *$}}
; CHECK: 2)Field LLVM Type: %struct.test03b*
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: No issues found

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03b
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Read{{ *$}}
; CHECK: Safety data: No issues found


; Load element zero using a pointer to the structure
%struct.test04 = type { i32, i32 }
define void @test04(%struct.test04* %pStruct) !dtrans_type !15 {
  %pStruct.as.p32 = bitcast %struct.test04* %pStruct to i32*
  %vField = load i32, i32* %pStruct.as.p32
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Read UnusedValue{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: No issues found

; Load element zero of a nested structure using a pointer to the structure
%struct.test05a = type { %struct.test05b }
%struct.test05b = type { i32, i32 }
define i32 @test05(%struct.test05a* %pStruct) !dtrans_type !19 {
  %pField = getelementptr %struct.test05a, %struct.test05a* %pStruct, i64 0, i32 0
  %pField.as.p32 = bitcast %struct.test05b* %pField to i32*
  %vField = load i32, i32* %pField.as.p32
  ret i32 %vField
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05a
; CHECK: 0)Field LLVM Type: %struct.test05b
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05b
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Read{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Nested structure{{ *$}}


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !3, !4}  ; void (%struct.test01*)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{i32 0, i32 1}  ; i32*
!7 = !{!"F", i1 false, i32 1, !3, !8}  ; void (%struct.test02*)
!8 = !{!9, i32 1}  ; %struct.test02*
!9 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!10 = !{!11, i32 1}  ; %struct.test03b*
!11 = !{!"R", %struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!12 = !{!"F", i1 false, i32 1, !1, !13}  ; i32 (%struct.test03a*)
!13 = !{!14, i32 1}  ; %struct.test03a*
!14 = !{!"R", %struct.test03a zeroinitializer, i32 0}  ; %struct.test03a
!15 = !{!"F", i1 false, i32 1, !3, !16}  ; void (%struct.test04*)
!16 = !{!17, i32 1}  ; %struct.test04*
!17 = !{!"R", %struct.test04 zeroinitializer, i32 0}  ; %struct.test04
!18 = !{!"R", %struct.test05b zeroinitializer, i32 0}  ; %struct.test05b
!19 = !{!"F", i1 false, i32 1, !1, !20}  ; i32 (%struct.test05a*)
!20 = !{!21, i32 1}  ; %struct.test05a*
!21 = !{!"R", %struct.test05a zeroinitializer, i32 0}  ; %struct.test05a
!22 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!23 = !{!"S", %struct.test02 zeroinitializer, i32 3, !6, !6, !6} ; { i32*, i32*, i32* }
!24 = !{!"S", %struct.test03a zeroinitializer, i32 3, !10, !10, !10} ; { %struct.test03b*, %struct.test03b*, %struct.test03b* }
!25 = !{!"S", %struct.test03b zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!26 = !{!"S", %struct.test04 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!27 = !{!"S", %struct.test05a zeroinitializer, i32 1, !18} ; { %struct.test05b }
!28 = !{!"S", %struct.test05b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!22, !23, !24, !25, !26, !27, !28}
