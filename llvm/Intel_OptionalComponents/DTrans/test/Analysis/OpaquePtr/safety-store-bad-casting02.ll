; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test storing a pointer to an aggregate type that gets used as multiple types
; does not result in a failure.

%struct.test01.base = type { i32 (...)** }
%struct.test01.derived1 = type { %struct.test01.base }
%struct.test01.derived2 = type { %struct.test01.base }

@testVar01 = internal unnamed_addr global %struct.test01.base* null, !intel_dtrans_type !5

define void @test01(%struct.test01.base* "intel_dtrans_func_index"="1" %pStruct.in) !intel.dtrans.func.type !6 {
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

define internal "intel_dtrans_func_index"="1" i8* @test01a(%struct.test01.derived1* "intel_dtrans_func_index"="2" %0, i64 %1) !intel.dtrans.func.type !9 {
  ret i8* null
}

define internal "intel_dtrans_func_index"="1" i8* @test01b(%struct.test01.derived2* "intel_dtrans_func_index"="2" %0, i64 %1) !intel.dtrans.func.type !11 {
  ret i8* null
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01.base
; CHECK: Safety data: Bad casting | Mismatched element access | Global pointer | Nested structure | Has vtable{{ *$}}
; CHECK: End LLVMType: %struct.test01.base

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01.derived1
; CHECK: Safety data: Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01.derived1

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01.derived2
; CHECK: Safety data: Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01.derived2


!1 = !{!"F", i1 true, i32 0, !2}  ; i32 (...)
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!1, i32 2}  ; i32 (...)**
!4 = !{%struct.test01.base zeroinitializer, i32 0}  ; %struct.test01.base
!5 = !{%struct.test01.base zeroinitializer, i32 1}  ; %struct.test01.base*
!6 = distinct !{!5}
!7 = !{i8 0, i32 1}  ; i8*
!8 = !{%struct.test01.derived1 zeroinitializer, i32 1}  ; %struct.test01.derived1*
!9 = distinct !{!7, !8}
!10 = !{%struct.test01.derived2 zeroinitializer, i32 1}  ; %struct.test01.derived2*
!11 = distinct !{!7, !10}
!12 = !{!"S", %struct.test01.base zeroinitializer, i32 1, !3} ; { i32 (...)** }
!13 = !{!"S", %struct.test01.derived1 zeroinitializer, i32 1, !4} ; { %struct.test01.base }
!14 = !{!"S", %struct.test01.derived2 zeroinitializer, i32 1, !4} ; { %struct.test01.base }

!intel.dtrans.types = !{!12, !13, !14}
