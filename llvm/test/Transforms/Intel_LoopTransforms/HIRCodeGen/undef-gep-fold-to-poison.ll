;RUN: opt -passes="hir-ssa-deconstruction,hir-cg" -force-hir-cg -S %s | FileCheck %s

; Verify that we are successfully able to generate code for this case.
; The load with undef base pointer is simplified to poison.

; Formed HIR-

; + DO i1 = 0, 0, 1   <DO_LOOP>
; |   %ld = (undef)[0][0].1[2];
; |   (%st.ptr)[i1] = %ld;
; + END LOOP

; CHECK: loop{{.*}}:
; CHECK-NEXT:  = load i32, ptr poison


%struct.key = type { ptr, [4 x i32] }

@keytab = external dso_local local_unnamed_addr global [28 x %struct.key], align 16

define void @foo(ptr %st.ptr) {
entry:
  br label %for.body

for.body:                                     ; preds = %for.body, %entry
  %iv = phi i64 [ %iv.inc, %for.body ], [ 0, %entry ]
  %gep1 = getelementptr inbounds [28 x %struct.key], ptr undef, i64 0, i64 0, i32 1, i64 2
  %ld = load i32, ptr %gep1
  %gep2 = getelementptr inbounds i32, ptr %st.ptr, i64 %iv
  store i32 %ld, ptr %gep2
  %iv.inc = add i64 %iv, 1
  %cmp.i.i = icmp eq i64 %iv.inc, 10
  br i1 %cmp.i.i, label %for.body, label %exit

exit:
  ret void
}
