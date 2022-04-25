; RUN: opt < %s -hir-ssa-deconstruction | opt -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0 | FileCheck %s
; RUN: opt < %s -hir-ssa-deconstruction | opt -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0 | FileCheck --check-prefix NEGATIVE %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 < %s | FileCheck --check-prefix NEGATIVE %s

; The C source for this IR looks something like this:
;
; void foo(int *A, int *B, int N) {
; #pragma ivdep loop
;   for(int i=0; i<N; i++) {
;     A[i] = B[i];
;     A[i+100] = i+1;
;   }
; }

; Here we verify that DDTests uses 'ivdep loop' to fully break the
; output dependence between the two stores. On its own, 'ivdep loop' only says
; that any dependence is not loop-carried, i.e., the direction vector is at
; most (=). However, in this case we shouldn't even have an (=) edge between
; the two stores since it is possible to reason that A[i] doesn't overlap with
; A[i+100] for any single value of i.

; Check that we preserved loop-independent edges between B/A.
; CHECK: DD graph for function foo:
; CHECK-DAG: (%B)[i1] --> (%A)[i1] ANTI (=)
; CHECK-DAG: (%B)[i1] --> (%A)[i1 + 100] ANTI (=)

; Separately, check that we did not generate an edge for the two stores.
; NEGATIVE: DD graph for function foo:
; NEGATIVE-NOT:  (%A)[i1 + 100] --> (%A)[i1]

define void @foo(i32* nocapture %A, i32* nocapture readonly %B, i32 %N) {
entry:
  %cmp13 = icmp sgt i32 %N, 0
  br i1 %cmp13, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count17 = zext i32 %N to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %0 = load i32, i32* %ptridx, align 4
  %ptridx2 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  store i32 %0, i32* %ptridx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %1 = add nuw nsw i64 %indvars.iv, 100
  %ptridx5 = getelementptr inbounds i32, i32* %A, i64 %1
  %2 = trunc i64 %indvars.iv.next to i32
  store i32 %2, i32* %ptridx5, align 4
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count17
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !6
}

!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.vectorize.ivdep_loop"}
