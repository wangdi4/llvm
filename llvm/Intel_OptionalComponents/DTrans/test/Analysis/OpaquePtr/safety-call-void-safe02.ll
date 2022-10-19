; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test passing a pointer to an aggregate type as an i8* type that is safe for
; DTrans.

; Test with using the i8* parameter as a type that is compatible with the use in
; the caller.
define void @test01(%struct.test01** "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !2 {
  %ps_addr0 = getelementptr %struct.test01*, %struct.test01** %pStruct, i64 0
  %ps_addr1 = getelementptr %struct.test01*, %struct.test01** %pStruct, i64 1
  %ps0 = load %struct.test01*, %struct.test01** %ps_addr0
  %ps1 = load %struct.test01*, %struct.test01** %ps_addr1
  %less = call i1 bitcast (i1 (i8*, i8*)* @test01less to
               i1 (%struct.test01*, %struct.test01*)*)(%struct.test01* %ps0, %struct.test01* %ps1)
  ret void
}

%struct.test01 = type { i32, i32 }
define i1 @test01less(i8* "intel_dtrans_func_index"="1" %p0, i8* "intel_dtrans_func_index"="2" %p1) !intel.dtrans.func.type !5 {
  %ps0 = bitcast i8* %p0 to %struct.test01*
  %ps1 = bitcast i8* %p1 to %struct.test01*
  %fs0 = getelementptr %struct.test01, %struct.test01* %ps0, i64 0, i32 0
  %fs1 = getelementptr %struct.test01, %struct.test01* %ps1, i64 0, i32 0
  %v0 = load i32, i32* %fs0
  %v1 = load i32, i32* %fs1
  %cmp = icmp slt i32 %v0, %v1
  ret i1 %cmp
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01


!1 = !{%struct.test01 zeroinitializer, i32 2}  ; %struct.test01**
!2 = distinct !{!1}
!3 = !{i32 0, i32 0}  ; i32
!4 = !{i8 0, i32 1}  ; i8*
!5 = distinct !{!4, !4}
!6 = !{!"S", %struct.test01 zeroinitializer, i32 2, !3, !3} ; { i32, i32 }

!intel.dtrans.types = !{!6}
