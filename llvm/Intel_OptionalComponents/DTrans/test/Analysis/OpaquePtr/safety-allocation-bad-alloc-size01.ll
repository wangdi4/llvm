; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test memory allocations that are not the expected structure size for DTrans

%struct.test01 = type { i32, i32, i32, i32 }
define void @test01() {
  ; mem = (struct test01*)malloc(20);
  %mem = call i8* @malloc(i64 20)
  %mem.as.pStruct = bitcast i8* %mem to %struct.test01*
  %a.addr = getelementptr %struct.test01, %struct.test01* %mem.as.pStruct, i64 0, i32 2
  store i32 1, i32* %a.addr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Bad alloc size{{ *$}}
; CHECK: End LLVMType: %struct.test01


; This structure will be allocated with a variable size argument that cannot
; be proven to be a multiple of the structure size.
%struct.test02 = type { i32, i32, i32 }
define void @test02(i64 %num1, i64 %num2, %struct.test02** "intel_dtrans_func_index"="1" %result) !intel.dtrans.func.type !3 {
  ; mem = (struct test02*)malloc((num1 * 19) * num2);
  %mul1 = mul i64 %num1, 19
  %mul2 = mul i64 %mul1, %num2
  %mem = call noalias i8* @malloc(i64 %mul2)
  %mem.as.pStruct = bitcast i8* %mem to %struct.test02*
  store %struct.test02* %mem.as.pStruct, %struct.test02** %result
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Bad alloc size{{ *$}}
; CHECK: End LLVMType: %struct.test02


; This structure will be allocated with a variable size argument that we don't
; know anything about.
%struct.test03 = type { i32, i32 }
@var03 = internal global %struct.test03* zeroinitializer, !intel_dtrans_type !4
define void @test03(i64 %size) {
  ; mem = (struct test03*)malloc(<some random size>);
  %mem = call noalias i8* @malloc(i64 %size)
  %mem.as.pStruct = bitcast i8* %mem to %struct.test03*
  store %struct.test03* %mem.as.pStruct, %struct.test03** @var03
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: Bad alloc size | Global pointer{{ *$}}
; CHECK: End LLVMType: %struct.test03


; Here we make sure that 'sext' instruction handling isn't masking a bad size.
%struct.test04 = type { i32, i32, i32 }
@var04 = internal global %struct.test04* zeroinitializer, !intel_dtrans_type !5
define void @test04(i32 %num1, i64 %num2) {
  ; mem = (struct test04*)malloc((num1 * 19) * num2);
  %mul1 = mul i32 %num1, 19
  %tmp = sext i32 %mul1 to i64
  %mul2 = mul i64 %tmp, %num2
  %mem = call noalias i8* @malloc(i64 %mul2)
  store i8* %mem, i8** bitcast (%struct.test04** @var04 to i8**)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04
; CHECK: Safety data: Bad alloc size | Global pointer{{ *$}}
; CHECK: End LLVMType: %struct.test04


; Here we make sure that we don't fail on zero size arrays.
%struct.test05 = type { i32, i32, i32 }
define void @test05() {
  %mem = call noalias i8* @malloc(i64 0)
  %mem.as.array = bitcast i8* %mem to [0 x %struct.test05]*
  %field = getelementptr [0 x %struct.test05], [0 x %struct.test05]* %mem.as.array, i64 0, i32 0, i32 1
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05
; CHECK: Safety data: Bad alloc size{{ *$}}
; CHECK: End LLVMType: %struct.test05


declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test02 zeroinitializer, i32 2}  ; %struct.test02**
!3 = distinct !{!2}
!4 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!5 = !{%struct.test04 zeroinitializer, i32 1}  ; %struct.test04*
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 4, !1, !1, !1, !1} ; { i32, i32, i32, i32 }
!9 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!10 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!11 = !{!"S", %struct.test04 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!12 = !{!"S", %struct.test05 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

!intel.dtrans.types = !{!8, !9, !10, !11, !12}
