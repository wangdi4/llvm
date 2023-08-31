; Test for generating mkl call for  matmul done with  linearzed subscript in C 

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-generate-mkl-call,hir-dead-store-elimination,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s
;
; void sub (double *restrict C,  double *restrict A,  double *restrict B,
;	  int N, int M, int K) {
;  
;  int i1,i2,i3; 
;  for (i1=0; i1 <= M; ++i1) {
;    for (i2=0; i2 <= N; ++i2) {
;      for (i3=0; i3 <= K; ++i3) {
;        //  C[i1][i3] += A[i1][i2] * B[i2][i3];
;	C[K * i1 + i3] +=  A[N * i1 + i2] * B[K * i2 + i3];
;      }
;    }
;  }
;}
;
; TODO: Missing functionality 
;
; XFAIL: * 

; HIR before this pass
;
;  + DO i1 = 0, zext.i32.i64(%M), 1   <DO_LOOP>  <MAX_TC_EST = 2147483648>  <LEGAL_MAX_TC = 2147483648>
;  |   + DO i2 = 0, sext.i32.i64(%N), 1   <DO_LOOP>  <MAX_TC_EST = 2147483648>  <LEGAL_MAX_TC = 2147483648>
;  |   |   + DO i3 = 0, sext.i32.i64(%K), 1   <DO_LOOP>  <MAX_TC_EST = 2147483648>  <LEGAL_MAX_TC = 2147483648>
;  |   |   |   %8 = (%A)[zext.i32.i64(%N) * i1 + i2];
;  |   |   |   %mul11 = (%B)[sext.i32.i64(%K) * i2 + i3]  *  %8;
;  |   |   |   %add16 = (%C)[sext.i32.i64(%K) * i1 + i3]  +  %mul11;
;  |   |   |   (%C)[sext.i32.i64(%K) * i1 + i3] = %add16;
;  |   |   + END LOOP
;  |   + END LOOP
;  + END LOOP
;  END REGION

; After HIR SinkingForPerfectLoopnest, GenerateMKLCall, DeadStoreElimination
;
;
;

;Module Before HIR
; ModuleID = 'double-linearize-matmul.c'
source_filename = "double-linearize-matmul.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local void @sub(ptr noalias nocapture noundef %C, ptr noalias nocapture noundef readonly %A, ptr noalias nocapture noundef readonly %B, i32 noundef %N, i32 noundef %M, i32 noundef %K) local_unnamed_addr #0 {
entry:
  %cmp.not39 = icmp slt i32 %M, 0
  br i1 %cmp.not39, label %for.end22, label %for.cond1.preheader.lr.ph

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp2.not37 = icmp slt i32 %N, 0
  %cmp5.not35 = icmp slt i32 %K, 0
  br i1 %cmp2.not37, label %for.end22, label %for.cond1.preheader.preheader

for.cond1.preheader.preheader:                    ; preds = %for.cond1.preheader.lr.ph
  %0 = add nuw nsw i32 %K, 1
  %1 = sext i32 %K to i64
  %2 = add nuw nsw i32 %N, 1
  %3 = zext i32 %N to i64
  %4 = add nuw nsw i32 %M, 1
  %wide.trip.count54 = zext i32 %4 to i64
  %wide.trip.count48 = sext i32 %2 to i64
  %wide.trip.count = sext i32 %0 to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.cond1.for.inc20_crit_edge
  %indvars.iv50 = phi i64 [ 0, %for.cond1.preheader.preheader ], [ %indvars.iv.next51, %for.cond1.for.inc20_crit_edge ]
  %5 = mul nsw i64 %indvars.iv50, %3
  %6 = mul nsw i64 %indvars.iv50, %1
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond1.preheader, %for.inc17
  %indvars.iv44 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next45, %for.inc17 ]
  br i1 %cmp5.not35, label %for.inc17, label %for.body6.lr.ph

for.body6.lr.ph:                                  ; preds = %for.cond4.preheader
  %7 = add nuw nsw i64 %indvars.iv44, %5
  %arrayidx = getelementptr inbounds double, ptr %A, i64 %7
  %8 = load double, ptr %arrayidx, align 8, !tbaa !3
  %9 = mul nsw i64 %indvars.iv44, %1
  br label %for.body6

for.body6:                                        ; preds = %for.body6.lr.ph, %for.body6
  %indvars.iv = phi i64 [ 0, %for.body6.lr.ph ], [ %indvars.iv.next, %for.body6 ]
  %10 = add nsw i64 %indvars.iv, %9
  %arrayidx10 = getelementptr inbounds double, ptr %B, i64 %10
  %11 = load double, ptr %arrayidx10, align 8, !tbaa !3
  %mul11 = fmul fast double %11, %8
  %12 = add nsw i64 %indvars.iv, %6
  %arrayidx15 = getelementptr inbounds double, ptr %C, i64 %12
  %13 = load double, ptr %arrayidx15, align 8, !tbaa !3
  %add16 = fadd fast double %13, %mul11
  store double %add16, ptr %arrayidx15, align 8, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.inc17.loopexit, label %for.body6, !llvm.loop !7

for.inc17.loopexit:                               ; preds = %for.body6
  br label %for.inc17

for.inc17:                                        ; preds = %for.inc17.loopexit, %for.cond4.preheader
  %indvars.iv.next45 = add nuw nsw i64 %indvars.iv44, 1
  %exitcond49 = icmp eq i64 %indvars.iv.next45, %wide.trip.count48
  br i1 %exitcond49, label %for.cond1.for.inc20_crit_edge, label %for.cond4.preheader, !llvm.loop !9

for.cond1.for.inc20_crit_edge:                    ; preds = %for.inc17
  %indvars.iv.next51 = add nuw nsw i64 %indvars.iv50, 1
  %exitcond55 = icmp eq i64 %indvars.iv.next51, %wide.trip.count54
  br i1 %exitcond55, label %for.end22.loopexit, label %for.cond1.preheader, !llvm.loop !10

for.end22.loopexit:                               ; preds = %for.cond1.for.inc20_crit_edge
  br label %for.end22

for.end22:                                        ; preds = %for.end22.loopexit, %for.cond1.preheader.lr.ph, %entry
  ret void
}

attributes #0 = { nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cmov,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"double", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
!9 = distinct !{!9, !8}
!10 = distinct !{!10, !8}




