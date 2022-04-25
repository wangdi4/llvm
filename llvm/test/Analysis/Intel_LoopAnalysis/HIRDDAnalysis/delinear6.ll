; RUN: opt -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-dd-analysis -hir-dd-analysis-verify=Region < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output < %s 2>&1 | FileCheck %s

; Test checks that tryDelinearize() function can correctly handle MemRef with global base value @c[0][zext.i32.i64(%N) * i1 + i2].

;          BEGIN REGION { }
;               + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
;               |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
;               |   |   %fload = (@c)[0][zext.i32.i64(%N) * i1 + i2];
;               |   |   (@c)[0][zext.i32.i64(%N) * i1 + i2] = %fload + 0.1f;
;               |   + END LOOP
;               + END LOOP
;          END REGION

; CHECK: (@c)[0][zext.i32.i64(%N) * i1 + i2] --> (@c)[0][zext.i32.i64(%N) * i1 + i2] ANTI (= =) (0 0)


target triple = "x86_64-unknown-linux-gnu"

@c = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@a = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

define dso_local void @multiply(i32 %N) local_unnamed_addr {
entry:
  %cmp54 = icmp sgt i32 %N, 0
  br i1 %cmp54, label %for.cond.preheader, label %exit

for.cond.preheader:                        ; preds = %entry
  %ext = zext i32 %N to i64
  br label %for.body1

for.body1:                                  ; preds = %for.cond.cleanup, %for.cond.preheader
  %indvars.iv2 = phi i64 [ 0, %for.cond.preheader ], [ %indvars.iv.next2, %for.cond.cleanup ]
  %mul = mul nsw i64 %indvars.iv2, %ext
  br label %for.body2

for.body2:                                  ; preds = %for.body2, %for.body1
  %indvars.iv1 = phi i64 [ 0, %for.body1 ], [ %indvars.iv.next1, %for.body2 ]
  %add = add nuw nsw i64 %indvars.iv1, %mul
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @c, i64 0, i64 %add
  %fload = load float, float* %arrayidx, align 4
  %fadd = fadd float %fload, 1.0
  store float %fadd, float* %arrayidx, align 4
  %indvars.iv.next1 = add nuw nsw i64 %indvars.iv1, 1
  %exitcond2 = icmp eq i64 %indvars.iv.next1, %ext
  br i1 %exitcond2, label %for.cond.cleanup, label %for.body2

for.cond.cleanup:                                ; preds = %for.body2
  %indvars.iv.next2 = add nuw nsw i64 %indvars.iv2, 1
  %exitcond1 = icmp eq i64 %indvars.iv.next2, %ext
  br i1 %exitcond1, label %for.cond.cleanup.loopexit, label %for.body1

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup
  br label %exit

exit:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void
}
