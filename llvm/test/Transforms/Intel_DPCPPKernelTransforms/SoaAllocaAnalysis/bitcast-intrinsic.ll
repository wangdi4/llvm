; RUN: opt -passes='print<dpcpp-kernel-soa-alloca-analysis>' %s -disable-output 2>&1 | FileCheck %s
; RUN: opt -analyze -enable-new-pm=0 -dpcpp-kernel-soa-alloca-analysis %s -S -o - | FileCheck %s

; Check SoaAllocaAnalysis decides optimize alloca bitcast and its supported
; intrinsic users.

; CHECK: SoaAllocaAnalysis for function test
; CHECK-NEXT: [[I:%.*]] = alloca i32, align 4 SR:[1] VR:[0] PR:[1]
; CHECK-NEXT: [[BC0:%.*]] = bitcast i32* [[I]] to i8* SR:[1] VR:[0] PR:[0]
; CHECK-NEXT: call void @llvm.lifetime.start.p0i8(i64 4, i8* [[BC0]]) #{{.*}} SR:[1] VR:[0] PR:[0]
; CHECK-NEXT: [[BC1:%.*]] = bitcast i32* [[I]] to i8* SR:[1] VR:[0] PR:[0]
; CHECK-NEXT: call void @llvm.lifetime.end.p0i8(i64 4, i8* [[BC1]]) #{{.*}} SR:[1] VR:[0] PR:[0]

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

; Function Attrs: convergent norecurse nounwind
define dso_local spir_kernel void @test() #0 {
entry:
  %i = alloca i32, align 4
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %1) #2
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { convergent norecurse nounwind "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!opencl.ocl.version = !{!0, !0}
!opencl.spir.version = !{!0, !0}

!0 = !{i32 2, i32 0}
