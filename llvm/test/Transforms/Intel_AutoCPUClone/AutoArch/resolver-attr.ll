; RUN: opt -passes=auto-cpu-clone -acd-enable-all < %s -S | FileCheck %s

; This lit test checks that a resolver function's "target-cpu" and "tune-cpu" attributes
; are set to the "target-cpu" and "tune-cpu" attributes of the cloned function respectively.
;
; This lit test also checks that a resolver function's "target-features" attribute
; is set to the feature-set of the "target-cpu", and never copied over from the
; "target-features" attribute of the base/generic clone.
;
; Source for this LLVM IR:
; int f1 () { return 0; }


;CHECK: define dso_local ptr @f1.resolver() #2
;CHECK: attributes #2 = { "advanced-optim"="false" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @f1() #0 !llvm.auto.arch !0 {
entry:
  ret i32 0
}

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+avx,+avx2,+avx512f,+crc32,+cx8,+f16c,+fma,+fxsr,+mmx,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!0 = !{!"skylake-avx512"}
