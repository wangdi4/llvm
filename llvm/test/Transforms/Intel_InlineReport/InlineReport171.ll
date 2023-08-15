; RUN: opt -passes='auto-cpu-clone,print<inline-report>' -inline-report=0xf847 -disable-output < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -passes='inlinereportsetup,auto-cpu-clone,inlinereportemitter' -inline-report=0xf8c6 -disable-output < %s 2>&1 | FileCheck %s


; Check that the names of the functions multiversioned by auto arch are
; correct in the inlining reports.
; Since the target triple specifies Linux OS, foo.resolver should be
; generated.

; CHECK-LABEL: COMPILE FUNC: foo.A
; CHECK-LABEL: COMPILE FUNC: foo.V
; CHECK-LABEL: COMPILE FUNC: foo.a
; CHECK-LABEL-CL: COMPILE FUNC: foo.resolver
; CHECK-CL: EXTERN: __intel_cpu_features_init_x

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local void @foo(ptr nocapture noundef %a, ptr nocapture noundef readonly %b, i32 noundef %n) local_unnamed_addr #0 !llvm.auto.arch !3 {
entry:
  %cmp10.not = icmp eq i32 %n, 0
  br i1 %cmp10.not, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds float, ptr %a, i64 %indvars.iv
  %0 = load float, ptr %arrayidx, align 4, !tbaa !4
  %arrayidx2 = getelementptr inbounds float, ptr %b, i64 %indvars.iv
  %1 = load float, ptr %arrayidx2, align 4, !tbaa !4
  %add = fadd fast float %1, %0
  store float %add, ptr %arrayidx, align 4, !tbaa !4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.end, label %for.body, !llvm.loop !8

for.end:                                          ; preds = %for.body, %entry
  ret void
}

attributes #0 = { nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!3 = !{!"core-avx2", !"skylake-avx512"}
!4 = !{!5, !5, i64 0}
!5 = !{!"float", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
