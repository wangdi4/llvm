; RUN: opt -whole-program-assume -dtrans-deletefieldop -S -o - %s | FileCheck %s
; RUN: opt -whole-program-assume -passes='dtrans-deletefieldop' -S -o - %s | FileCheck %s

; Check that the memset call size is updated for a call involving the outer
; structure type, since that is a dependent type.

%struct.A = type { i32, %struct.B, i32 }
%struct.B = type { i8, i16, i32 }
; CHECK-DAG: %__DFDT_struct.A = type { i32, %__DFT_struct.B, i32 }
; CHECK-DAG: %__DFT_struct.B = type { i16 }

define i32 @foo(%struct.A* "intel_dtrans_func_index"="1" %a) !intel.dtrans.func.type !6 {
entry:
  %p = bitcast %struct.A* %a to i8*
  call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 16, i1 false)

  %x = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 0
  %y = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 2
  %0 = load i32, i32* %x, align 4
  %1 = load i32, i32* %y, align 4
  %2 = add i32 %0, %1

  ret i32 %2
}
; CHECK-LABEL: define internal i32 @foo
; CHECK: call void @llvm.memset
; CHECK-SAME: i64 12

; CHECK: getelementptr inbounds %__DFDT_struct.A, {{.*}} %a, i64 0, i32 0
; CHECK: getelementptr inbounds %__DFDT_struct.A, {{.*}} %a, i64 0, i32 2

define i16 @bar(%struct.B* "intel_dtrans_func_index"="1" %b) !intel.dtrans.func.type !8 {
entry:
  %y = getelementptr inbounds %struct.B, %struct.B* %b, i64 0, i32 1
  %0 = load i16, i16* %y, align 4
  ret i16 %0
}
; CHECK-LABEL: define internal i16 @bar
; CHECK: getelementptr inbounds %__DFT_struct.B, {{.*}} %b, i64 0, i32 0

declare !intel.dtrans.func.type !10 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.B zeroinitializer, i32 0}  ; %struct.B
!3 = !{i8 0, i32 0}  ; i8
!4 = !{i16 0, i32 0}  ; i16
!5 = !{%struct.A zeroinitializer, i32 1}  ; %struct.A*
!6 = distinct !{!5}
!7 = !{%struct.B zeroinitializer, i32 1}  ; %struct.B*
!8 = distinct !{!7}
!9 = !{i8 0, i32 1}  ; i8*
!10 = distinct !{!9}
!11 = !{!"S", %struct.A zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.B, i32 }
!12 = !{!"S", %struct.B zeroinitializer, i32 3, !3, !4, !1} ; { i8, i16, i32 }

!intel.dtrans.types = !{!11, !12}
