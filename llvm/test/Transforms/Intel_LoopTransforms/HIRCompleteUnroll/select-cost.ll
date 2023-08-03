; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-post-vec-complete-unroll" -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s

; HIR-
; + DO i1 = 0, 9, 1   <DO_LOOP>
; |   %ld = (%ld.ptr)[i1];
; |   %.mux = (%ld == 0) ? 8 : 0;
; |   (%st.ptr)[i1] = %.mux;
; + END LOOP

; Check that the non-GEP cost of the loop is 2 * 10 = 20. Cost of select is 2 and trip count is 10.
; CHECK: Cost: 20

define void @foo(ptr %ld.ptr, ptr %st.ptr) {
entry:
 br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %ld.ptr, i64 %indvars.iv
  %ld = load i32, ptr %arrayidx, align 4
  %tobool = icmp eq i32 %ld, 0
  %.mux = select i1 %tobool, i32 8, i32 0
  %arrayidx1 = getelementptr inbounds i32, ptr %st.ptr, i64 %indvars.iv
  store i32 %.mux, ptr %arrayidx1, align 4
  %indvars.iv.next = add i64 %indvars.iv, 1
  %cmp = icmp eq i64 %indvars.iv.next, 10
  br i1 %cmp, label %exit, label %for.body

exit:
  ret void
}

