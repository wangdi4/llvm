
; RUN: opt -S -passes=pgo-instr-gen -disable-looptc-vp=false -looptc-min-depth=1 %s | FileCheck %s

; Simple test to check the instrumentation pass insertion of value profiling
; data collection for loop trip counts.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_Z8ConvolvePfS_Pii(ptr noundef %sum, ptr noundef %P, ptr noundef %Kernel, i32 noundef %channel){
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %sum0.0 = phi float [ 0.000000e+00, %entry ], [ %add, %for.body ]
  %c.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp slt i32 %c.0, %channel
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  store float %sum0.0, ptr %sum
  ret void

for.body:                                         ; preds = %for.cond
  %idxprom = zext i32 %c.0 to i64
  %arrayidx = getelementptr inbounds float, ptr %P, i64 %idxprom
  %0 = load float, ptr %arrayidx
  %arrayidx2 = getelementptr inbounds i32, ptr %Kernel, i64 %idxprom
  %1 = load i32, ptr %arrayidx2
  %conv = sitofp i32 %1 to float
  %mul = fmul fast float %0, %conv
  %add = fadd fast float %sum0.0, %mul
  %inc = add nuw nsw i32 %c.0, 1
  br label %for.cond, !llvm.loop !0
}

; CHECK: for.cond:
; CHECK: %lc = phi i64 [ %lc_incr, %for.body ], [ 0, %entry ]
; CHECK: %lc_incr = add nuw nsw i64 %lc, 1

; CHECK: for.cond.cleanup:
; CHECK: %lc_exit = phi i64 [ %lc, %for.cond ]
; CHECK: call void @llvm.instrprof.value.profile(ptr @__profn__Z8ConvolvePfS_Pii, i64 {{[0-9]+}}, i64 %lc_exit, i32 2, i32 0)

attributes #0 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.mustprogress"}
