; Check that call site attributes are preserved when a function call is vectorized in LV.

; RUN: opt -vector-library=SVML -inject-tli-mappings -loop-vectorize -force-vector-width=4 -force-vector-interleave=1 -mattr=avx -S < %s 2>&1 | FileCheck %s

; CHECK-LABEL: @foo
; CHECK: {{.*}} = call <4 x double> @__svml_sin4(<4 x double> {{.*}}) #1

; CHECK: attributes #1 = { nounwind memory(none) "imf-arch-consistency"="true"

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(double* nocapture %varray) {
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %tmp = trunc i64 %iv to i32
  %conv = sitofp i32 %tmp to double
  %call = tail call double @sin(double %conv) #0
  %arrayidx = getelementptr inbounds double, double* %varray, i64 %iv
  store double %call, double* %arrayidx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

declare double @sin(double)

attributes #0 = { nounwind memory(none) "imf-arch-consistency"="true" }
