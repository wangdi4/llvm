; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test where the result of a GEP does not lead directly to the load/store.

; The fields indexed by the GEP in this case need to be marked with the
; 'ComplexUse' flag because transformations such as 'Delete field' only
; support cases where the GEP result is used for a load/store because
; those instructions can easily be deleted.
%struct.test01 = type { i32, i32, i32 }
define internal i32 @test01(%struct.test01* %pStruct) !dtrans_type !7 {
  %field0 = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 0
  %field2 = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 2
  %fieldAddr = select i1 undef, i32* %field0, i32* %field2
  %val = load i32, i32* %fieldAddr
  ret i32 %val
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:
; CHECK-SAME: ComplexUse
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:
; CHECK-NOT: ComplexUse
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info:
; CHECK-SAME: ComplexUse
; CHECK: Safety data: No issues found


; Getting the address of a field for a call to 'memset' is also considered a
; 'ComplexUse'
%struct.test02 = type { i32, i16, i32 }
define internal void @test02(i32 %x) {
  %local = alloca %struct.test02
  %flat = bitcast %struct.test02* %local to i8*
  %pad_addr = getelementptr i8, i8* %flat, i32 0
  call void @llvm.memset.p0i8.i64(i8* %pad_addr, i8 0, i64 12, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:
; CHECK-SAME: ComplexUse
; CHECK: 1)Field LLVM Type: i16
; CHECK: Field info:
; CHECK-NOT: ComplexUse
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info:
; CHECK-NOT: ComplexUse


declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !1, !3}  ; i32 (%struct.test01*)
!3 = !{!4, i32 1}  ; %struct.test01*
!4 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!5 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

!dtrans_types = !{!5}

!6 = !{i32 0, i32 0}  ; i32
!7 = !{!"F", i1 false, i32 1, !6, !8}  ; i32 (%struct.test01*)
!8 = !{!9, i32 1}  ; %struct.test01*
!9 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!10 = !{i16 0, i32 0}  ; i16
!11 = !{!"S", %struct.test01 zeroinitializer, i32 3, !6, !6, !6} ; { i32, i32, i32 }
!12 = !{!"S", %struct.test02 zeroinitializer, i32 3, !6, !10, !6} ; { i32, i16, i32 }

!dtrans_types = !{!11, !12}
