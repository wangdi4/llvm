; RUN: opt -aa-pipeline=basic-aa -passes="hir-ssa-deconstruction,print<hir-sparse-array-reduction-analysis>" -force-hir-sparse-array-reduction-analysis -disable-output < %s 2>&1 | FileCheck %s

; Test checks that Sparse Array Reduction is recognized in presence of sign extension on non-linear index.

; int a1[1000];
; float f[1000];
;
; float foo(int N, long foff) {
;  int i, at1;
;  for (i = 0; i < N; i++) {
;    at1 = 4 * a1[i];
;    f[foff + at1] += i;
;  }
;   return f[foff] ;
; }

;  BEGIN REGION { }
;    + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
;    |   %0 = (@a1)[0][i1];
;    |   %conv = sitofp.i32.float(i1);
;    |   %add3 = (@f)[0][sext.i32.i64((4 * %0)) + %foff]  +  %conv;
;    |   (@f)[0][sext.i32.i64((4 * %0)) + %foff] = %add3;
;    + END LOOP

; CHECK:   <Sparse Array Reduction>
; CHECK-NEXT:   <Sparse Array Reduction>
; CHECK-NEXT:   <Sparse Array Reduction>

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a1 = dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@f = dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local float @foo(i32 %N, i64 %foff) local_unnamed_addr #0 {
entry:
  %cmp11 = icmp sgt i32 %N, 0
  br i1 %cmp11, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count14 = zext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @a1, i64 0, i64 %indvars.iv, !intel-tbaa !3
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !3
  %mul = shl nsw i32 %0, 2
  %1 = trunc i64 %indvars.iv to i32
  %conv = sitofp i32 %1 to float
  %conv1 = sext i32 %mul to i64
  %add = add nsw i64 %conv1, %foff
  %arrayidx2 = getelementptr inbounds [1000 x float], ptr @f, i64 0, i64 %add, !intel-tbaa !8
  %2 = load float, ptr %arrayidx2, align 4, !tbaa !8
  %add3 = fadd fast float %2, %conv
  store float %add3, ptr %arrayidx2, align 4, !tbaa !8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count14
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !11

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %arrayidx4 = getelementptr inbounds [1000 x float], ptr @f, i64 0, i64 %foff, !intel-tbaa !8
  %3 = load float, ptr %arrayidx4, align 4, !tbaa !8
  ret float %3
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!" "}
!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA1000_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !10, i64 0}
!9 = !{!"array@_ZTSA1000_f", !10, i64 0}
!10 = !{!"float", !6, i64 0}
!11 = distinct !{!11, !12}
!12 = !{!"llvm.loop.mustprogress"}
