; RUN: opt < %s -analyze -enable-new-pm=0 -hir-ssa-deconstruction -hir-framework 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

; Verify that the max trip count estimate of the loop is set to 25 based on the
; max array type of [25 x i32] being accessed.

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 25>
; CHECK: |   (%p)[i1] = i1;
; CHECK: + END LOOP


define void @foo([10 x i32]* %ptr1, [25 x i32]* %ptr2, i64 %n, i64 %t) {
entry:
  %cmp1 = icmp slt i64 %t, 5
  br i1 %cmp1, label %then, label %else

then:
  %p.0 = getelementptr inbounds [10 x i32], [10 x i32]* %ptr1, i64 0, i64 0
  br label %for.body.preheader

else:
  %p.1 = getelementptr inbounds [25 x i32], [25 x i32]* %ptr2, i64 0, i64 0
  br label %for.body.preheader

for.body.preheader:                               ; preds = %then, %else
  %p = phi i32* [ %p.0, %then ], [ %p.1, %else ]
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %p.addr.07 = phi i32* [ %incdec.ptr, %for.body ], [ %p, %for.body.preheader ]
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %p.addr.07, align 4
  %incdec.ptr = getelementptr inbounds i32, i32* %p.addr.07, i64 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp eq i64 %indvars.iv.next, %n
  br i1 %cmp, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

