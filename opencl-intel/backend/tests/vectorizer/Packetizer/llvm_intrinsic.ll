; RUN: %oclopt -S -runtimelib %p/../Full/runtime.bc -packet-size=4 -packetize -dce %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @MainKernel(float* %src, float* %dst) local_unnamed_addr {
; CHECK-LABEL: define void @MainKernel
; CHECK: [[LOAD:%.*]] = load <4 x float>, <4 x float>* {{%.*}}, align 4
; CHECK: [[RES:%.*]] = call <4 x float> @llvm.maxnum.v4f32(<4 x float> [[LOAD]], <4 x float> zeroinitializer)
; CHECK-NOT: call float @llvm.maxnum.f32
; CHECK: store <4 x float> [[RES]], <4 x float>* {{%.*}}, align 4
entry:
  %call = call i64 @_Z12get_local_idj(i32 0)
  %load.addr = getelementptr inbounds float, float* %src, i64 %call
  %store.addr = getelementptr inbounds float, float* %dst, i64 %call
  %load = load float, float* %load.addr, align 4
  %res = call float @llvm.maxnum.f32(float %load, float 0.000000e+00)
  store float %res, float* %store.addr, align 4
  ret void
}

declare i64 @_Z12get_local_idj(i32) local_unnamed_addr
declare float @llvm.maxnum.f32(float, float) #0

attributes #0 = { nounwind readnone speculatable willreturn }

!sycl.kernels = !{!0}

!0 = !{void (float*, float*)* @MainKernel}
