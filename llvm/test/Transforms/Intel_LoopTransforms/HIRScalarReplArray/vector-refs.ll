; RUN: opt %s -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,hir-scalarrepl-array,print<hir>" -vplan-force-vf=4 -hir-details -disable-output 2>&1 | FileCheck %s

; Verify that we are successfully able to handle vector refs. In this case the inner loop is vectorized and completely unrolled. The vector refs of @B[][] are then scalar replaced.

; Formed HIR-
; + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 99>
; |   + DO i2 = 0, 3, 1   <DO_LOOP>
; |   |   %0 = (@B)[0][i2][i1];
; |   |   %1 = (@B)[0][i2][i1 + 1];
; |   |   %add10 = %0  +  %1;
; |   |   (@A)[0][i2][i1] = %add10;
; |   |   %inc = %0  +  1.000000e+00;
; |   |   (@B)[0][i2][i1] = %inc;
; |   |   %inc24 = %1  +  1.000000e+00;
; |   |   (@B)[0][i2][i1 + 1] = %inc24;
; |   + END LOOP
; + END LOOP


; CHECK: %scalarepl = (<4 x float>*)(@B)[0][<i64 0, i64 1, i64 2, i64 3>][0];

; CHECK: + DO i64 i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 99>
; CHECK: |   %.vec = %scalarepl;
; CHECK: |   %scalarepl[[NUM:3|6]] = (<4 x float>*)(@B)[0][<i64 0, i64 1, i64 2, i64 3>][i1 + 1];
; CHECK: |   %.vec2 = %scalarepl[[NUM]];
; CHECK: |   [[ADDVEC:%.*]] = %.vec  +  %.vec2;
; CHECK: |   (<4 x float>*)(@A)[0][<i64 0, i64 1, i64 2, i64 3>][i1] = [[ADDVEC]];
; CHECK: |   [[INCVEC:%.*]] = %.vec  +  1.000000e+00;
; CHECK: |   %scalarepl = [[INCVEC]];
; CHECK: |   (<4 x float>*)(@B)[0][<i64 0, i64 1, i64 2, i64 3>][i1] = %scalarepl;
; CHECK: |   [[INCVEC2:%.*]] = %.vec2  +  1.000000e+00;
; CHECK: |   %scalarepl[[NUM]] = [[INCVEC2]];
; CHECK: |   %scalarepl = %scalarepl[[NUM]];
; CHECK: + END LOOP

; CHECK: (<4 x float>*)(@B)[0][<i64 0, i64 1, i64 2, i64 3>][sext.i32.i64(%n)] = %scalarepl;
; CHECK:  <LVAL-REG> {al:4}(<4 x float>*)(LINEAR ptr @B)[<4 x i64> 0][<4 x i64> <i64 0, i64 1, i64 2, i64 3>][LINEAR <4 x i64> sext.i32.i64(%n)]


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [4 x [100 x float]] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [4 x [100 x float]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp44 = icmp sgt i32 %n, 0
  br i1 %cmp44, label %for.cond1.preheader.preheader, label %for.end28

for.cond1.preheader.preheader:                    ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc26, %for.cond1.preheader.preheader
  %indvars.iv46 = phi i64 [ 0, %for.cond1.preheader.preheader ], [ %indvars.iv.next47, %for.inc26 ]
  %indvars.iv.next47 = add nuw nsw i64 %indvars.iv46, 1
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [4 x [100 x float]], ptr @B, i64 0, i64 %indvars.iv, i64 %indvars.iv46
  %0 = load float, ptr %arrayidx5, align 4
  %arrayidx9 = getelementptr inbounds [4 x [100 x float]], ptr @B, i64 0, i64 %indvars.iv, i64 %indvars.iv.next47
  %1 = load float, ptr %arrayidx9, align 4
  %add10 = fadd float %0, %1
  %arrayidx14 = getelementptr inbounds [4 x [100 x float]], ptr @A, i64 0, i64 %indvars.iv, i64 %indvars.iv46
  store float %add10, ptr %arrayidx14, align 4
  %inc = fadd float %0, 1.000000e+00
  store float %inc, ptr %arrayidx5, align 4
  %inc24 = fadd float %1, 1.000000e+00
  store float %inc24, ptr %arrayidx9, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.inc26, label %for.body3

for.inc26:                                        ; preds = %for.body3
  %exitcond48 = icmp eq i64 %indvars.iv.next47, %wide.trip.count
  br i1 %exitcond48, label %for.end28.loopexit, label %for.cond1.preheader

for.end28.loopexit:                               ; preds = %for.inc26
  br label %for.end28

for.end28:                                        ; preds = %for.end28.loopexit, %entry
  ret void
}

