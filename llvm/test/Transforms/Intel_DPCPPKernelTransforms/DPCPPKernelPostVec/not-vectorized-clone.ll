; If the kernel is not vectorized, then the cloned kernel is removed.
; RUN: opt -passes=dpcpp-kernel-postvec %s -S | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-postvec %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -dpcpp-kernel-postvec %s -S | FileCheck %s
; RUN: opt -dpcpp-kernel-postvec %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

; CHECK-NOT: define void @_ZGVeN16uu_30ParallelForNDRangeImplKernel1DPiS_

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z30ParallelForNDRangeImplKernel1DPiS_(i32* nocapture readonly %in, i32* nocapture %out) #0 {
entry:
  %call = tail call i64 @_Z12get_local_idj(i64 0) #3
  %arrayidx = getelementptr inbounds i32, i32* %in, i64 %call
  %0 = load i32, i32* %arrayidx, align 4
  %call1 = tail call i64 @_Z12get_local_idj(i64 0) #3
  %arrayidx2 = getelementptr inbounds i32, i32* %out, i64 %call1
  store i32 %0, i32* %arrayidx2, align 4
  ret void
}

declare dso_local i64 @_Z12get_local_idj(i64)

define dso_local void @_ZGVeN16uu_30ParallelForNDRangeImplKernel1DPiS_(i32* nocapture readonly %in, i32* nocapture %out) #1 {
entry:
  %alloca.in = alloca i32*
  store i32* %in, i32** %alloca.in
  %alloca.out = alloca i32*
  store i32* %out, i32** %alloca.out
  %call1 = tail call i64 @_Z12get_local_idj(i64 0) #3
  %0 = trunc i64 %call1 to i32
  %call = tail call i64 @_Z12get_local_idj(i64 0) #3
  %1 = trunc i64 %call to i32
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 16), "QUAL.OMP.UNIFORM"(i32** %alloca.in, i32** %alloca.out) ]
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  %load.out = load i32*, i32** %alloca.out
  %load.in = load i32*, i32** %alloca.in
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.loop.preheader
  %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.exit ]
  %add1 = add nuw i32 %1, %index
  %2 = sext i32 %add1 to i64
  %add = add nuw i32 %0, %index
  %3 = sext i32 %add to i64
  %arrayidx = getelementptr inbounds i32, i32* %load.in, i64 %2
  %4 = load i32, i32* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds i32, i32* %load.out, i64 %3
  store i32 %4, i32* %arrayidx2, align 4
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 16
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !0

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

attributes #0 = { "recommended-vector-length"="16" "prefer-vector-width"="512" "scalar-kernel" "target-cpu"="skylake-avx512" "vector-variants"="_ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_" "vectorized-kernel"="_ZGVeN16uu_30ParallelForNDRangeImplKernel1DPiS_" "vectorized-width"="1" }
attributes #1 = { "recommended-vector-length"="16" "prefer-vector-width"="512" "scalar-kernel"="_Z30ParallelForNDRangeImplKernel1DPiS_" "target-cpu"="skylake-avx512" "vector-variants"="_ZGVeN16uu__Z30ParallelForNDRangeImplKernel1DPiS_" "vectorized-kernel" "vectorized-width"="16" }

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.disable"}


!sycl.kernels = !{!2}
!2 = !{void (i32*, i32*)* @_Z30ParallelForNDRangeImplKernel1DPiS_}

; DEBUGIFY-NOT: WARNING
