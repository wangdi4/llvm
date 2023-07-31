; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-interchange,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck  %s

; Verify that flow-dependency through a non-livein temp doesn't
; impede loop interchange. Eg, %4 --> %4,  from %4 = %add to %add72 = %4 + %0

; CHECK: Function: foo
;
; CHECK:          BEGIN REGION { }
; CHECK:                + DO i1 = 0, 1023, 1   <DO_LOOP>
; CHECK:                |   + DO i2 = 0, 1023, 1   <DO_LOOP>
; CHECK:                |   |   %0 = (@B)[0][i2][i1];
; CHECK:                |   |   %mul = %0  *  1.000000e+01;
; CHECK:                |   |   %1 = @llvm.sqrt.f64(%mul);
; CHECK:                |   |   %4 = %mul;
; CHECK:                |   |   if (%1 > 5.000000e+00)
; CHECK:                |   |   {
; CHECK:                |   |      %div = %0  *  2.000000e-01;
; CHECK:                |   |      %div31 = %0  /  (@C)[0][i2][i1];
; CHECK:                |   |      %add = %div31  +  %div;
; CHECK:                |   |      %3 = @llvm.sqrt.f64(%0);
; CHECK:                |   |      %factor = %add  *  %3;
; CHECK:                |   |      %reass.add = %factor  +  %0;
; CHECK:                |   |      %reass.mul = %reass.add  *  2.000000e+00;
; CHECK:                |   |      (@C)[0][i2][i1] = %reass.mul;
; CHECK:                |   |      %4 = %add;
; CHECK:                |   |   }
; CHECK:                |   |   %add72 = %4  +  %0;
; CHECK:                |   |   (@A)[0][i2][i1] = %add72;
; CHECK:                |   + END LOOP
; CHECK:                + END LOOP
; CHECK:          END REGION
;
; CHECK: Function: foo
;
; CHECK:          BEGIN REGION { modified }
; CHECK:                + DO i1 = 0, 1023, 1   <DO_LOOP>
; CHECK:                |   + DO i2 = 0, 1023, 1   <DO_LOOP>
; CHECK:                |   |   %0 = (@B)[0][i1][i2];
; CHECK:                |   |   %mul = %0  *  1.000000e+01;
; CHECK:                |   |   %1 = @llvm.sqrt.f64(%mul);
; CHECK:                |   |   %4 = %mul;
; CHECK:                |   |   if (%1 > 5.000000e+00)
; CHECK:                |   |   {
; CHECK:                |   |      %div = %0  *  2.000000e-01;
; CHECK:                |   |      %div31 = %0  /  (@C)[0][i1][i2];
; CHECK:                |   |      %add = %div31  +  %div;
; CHECK:                |   |      %3 = @llvm.sqrt.f64(%0);
; CHECK:                |   |      %factor = %add  *  %3;
; CHECK:                |   |      %reass.add = %factor  +  %0;
; CHECK:                |   |      %reass.mul = %reass.add  *  2.000000e+00;
; CHECK:                |   |      (@C)[0][i1][i2] = %reass.mul;
; CHECK:                |   |      %4 = %add;
; CHECK:                |   |   }
; CHECK:                |   |   %add72 = %4  +  %0;
; CHECK:                |   |   (@A)[0][i1][i2] = %add72;
; CHECK:                |   + END LOOP
; CHECK:                + END LOOP
; CHECK:          END REGION

;Module Before HIR
; ModuleID = 'foo.c'
source_filename = "foo.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = dso_local local_unnamed_addr global [1024 x [1024 x double]] zeroinitializer, align 16
@A = dso_local local_unnamed_addr global [1024 x [1024 x double]] zeroinitializer, align 16
@C = dso_local local_unnamed_addr global [1024 x [1024 x double]] zeroinitializer, align 16
@D = dso_local local_unnamed_addr global [1024 x [1024 x double]] zeroinitializer, align 16

; Function Attrs: nofree nosync nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc77
  %indvars.iv121 = phi i64 [ 0, %entry ], [ %indvars.iv.next122, %for.inc77 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %if.end
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %if.end ]
  %arrayidx5 = getelementptr inbounds [1024 x [1024 x double]], ptr @B, i64 0, i64 %indvars.iv, i64 %indvars.iv121, !intel-tbaa !3
  %0 = load double, ptr %arrayidx5, align 8, !tbaa !3
  %mul = fmul fast double %0, 1.000000e+01
  %arrayidx9 = getelementptr inbounds [1024 x [1024 x double]], ptr @A, i64 0, i64 %indvars.iv, i64 %indvars.iv121, !intel-tbaa !3
  %1 = tail call fast double @llvm.sqrt.f64(double %mul)
  %cmp14 = fcmp fast ogt double %1, 5.000000e+00
  br i1 %cmp14, label %if.then, label %if.end

if.then:                                          ; preds = %for.body3
  %div = fmul fast double %0, 2.000000e-01
  %arrayidx30 = getelementptr inbounds [1024 x [1024 x double]], ptr @C, i64 0, i64 %indvars.iv, i64 %indvars.iv121, !intel-tbaa !3
  %2 = load double, ptr %arrayidx30, align 8, !tbaa !3
  %div31 = fdiv fast double %0, %2
  %add = fadd fast double %div31, %div
  %3 = tail call fast double @llvm.sqrt.f64(double %0)
  %factor = fmul fast double %add, %3
  %reass.add = fadd fast double %factor, %0
  %reass.mul = fmul fast double %reass.add, 2.000000e+00
  store double %reass.mul, ptr %arrayidx30, align 8, !tbaa !3
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body3
  %4 = phi double [ %add, %if.then ], [ %mul, %for.body3 ]
  %add72 = fadd fast double %4, %0
  store double %add72, ptr %arrayidx9, align 8, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.inc77, label %for.body3, !llvm.loop !9

for.inc77:                                        ; preds = %if.end
  %indvars.iv.next122 = add nuw nsw i64 %indvars.iv121, 1
  %exitcond123.not = icmp eq i64 %indvars.iv.next122, 1024
  br i1 %exitcond123.not, label %for.end79, label %for.cond1.preheader, !llvm.loop !11

for.end79:                                        ; preds = %for.inc77
  ret void
}

; Function Attrs: mustprogress nofree nosync nounwind readnone speculatable willreturn
declare double @llvm.sqrt.f64(double) #1

attributes #0 = { nofree nosync nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nofree nosync nounwind readnone speculatable willreturn }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !6, i64 0}
!4 = !{!"array@_ZTSA1024_A1024_d", !5, i64 0}
!5 = !{!"array@_ZTSA1024_d", !6, i64 0}
!6 = !{!"double", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
!11 = distinct !{!11, !10}
