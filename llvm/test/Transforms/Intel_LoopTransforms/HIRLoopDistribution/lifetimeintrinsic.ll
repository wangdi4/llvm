; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-distribute-loopnest,print<hir>" -S -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Check that loopnest with lifetime intrinsics is not distributed. This is
; a heuristic which allows later passes such as unrolling/SROA to optimize
; away variables w/lifetimes which are trivial to analyze (usually allocas).
; In this test, %out.priv.priv can be optimized away after being unrolled.

; BEGIN REGION { }
; CHECK-COUNT-1: DO i1
;        + DO i1 = 0, -1 * %t5 + %t7 + -1, 1   <DO_LOOP> <ivdep>
;        |   @llvm.lifetime.start.p0(64,  &((i8*)(%out.priv.priv)[0]));
;        |
;        |   + DO i2 = 0, 15, 1   <DO_LOOP>
;        |   |   (%out.priv.priv)[0][i2] = 0;
;        |   + END LOOP
;        |
;        |
;        |   + DO i2 = 0, 15, 1   <DO_LOOP>
;        |   |   %t18 = (%out.priv.priv)[0][i2];
;        |   |   (%tmpArray.map.ptr.tmp.priv.val)[16 * i1 + i2 + 16 * trunc.i64.i32(%t5)] = %t18;
;        |   + END LOOP
;        |
;        |   @llvm.lifetime.end.p0(64,  &((i8*)(%out.priv.priv)[0]));
;        + END LOOP
;  END REGION

define void @foo(i64 %t5, i64 %t7, ptr nocapture %tmpArray.map.ptr.tmp.priv.val) {
entry:
  %out.priv.priv = alloca [16 x i32], align 16
  %t3 = bitcast ptr %out.priv.priv to ptr
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %for.cond.cleanup54, %entry
  %indvars.iv38 = phi i64 [ %t5, %entry ], [ %indvars.iv.next39, %for.cond.cleanup54 ]
  call void @llvm.lifetime.start.p0(i64 64, ptr nonnull %t3) #3
  br label %for.body

for.body:                                         ; preds = %for.body, %omp.inner.for.body
  %indvars.iv31 = phi i64 [ 0, %omp.inner.for.body ], [ %indvars.iv.next32, %for.body ]
  %arrayidx15 = getelementptr inbounds [16 x i32], ptr %out.priv.priv, i64 0, i64 %indvars.iv31
  store i32 0, ptr %arrayidx15, align 4
  %indvars.iv.next32 = add nuw nsw i64 %indvars.iv31, 1
  %exitcond33.not = icmp eq i64 %indvars.iv.next32, 16
  br i1 %exitcond33.not, label %for.cond16.preheader, label %for.body

for.cond16.preheader:                             ; preds = %for.body
  %mul58 = shl i64 %indvars.iv38, 4
  br label %for.body55

for.body55:                                       ; preds = %for.body55, %for.cond16.preheader
  %indvars.iv34 = phi i64 [ 0, %for.cond16.preheader ], [ %indvars.iv.next35, %for.body55 ]
  %arrayidx57 = getelementptr inbounds [16 x i32], ptr %out.priv.priv, i64 0, i64 %indvars.iv34
  %t18 = load i32, ptr %arrayidx57, align 4
  %t19 = add i64 %indvars.iv34, %mul58
  %sext42 = shl i64 %t19, 32
  %idxprom60 = ashr exact i64 %sext42, 32
  %arrayidx61 = getelementptr inbounds i32, ptr %tmpArray.map.ptr.tmp.priv.val, i64 %idxprom60
  store i32 %t18, ptr %arrayidx61, align 4
  %indvars.iv.next35 = add nuw nsw i64 %indvars.iv34, 1
  %exitcond37.not = icmp eq i64 %indvars.iv.next35, 16
  br i1 %exitcond37.not, label %for.cond.cleanup54, label %for.body55

for.cond.cleanup54:                               ; preds = %for.body55
  call void @llvm.lifetime.end.p0(i64 64, ptr nonnull %t3) #3
  %indvars.iv.next39 = add nuw nsw i64 %indvars.iv38, 1
  %exitcond41 = icmp eq i64 %indvars.iv.next39, %t7
  br i1 %exitcond41, label %loop.region.exit.loopexit, label %omp.inner.for.body, !llvm.loop !10

loop.region.exit.loopexit:                        ; preds = %for.cond.cleanup54
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #0

attributes #0 = { argmemonly nofree nosync nounwind willreturn }

!9 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
!10 = !{!10, !9}
