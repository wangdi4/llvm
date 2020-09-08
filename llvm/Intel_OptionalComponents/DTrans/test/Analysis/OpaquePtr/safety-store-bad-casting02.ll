; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test storing a pointer to an aggregate type that gets used as multiple types
; does not result in a failure.

%struct.test01.base = type { i32 (...)** }
%struct.test01.derived1 = type { %struct.test01.base }
%struct.test01.derived2 = type { %struct.test01.base }

@testVar01 = internal unnamed_addr global %struct.test01.base* null, !dtrans_type !5

define void @test01(%struct.test01.base* %pStruct.in) !dtrans_type !6 {
  ; This store instruction was causing a crash on the DTransSafetyAnalyzer
  ; pass in an earlier change set.
  store %struct.test01.base* %pStruct.in, %struct.test01.base** @testVar01

  ; Use the pointer as different pointer types than it was declared as.
  %tmp5 = bitcast %struct.test01.base* %pStruct.in to i8* (%struct.test01.base*, i64)***
  %tmp6 = load i8* (%struct.test01.base*, i64)**, i8* (%struct.test01.base*, i64)*** %tmp5
  %tmp7 = getelementptr inbounds i8* (%struct.test01.base*, i64)*, i8* (%struct.test01.base*, i64)** %tmp6, i64 2
  %tmp8 = load i8* (%struct.test01.base*, i64)*, i8* (%struct.test01.base*, i64)** %tmp7
  %tmp9 = bitcast i8* (%struct.test01.base*, i64)* %tmp8 to i8*
  %tmp10 = bitcast i8* (%struct.test01.derived1*, i64)* @test01a to i8*
  %tmp11 = icmp eq i8* %tmp9, %tmp10
  br i1 %tmp11, label %type1, label %type2

type1:
  %tmp13 = tail call i8* bitcast (i8* (%struct.test01.derived1*, i64)* @test01a to i8* (%struct.test01.base*, i64)*)(%struct.test01.base* nonnull %pStruct.in, i64 48)
  br label %done

type2:
  %tmp15 = tail call i8* bitcast (i8* (%struct.test01.derived2*, i64)* @test01b to i8* (%struct.test01.base*, i64)*)(%struct.test01.base* nonnull %pStruct.in, i64 48)
  br label %done
done:
  ret void
}

define internal i8* @test01a(%struct.test01.derived1* %0, i64 %1) !dtrans_type !8 {
  ret i8* null
}

define internal i8* @test01b(%struct.test01.derived2* %0, i64 %1) !dtrans_type !13 {
  ret i8* null
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01.base
; CHECK: Safety data: Bad casting | Mismatched element access | Global pointer | Nested structure | Has vtable{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01.derived1
; CHECK: Safety data: Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01.derived2
; CHECK: Safety data: Contains nested structure{{ *$}}


!1 = !{!"F", i1 true, i32 0, !2}  ; i32 (...)
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!1, i32 2}  ; i32 (...)**
!4 = !{!"R", %struct.test01.base zeroinitializer, i32 0}  ; %struct.test01.base
!5 = !{!4, i32 1}  ; %struct.test01.base*
!6 = !{!"F", i1 false, i32 1, !7, !5}  ; void (%struct.test01.base*)
!7 = !{!"void", i32 0}  ; void
!8 = !{!"F", i1 false, i32 2, !9, !10, !12}  ; i8* (%struct.test01.derived1*, i64)
!9 = !{i8 0, i32 1}  ; i8*
!10 = !{!11, i32 1}  ; %struct.test01.derived1*
!11 = !{!"R", %struct.test01.derived1 zeroinitializer, i32 0}  ; %struct.test01.derived1
!12 = !{i64 0, i32 0}  ; i64
!13 = !{!"F", i1 false, i32 2, !9, !14, !12}  ; i8* (%struct.test01.derived2*, i64)
!14 = !{!15, i32 1}  ; %struct.test01.derived2*
!15 = !{!"R", %struct.test01.derived2 zeroinitializer, i32 0}  ; %struct.test01.derived2
!16 = !{!"S", %struct.test01.base zeroinitializer, i32 1, !3} ; { i32 (...)** }
!17 = !{!"S", %struct.test01.derived1 zeroinitializer, i32 1, !4} ; { %struct.test01.base }
!18 = !{!"S", %struct.test01.derived2 zeroinitializer, i32 1, !4} ; { %struct.test01.base }

!dtrans_types = !{!16, !17, !18}
