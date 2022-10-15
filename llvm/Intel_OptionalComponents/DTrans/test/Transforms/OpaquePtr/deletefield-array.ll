; RUN: opt %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -S 2>&1 | FileCheck %s
; RUN: opt %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-deletefieldop -S 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the DTrans delete pass correctly transforms
; structure element accesses for an array of structures.

; Check that all unused fields are deleted and GEPs are updated.
; Fields (A:0) and (A:2) should stay.

%struct.A = type { i32, i32, i32 }
; CHECK: %__DFT_struct.A = type { i32, i32 }

define i32 @bar([4 x %struct.A]* "intel_dtrans_func_index"="1" %a) !intel.dtrans.func.type !5 {
entry:
  %x = getelementptr inbounds [4 x %struct.A], [4 x %struct.A]* %a, i64 0, i64 0, i32 0
  %tmp0 = load i32, i32* %x, align 4
  %z = getelementptr inbounds [4 x %struct.A], [4 x %struct.A]* %a, i64 0, i64 3, i32 2
  %tmp1 = load i32, i32* %z, align 4
  %add = add nsw i32 %tmp1, %tmp0
  ret i32 %add
}
; CHECK-LABEL: define internal i32 @bar
; CHECK: %x = getelementptr inbounds [4 x %__DFT_struct.A], {{.*}} %a, i64 0, i64 0, i32 0
; CHECK: %z = getelementptr inbounds [4 x %__DFT_struct.A], {{.*}} %a, i64 0, i64 3, i32 1


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!3, i32 1}  ; [4 x %struct.A]*
!3 = !{!"A", i32 4, !4}  ; [4 x %struct.A]
!4 = !{%struct.A zeroinitializer, i32 0}  ; %struct.A
!5 = distinct !{!2}
!6 = !{!"S", %struct.A zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

!intel.dtrans.types = !{!6}
