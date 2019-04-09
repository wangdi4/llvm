; Source code
;float A[1000];
;float B[1000];
;float C[1000];
;float D[1000];
;float E[1000];
;float F[1000];
;int m[1000];
;void sub(int n) {
;  int i;
;#pragma novector
;#pragma unroll(0)
;  for (i=0; i< n; i++) {
;    A[i] = B[i]+ E[i] + F[i] + C[i];
;    //we are reusing the addition of B[i+1] + C[i+1] in the next loop iteration
;    A[m[i]] = B[i+1]+ C[i+1];
;    D[i] = C[i] + C[i+1];
;   }
;}
;
; Check reassociation happens on instruction chains
; %1 = fadd fast float %t44.0, %gepload55
; %2 = fadd fast float %1, %gepload57
; %3 = fadd fast float %2, %t46.0
; %t44.0 and %t46.0 are two Phi nodes
;
; RUN: opt < %s -loop-carried-cse -S 2>&1 | FileCheck %s
; RUN: opt -passes="loop-carried-cse" -S 2>&1 < %s | FileCheck %s
;
; CHECK: %1 = fadd float %gepload, %gepload54
; CHECK: %t46.0 = phi float
; CHECK: %t44.0.lccse = phi float [ %1, %for.body.preheader ], [ %5, %loop.34 ]
; CHECK: %2 = fadd fast float %t44.0.lccse, %gepload57
; CHECK: %3 = fadd fast float %2, %gepload55
;
;
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@E = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@F = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@m = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@D = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @sub(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp43 = icmp sgt i32 %n, 0
  br i1 %cmp43, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  %gepload = load float, float* getelementptr inbounds ([1000 x float], [1000 x float]* @B, i64 0, i64 0), align 4, !tbaa !2
  %gepload54 = load float, float* getelementptr inbounds ([1000 x float], [1000 x float]* @C, i64 0, i64 0), align 4, !tbaa !2
  %0 = add i64 %wide.trip.count, -1
  br label %loop.34

for.end:                                          ; preds = %loop.34, %entry
  ret void

loop.34:                                          ; preds = %loop.34, %for.body.preheader
  %i1.i64.0 = phi i64 [ 0, %for.body.preheader ], [ %4, %loop.34 ]
  %t46.0 = phi float [ %gepload54, %for.body.preheader ], [ %gepload60, %loop.34 ]
  %t44.0 = phi float [ %gepload, %for.body.preheader ], [ %gepload62, %loop.34 ]
  %arrayIdx = getelementptr inbounds [1000 x float], [1000 x float]* @E, i64 0, i64 %i1.i64.0
  %gepload55 = load float, float* %arrayIdx, align 4, !tbaa !2
  %1 = fadd fast float %t44.0, %gepload55
  %arrayIdx56 = getelementptr inbounds [1000 x float], [1000 x float]* @F, i64 0, i64 %i1.i64.0
  %gepload57 = load float, float* %arrayIdx56, align 4, !tbaa !2
  %2 = fadd fast float %1, %gepload57
  %3 = fadd fast float %2, %t46.0
  %arrayIdx58 = getelementptr inbounds [1000 x float], [1000 x float]* @A, i64 0, i64 %i1.i64.0
  store float %3, float* %arrayIdx58, align 4, !tbaa !2
  %4 = add i64 %i1.i64.0, 1
  %arrayIdx59 = getelementptr inbounds [1000 x float], [1000 x float]* @C, i64 0, i64 %4
  %gepload60 = load float, float* %arrayIdx59, align 4, !tbaa !2
  %arrayIdx61 = getelementptr inbounds [1000 x float], [1000 x float]* @B, i64 0, i64 %4
  %gepload62 = load float, float* %arrayIdx61, align 4, !tbaa !2
  %5 = fadd fast float %gepload62, %gepload60
  %arrayIdx63 = getelementptr inbounds [1000 x i32], [1000 x i32]* @m, i64 0, i64 %i1.i64.0
  %gepload64 = load i32, i32* %arrayIdx63, align 4, !tbaa !7
  %6 = sext i32 %gepload64 to i64
  %arrayIdx65 = getelementptr inbounds [1000 x float], [1000 x float]* @A, i64 0, i64 %6
  store float %5, float* %arrayIdx65, align 4, !tbaa !2
  %7 = fadd float %t46.0, %gepload60
  %arrayIdx68 = getelementptr inbounds [1000 x float], [1000 x float]* @D, i64 0, i64 %i1.i64.0
  store float %7, float* %arrayIdx68, align 4, !tbaa !2
  %condloop.34 = icmp sle i64 %4, %0
  br i1 %condloop.34, label %loop.34, label %for.end, !llvm.loop !10
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) 2019.8.2.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !9, i64 0}
!8 = !{!"array@_ZTSA1000_i", !9, i64 0}
!9 = !{!"int", !5, i64 0}
!10 = distinct !{!10, !11, !12}
!11 = !{!"llvm.loop.vectorize.width", i32 1}
!12 = !{!"llvm.loop.unroll.disable"}

