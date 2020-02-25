; RUN: llc < %s | FileCheck %s --check-prefixes=CHECK
; Test for making broadcast loads rematerializable and foldable

; CHECK: test_broadcast_load_remat_fold:

; Verify that there are no spills for the rematerializable 32-byte broadcast loads
; CHECK-NOT: {{32-byte Spill}}

; Verify that broadcast load is folded
; CHECK: vmulpd {{.*}}(%rip){1to4}, %ymm{{[-0-9]+}}, %ymm{{[-0-9]+}}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
; Function Attrs: nofree nounwind uwtable
define dso_local void @test_broadcast_load_remat_fold(double* nocapture readonly %a, double* nocapture readonly %b, double* nocapture readonly %c, double* nocapture %d, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp110 = icmp eq i32 %n, 0
  br i1 %cmp110, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  %min.iters.check = icmp ult i32 %n, 4
  br i1 %min.iters.check, label %scalar.ph, label %vector.memcheck

vector.memcheck:                                  ; preds = %for.body.preheader
  %scevgep = getelementptr double, double* %d, i64 %wide.trip.count
  %scevgep4 = getelementptr double, double* %a, i64 %wide.trip.count
  %scevgep7 = getelementptr double, double* %b, i64 %wide.trip.count
  %scevgep10 = getelementptr double, double* %c, i64 %wide.trip.count
  %bound0 = icmp ugt double* %scevgep4, %d
  %bound1 = icmp ugt double* %scevgep, %a
  %found.conflict = and i1 %bound0, %bound1
  %bound012 = icmp ugt double* %scevgep7, %d
  %bound113 = icmp ugt double* %scevgep, %b
  %found.conflict14 = and i1 %bound012, %bound113
  %conflict.rdx = or i1 %found.conflict, %found.conflict14
  %bound015 = icmp ugt double* %scevgep10, %d
  %bound116 = icmp ugt double* %scevgep, %c
  %found.conflict17 = and i1 %bound015, %bound116
  %conflict.rdx18 = or i1 %conflict.rdx, %found.conflict17
  br i1 %conflict.rdx18, label %scalar.ph, label %vector.ph

vector.ph:                                        ; preds = %vector.memcheck
  %n.vec = and i64 %wide.trip.count, 4294967292
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %vector.ph
  %index = phi i64 [ 0, %vector.ph ], [ %index.next, %vector.body ]
  %0 = getelementptr inbounds double, double* %a, i64 %index
  %1 = bitcast double* %0 to <4 x double>*
  %wide.load = load <4 x double>, <4 x double>* %1, align 8, !tbaa !2, !alias.scope !6
  %2 = getelementptr inbounds double, double* %b, i64 %index
  %3 = bitcast double* %2 to <4 x double>*
  %wide.load19 = load <4 x double>, <4 x double>* %3, align 8, !tbaa !2, !alias.scope !9
  %4 = getelementptr inbounds double, double* %c, i64 %index
  %5 = bitcast double* %4 to <4 x double>*
  %wide.load20 = load <4 x double>, <4 x double>* %5, align 8, !tbaa !2, !alias.scope !11
  %6 = getelementptr inbounds double, double* %d, i64 %index
  %7 = bitcast double* %6 to <4 x double>*
  %wide.load21 = load <4 x double>, <4 x double>* %7, align 8, !tbaa !2, !alias.scope !13, !noalias !15
  %8 = call <4 x double> @__svml_log4(<4 x double> %wide.load) #2
  %9 = call <4 x double> @__svml_log4(<4 x double> %wide.load19) #2
  %10 = call <4 x double> @__svml_log4(<4 x double> %wide.load20) #2
  %11 = call <4 x double> @__svml_log4(<4 x double> %wide.load21) #2
  %12 = fdiv <4 x double> %8, %9
  %13 = fsub <4 x double> %8, %wide.load
  %14 = fsub <4 x double> %13, %12
  %15 = call <4 x double> @__svml_exp4(<4 x double> %14) #2
  %16 = fsub <4 x double> %9, %wide.load19
  %17 = call <4 x double> @__svml_exp4(<4 x double> %16) #2
  %18 = fsub <4 x double> %10, %wide.load20
  %19 = call <4 x double> @__svml_exp4(<4 x double> %18) #2
  %20 = fsub <4 x double> %11, %wide.load21
  %21 = call <4 x double> @__svml_exp4(<4 x double> %20) #2
  %22 = fdiv <4 x double> %19, %21
  %23 = fsub <4 x double> %15, %22
  %24 = call <4 x double> @__svml_pow4(<4 x double> %8, <4 x double> %23) #2
  %25 = call <4 x double> @__svml_pow4(<4 x double> %9, <4 x double> %17) #2
  %26 = call <4 x double> @__svml_pow4(<4 x double> %8, <4 x double> %15) #2
  %27 = call <4 x double> @__svml_pow4(<4 x double> %11, <4 x double> %21) #2
  %28 = fmul <4 x double> %8, %24
  %29 = fmul <4 x double> %9, %25
  %30 = fmul <4 x double> %10, %26
  %31 = fmul <4 x double> %11, %27
  %32 = fmul <4 x double> %wide.load, %15
  %33 = fmul <4 x double> %wide.load19, %17
  %34 = fmul <4 x double> %wide.load20, %19
  %35 = fmul <4 x double> %wide.load21, %21
  %36 = fadd <4 x double> %12, %22
  %37 = fmul <4 x double> %28, <double 3.000000e+00, double 3.000000e+00, double 3.000000e+00, double 3.000000e+00>
  %38 = fadd <4 x double> %36, %37
  %39 = fmul <4 x double> %29, <double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00>
  %40 = fsub <4 x double> %38, %39
  %41 = fmul <4 x double> %30, <double 5.000000e+00, double 5.000000e+00, double 5.000000e+00, double 5.000000e+00>
  %42 = fadd <4 x double> %40, %41
  %43 = fmul <4 x double> %31, <double 6.000000e+00, double 6.000000e+00, double 6.000000e+00, double 6.000000e+00>
  %44 = fsub <4 x double> %42, %43
  %45 = fmul <4 x double> %32, <double 7.000000e+00, double 7.000000e+00, double 7.000000e+00, double 7.000000e+00>
  %46 = fmul <4 x double> %33, <double 8.000000e+00, double 8.000000e+00, double 8.000000e+00, double 8.000000e+00>
  %47 = fsub <4 x double> %45, %46
  %48 = fmul <4 x double> %34, <double 9.000000e+00, double 9.000000e+00, double 9.000000e+00, double 9.000000e+00>
  %49 = fadd <4 x double> %47, %48
  %50 = fmul <4 x double> %35, <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>
  %51 = fadd <4 x double> %49, %50
  %52 = fdiv <4 x double> %44, %51
  %53 = bitcast double* %6 to <4 x double>*
  store <4 x double> %52, <4 x double>* %53, align 8, !tbaa !2, !alias.scope !13, !noalias !15
  %index.next = add i64 %index, 4
  %54 = icmp eq i64 %index.next, %n.vec
  br i1 %54, label %middle.block, label %vector.body, !llvm.loop !16

middle.block:                                     ; preds = %vector.body
  %cmp.n = icmp eq i64 %n.vec, %wide.trip.count
  br i1 %cmp.n, label %for.end, label %scalar.ph

scalar.ph:                                        ; preds = %middle.block, %vector.memcheck, %for.body.preheader
  %bc.resume.val = phi i64 [ %n.vec, %middle.block ], [ 0, %for.body.preheader ], [ 0, %vector.memcheck ]
  br label %for.body

for.body:                                         ; preds = %for.body, %scalar.ph
  %indvars.iv = phi i64 [ %bc.resume.val, %scalar.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds double, double* %a, i64 %indvars.iv
  %55 = load double, double* %arrayidx, align 8, !tbaa !2
  %arrayidx2 = getelementptr inbounds double, double* %b, i64 %indvars.iv
  %56 = load double, double* %arrayidx2, align 8, !tbaa !2
  %arrayidx4 = getelementptr inbounds double, double* %c, i64 %indvars.iv
  %57 = load double, double* %arrayidx4, align 8, !tbaa !2
  %arrayidx6 = getelementptr inbounds double, double* %d, i64 %indvars.iv
  %58 = load double, double* %arrayidx6, align 8, !tbaa !2
  %call = tail call double @log(double %55) #2
  %call7 = tail call double @log(double %56) #2
  %call8 = tail call double @log(double %57) #2
  %call9 = tail call double @log(double %58) #2
  %div = fdiv double %call, %call7
  %sub = fsub double %call, %55
  %sub10 = fsub double %sub, %div
  %call11 = tail call double @exp(double %sub10) #2
  %sub12 = fsub double %call7, %56
  %call13 = tail call double @exp(double %sub12) #2
  %sub14 = fsub double %call8, %57
  %call15 = tail call double @exp(double %sub14) #2
  %sub16 = fsub double %call9, %58
  %call17 = tail call double @exp(double %sub16) #2
  %div18 = fdiv double %call15, %call17
  %sub19 = fsub double %call11, %div18
  %call20 = tail call double @pow(double %call, double %sub19) #2
  %call21 = tail call double @pow(double %call7, double %call13) #2
  %call22 = tail call double @pow(double %call, double %call11) #2
  %call23 = tail call double @pow(double %call9, double %call17) #2
  %mul = fmul double %call, %call20
  %mul24 = fmul double %call7, %call21
  %mul25 = fmul double %call8, %call22
  %mul26 = fmul double %call9, %call23
  %mul27 = fmul double %55, %call11
  %mul28 = fmul double %56, %call13
  %mul29 = fmul double %57, %call15
  %mul30 = fmul double %58, %call17
  %add = fadd double %div, %div18
  %mul31 = fmul double %mul, 3.000000e+00
  %add32 = fadd double %add, %mul31
  %mul33 = fmul double %mul24, 4.000000e+00
  %sub34 = fsub double %add32, %mul33
  %mul35 = fmul double %mul25, 5.000000e+00
  %add36 = fadd double %sub34, %mul35
  %mul37 = fmul double %mul26, 6.000000e+00
  %sub38 = fsub double %add36, %mul37
  %mul39 = fmul double %mul27, 7.000000e+00
  %mul40 = fmul double %mul28, 8.000000e+00
  %sub41 = fsub double %mul39, %mul40
  %mul42 = fmul double %mul29, 9.000000e+00
  %add43 = fadd double %sub41, %mul42
  %mul44 = fmul double %mul30, 1.000000e+01
  %add45 = fadd double %add43, %mul44
  %div46 = fdiv double %sub38, %add45
  store double %div46, double* %arrayidx6, align 8, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !18

for.end:                                          ; preds = %for.body, %middle.block, %entry
  ret void
}
; Function Attrs: nofree nounwind
declare dso_local double @log(double) local_unnamed_addr #1
; Function Attrs: nofree nounwind
declare dso_local double @exp(double) local_unnamed_addr #1
; Function Attrs: nofree nounwind
declare dso_local double @pow(double, double) local_unnamed_addr #1
; Function Attrs: nofree nounwind
declare dso_local <4 x double> @__svml_log4(<4 x double>) local_unnamed_addr #1
; Function Attrs: nofree nounwind
declare dso_local <4 x double> @__svml_exp4(<4 x double>) local_unnamed_addr #1
; Function Attrs: nofree nounwind
declare dso_local <4 x double> @__svml_pow4(<4 x double>, <4 x double>) local_unnamed_addr #1

attributes #0 = { nofree nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nofree nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"double", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7}
!7 = distinct !{!7, !8}
!8 = distinct !{!8, !"LVerDomain"}
!9 = !{!10}
!10 = distinct !{!10, !8}
!11 = !{!12}
!12 = distinct !{!12, !8}
!13 = !{!14}
!14 = distinct !{!14, !8}
!15 = !{!7, !10, !12}
!16 = distinct !{!16, !17}
!17 = !{!"llvm.loop.isvectorized", i32 1}
!18 = distinct !{!18, !17}
