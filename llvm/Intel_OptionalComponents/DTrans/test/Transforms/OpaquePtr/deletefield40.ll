; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='dtrans-deletefieldop' -dtrans-outofboundsok=false -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test checks that the delete fields transformation happens with enclosed
; structures. For this case, the fields 0 and 2 in %struct.B will be deleted
; even if it is enclosed in %struct.A.

%struct.A = type { i32, %struct.B, i32 }
%struct.B = type { i8, i16, i32 }
; CHECK-DAG: %__DFDT_struct.A = type { i32, %__DFT_struct.B, i32 }
; CHECK-DAG: %__DFT_struct.B = type { i16 }

define i32 @foo(ptr "intel_dtrans_func_index"="1" %a, ptr "intel_dtrans_func_index"="2" %q) !intel.dtrans.func.type !7 {
entry:
  call void @llvm.memset.p0.i64(ptr %a, i8 0, i64 16, i1 false)

  %x = getelementptr inbounds %struct.A, ptr %a, i64 0, i32 0
  %y = getelementptr inbounds %struct.A, ptr %a, i64 0, i32 2
  %0 = load i32, ptr %x, align 4
  %1 = load i32, ptr %y, align 4
  %2 = add i32 %0, %1

  store ptr %x, ptr %q

  ret i32 %2
}
; CHECK-LABEL: define i32 @foo
; CHECK: call void @llvm.memset
; CHECK-SAME: i64 12
; CHECK: %x = getelementptr inbounds %__DFDT_struct.A, ptr %a, i64 0, i32 0
; CHECK: %y = getelementptr inbounds %__DFDT_struct.A, ptr %a, i64 0, i32 2

define i16 @bar(ptr "intel_dtrans_func_index"="1" %b) !intel.dtrans.func.type !9 {
entry:
  %y = getelementptr inbounds %struct.B, ptr %b, i64 0, i32 1
  %0 = load i16, ptr %y, align 4
  ret i16 %0
}
; CHECK-LABEL: define i16 @bar
; CHECK: %y = getelementptr inbounds %__DFT_struct.B, ptr %b, i64 0, i32 0

declare !intel.dtrans.func.type !11 void @llvm.memset.p0.i64(ptr "intel_dtrans_func_index"="1", i8, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.B zeroinitializer, i32 0}  ; %struct.B
!3 = !{i8 0, i32 0}  ; i8
!4 = !{i16 0, i32 0}  ; i16
!5 = !{%struct.A zeroinitializer, i32 1}  ; %struct.A*
!6 = !{i32 0, i32 2}  ; i32**
!7 = distinct !{!5, !6}
!8 = !{%struct.B zeroinitializer, i32 1}  ; %struct.B*
!9 = distinct !{!8}
!10 = !{i8 0, i32 1}  ; i8*
!11 = distinct !{!10}
!12 = !{!"S", %struct.A zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.B, i32 }
!13 = !{!"S", %struct.B zeroinitializer, i32 3, !3, !4, !1} ; { i8, i16, i32 }

!intel.dtrans.types = !{!12, !13}
