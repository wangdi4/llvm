; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -S 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Check that the memcpy sizes are adjusted for cases of nested structures when
; fields are deleted from both the outer and inner structures.

; One field should be removed from each structure.
%struct.test01 = type { i32, i16, %struct.test01t }
%struct.test01t = type { i16, i32 }
; CHECK: %__DFT_struct.test01 = type { i32, %__DFT_struct.test01t }
; CHECK: %__DFT_struct.test01t = type { i32 }

; This case should update the memcpy size parameter based on the new outer
; structure type.
define i32 @test01(ptr "intel_dtrans_func_index"="1" %s1, ptr "intel_dtrans_func_index"="2" %s2) !intel.dtrans.func.type !5 {
  call void @llvm.memcpy.p0.p0.i64(ptr %s1, ptr %s2, i64 16, i1 false)
  %p3 = getelementptr %struct.test01, ptr %s1, i64 0, i32 0
  %r1 = load i32, ptr %p3
  %p4 = getelementptr %struct.test01, ptr %s1, i64 0, i32 2, i32 1
  %r2 = load i32, ptr %p4
  %r = add i32 %r1, %r2
  ret i32 %r
}
; CHECK-LABEL: define i32 @test01
; CHECK: call void @llvm.memcpy
; CHECK-SAME: i64 8
; CHECK: getelementptr %__DFT_struct.test01, ptr %s1, i64 0, i32 0
; CHECK: getelementptr %__DFT_struct.test01, ptr %s1, i64 0, i32 1, i32 0

; This case should update the memcpy size parameter based on the new inner
; structure type.
define i32 @test02(ptr "intel_dtrans_func_index"="1" %s1, ptr "intel_dtrans_func_index"="2" %s2) !intel.dtrans.func.type !7 {
  %pField = getelementptr %struct.test01, ptr %s2, i64 0, i32 2
  call void @llvm.memcpy.p0.p0.i64(ptr %s1, ptr %pField, i64 8, i1 false)
  ret i32 0
}
; CHECK-LABEL: define i32 @test02
; CHECK: getelementptr %__DFT_struct.test01, ptr %s2, i64 0, i32 1
; CHECK: call void @llvm.memcpy
; CHECK-SAME: i64 4

; This case should update the memcpy size parameter based on the new inner
; structure type.
define i32 @test03(ptr "intel_dtrans_func_index"="1" %s1, ptr "intel_dtrans_func_index"="2" %s2) !intel.dtrans.func.type !8 {
  %pField = getelementptr %struct.test01, ptr %s2, i64 0, i32 2
  call void @llvm.memcpy.p0.p0.i64(ptr %pField, ptr %s1, i64 8, i1 false)
  ret i32 0
}
; CHECK-LABEL: define i32 @test03
; CHECK: getelementptr %__DFT_struct.test01, ptr %s2, i64 0, i32 1
; CHECK: call void @llvm.memcpy
; CHECK-SAME: i64 4

declare !intel.dtrans.func.type !10 void @llvm.memcpy.p0.p0.i64(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i16 0, i32 0}  ; i16
!3 = !{%struct.test01t zeroinitializer, i32 0}  ; %struct.test01t
!4 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!5 = distinct !{!4, !4}
!6 = !{%struct.test01t zeroinitializer, i32 1}  ; %struct.test01t*
!7 = distinct !{!6, !4}
!8 = distinct !{!6, !4}
!9 = !{i8 0, i32 1}  ; i8*
!10 = distinct !{!9, !9}
!11 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, %struct.test01t }
!12 = !{!"S", %struct.test01t zeroinitializer, i32 2, !2, !1} ; { i16, i32 }

!intel.dtrans.types = !{!11, !12}
