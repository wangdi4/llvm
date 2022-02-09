; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -o - | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core %s -S -o - | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z30ParallelForNDRangeImplKernel1DPiS_(i32* nocapture readonly %in, i32* nocapture %out) !recommended_vector_length !1 {
entry:
  %call = tail call i64 @_Z12get_local_idj(i64 0)
  %arrayidx = getelementptr inbounds i32, i32* %in, i64 %call
  %0 = load i32, i32* %arrayidx, align 4
  %call1 = tail call i64 @_Z12get_local_idj(i64 0)
  %arrayidx2 = getelementptr inbounds i32, i32* %out, i64 %call1
  store i32 %0, i32* %arrayidx2, align 4
  ret void
}

; CHECK: @_Z30ParallelForNDRangeImplKernel1DPiS_
; CHECK-SAME: !recommended_vector_length ![[NODE0:[0-9]+]] !vectorized_width ![[NODE1:[0-9]+]] !vectorized_kernel ![[NODE2:[0-9]+]]

; CHECK: @_ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_
; CHECK-SAME: !recommended_vector_length ![[NODE0]] !vectorized_width ![[NODE0]]
; CHECK-SAME: !scalar_kernel ![[NODE3:[0-9]+]]

; CHECK-DAG: ![[NODE0]] = !{i32 16}
; CHECK-DAG: ![[NODE1]] = !{i32 1}
; CHECK-DAG: ![[NODE2]] = !{void (i32*, i32*)* @_ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_}
; CHECK-DAG: ![[NODE3]] = !{void (i32*, i32*)* @_Z30ParallelForNDRangeImplKernel1DPiS_}

declare dso_local i64 @_Z12get_local_idj(i64 %0)

!sycl.kernels = !{!0}
!0 = !{void (i32*, i32*)* @_Z30ParallelForNDRangeImplKernel1DPiS_}
!1 = !{i32 16}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_ {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_ {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_ {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_ {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_ {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_ {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_ {{.*}} br
; DEBUGIFY-NOT: WARNING
