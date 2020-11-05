; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-sinking-for-perfect-loopnest -hir-pragma-loop-blocking -print-after=hir-pragma-loop-blocking -disable-output < %s 2>&1 | FileCheck %s

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-pragma-loop-blocking,print<hir>" -aa-pipeline="basic-aa" -disable-output 2>&1 < %s | FileCheck %s

; Check that blocking for loop with boundary condition happens for level 1 as specified in pragma

; Source

;  #pragma block_loop  factor(256) level(1)
;  for (int i = 1; i <1000; i++) {
;    for (int j = 1; j <1000; j++) {
;    	A[i] += A[i] * B[i][j];
;    }
;  }

;CHECK:           BEGIN REGION { modified }
;CHECK:          + DO i1 = 0, 3, 1   <DO_LOOP>
;                |   %min = (-256 * i1 + 998 <= 255) ? -256 * i1 + 998 : 255;
;                |
;CHECK:          |   + DO i2 = 0, %min, 1   <DO_LOOP>
;CHECK:          |   |   + DO i3 = 0, 998, 1   <DO_LOOP>
;CHECK-NOT: DO i4
;                |   |   |   %1 = (%B)[256 * i1 + i2 + 1];
;                |   |   |   %add26 = (%A)[256 * i1 + i2 + 1];
;                |   |   |   %mul = (%1)[i3 + 1]  *  %add26;
;                |   |   |   %add26 = %mul  +  %add26;
;                |   |   |   (%A)[256 * i1 + i2 + 1] = %add26;
;                |   |   + END LOOP
;                |   + END LOOP
;                + END LOOP



target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(double* noalias nocapture %A, double** noalias nocapture readonly %B, double* noalias nocapture readnone %C) local_unnamed_addr #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(), "QUAL.PRAGMA.LEVEL"(i32 1), "QUAL.PRAGMA.FACTOR"(i32 256) ]
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.cond.cleanup3
  %indvars.iv27 = phi i64 [ 1, %entry ], [ %indvars.iv.next28, %for.cond.cleanup3 ]
  %ptridx = getelementptr inbounds double, double* %A, i64 %indvars.iv27
  %ptridx6 = getelementptr inbounds double*, double** %B, i64 %indvars.iv27
  %1 = load double*, double** %ptridx6, align 8, !tbaa !2
  %ptridx.promoted = load double, double* %ptridx, align 8, !tbaa !6
  br label %for.body4

for.cond.cleanup3:                                ; preds = %for.body4
  %add.lcssa = phi double [ %add, %for.body4 ]
  store double %add.lcssa, double* %ptridx, align 8, !tbaa !6
  %indvars.iv.next28 = add nuw nsw i64 %indvars.iv27, 1
  %exitcond29 = icmp eq i64 %indvars.iv.next28, 1000
  br i1 %exitcond29, label %for.end13, label %for.cond1.preheader

for.body4:                                        ; preds = %for.cond1.preheader, %for.body4
  %indvars.iv = phi i64 [ 1, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %add26 = phi double [ %ptridx.promoted, %for.cond1.preheader ], [ %add, %for.body4 ]
  %ptridx8 = getelementptr inbounds double, double* %1, i64 %indvars.iv
  %2 = load double, double* %ptridx8, align 8, !tbaa !6
  %mul = fmul fast double %2, %add26
  %add = fadd fast double %mul, %add26
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4

for.end13:                                        ; preds = %for.cond.cleanup3
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPd", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"double", !4, i64 0}
