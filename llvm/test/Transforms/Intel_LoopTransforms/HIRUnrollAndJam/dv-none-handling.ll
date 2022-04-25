; RUN: opt < %s -scoped-noalias-aa -hir-ssa-deconstruction -hir-temp-cleanup -hir-unroll-and-jam -print-after=hir-unroll-and-jam 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-unroll-and-jam,print<hir>" -aa-pipeline="scoped-noalias-aa,basic-aa" < %s 2>&1 | FileCheck %s

; Verify that unroll & jam successfully handles an edge which has DV::NONE.
; Unroll & jam currently gets conservative in its presence because of possible
; issues with how DD dervies NONE DV.

; In the test case below, (%Ap)[i1] and (%Bp)[i1] base ptr loads are marked
; noalias in the outer loop. (%A)[1025 * i1 + i2] and (%B)[1025 * i1 + i2]
; stores are marked noalias in the inner loop. The DV of (*, 0) is derived using
; this info but it seems incorrect. We need to model 'loop carried' semantics
; with noalias metadata to fix the problem. The fix should probably come from
; 'full restrict' support in LLVM.


; CHECK: + DO i1 = 0, 1023, 1   <DO_LOOP>
; CHECK: |   %A = (%Ap)[i1];
; CHECK: |   %B = (%Bp)[i1];
; CHECK: |
; CHECK: |   + DO i2 = 0, 1023, 1   <DO_LOOP>
; CHECK: |   |   (%A)[1025 * i1 + i2] = 4.000000e+00;
; CHECK: |   |   (%B)[1025 * i1 + i2] = 5.000000e+00;
; CHECK: |   |   (%iptr)[i2] = 0;
; CHECK: |   + END LOOP
; CHECK: + END LOOP

; RUN: opt < %s -scoped-noalias-aa -hir-ssa-deconstruction -hir-temp-cleanup -hir-dd-analysis -analyze -enable-new-pm=0 -hir-dd-analysis-verify=Region 2>&1 | FileCheck %s --check-prefix=CHECK-DD
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -aa-pipeline="scoped-noalias-aa,basic-aa" < %s 2>&1 | FileCheck %s --check-prefix=CHECK-DD

; Check output edge with inner DV element of NONE.
; CHECK-DD: (%A)[1025 * i1 + i2] --> (%B)[1025 * i1 + i2] OUTPUT (* 0)


define void @foo(float** noalias nocapture readonly %Ap, float** noalias nocapture readonly %Bp, i32* noalias %iptr) {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup5
  ret void

for.body:                                         ; preds = %entry, %for.cond.cleanup5
  %indvars.iv27 = phi i64 [ 0, %entry ], [ %indvars.iv.next28, %for.cond.cleanup5 ]
  %ptridx = getelementptr inbounds float*, float** %Ap, i64 %indvars.iv27
  %A = load float*, float** %ptridx, align 8, !alias.scope !6, !noalias !9
  %ptridx2 = getelementptr inbounds float*, float** %Bp, i64 %indvars.iv27
  %B = load float*, float** %ptridx2, align 8, !alias.scope !9, !noalias !6
  br label %for.body6

for.cond.cleanup5:                                ; preds = %for.body6
  %indvars.iv.next28 = add nuw nsw i64 %indvars.iv27, 1
  %exitcond29 = icmp eq i64 %indvars.iv.next28, 1024
  br i1 %exitcond29, label %for.cond.cleanup, label %for.body

for.body6:                                        ; preds = %for.body, %for.body6
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body6 ]
  %mul = mul nsw i64 1025, %indvars.iv27
  %add = add nsw i64 %mul, %indvars.iv
  %ptridx10 = getelementptr inbounds float, float* %A, i64 %add
  store float 4.000000e+00, float* %ptridx10, align 4, !alias.scope !4, !noalias !1
  %ptridx8 = getelementptr inbounds float, float* %B, i64 %add
  store float 5.000000e+00, float* %ptridx8, align 4, !alias.scope !1, !noalias !4
  %gep = getelementptr inbounds i32, i32* %iptr, i64 %indvars.iv
  store i32 0, i32* %gep
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.cond.cleanup5, label %for.body6
}

!1 = !{!2}
!2 = distinct !{!2, !3, !"foo: B"}
!3 = distinct !{!3, !"foo"}
!4 = !{!5}
!5 = distinct !{!5, !3, !"foo: A"}
!6 = !{!7}
!7 = distinct !{!7, !8, !"foo: C"}
!8 = distinct !{!8, !"foo"}
!9 = !{!10}
!10 = distinct !{!10, !8, !"foo: D"}

