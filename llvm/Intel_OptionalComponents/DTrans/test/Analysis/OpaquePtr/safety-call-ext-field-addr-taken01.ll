; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test passing the address of a field to a function to trigger the "Field
; address taken call" safety bit when calling an external function.

@str = private constant [4 x i8] c"YES\00"
%struct.test01 = type { i32, i8, i8, i8, i8 }
define void @test01() {
  %pStruct = alloca %struct.test01
  %pFieldAddr = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 1
  %src = getelementptr [4 x i8], [4 x i8]* @str, i64 0, i32 0
  %res = call i8* @strcpy(i8* %pFieldAddr, i8* %src)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Local instance | Field address taken call{{ *$}}
; CHECK: End LLVMType: %struct.test01


declare !intel.dtrans.func.type !4 "intel_dtrans_func_index"="1" i8* @strcpy(i8* "intel_dtrans_func_index"="2", i8* "intel_dtrans_func_index"="3")

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 0}  ; i8
!3 = !{i8 0, i32 1}  ; i8*
!4 = distinct !{!3, !3, !3}
!5 = !{!"S", %struct.test01 zeroinitializer, i32 5, !1, !2, !2, !2, !2} ; { i32, i8, i8, i8, i8 }

!intel.dtrans.types = !{!5}
