; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='dtrans-deletefieldop' -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Check that byte-flattened GEP indices get updated for nested types.
; Also, checks that the memset call size is updated for the call
; involving the outer structure type.

%struct.A = type { i32, %struct.B, i32 }
%struct.B = type { i8, i16, i32 }
; CHECK-DAG: %__DFDT_struct.A = type { i32, %__DFT_struct.B, i32 }
; CHECK-DAG: %__DFT_struct.B = type { i16 }

define i32 @foo(ptr "intel_dtrans_func_index"="1" %a) !intel.dtrans.func.type !6 {
entry:
  call void @llvm.memset.p0.i64(ptr %a, i8 0, i64 16, i1 false)

  %px = getelementptr inbounds i8, ptr %a, i64 0 ; (A:0)
  %py = getelementptr inbounds i8, ptr %a, i64 12 ; (A:2)
  %0 = load i32, ptr %px, align 4
  %1 = load i32, ptr %py, align 4
  %2 = add i32 %0, %1

  ret i32 %2
}
; CHECK-LABEL: @foo
; CHECK: call void @llvm.memset
; CHECK-SAME: i64 12

; CHECK: getelementptr inbounds i8, ptr %a, i64 0
; CHECK: getelementptr inbounds i8, ptr %a, i64 8

define i16 @bar(ptr "intel_dtrans_func_index"="1" %a) !intel.dtrans.func.type !7 {
entry:
  %py = getelementptr inbounds i8, ptr %a, i64 6 ; (A:1:1)
  %0 = load i16, ptr %py, align 4
  ret i16 %0
}
; CHECK-LABEL: @bar
; CHECK: getelementptr inbounds i8, ptr %a, i64 0

declare !intel.dtrans.func.type !9 void @llvm.memset.p0.i64(ptr "intel_dtrans_func_index"="1", i8, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.B zeroinitializer, i32 0}  ; %struct.B
!3 = !{i8 0, i32 0}  ; i8
!4 = !{i16 0, i32 0}  ; i16
!5 = !{%struct.A zeroinitializer, i32 1}  ; %struct.A*
!6 = distinct !{!5}
!7 = distinct !{!5}
!8 = !{i8 0, i32 1}  ; i8*
!9 = distinct !{!8}
!10 = !{!"S", %struct.A zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.B, i32 }
!11 = !{!"S", %struct.B zeroinitializer, i32 3, !3, !4, !1} ; { i8, i16, i32 }

!intel.dtrans.types = !{!10, !11}
