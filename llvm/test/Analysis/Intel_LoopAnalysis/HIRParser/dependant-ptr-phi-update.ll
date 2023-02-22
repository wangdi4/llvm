; RUN: opt -opaque-pointers=0 %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Verify that the update of %p1.phi comes before update of %p2.phi as it is
; updated using %p2.phi.

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   %bc1 = bitcast.%struct.LookasideSlot*.i8*(&((%p2.phi)[0]));
; CHECK: |   (%p2.phi)[0].0 = &((%p1.phi)[0]);
; CHECK: |   %p1.phi = &((%p2.phi)[0]);
; CHECK: |   %p2.phi = &((%struct.LookasideSlot*)(%bc1)[%s]);
; CHECK: + END LOOP


%struct.LookasideSlot = type { %struct.LookasideSlot* }

define void @foo(%struct.LookasideSlot* %p2, i32 %n, i64 %s) {
entry:
  br label %loop

loop:
  %p1.phi = phi %struct.LookasideSlot* [ null, %entry ], [ %p2.phi, %loop ]
  %p2.phi = phi %struct.LookasideSlot* [ %p2, %entry ], [ %bc2, %loop ]
  %iv = phi i32 [ 0, %entry ], [ %iv.inc, %loop ]
  %bc1 = bitcast %struct.LookasideSlot* %p2.phi to i8*
  %gep1 = getelementptr inbounds %struct.LookasideSlot, %struct.LookasideSlot* %p2.phi, i64 0, i32 0
  store %struct.LookasideSlot* %p1.phi, %struct.LookasideSlot** %gep1, align 8
  %gep2 = getelementptr inbounds i8, i8* %bc1, i64 %s
  %iv.inc = add nuw nsw i32 %iv, 1
  %bc2 = bitcast i8* %gep2 to %struct.LookasideSlot*
  %cmp = icmp eq i32 %iv.inc, %n
  br i1 %cmp, label %exit, label %loop

exit:
  ret void
}

