; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-loop-distribute-always-stripmine -hir-loop-distribute-max-mem=3 -hir-loop-distribute-scex-cost=3 < %s 2>&1 | FileCheck %s

; Verify that distribution succeeds when we need to create a new instruction to redefine
; the lower bound temp. i1 is a 32 bit IV but the addition to %s is of type i4 and would
; previously fail when trying to normalize the loop.

;   HIR before Distribution
;         BEGIN REGION { }
;               + DO i1 = 0, %it + -1, 1
;               |   if (%z > %cmp)
;               |   {
;               |      %x.addr.020 = %x.addr.020  +  1;
;               |      %y.addr.019 = %y.addr.019  +  1;
;               |      %0 = (@A)[0][i1 + %s];
;               |      %1 = (@B)[0][%y.addr.019];
;               |      %2 = (@A1)[0][%x.addr.020];
;               |      (@B1)[0][%x.addr.020] = %2 + (%0 * %1);
;               |   }
;               + END LOOP
;         END REGION

; |      %0 = (@A)[0][i1 + %s];
; |      <LVAL-REG> NON-LINEAR i32 %0 {sb:18}
; |      <RVAL-REG> {al:4}(LINEAR [10 x i32]* @A)[i64 0][LINEAR sext.i4.i64(i1 + %s)] inbounds  !tbaa !5 {sb:32}
; |         <BLOB> LINEAR i4 %s {sb:17}
; |         <BLOB> LINEAR [10 x i32]* @A {sb:16}

;   HIR after Distribution

;         BEGIN REGION { modified }
;               + DO i1 = 0, (%it + -1)/u64, 1
;               |   %min = (-64 * i1 + %it + -1 <= 63) ? -64 * i1 + %it + -1 : 63;
;               |
;               |      %lb = 64 * i1;
;               |   + DO i2 = 0, 64 * i1 + %min + -1 * %lb, 1
;               |   |   if (%z > %cmp)
;               |   |   {
;               |   |      %x.addr.020 = %x.addr.020  +  1;
;               |   |      (%.TempArray)[0][i2 + %lb] = %x.addr.020;
;               |   |      %y.addr.019 = %y.addr.019  +  1;
;               |   |      %0 = (@A)[0][i2 + %s + trunc.i32.i4(%lb)];
;               |   |      %1 = (@B)[0][%y.addr.019];
;               |   |      (%.TempArray2)[0][i2 + %lb] = %1;
;               |   |      %2 = (@A1)[0][%x.addr.020];
;               |   |   }
;               |   + END LOOP
;               |
;               |
;               |      %lb4 = 64 * i1;
;               |   + DO i2 = 0, 64 * i1 + %min + -1 * %lb4, 1
;               |   |   if (%z > %cmp)
;               |   |   {
;               |   |      %x.addr.020 = (%.TempArray)[0][i2 + %lb4];
;               |   |      %0 = (@A)[0][i2 + %s + trunc.i32.i4(%lb4)];
;               |   |      %1 = (%.TempArray2)[0][i2 + %lb4];
;               |   |      %2 = (@A1)[0][%x.addr.020];
;               |   |      (@B1)[0][%x.addr.020] = %2 + (%0 * %1);
;               |   |   }
;               |   + END LOOP
;               + END LOOP
;         END REGION

; CHECK: BEGIN REGION { modified }
; CHECK: DO i1
; CHECK: DO i2
; CHECK: END LOOP
; CHECK: DO i2
; CHECK: END LOOP
; CHECK: END LOOP

; Note %min and %lb are both i32 type

; |   |      %0 = (@A)[0][i2 + %s + trunc.i32.i4(%lb)];
; |   |      <LVAL-REG> NON-LINEAR i32 %0 {sb:18}
; |   |      <RVAL-REG> {al:4}(LINEAR [10 x i32]* @A)[i64 0][LINEAR sext.i4.i64(i2 + %s + trunc.i32.i4(%lb)){def@1}] inbounds  !tbaa !5 {sb:32}
; |   |         <BLOB> LINEAR i4 %s {sb:17}
; |   |         <BLOB> LINEAR [10 x i32]* @A {sb:16}
; |   |         <BLOB> LINEAR i32 %lb{def@1} {sb:41}



target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@A1 = dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@B1 = dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@B2 = dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local i32 @foo(i32 %x, i32 %y, i32 %z, i32 %cmp, i32 %it, i4 %s) local_unnamed_addr #0 {
entry:
  %cmp2 = icmp sgt i32 %z, %cmp
  %cmp118 = icmp sgt i32 %it, 0
  br i1 %cmp118, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  %x.addr.1.lcssa = phi i32 [ %x.addr.1, %for.inc ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %x.addr.0.lcssa = phi i32 [ %x, %entry ], [ %x.addr.1.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %x.addr.0.lcssa

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %i.021 = phi i32 [ %inc, %for.inc ], [ 0, %for.body.preheader ]
  %x.addr.020 = phi i32 [ %x.addr.1, %for.inc ], [ %x, %for.body.preheader ]
  %y.addr.019 = phi i32 [ %y.addr.1, %for.inc ], [ %y, %for.body.preheader ]
  br i1 %cmp2, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %add = add nsw i32 %x.addr.020, 1
  %add3 = add nsw i32 %y.addr.019, 1
  %idxprom = sext i32 %add to i64
  %trunc = trunc i32 %i.021 to i4
  %add4 = add nsw i4 %trunc, %s
  %ext = sext i4 %add4 to i64
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* @A, i64 0, i64 %ext, !intel-tbaa !3
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %idxprom4 = sext i32 %add3 to i64
  %arrayidx5 = getelementptr inbounds [10 x i32], [10 x i32]* @B, i64 0, i64 %idxprom4, !intel-tbaa !3
  %1 = load i32, i32* %arrayidx5, align 4, !tbaa !3
  %mul = mul nsw i32 %1, %0
  %arrayidx7 = getelementptr inbounds [10 x i32], [10 x i32]* @A1, i64 0, i64 %idxprom, !intel-tbaa !3
  %2 = load i32, i32* %arrayidx7, align 4, !tbaa !3
  %add8 = add nsw i32 %mul, %2
  %arrayidx10 = getelementptr inbounds [10 x i32], [10 x i32]* @B1, i64 0, i64 %idxprom, !intel-tbaa !3
  store i32 %add8, i32* %arrayidx10, align 4, !tbaa !3
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %y.addr.1 = phi i32 [ %add3, %if.then ], [ %y.addr.019, %for.body ]
  %x.addr.1 = phi i32 [ %add, %if.then ], [ %x.addr.020, %for.body ]
  %inc = add nuw nsw i32 %i.021, 1
  %exitcond.not = icmp eq i32 %inc, %it
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !8
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA10_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}

