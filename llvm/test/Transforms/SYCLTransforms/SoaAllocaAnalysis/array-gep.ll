; RUN: opt -passes='print<sycl-kernel-soa-alloca-analysis>' %s -disable-output 2>&1 | FileCheck %s

; Check SoaAllocaAnalysis decides to optimize alloca of i32 array.

; CHECK: SoaAllocaAnalysis for function test
; CHECK-NEXT: [[I:%.*]] = alloca [4 x i32], align 4 SR:[1] VR:[0] PR:[1]
; CHECK-NEXT: [[GEP:%.*]] = getelementptr inbounds [4 x i32], ptr [[I]], i64 0, i64 0 SR:[1] VR:[0] PR:[1]
; CHECK-NEXT: store i32 0, ptr [[GEP]], align 4, !tbaa !{{.*}} SR:[1] VR:[0] PR:[0]

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

; Function Attrs: convergent norecurse nounwind
define dso_local spir_kernel void @test() #0 {
entry:
  %i = alloca [4 x i32], align 4
  %arrayidx = getelementptr inbounds [4 x i32], ptr %i, i64 0, i64 0
  store i32 0, ptr %arrayidx, align 4, !tbaa !1
  ret void
}

attributes #0 = { convergent norecurse nounwind "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }

!opencl.spir.version = !{!0}

!0 = !{i32 2, i32 0}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
