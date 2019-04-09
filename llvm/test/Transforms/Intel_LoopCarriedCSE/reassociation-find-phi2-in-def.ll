; Souce code
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
;    D[i] = B[i] + B[i+1];
;   }
;}
;
; Check reassociation happens on instruction chains by searching the def
; %1 = fadd fast float %t43.0, %gepload44
; %2 = fadd fast float %1, %gepload46
; %3 = fadd fast float %2, %t45.0
; %t45.0 has only one user and is considered as Phi1, while %t43.0 has two users and is considered as Phi2
;
; RUN: opt < %s -loop-carried-cse -S 2>&1 | FileCheck %s
; RUN: opt -passes="loop-carried-cse" -S 2>&1 < %s | FileCheck %s
;
; CHECK: %1 = fadd float %gepload43, %gepload
; CHECK: %t45.0.lccse = phi float [ %1, %region.0 ], [ %5, %loop.33 ]
; CHECK: %t43.0 = phi float
; CHECK: %2 = fadd fast float %t45.0.lccse, %gepload46
; CHECK: %3 = fadd fast float %2, %gepload44
;
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 4
@E = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 4
@F = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 4
@C = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 4
@A = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 4
@m = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 4
@D = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 4

; Function Attrs: norecurse nounwind
define dso_local void @sub(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp32 = icmp sgt i32 %n, 0
  br i1 %cmp32, label %region.0, label %for.end

for.end:                                          ; preds = %loop.33, %entry
  ret void

region.0:                                         ; preds = %entry
  %gepload = load float, float* getelementptr inbounds ([1000 x float], [1000 x float]* @B, i32 0, i32 0), align 4, !tbaa !3
  %gepload43 = load float, float* getelementptr inbounds ([1000 x float], [1000 x float]* @C, i32 0, i32 0), align 4, !tbaa !3
  %0 = add i32 %n, -1
  br label %loop.33

loop.33:                                          ; preds = %loop.33, %region.0
  %i1.i32.0 = phi i32 [ 0, %region.0 ], [ %4, %loop.33 ]
  %t45.0 = phi float [ %gepload43, %region.0 ], [ %gepload51, %loop.33 ]
  %t43.0 = phi float [ %gepload, %region.0 ], [ %gepload49, %loop.33 ]
  %arrayIdx = getelementptr inbounds [1000 x float], [1000 x float]* @E, i32 0, i32 %i1.i32.0
  %gepload44 = load float, float* %arrayIdx, align 4, !tbaa !3
  %1 = fadd fast float %t43.0, %gepload44
  %arrayIdx45 = getelementptr inbounds [1000 x float], [1000 x float]* @F, i32 0, i32 %i1.i32.0
  %gepload46 = load float, float* %arrayIdx45, align 4, !tbaa !3
  %2 = fadd fast float %1, %gepload46
  %3 = fadd fast float %2, %t45.0
  %arrayIdx47 = getelementptr inbounds [1000 x float], [1000 x float]* @A, i32 0, i32 %i1.i32.0
  store float %3, float* %arrayIdx47, align 4, !tbaa !3
  %4 = add i32 %i1.i32.0, 1
  %arrayIdx48 = getelementptr inbounds [1000 x float], [1000 x float]* @B, i32 0, i32 %4
  %gepload49 = load float, float* %arrayIdx48, align 4, !tbaa !3
  %arrayIdx50 = getelementptr inbounds [1000 x float], [1000 x float]* @C, i32 0, i32 %4
  %gepload51 = load float, float* %arrayIdx50, align 4, !tbaa !3
  %5 = fadd fast float %gepload49, %gepload51
  %arrayIdx52 = getelementptr inbounds [1000 x i32], [1000 x i32]* @m, i32 0, i32 %i1.i32.0
  %gepload53 = load i32, i32* %arrayIdx52, align 4, !tbaa !8
  %arrayIdx54 = getelementptr inbounds [1000 x float], [1000 x float]* @A, i32 0, i32 %gepload53
  store float %5, float* %arrayIdx54, align 4, !tbaa !3
  %6 = fadd fast float %t43.0, %gepload49
  %arrayIdx57 = getelementptr inbounds [1000 x float], [1000 x float]* @D, i32 0, i32 %i1.i32.0
  store float %6, float* %arrayIdx57, align 4, !tbaa !3
  %condloop.33 = icmp sle i32 %4, %0
  br i1 %condloop.33, label %loop.33, label %for.end, !llvm.loop !11
}

attributes #0 = { norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"NumRegisterParameters", i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"icx (ICX) 2019.8.2.0"}
!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA1000_f", !5, i64 0}
!5 = !{!"float", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !10, i64 0}
!9 = !{!"array@_ZTSA1000_i", !10, i64 0}
!10 = !{!"int", !6, i64 0}
!11 = distinct !{!11, !12, !13}
!12 = !{!"llvm.loop.vectorize.width", i32 1}
!13 = !{!"llvm.loop.unroll.disable"}

