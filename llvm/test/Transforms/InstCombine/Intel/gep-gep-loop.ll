; RUN: opt -passes="instcombine" -S %s | FileCheck %s

; 30207: A GEP outside a loop, is being converted into an add inside a loop.
; This has a big performance impact on small loops.

; CHECK-NOT: idx = add nsw i64 %0, %conv7.i
; CHECK: getelementptr inbounds{{.*}}%add.ptr.i53, i64 %conv7.i

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-sycldevice"

$_ZTSZZ9reduceStdRN2cl4sycl6bufferIKfLi1ENS0_6detail17aligned_allocatorIcEEvEERNS1_IfLi1ES5_vEEPffRmR4SYCLmmENKUlRNS0_7handlerEE_clESF_E11StandardDev = comdat any

@result = external dso_local local_unnamed_addr global float, align 8

define dso_local spir_kernel void @_ZTSZZ9reduceStdRN2cl4sycl6bufferIKfLi1ENS0_6detail17aligned_allocatorIcEEvEERNS1_IfLi1ES5_vEEPffRmR4SYCLmmENKUlRNS0_7handlerEE_clESF_E11StandardDev(i64 %_arg_, ptr addrspace(1) %_arg_3, float %_arg_8, i1 %cond) local_unnamed_addr #0 comdat !kernel_arg_buffer_location !1 {
entry:
  %0 = load i64, ptr addrspace(4) null, align 8
  %add.ptr.i53 = getelementptr inbounds float, ptr addrspace(1) %_arg_3, i64 %0
  %1 = trunc i64 %_arg_ to i32
  %conv6.i = mul i32 %1, 96
  br label %for.cond.i

for.cond.i:                                       ; preds = %for.body.i, %entry
  %i.0.i = phi i32 [ 11, %entry ], [ %add11.i, %for.body.i ]
  %sum2_local.0.i = phi float [ 0.000000e+00, %entry ], [ %add.i, %for.body.i ]
  %conv7.i = sext i32 %i.0.i to i64
  br i1 %cond, label %for.body.i, label %for.cond.i.i.i.preheader

for.cond.i.i.i.preheader:                         ; preds = %for.cond.i
  %cmp14.i = icmp ugt i64 1, 0
  %2 = select i1 %cmp14.i, float %sum2_local.0.i, float 0.000000e+00
  %rcast = addrspacecast ptr @result to ptr addrspace(4)
  store float %2, ptr addrspace(4) %rcast, align 4
  ret void

for.body.i:                                       ; preds = %for.cond.i
  %arrayidx.i84.i = getelementptr inbounds float, ptr addrspace(1) %add.ptr.i53, i64 %conv7.i
  %arrayidx.ascast.i85.i = addrspacecast ptr addrspace(1) %arrayidx.i84.i to ptr addrspace(4)
  %3 = load float, ptr addrspace(4) %arrayidx.ascast.i85.i, align 4
  %sub.i = fsub fast float %3, %_arg_8
  %mul10.i = fmul fast float %sub.i, %sub.i
  %add.i = fadd fast float %sum2_local.0.i, %mul10.i
  %add11.i = add nsw i32 %i.0.i, %conv6.i
  br label %for.cond.i, !llvm.loop !2
}

attributes #0 = { "unsafe-fp-math"="true" }

!llvm.ident = !{!0}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!1 = !{i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.mustprogress"}
