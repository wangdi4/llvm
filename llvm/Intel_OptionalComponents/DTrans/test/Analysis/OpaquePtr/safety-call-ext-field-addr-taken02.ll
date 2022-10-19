; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test passing the address of an array element which has the same address as a
; structure type. This should cause the "Field address taken call" to be set.
; When the function takes an i8* type and the array is composed of i8 types, it
; should not set "Address taken" on the structure's "Safety data" flags for
; a call to a safe library function. NOTE: The field is still marked with
; "AddressTaken" for the fields that trigger the "Field address taken call"
; safety flag.

; Pass the address to a library function that is known to take i8* parameters.
%struct.test01 = type { [200 x i8], [200 x i8], i64, i64 }
@net01 = internal global %struct.test01 zeroinitializer
define void @test01() {
  %tmp = call i8* @strcpy(i8* getelementptr (%struct.test01, %struct.test01* @net01, i64 0, i32 0, i64 0),
                          i8* getelementptr (%struct.test01, %struct.test01* @net01, i64 0, i32 1, i64 0))
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: 0)Field
; CHECK: Field info: AddressTaken{{ *$}}
; CHECK: 1)Field
; CHECK: Field info: AddressTaken{{ *$}}
; CHECK: 2)Field
; CHECK: Field info:{{ *$}}
; CHECK: 3)Field
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Global instance | Field address taken call{{ *$}}
; CHECK: End LLVMType: %struct.test01


; Pass the address in a VarArg position of a library function call. When the
; definition of a called function cannot be seen, this would normally need to
; result in "Address taken", but for some known library functions we need to
; treat them as safe.
%struct.test02 = type { [200 x i8], [200 x i8], i64, i64 }
@net02 = internal global %struct.test02 zeroinitializer
@str02 = private constant [31 x i8] c"network %s: not enough memory\0A\00"
define void @test02() {
  %tmp = call i32 (i8*, ...) @printf(i8* getelementptr ([31 x i8], [31 x i8]* @str02, i64 0, i64 0),
                                     i8* getelementptr (%struct.test02, %struct.test02* @net02, i64 0, i32 0, i64 0))
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: 0)Field
; CHECK: Field info: AddressTaken{{ *$}}
; CHECK: 1)Field
; CHECK: Field info:{{ *$}}
; CHECK: 2)Field
; CHECK: Field info:{{ *$}}
; CHECK: 3)Field
; CHECK: Field info:{{ *$}}
; CHECK: Safety data:  Global instance | Field address taken call{{ *$}}
; CHECK: End LLVMType: %struct.test02


declare !intel.dtrans.func.type !5 "intel_dtrans_func_index"="1" i8* @strcpy(i8* "intel_dtrans_func_index"="2", i8* "intel_dtrans_func_index"="3")
declare !intel.dtrans.func.type !6 i32 @printf(i8* "intel_dtrans_func_index"="1", ...)

!1 = !{!"A", i32 200, !2}  ; [200 x i8]
!2 = !{i8 0, i32 0}  ; i8
!3 = !{i64 0, i32 0}  ; i64
!4 = !{i8 0, i32 1}  ; i8*
!5 = distinct !{!4, !4, !4}
!6 = distinct !{!4}
!7 = !{!"S", %struct.test01 zeroinitializer, i32 4, !1, !1, !3, !3} ; { [200 x i8], [200 x i8], i64, i64 }
!8 = !{!"S", %struct.test02 zeroinitializer, i32 4, !1, !1, !3, !3} ; { [200 x i8], [200 x i8], i64, i64 }

!intel.dtrans.types = !{!7, !8}
