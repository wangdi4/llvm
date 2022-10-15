; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; This test validates correct handling of various pointer arithmetic idioms
; involving subtraction with a constant integer.

; Pointer arithmetic to produce an integer, followed by integer arithmetic. This
; case is supported because it uses ptr-to-ptr types, and the subtraction result
; is only required to feed a division operation when the pointers have a single
; level of dereferencing.
%struct.test01 = type { i32, i32 }
define void @test01(%struct.test01** "intel_dtrans_func_index"="1" %p1, %struct.test01** "intel_dtrans_func_index"="2" %p2) !intel.dtrans.func.type !3 {
  ; (%p1 - %p2) - 8
  %t1 = ptrtoint %struct.test01** %p1 to i64
  %t2 = ptrtoint %struct.test01** %p2 to i64
  %delta = sub i64 %t1, %t2
  %offset = sub i64 %delta, 8
  %div = sdiv i64 %offset, 8
  ret void
}
; CHECK: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01

; This test is equivalent to @test01, except the evalution order of the
; subtraction has changed.
%struct.test02 = type { i32, i32 }
define void @test02(%struct.test02** "intel_dtrans_func_index"="1" %p1, %struct.test02** "intel_dtrans_func_index"="2" %p2) !intel.dtrans.func.type !5 {
  ; (%p1 - 8) - %p2
  %t1 = ptrtoint %struct.test02** %p1 to i64
  %t2 = ptrtoint %struct.test02** %p2 to i64
  %tmp = sub i64 %t1, 8
  %offset = sub i64 %tmp, %t2
  %div = sdiv i64 %offset, 8
  ret void
}
; CHECK: LLVMType: %struct.test02 = type { i32, i32 }
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test02

; This case is marked as "Bad pointer manipulation" due to not being a pattern
; that is needed currently. It could be supported in the future.
%struct.test03 = type { i32, i32 }
define void @test03(%struct.test03** "intel_dtrans_func_index"="1" %p1, %struct.test03** "intel_dtrans_func_index"="2" %p2) !intel.dtrans.func.type !7 {
    ; %p1 - (%p2 - 8)
  %t1 = ptrtoint %struct.test03** %p1 to i64
  %t2 = ptrtoint %struct.test03** %p2 to i64
  %tmp = sub i64 %t2, 8
  %offset = sub i64 %t1, %tmp
  %div = sdiv i64 %offset, 8
  ret void
}
; CHECK: LLVMType: %struct.test03 = type { i32, i32 }
; CHECK: Safety data: Bad pointer manipulation
; CHECK: End LLVMType: %struct.test03

; This case is set as "Bad pointer manipulation" because it is not a
; pointer-to-pointer, and the result of the first pointer subtraction does not
; feed a division instruction. It needs to be "Bad pointer manipulation" because
; the transformations do not support locating the constant used for the subtract
; if the size of structure needs to be changed.
%struct.test04 = type { i32, i32 }
define void @test04(%struct.test04* "intel_dtrans_func_index"="1" %p1, %struct.test04* "intel_dtrans_func_index"="2" %p2) !intel.dtrans.func.type !9 {
  ; (%p1 - 8) - %p2
  %t1 = ptrtoint %struct.test04* %p1 to i64
  %t2 = ptrtoint %struct.test04* %p2 to i64
  %tmp = sub i64 %t1, 8
  %offset = sub i64 %tmp, %t2
  %div = sdiv i64 %offset, 8
  ret void
}
; CHECK: LLVMType: %struct.test04 = type { i32, i32 }
; CHECK: Safety data: Bad pointer manipulation
; CHECK: End LLVMType: %struct.test04

; This case is set as "Bad pointer manipulation" because the pointers used in
; the subtract instruction are addresses of structure fields.
%struct.test05 = type { i32, i32 }
define void @test05(%struct.test05* "intel_dtrans_func_index"="1" %s1, %struct.test05* "intel_dtrans_func_index"="2" %s2) !intel.dtrans.func.type !11 {
  ; (%p1 - 4) - %p2
  %p1 = getelementptr %struct.test05, %struct.test05* %s1, i64 0, i32 1
  %p2 = getelementptr %struct.test05, %struct.test05* %s2, i64 0, i32 1
  %t1 = ptrtoint i32* %p1 to i64
  %t2 = ptrtoint i32* %p2 to i64
  %tmp = sub i64 %t1, 4
  %offset = sub i64 %tmp, %t2
  %div = sdiv i64 %offset, 4
  ret void
}
; CHECK: LLVMType: %struct.test05 = type { i32, i32 }
; CHECK: Safety data: Bad pointer manipulation
; CHECK: End LLVMType: %struct.test05

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 2}  ; %struct.test01**
!3 = distinct !{!2, !2}
!4 = !{%struct.test02 zeroinitializer, i32 2}  ; %struct.test02**
!5 = distinct !{!4, !4}
!6 = !{%struct.test03 zeroinitializer, i32 2}  ; %struct.test03**
!7 = distinct !{!6, !6}
!8 = !{%struct.test04 zeroinitializer, i32 1}  ; %struct.test04*
!9 = distinct !{!8, !8}
!10 = !{%struct.test05 zeroinitializer, i32 1}  ; %struct.test05*
!11 = distinct !{!10, !10}
!12 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!13 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!14 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!15 = !{!"S", %struct.test04 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!16 = !{!"S", %struct.test05 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!12, !13, !14, !15, !16}
