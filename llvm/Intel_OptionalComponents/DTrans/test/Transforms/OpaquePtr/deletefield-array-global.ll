; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-deletefieldop -S -o - %s | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='dtrans-deletefieldop' -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Check that all unused fields are deleted and GEPs are updated.
; Fields (A:0) and (A:2) should stay.

%struct.A = type { i32, i32, i32 }
; CHECK: %__DFT_struct.A = type { i32, i32 }

@a = internal global [4 x %struct.A] zeroinitializer

define i32 @bar() {
entry:
  %tmp0 = load i32, i32* getelementptr inbounds ([4 x %struct.A], [4 x %struct.A]* @a, i64 0, i64 0, i32 0)
  %tmp1 = load i32, i32* getelementptr inbounds ([4 x %struct.A], [4 x %struct.A]* @a, i64 0, i64 3, i32 2)
  %add = add nsw i32 %tmp1, %tmp0
  ret i32 %add
}

; CHECK-LABEL: define i32 @bar
; CHECK: %tmp0 = load i32, {{.*}} getelementptr inbounds ([4 x %__DFT_struct.A], {{.*}} @a, i64 0, i64 0, i32 0)
; CHECK: %tmp1 = load i32, {{.*}} getelementptr inbounds ([4 x %__DFT_struct.A], {{.*}} @a, i64 0, i64 3, i32 1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"S", %struct.A zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

!intel.dtrans.types = !{!2}
