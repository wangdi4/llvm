; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -packetize -packet-size=16 -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

declare void @_Z8prefetchPU3AS1Kfm(float addrspace(1)*, i64)

; CHECK: @test_consecutive
; CHECK: @_Z8prefetchPDv16_fm
; CHECK-NOT: @_Z8prefetchPU3AS1Kfm
define void @test_consecutive(float addrspace(1)* noalias %A, float addrspace(1)* noalias %B) nounwind {
entry:
  %call = call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %conv = trunc i64 %call to i32
  %idx.ext = sext i32 %conv to i64
  %add.ptr = getelementptr inbounds float, float addrspace(1)* %A, i64 %idx.ext
  call void @_Z8prefetchPU3AS1Kfm(float addrspace(1)* %add.ptr, i64 1) nounwind
  %idxprom = sext i32 %conv to i64
  %arrayidx = getelementptr inbounds float, float addrspace(1)* %A, i64 %idxprom
  %0 = load float, float addrspace(1)* %arrayidx, align 4
  %mul = fmul float %0, 2.000000e+00
  %idxprom1 = sext i32 %conv to i64
  %arrayidx2 = getelementptr inbounds float, float addrspace(1)* %B, i64 %idxprom1
  store float %mul, float addrspace(1)* %arrayidx2, align 4
  ret void
}

declare i64 @_Z13get_global_idj(i32) nounwind readnone

; CHECK: @test_consecutive_masked
; CHECK: @_Z8prefetchPDv16_fm
; CHECK-NOT: @_Z8prefetchPU3AS1Kfm
define void @test_consecutive_masked(float addrspace(1)* noalias %A, float addrspace(1)* noalias %B, i32 %threshold) nounwind {
entry:
  %call = call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %conv = trunc i64 %call to i32
  %cmp = icmp slt i32 %conv, %threshold
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %idx.ext = sext i32 %conv to i64
  %add.ptr = getelementptr inbounds float, float addrspace(1)* %A, i64 %idx.ext
  call void @_Z8prefetchPU3AS1Kfm(float addrspace(1)* %add.ptr, i64 1) nounwind
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %idxprom = sext i32 %conv to i64
  %arrayidx = getelementptr inbounds float, float addrspace(1)* %A, i64 %idxprom
  %0 = load float, float addrspace(1)* %arrayidx, align 4
  %mul = fmul float %0, 2.000000e+00
  %idxprom2 = sext i32 %conv to i64
  %arrayidx3 = getelementptr inbounds float, float addrspace(1)* %B, i64 %idxprom2
  store float %mul, float addrspace(1)* %arrayidx3, align 4
  ret void
}

; CHECK: @test_uniform
; CHECK: @_Z8prefetchPU3AS1Kfm
; CHECK-NOT: @_Z8prefetchPU3AS1Kfm
define void @test_uniform(float addrspace(1)* noalias %A, float addrspace(1)* noalias %B) nounwind {
entry:
  %call = call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  call void @_Z8prefetchPU3AS1Kfm(float addrspace(1)* %A, i64 1) nounwind
  %0 = load float, float addrspace(1)* %A, align 4
  %mul = fmul float %0, 2.000000e+00
  %sext = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx1 = getelementptr inbounds float, float addrspace(1)* %B, i64 %idxprom
  store float %mul, float addrspace(1)* %arrayidx1, align 4
  ret void
}

; CHECK: @test_uniform_masked
; CHECK: @_Z8prefetchPU3AS1Kfm
; CHECK-NOT: @_Z8prefetchPU3AS1Kfm
define void @test_uniform_masked(float addrspace(1)* noalias %A, float addrspace(1)* noalias %B) nounwind {
entry:
  %call = call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %conv = trunc i64 %call to i32
  %cmp = icmp eq i32 %conv, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  call void @_Z8prefetchPU3AS1Kfm(float addrspace(1)* %A, i64 1) nounwind
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %0 = load float, float addrspace(1)* %A, align 4
  %mul = fmul float %0, 2.000000e+00
  %idxprom = sext i32 %conv to i64
  %arrayidx2 = getelementptr inbounds float, float addrspace(1)* %B, i64 %idxprom
  store float %mul, float addrspace(1)* %arrayidx2, align 4
  ret void
}

!opencl.kernels = !{!0, !2, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{void (float addrspace(1)*, float addrspace(1)*)* @test_consecutive, !1}
!1 = !{!"image_access_qualifier", i32 3, i32 3}
!2 = !{void (float addrspace(1)*, float addrspace(1)*, i32)* @test_consecutive_masked, !3}
!3 = !{!"image_access_qualifier", i32 3, i32 3, i32 3}
!4 = !{void (float addrspace(1)*, float addrspace(1)*)* @test_uniform, !1}
!5 = !{void (float addrspace(1)*, float addrspace(1)*)* @test_uniform_masked, !1}
!6 = !{!"-cl-std=CL1.2"}
