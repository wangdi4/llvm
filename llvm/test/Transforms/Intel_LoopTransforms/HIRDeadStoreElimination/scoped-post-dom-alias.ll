; RUN: opt -aa-pipeline="scoped-noalias-aa" -hir-create-function-level-region -passes="hir-ssa-deconstruction,print<hir>,hir-dead-store-elimination,print<hir>" 2>&1 -disable-output < %s | FileCheck %s

; Verify that first store (%ptr1)[0] is eliminated using noalias metadata even
; though the load and post-dominating store may alias.

; Print Before-

; CHECK: Function: foo

; CHECK: (%ptr1)[0] = 0;
; CHECK: %load = (%ptr2)[0];
; CHECK: (%ptr1)[0] = 1;

; Print After-

; CHECK: Function: foo

; CHECK-NOT: (%ptr1)[0] = 0;

define void @foo(ptr %ptr1, i16 *%ptr2) {
entry:
  br label %bb

bb:
  store i16 0, ptr %ptr1, align 4, !alias.scope !4, !noalias !1
  %load = load i16, ptr %ptr2, align 4, !alias.scope !1, !noalias !4
  store i32 1, ptr %ptr1, align 4
  ret void
}

!1 = !{!2}
!2 = distinct !{!2, !3, !"foo: B"}
!3 = distinct !{!3, !"foo"}
!4 = !{!5}
!5 = distinct !{!5, !3, !"foo: A"}


