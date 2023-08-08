; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Verify that the update of %p1.phi comes before update of %p2.phi as it is
; updated using %p2.phi.

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   (%p2.phi)[0].0 = &((%p1.phi)[0]);
; CHECK: |   %p1.phi = &((%p2)[%s * i1]);
; CHECK: |   %p2.phi = &((%p2)[%s * i1 + %s]);
; CHECK: + END LOOP


%struct.LookasideSlot = type { ptr }

define void @foo(ptr %p2, i32 %n, i64 %s) {
entry:
  br label %loop

loop:
  %p1.phi = phi ptr [ null, %entry ], [ %p2.phi, %loop ]
  %p2.phi = phi ptr [ %p2, %entry ], [ %gep2, %loop ]
  %iv = phi i32 [ 0, %entry ], [ %iv.inc, %loop ]
  %gep1 = getelementptr inbounds %struct.LookasideSlot, ptr %p2.phi, i64 0, i32 0
  store ptr %p1.phi, ptr %gep1, align 8
  %gep2 = getelementptr inbounds i8, ptr %p2.phi, i64 %s
  %iv.inc = add nuw nsw i32 %iv, 1
  %cmp = icmp eq i32 %iv.inc, %n
  br i1 %cmp, label %exit, label %loop

exit:
  ret void
}

