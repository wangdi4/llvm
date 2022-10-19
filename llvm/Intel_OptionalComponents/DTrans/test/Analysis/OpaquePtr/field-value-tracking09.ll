; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that a call to memset updates the field value info.

; Test memset that writes 0 to the entire structure.
%struct.test01 = type { i32, float }
define "intel_dtrans_func_index"="1" %struct.test01* @test01() !intel.dtrans.func.type !4 {
  %ptr = call i8* @malloc(i64 8)
  call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 8, i1 false)

  %obj = bitcast i8* %ptr to %struct.test01*
  ret %struct.test01* %obj
}
; CHECK-LABEL: LLVMType: %struct.test01
; CHECK:   0)Field LLVM Type: i32
; CHECK:     Single Value: i32 0
; CHECK:   1)Field LLVM Type: float
; CHECK:     Single Value: float 0.000000e+00
; CHECK:   Safety data: No issues found{{ *$}}
; CHECK: End LLVMType: %struct.test01


; Test memset call that sets a non-zero value.
%struct.test02 = type { i32, float }
define "intel_dtrans_func_index"="1" %struct.test02* @test02() !intel.dtrans.func.type !6 {
  %ptr = call i8* @malloc(i64 8)
  call void @llvm.memset.p0i8.i64(i8* %ptr, i8 205, i64 8, i1 false)

  %obj = bitcast i8* %ptr to %struct.test02*
  ret %struct.test02* %obj
}
; CHECK-LABEL: LLVMType: %struct.test02
; CHECK:   0)Field LLVM Type: i32
; CHECK:     Multiple Value: [  ] <incomplete>
; CHECK:   1)Field LLVM Type: float
; CHECK:     Multiple Value: [  ] <incomplete>
; CHECK:   Safety data: No issues found{{ *$}}
; CHECK: End LLVMType: %struct.test02


; Test memset that writes 0 to the entire structure when allocating and setting
; a multiple of the structure size.
%struct.test03 = type { i32, float }
define "intel_dtrans_func_index"="1" %struct.test03* @test03() !intel.dtrans.func.type !8 {
  %ptr = call i8* @malloc(i64 64)
  call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 64, i1 false)

  %obj = bitcast i8* %ptr to %struct.test03*
  ret %struct.test03* %obj
}
; CHECK-LABEL: LLVMType: %struct.test03
; CHECK:   0)Field LLVM Type: i32
; CHECK:     Single Value: i32 0
; CHECK:   1)Field LLVM Type: float
; CHECK:     Single Value: float 0.000000e+00
; CHECK:   Safety data: No issues found{{ *$}}
; CHECK: End LLVMType: %struct.test03


; Test memset that writes 0 to part of the structure.
%struct.test04 = type { i32, i32, i32 }
define "intel_dtrans_func_index"="1" %struct.test04* @test04() !intel.dtrans.func.type !10 {
  %ptr = call i8* @malloc(i64 12)
  call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 8, i1 false)

  %obj = bitcast i8* %ptr to %struct.test04*
  ret %struct.test04* %obj
}
; CHECK-LABEL: LLVMType: %struct.test04
; CHECK:   0)Field LLVM Type: i32
; CHECK:     Single Value: i32 0
; CHECK:   1)Field LLVM Type: i32
; CHECK:     Single Value: i32 0
; CHECK:   2)Field LLVM Type: i32
; CHECK:     No Value
; CHECK:   Safety data: Memfunc partial write{{ *$}}
; CHECK: End LLVMType: %struct.test04


; Test memset that writes 0 to part of the structure that does not set all
; of the bytes for a field.
%struct.test05 = type { i32, i32, i32 }
define "intel_dtrans_func_index"="1" %struct.test05* @test05() !intel.dtrans.func.type !12 {
  %ptr = call i8* @malloc(i64 12)
  call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 6, i1 false)

  %obj = bitcast i8* %ptr to %struct.test05*
  ret %struct.test05* %obj
}
; The analysis for this case can be improved to not mark fields 0 and 2 as
; incomplete. Instead, field 0, could be a single value of 0, and field 2
; could be left as 'No value'.

; CHECK-LABEL: LLVMType: %struct.test05
; CHECK:   0)Field LLVM Type: i32
; CHECK:     Multiple Value: [  ] <incomplete>
; CHECK:   1)Field LLVM Type: i32
; CHECK:     Multiple Value: [  ] <incomplete>
; CHECK:   2)Field LLVM Type: i32
; CHECK:     Multiple Value: [  ] <incomplete>
; CHECK:   Safety data: Bad memfunc size{{ *$}}
; CHECK: End LLVMType: %struct.test05


declare !intel.dtrans.func.type !14 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1" nocapture writeonly, i8, i64, i1)
declare !intel.dtrans.func.type !15 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{float 0.0e+00, i32 0}  ; float
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = distinct !{!3}
!5 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!6 = distinct !{!5}
!7 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!8 = distinct !{!7}
!9 = !{%struct.test04 zeroinitializer, i32 1}  ; %struct.test04*
!10 = distinct !{!9}
!11 = !{%struct.test05 zeroinitializer, i32 1}  ; %struct.test05*
!12 = distinct !{!11}
!13 = !{i8 0, i32 1}  ; i8*
!14 = distinct !{!13}
!15 = distinct !{!13}
!16 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i32, float }
!17 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !2} ; { i32, float }
!18 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !2} ; { i32, float }
!19 = !{!"S", %struct.test04 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!20 = !{!"S", %struct.test05 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

!intel.dtrans.types = !{!16, !17, !18, !19, !20}
