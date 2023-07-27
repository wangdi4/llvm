
; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-blocking,print<hir>" -disable-output -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s

; Check that blocking can happen for non-linear refs inside non lval-memrefs.
; Observe %rem33 ref is non-linear, used for rval load (@B)[0][i1][%rem33];

; Before Loop Blocking
;    BEGIN REGION { }
;         + DO i1 = 0, 399, 1   <DO_LOOP>
;         |   + DO i2 = 0, 399, 1   <DO_LOOP>
;         |   |   + DO i3 = 0, 399, 1   <DO_LOOP>
;         |   |   |   %0 = (@A)[0][i1][i2];
;         |   |   |   %rem33 = i3 + 1  %u  400;
;         |   |   |   %0 = %0  +  (@B)[0][i1][%rem33];
;         |   |   |   (@A)[0][i1][i2] = %0;
;         |   |   + END LOOP
;         |   + END LOOP
;         + END LOOP
;    END REGION

; After Loop Blocking

;    BEGIN REGION { modified }
;         + DO i1 = 0, 6, 1   <DO_LOOP>
;         |   %min = (-64 * i1 + 399 <= 63) ? -64 * i1 + 399 : 63;
;         |
;         |   + DO i2 = 0, 6, 1   <DO_LOOP>
;         |   |   %min3 = (-64 * i2 + 399 <= 63) ? -64 * i2 + 399 : 63;
;         |   |
;         |   |   + DO i3 = 0, 399, 1   <DO_LOOP>
;         |   |   |   + DO i4 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>
;         |   |   |   |   + DO i5 = 0, %min3, 1   <DO_LOOP>  <MAX_TC_EST = 64>
;         |   |   |   |   |   %0 = (@A)[0][i3][64 * i1 + i4];
;         |   |   |   |   |   %rem33 = 64 * i2 + i5 + 1  %u  400;
;         |   |   |   |   |   %0 = %0  +  (@B)[0][i3][%rem33];
;         |   |   |   |   |   (@A)[0][i3][64 * i1 + i4] = %0;
;         |   |   |   |   + END LOOP
;         |   |   |   + END LOOP
;         |   |   + END LOOP
;         |   + END LOOP
;         + END LOOP
;    END REGION

; CHECK: BEGIN REGION { modified }
; CHECK: DO i3
; CHECK: DO i4


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = dso_local local_unnamed_addr global [400 x [400 x i32]] zeroinitializer, align 16
@A = dso_local local_unnamed_addr global [400 x [400 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.cond.cleanup3
  %indvars.iv38 = phi i64 [ 0, %entry ], [ %indvars.iv.next39, %for.cond.cleanup3 ]
  br label %for.cond5.preheader

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond5.preheader:                              ; preds = %for.cond1.preheader, %for.cond.cleanup7
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.cond.cleanup7 ]
  %arrayidx14 = getelementptr inbounds [400 x [400 x i32]], ptr @A, i64 0, i64 %indvars.iv38, i64 %indvars.iv, !intel-tbaa !2
  %arrayidx14.promoted = load i32, ptr %arrayidx14, align 4, !tbaa !2
  br label %for.body8

for.cond.cleanup3:                                ; preds = %for.cond.cleanup7
  %indvars.iv.next39 = add nuw nsw i64 %indvars.iv38, 1
  %exitcond40 = icmp eq i64 %indvars.iv.next39, 400
  br i1 %exitcond40, label %for.cond.cleanup, label %for.cond1.preheader

for.cond.cleanup7:                                ; preds = %for.body8
  %add15.lcssa = phi i32 [ %add15, %for.body8 ]
  store i32 %add15.lcssa, ptr %arrayidx14, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond37 = icmp eq i64 %indvars.iv.next, 400
  br i1 %exitcond37, label %for.cond.cleanup3, label %for.cond5.preheader

for.body8:                                        ; preds = %for.cond5.preheader, %for.body8
  %0 = phi i32 [ %arrayidx14.promoted, %for.cond5.preheader ], [ %add15, %for.body8 ]
  %k.034 = phi i32 [ 0, %for.cond5.preheader ], [ %add, %for.body8 ]
  %add = add nuw nsw i32 %k.034, 1
  %rem.lhs.trunc = trunc i32 %add to i16
  %rem33 = urem i16 %rem.lhs.trunc, 400
  %idxprom9 = zext i16 %rem33 to i64
  %arrayidx10 = getelementptr inbounds [400 x [400 x i32]], ptr @B, i64 0, i64 %indvars.iv38, i64 %idxprom9, !intel-tbaa !2
  %1 = load i32, ptr %arrayidx10, align 4, !tbaa !2
  %add15 = add nsw i32 %0, %1
  %exitcond = icmp eq i32 %add, 400
  br i1 %exitcond, label %for.cond.cleanup7, label %for.body8
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA400_A400_i", !4, i64 0}
!4 = !{!"array@_ZTSA400_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
