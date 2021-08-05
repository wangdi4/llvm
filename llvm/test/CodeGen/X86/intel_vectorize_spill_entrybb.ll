; RUN: llc < %s -mtriple=x86_64-- -mattr=+avx512f -mattr=+avx512bw -enable-intel-advanced-opts -O3 -vec-spill-threshold=4 | FileCheck %s
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@gLoop = dso_local local_unnamed_addr global i8 0, align 1
@g4 = dso_local local_unnamed_addr global i64 0, align 8
@g7 = dso_local local_unnamed_addr global i8 0, align 1
@g8 = dso_local local_unnamed_addr global i16 0, align 2
@g9 = dso_local local_unnamed_addr global i8 0, align 1

; Function Attrs: nounwind uwtable
define dso_local float @foo(float %f, i32 %n) local_unnamed_addr #0 {
; CHECK-LABEL: foo:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    pushq %rbp
; CHECK:    vpxorq %xmm16, %xmm16, %xmm16
; CHECK-NEXT:    vmovdqu64 %ymm16, {{[-0-9]+}}(%r{{[sb]}}p) # 4-byte Spill
entry:
  %conv = fpext float %f to double
  %sub = sub nsw i32 1, %n
  %mul = fmul double %conv, 1.000000e+01
  %conv7 = sext i32 %n to i64
  %mul8 = mul nsw i64 %conv7, 10
  %0 = insertelement <2 x double> poison, double %conv, i32 0
  %1 = shufflevector <2 x double> %0, <2 x double> poison, <2 x i32> zeroinitializer
  %2 = fadd <2 x double> %1, <double 1.000000e+00, double poison>
  %3 = fsub <2 x double> <double poison, double 2.000000e+01>, %1
  %4 = shufflevector <2 x double> %2, <2 x double> %3, <2 x i32> <i32 0, i32 3>
  %and = and i32 %n, 30
  %div = sdiv i32 %n, 40
  %rem = srem i32 %n, 50
  br label %do.body

do.body:                                          ; preds = %do.body, %entry
  %r2.0 = phi i32 [ 0, %entry ], [ %add4, %do.body ]
  %r3.0 = phi double [ 0.000000e+00, %entry ], [ %add6, %do.body ]
  %r4.0 = phi i64 [ 0, %entry ], [ %add9, %do.body ]
  %r7.0 = phi i32 [ 0, %entry ], [ %add16, %do.body ]
  %r8.0 = phi i32 [ 0, %entry ], [ %add19, %do.body ]
  %r9.0 = phi i32 [ 0, %entry ], [ %add22, %do.body ]
  %5 = phi <2 x float> [ zeroinitializer, %entry ], [ %8, %do.body ]
  tail call void asm sideeffect "", "~{rbp},~{rbx},~{r12},~{r13},~{r14},~{r15},~{dirflag},~{fpsr},~{flags}"() #2, !srcloc !3
  tail call void @foo3() #2
  %add4 = add nsw i32 %r2.0, %sub
  %add6 = fadd double %mul, %r3.0
  %add9 = add nsw i64 %r4.0, %mul8
  %6 = fpext <2 x float> %5 to <2 x double>
  %7 = fadd <2 x double> %4, %6
  %8 = fptrunc <2 x double> %7 to <2 x float>
  %sext = shl i32 %r7.0, 24
  %conv15 = ashr exact i32 %sext, 24
  %add16 = add nsw i32 %conv15, %and
  %sext51 = shl i32 %r8.0, 16
  %conv18 = ashr exact i32 %sext51, 16
  %add19 = add nsw i32 %conv18, %div
  %sext52 = shl i32 %r9.0, 24
  %conv21 = ashr exact i32 %sext52, 24
  %add22 = add nsw i32 %conv21, %rem
  tail call void asm sideeffect "", "~{rbp},~{rbx},~{r12},~{r13},~{r14},~{r15},~{dirflag},~{fpsr},~{flags}"() #2, !srcloc !4
  tail call void @foo3() #2
  %9 = load i8, i8* @gLoop, align 1, !tbaa !5
  %tobool.not = icmp eq i8 %9, 0
  br i1 %tobool.not, label %do.end, label %do.body, !llvm.loop !8

do.end:                                           ; preds = %do.body
  %conv23 = trunc i32 %add22 to i8
  %conv20 = trunc i32 %add19 to i16
  %conv17 = trunc i32 %add16 to i8
  tail call void asm sideeffect "", "~{rbp},~{rbx},~{r12},~{r13},~{r14},~{r15},~{dirflag},~{fpsr},~{flags}"() #2, !srcloc !10
  tail call void @foo3() #2
  store i64 %add9, i64* @g4, align 8, !tbaa !11
  store i8 %conv17, i8* @g7, align 1, !tbaa !5
  store i16 %conv20, i16* @g8, align 2, !tbaa !13
  store i8 %conv23, i8* @g9, align 1, !tbaa !5
  %conv24 = sitofp i32 %add4 to float
  %10 = extractelement <2 x float> %8, i32 0
  %mul25 = fmul float %10, %conv24
  %conv26 = fpext float %mul25 to double
  %mul27 = fmul double %add6, %conv26
  %11 = extractelement <2 x float> %8, i32 1
  %conv28 = fpext float %11 to double
  %mul29 = fmul double %mul27, %conv28
  %conv30 = fptrunc double %mul29 to float
  ret float %conv30
}

declare dso_local void @foo3() local_unnamed_addr #1

attributes #0 = { nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!3 = !{i32 341}
!4 = !{i32 600}
!5 = !{!6, !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
!10 = !{i32 699}
!11 = !{!12, !12, i64 0}
!12 = !{!"long", !6, i64 0}
!13 = !{!14, !14, i64 0}
!14 = !{!"short", !6, i64 0}
