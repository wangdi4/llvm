; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-pragma-loop-blocking,hir-loop-blocking,print<hir>" -aa-pipeline="basic-aa" -disable-output 2>&1 < %s | FileCheck %s

; Check that blocking factor of 0 compiles successfully but does not result in any blocking

; Source
; void foo(double A[1000][1000], double B[1000][1000], double C[1000][1000])
; {
;   int i, j;
;   #pragma block_loop factor(0) level(2)
;   #pragma block_loop factor(0) level(1)
;   for (i = 1; i <1000; i++) {
;     for (j = 1; j <1000; j++) {
;       A[i][j] += C[i][j] * B[i][j];
;     }
;   }
; }

;CHECK:   BEGIN REGION { }
;CHECK-NOT: DO i3

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(ptr nocapture %A, ptr nocapture readonly %B, ptr nocapture readonly %C) local_unnamed_addr #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(), "QUAL.PRAGMA.LEVEL"(i32 1), "QUAL.PRAGMA.FACTOR"(i32 0), "QUAL.PRAGMA.LEVEL"(i32 2), "QUAL.PRAGMA.FACTOR"(i32 0) ]
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc13
  %indvars.iv28 = phi i64 [ 1, %entry ], [ %indvars.iv.next29, %for.inc13 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 1, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx = getelementptr inbounds [1000 x double], ptr %C, i64 %indvars.iv28, i64 %indvars.iv
  %1 = load double, ptr %arrayidx, align 8, !tbaa !2
  %arrayidx8 = getelementptr inbounds [1000 x double], ptr %B, i64 %indvars.iv28, i64 %indvars.iv
  %2 = load double, ptr %arrayidx8, align 8, !tbaa !2
  %mul = fmul fast double %2, %1
  %arrayidx12 = getelementptr inbounds [1000 x double], ptr %A, i64 %indvars.iv28, i64 %indvars.iv
  %3 = load double, ptr %arrayidx12, align 8, !tbaa !2
  %add = fadd fast double %3, %mul
  store double %add, ptr %arrayidx12, align 8, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.inc13, label %for.body3

for.inc13:                                        ; preds = %for.body3
  %indvars.iv.next29 = add nuw nsw i64 %indvars.iv28, 1
  %exitcond30 = icmp eq i64 %indvars.iv.next29, 1000
  br i1 %exitcond30, label %for.end15, label %for.cond1.preheader

for.end15:                                        ; preds = %for.inc13
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_d", !4, i64 0}
!4 = !{!"double", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
