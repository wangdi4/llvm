; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases of store instructions that should be identified as "safe"

%struct.test01 = type { i32, i32, i32 }
define void @test01(%struct.test01* %pStruct, i32 %value) !dtrans_type !2 {
  %pField = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 0
  store i32 %value, i32* %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK:   Field info: Written{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK:   Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: i32
; CHECK:   Field info:{{ *$}}
; CHECK: Safety data: No issues found


%struct.test02 = type { i32*, i32*, i32* }
define void @test02(%struct.test02* %pStruct, i32* %value) !dtrans_type !7 {
  %pField = getelementptr %struct.test02, %struct.test02* %pStruct, i64 0, i32 1
  store i32* %value, i32** %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: 0)Field LLVM Type: i32*
; CHECK:   Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32*
; CHECK:   Field info: Written{{ *$}}
; CHECK: 2)Field LLVM Type: i32*
; CHECK:   Field info:{{ *$}}
; CHECK: Safety data: No issues found


%struct.test03a = type { %struct.test03b*, %struct.test03b*, %struct.test03b* }
%struct.test03b = type { i32, i32, i32 }
define void @test03(%struct.test03a* %pStruct, %struct.test03b* %value) !dtrans_type !12 {
  %pField = getelementptr %struct.test03a, %struct.test03a* %pStruct, i64 0, i32 2
  store %struct.test03b* %value, %struct.test03b** %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03a
; CHECK: 0)Field LLVM Type: %struct.test03b*
; CHECK:   Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: %struct.test03b*
; CHECK:   Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: %struct.test03b*
; CHECK:   Field info: Written{{ *$}}
; CHECK: Safety data: No issues found

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03b
; CHECK: 0)Field LLVM Type: i32
; CHECK:   Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK:   Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: i32
; CHECK:   Field info:{{ *$}}
; CHECK: Safety data: No issues found


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 2, !3, !4, !1}  ; void (%struct.test01*, i32)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{i32 0, i32 1}  ; i32*
!7 = !{!"F", i1 false, i32 2, !3, !8, !6}  ; void (%struct.test02*, i32*)
!8 = !{!9, i32 1}  ; %struct.test02*
!9 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!10 = !{!11, i32 1}  ; %struct.test03b*
!11 = !{!"R", %struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!12 = !{!"F", i1 false, i32 2, !3, !13, !10}  ; void (%struct.test03a*, %struct.test03b*)
!13 = !{!14, i32 1}  ; %struct.test03a*
!14 = !{!"R", %struct.test03a zeroinitializer, i32 0}  ; %struct.test03a
!15 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!16 = !{!"S", %struct.test02 zeroinitializer, i32 3, !6, !6, !6} ; { i32*, i32*, i32* }
!17 = !{!"S", %struct.test03a zeroinitializer, i32 3, !10, !10, !10} ; { %struct.test03b*, %struct.test03b*, %struct.test03b* }
!18 = !{!"S", %struct.test03b zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

!dtrans_types = !{!15, !16, !17, !18}
