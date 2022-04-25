; RUN: opt < %s -hir-ssa-deconstruction | opt -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 < %s | FileCheck %s

; Check that "ivdep loop" does not break loop-carried dependencies on a non-innermost loop.
; This woul be correct to do, except some parts of DD are not prepared for the
; direction vectors which may result from such an adjustment.

; The source code for this IR looks like:

; void foo(int A[1024][1024], int B[1024][1024], int N, int M) {
; #pragma ivdep loop
;   for(int i=0; i<N; i++) {
;     for(int j=0; j<M; j++) {
;       A[i][j] = i+j;
;       A[i+100][j+100] = i*j;
;     }
;   }
; }

; CHECK-DAG: (%A)[i1 + 100][i2 + 100] --> (%A)[i1][i2] OUTPUT (< <) (100 100)

define void @foo([1024 x i32]* nocapture %A, [1024 x i32]* nocapture readnone %B, i32 %N, i32 %M) {
entry:
  %cmp30 = icmp sgt i32 %N, 0
  %cmp228 = icmp sgt i32 %M, 0
  %or.cond = and i1 %cmp30, %cmp228
  br i1 %or.cond, label %for.cond1.preheader.us.preheader, label %for.cond.cleanup

for.cond1.preheader.us.preheader:                 ; preds = %entry
  %wide.trip.count3941 = zext i32 %N to i64
  %wide.trip.count = sext i32 %M to i64
  br label %for.cond1.preheader.us

for.cond1.preheader.us:                           ; preds = %for.cond1.preheader.us.preheader, %for.cond1.for.cond.cleanup3_crit_edge.us
  %indvars.iv36 = phi i64 [ 0, %for.cond1.preheader.us.preheader ], [ %indvars.iv.next37, %for.cond1.for.cond.cleanup3_crit_edge.us ]
  %0 = add nuw nsw i64 %indvars.iv36, 100
  %1 = trunc i64 %indvars.iv36 to i32
  br label %for.body4.us

for.body4.us:                                     ; preds = %for.cond1.preheader.us, %for.body4.us
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader.us ], [ %indvars.iv.next, %for.body4.us ]
  %2 = add nuw nsw i64 %indvars.iv, %indvars.iv36
  %arrayidx.us = getelementptr inbounds [1024 x i32], [1024 x i32]* %A, i64 %indvars.iv36, i64 %indvars.iv
  %3 = trunc i64 %2 to i32
  store i32 %3, i32* %arrayidx.us, align 4
  %4 = trunc i64 %indvars.iv to i32
  %mul.us = mul nsw i32 %4, %1
  %5 = add nuw nsw i64 %indvars.iv, 100
  %arrayidx11.us = getelementptr inbounds [1024 x i32], [1024 x i32]* %A, i64 %0, i64 %5
  store i32 %mul.us, i32* %arrayidx11.us, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond1.for.cond.cleanup3_crit_edge.us, label %for.body4.us, !llvm.loop !7

for.cond1.for.cond.cleanup3_crit_edge.us:         ; preds = %for.body4.us
  %indvars.iv.next37 = add nuw nsw i64 %indvars.iv36, 1
  %exitcond40 = icmp eq i64 %indvars.iv.next37, %wide.trip.count3941
  br i1 %exitcond40, label %for.cond.cleanup.loopexit, label %for.cond1.preheader.us, !llvm.loop !10

for.cond.cleanup.loopexit:                        ; preds = %for.cond1.for.cond.cleanup3_crit_edge.us
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void
}

!9 = !{!"llvm.loop.vectorize.ivdep_loop"}
!7 = distinct !{!7}
!10 = distinct !{!10, !9}
