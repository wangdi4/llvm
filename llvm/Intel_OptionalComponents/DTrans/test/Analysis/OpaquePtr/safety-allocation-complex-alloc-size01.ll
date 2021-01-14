; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test memory allocations that are of the form: malloc(c1 + c2 * size)

; This case is recognized as "Complex alloc size" for the EliminateROAccess
; transformation
%struct.test01 = type { i32 }
@var01 = internal global %struct.test01* zeroinitializer, !dtrans_type !2
define void @test01(i64 %n) {
  %shl = mul i64 %n, 4
  %add = add i64 %shl, 4
  %call = call noalias i8* @malloc(i64 %add)
  store i8* %call, i8** bitcast (%struct.test01** @var01 to i8**)
  ret void
}
define void @test01caller() {
  ; We need this call to verify the input to @test01 is constant
  call void @test01(i64 1)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Global pointer | Complex alloc size{{ *$}}


; This case is not supported because the multiplication is not a multiple of
; the structure size, and will marked as "Bad alloc size""
%struct.test02 = type { i32 }
@var02 = internal global %struct.test02* zeroinitializer, !dtrans_type !4
define void @test02(i64 %n) {
  %shl = mul i64 %n, 3
  %add = add i64 %shl, 2
  %call = call noalias i8* @malloc(i64 %add)
  store i8* %call, i8** bitcast (%struct.test02** @var02 to i8**)
  ret void
}
define void @test02caller() {
  ; We need this call to verify the input to @test02 is constant
  call void @test02(i64 1)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Bad alloc size | Global pointer{{ *$}}


declare i8* @malloc(i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!3, i32 1}  ; %struct.test01*
!3 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!4 = !{!5, i32 1}  ; %struct.test02*
!5 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!6 = !{!"S", %struct.test01 zeroinitializer, i32 1, !1} ; { i32 }
!7 = !{!"S", %struct.test02 zeroinitializer, i32 1, !1} ; { i32 }

!dtrans_types = !{!6, !7}
