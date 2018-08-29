; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-unroll-and-jam -print-after=hir-unroll-and-jam 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-unroll-and-jam,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that we unroll i1 loop by 4 and i2 loop by 4 by 'equalizing' the unroll factors.

; + DO i1 = 0, sext.i32.i64(%n1) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; |   + DO i2 = 0, sext.i32.i64(%n2) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; |   |      %0 = (@C)[0][i1][i2];
; |   |   + DO i3 = 0, sext.i32.i64(%n3) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; |   |   |   %mul = (@A)[0][i1][i3]  *  (@B)[0][i3][i2];
; |   |   |   %0 = %0  +  %mul;
; |   |   + END LOOP
; |   |      (@C)[0][i1][i2] = %0;
; |   + END LOOP
; + END LOOP

; CHECK: %tgu = (sext.i32.i64(%n1))/u4;

; CHECK: + DO i1 = 0, %tgu + -1, 1   <DO_LOOP>  <MAX_TC_EST = 25>
; CHECK: |   if (%n2 > 0)
; CHECK: |   {
; CHECK: |      %tgu5 = (sext.i32.i64(%n2))/u4;
; CHECK: |
; CHECK: |      + DO i2 = 0, %tgu5 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 25>

; Skipping unrolled instructions as it is too big.



target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global [100 x [100 x double]] zeroinitializer, align 16
@B = common local_unnamed_addr global [100 x [100 x double]] zeroinitializer, align 16
@C = common local_unnamed_addr global [100 x [100 x double]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @matmul(i32 %n1, i32 %n2, i32 %n3) local_unnamed_addr #0 {
entry:
  %cmp39 = icmp sgt i32 %n1, 0
  br i1 %cmp39, label %for.body.lr.ph, label %for.end22

for.body.lr.ph:                                   ; preds = %entry
  %cmp237 = icmp sgt i32 %n2, 0
  %cmp535 = icmp sgt i32 %n3, 0
  %wide.trip.count = sext i32 %n3 to i64
  %wide.trip.count43 = sext i32 %n2 to i64
  %wide.trip.count47 = sext i32 %n1 to i64
  br label %for.body

for.body:                                         ; preds = %for.inc20, %for.body.lr.ph
  %indvars.iv45 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next46, %for.inc20 ]
  br i1 %cmp237, label %for.body3.lr.ph, label %for.inc20

for.body3.lr.ph:                                  ; preds = %for.body
  br label %for.body3

for.body3:                                        ; preds = %for.inc17, %for.body3.lr.ph
  %indvars.iv41 = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.next42, %for.inc17 ]
  br i1 %cmp535, label %for.body6.lr.ph, label %for.inc17

for.body6.lr.ph:                                  ; preds = %for.body3
  %arrayidx16 = getelementptr inbounds [100 x [100 x double]], [100 x [100 x double]]* @C, i64 0, i64 %indvars.iv45, i64 %indvars.iv41
  %arrayidx16.promoted = load double, double* %arrayidx16, align 8, !tbaa !2
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.body6.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body6.lr.ph ], [ %indvars.iv.next, %for.body6 ]
  %0 = phi double [ %arrayidx16.promoted, %for.body6.lr.ph ], [ %add, %for.body6 ]
  %arrayidx8 = getelementptr inbounds [100 x [100 x double]], [100 x [100 x double]]* @A, i64 0, i64 %indvars.iv45, i64 %indvars.iv
  %1 = load double, double* %arrayidx8, align 8, !tbaa !2
  %arrayidx12 = getelementptr inbounds [100 x [100 x double]], [100 x [100 x double]]* @B, i64 0, i64 %indvars.iv, i64 %indvars.iv41
  %2 = load double, double* %arrayidx12, align 8, !tbaa !2
  %mul = fmul double %1, %2
  %add = fadd double %0, %mul
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond4.for.inc17_crit_edge, label %for.body6

for.cond4.for.inc17_crit_edge:                    ; preds = %for.body6
  %add.lcssa = phi double [ %add, %for.body6 ]
  store double %add.lcssa, double* %arrayidx16, align 8, !tbaa !2
  br label %for.inc17

for.inc17:                                        ; preds = %for.cond4.for.inc17_crit_edge, %for.body3
  %indvars.iv.next42 = add nuw nsw i64 %indvars.iv41, 1
  %exitcond44 = icmp eq i64 %indvars.iv.next42, %wide.trip.count43
  br i1 %exitcond44, label %for.inc20.loopexit, label %for.body3

for.inc20.loopexit:                               ; preds = %for.inc17
  br label %for.inc20

for.inc20:                                        ; preds = %for.inc20.loopexit, %for.body
  %indvars.iv.next46 = add nuw nsw i64 %indvars.iv45, 1
  %exitcond48 = icmp eq i64 %indvars.iv.next46, %wide.trip.count47
  br i1 %exitcond48, label %for.end22.loopexit, label %for.body

for.end22.loopexit:                               ; preds = %for.inc20
  br label %for.end22

for.end22:                                        ; preds = %for.end22.loopexit, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 57303327e688d928c77069562958db1ee842a174) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 943a74234a5160fb2327b0d0b6f78886b1ae3353)"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA100_A100_d", !4, i64 0}
!4 = !{!"array@_ZTSA100_d", !5, i64 0}
!5 = !{!"double", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
