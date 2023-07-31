; RUN: opt -disable-hir-inter-loop-blocking=false -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-inter-loop-blocking" -aa-pipeline="basic-aa" -debug-only=hir-inter-loop-blocking-profit 2>&1 < %s | FileCheck %s
; REQUIRES: asserts

; CHECK: Not profitable : loop is already blocked or has blocking pragma

; CHECK-NOT: Profitable
; CHECK-NOT: Legal

;Module Before HIR
; ModuleID = 'neg-pragma.c'
source_filename = "neg-pragma.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local double @sub1(ptr nocapture %A, ptr nocapture %B, i32 %M, i32 %N, i32 %ntimes) local_unnamed_addr #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(), "QUAL.PRAGMA.LEVEL"(i32 1), "QUAL.PRAGMA.FACTOR"(i32 256) ]
  %cmp88 = icmp sgt i32 %ntimes, 0
  br i1 %cmp88, label %for.cond1.preheader.preheader, label %for.end44

for.cond1.preheader.preheader:                    ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.cond.cleanup20
  %k.089 = phi i32 [ %inc43, %for.cond.cleanup20 ], [ 0, %for.cond1.preheader.preheader ]
  br label %for.cond5.preheader

for.cond5.preheader:                              ; preds = %for.cond1.preheader, %for.cond.cleanup7
  %indvars.iv90 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next91, %for.cond.cleanup7 ]
  br label %for.body8

for.cond.cleanup7:                                ; preds = %for.body8
  %indvars.iv.next91 = add nuw nsw i64 %indvars.iv90, 1
  %exitcond92 = icmp eq i64 %indvars.iv.next91, 3
  br i1 %exitcond92, label %for.cond23.preheader.preheader, label %for.cond5.preheader

for.cond23.preheader.preheader:                   ; preds = %for.cond.cleanup7
  br label %for.cond23.preheader

for.body8:                                        ; preds = %for.cond5.preheader, %for.body8
  %indvars.iv = phi i64 [ 0, %for.cond5.preheader ], [ %indvars.iv.next, %for.body8 ]
  %arrayidx = getelementptr inbounds [512 x double], ptr %B, i64 %indvars.iv90, i64 %indvars.iv
  %1 = load double, ptr %arrayidx, align 8, !tbaa !2
  %add = fadd fast double %1, 1.000000e+00
  %arrayidx13 = getelementptr inbounds [412 x double], ptr %A, i64 %indvars.iv90, i64 %indvars.iv
  store double %add, ptr %arrayidx13, align 8, !tbaa !7
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 3
  br i1 %exitcond, label %for.cond.cleanup7, label %for.body8

for.cond23.preheader:                             ; preds = %for.cond23.preheader.preheader, %for.cond.cleanup25
  %cmp19 = phi i1 [ false, %for.cond.cleanup25 ], [ true, %for.cond23.preheader.preheader ]
  %indvars.iv96 = phi i64 [ 1, %for.cond.cleanup25 ], [ 0, %for.cond23.preheader.preheader ]
  br label %for.body26

for.cond.cleanup20:                               ; preds = %for.cond.cleanup25
  %inc43 = add nuw nsw i32 %k.089, 1
  %exitcond98 = icmp eq i32 %inc43, %ntimes
  br i1 %exitcond98, label %for.end44.loopexit, label %for.cond1.preheader

for.cond.cleanup25:                               ; preds = %for.body26
  br i1 %cmp19, label %for.cond23.preheader, label %for.cond.cleanup20

for.body26:                                       ; preds = %for.cond23.preheader, %for.body26
  %indvars.iv93 = phi i64 [ 0, %for.cond23.preheader ], [ %indvars.iv.next94, %for.body26 ]
  %arrayidx30 = getelementptr inbounds [412 x double], ptr %A, i64 %indvars.iv96, i64 %indvars.iv93
  %2 = load double, ptr %arrayidx30, align 8, !tbaa !7
  %add31 = fadd fast double %2, 2.000000e+00
  %arrayidx35 = getelementptr inbounds [512 x double], ptr %B, i64 %indvars.iv96, i64 %indvars.iv93
  store double %add31, ptr %arrayidx35, align 8, !tbaa !2
  %indvars.iv.next94 = add nuw nsw i64 %indvars.iv93, 1
  %exitcond95 = icmp eq i64 %indvars.iv.next94, 3
  br i1 %exitcond95, label %for.cond.cleanup25, label %for.body26

for.end44.loopexit:                               ; preds = %for.cond.cleanup20
  br label %for.end44

for.end44:                                        ; preds = %for.end44.loopexit, %entry
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
  %sub = add nsw i32 %M, -1
  %idxprom45 = sext i32 %sub to i64
  %sub47 = add nsw i32 %N, -1
  %idxprom48 = sext i32 %sub47 to i64
  %arrayidx49 = getelementptr inbounds [412 x double], ptr %A, i64 %idxprom45, i64 %idxprom48
  %3 = load double, ptr %arrayidx49, align 8, !tbaa !7
  %arrayidx55 = getelementptr inbounds [512 x double], ptr %B, i64 %idxprom45, i64 %idxprom48
  %4 = load double, ptr %arrayidx55, align 8, !tbaa !2
  %arrayidx57 = getelementptr inbounds [412 x double], ptr %A, i64 0, i64 1, !intel-tbaa !7
  %5 = load double, ptr %arrayidx57, align 8, !tbaa !7
  %mul = fmul fast double %5, %4
  %add58 = fadd fast double %mul, %3
  ret double %add58
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "intel-lang"="fortran" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA512_d", !4, i64 0}
!4 = !{!"double", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !4, i64 0}
!8 = !{!"array@_ZTSA412_d", !4, i64 0}
