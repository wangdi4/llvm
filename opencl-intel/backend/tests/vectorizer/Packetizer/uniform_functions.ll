; XFAIL: *
; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -packetize -packet-size=4 -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

declare i32 @_Z13get_global_idj(i32) nounwind readnone
declare i32 @async_work_group_copy(float addrspace(3)*, float addrspace(1)*, i32, i32)
declare i32 @async_work_group_strided_copy(float addrspace(3)*, float addrspace(1)*, i32, i32, i32)
declare i32 @_Z21async_work_group_copyPU3AS3fPU3AS1Kfjj(float addrspace(3)*, float addrspace(1)*, i32, i32)
declare i32 @_Z21async_work_group_copyPU3AS1fPU3AS3Kfjj(float addrspace(1)*, float addrspace(3)*, i32, i32)
declare i32 @_Z29async_work_group_strided_copyPU3AS3fPU3AS1Kfjjj(float addrspace(3)*, float addrspace(1)*, i32, i32, i32)
declare i32 @_Z29async_work_group_strided_copyPU3AS1fPU3AS3Kfjjj(float addrspace(1)*, float addrspace(3)*, i32, i32, i32)
declare void @_Z17wait_group_eventsiPj(i32, i32*)
declare void @mem_fence(i32)
declare void @read_mem_fence(i32)
declare void @write_mem_fence(i32)
declare i32 @get_global_offset(i32) nounwind readnone
declare double @_Z3cosd(double) nounwind readnone

; CHECK: @test_async_copy
; CHECK:      @async_work_group_copy
; CHECK-NEXT: @_Z17wait_group_eventsiPj
; CHECK-NEXT: ret

define void @test_async_copy(float addrspace(1)* %memA, float addrspace(1)* nocapture %memB, float addrspace(3)* %memC, float %s) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float, float addrspace(1)* %memA, i32 %call
  %0 = load float, float addrspace(1)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds float, float addrspace(1)* %memB, i32 %call
  %1 = load float, float addrspace(1)* %arrayidx1, align 4
  %mul = fmul float %1, %s
  %add = fadd float %0, %mul
  %arrayidx2 = getelementptr inbounds float, float addrspace(3)* %memC, i32 %call
  store float %add, float addrspace(3)* %arrayidx2, align 4
  %call3 = tail call i32 @async_work_group_copy(float addrspace(3)* %memC, float addrspace(1)* %memA, i32 1, i32 0) nounwind
  tail call void @_Z17wait_group_eventsiPj(i32 1, i32* null) nounwind
  ret void
}

; CHECK: @test_async_global2local
; CHECK:      @_Z21async_work_group_copyPU3AS3fPU3AS1Kfjj
; CHECK-NEXT: @_Z17wait_group_eventsiPj
; CHECK-NEXT: ret

define void @test_async_global2local(float addrspace(1)* %memA, float addrspace(1)* nocapture %memB, float addrspace(3)* %memC, float %s) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float, float addrspace(1)* %memA, i32 %call
  %0 = load float, float addrspace(1)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds float, float addrspace(1)* %memB, i32 %call
  %1 = load float, float addrspace(1)* %arrayidx1, align 4
  %mul = fmul float %1, %s
  %add = fadd float %0, %mul
  %arrayidx2 = getelementptr inbounds float, float addrspace(3)* %memC, i32 %call
  store float %add, float addrspace(3)* %arrayidx2, align 4
  %call3 = tail call i32 @_Z21async_work_group_copyPU3AS3fPU3AS1Kfjj(float addrspace(3)* %memC, float addrspace(1)* %memA, i32 1, i32 0) nounwind
  tail call void @_Z17wait_group_eventsiPj(i32 1, i32* null) nounwind
  ret void
}

; CHECK: @test_async_local2global
; CHECK:      @_Z21async_work_group_copyPU3AS1fPU3AS3Kfjj
; CHECK-NEXT: @_Z17wait_group_eventsiPj
; CHECK-NEXT: ret

define void @test_async_local2global(float addrspace(3)* %memA, float addrspace(1)* nocapture %memB, float addrspace(1)* %memC, float %s) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float, float addrspace(3)* %memA, i32 %call
  %0 = load float, float addrspace(3)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds float, float addrspace(1)* %memB, i32 %call
  %1 = load float, float addrspace(1)* %arrayidx1, align 4
  %mul = fmul float %1, %s
  %add = fadd float %0, %mul
  %arrayidx2 = getelementptr inbounds float, float addrspace(1)* %memC, i32 %call
  store float %add, float addrspace(1)* %arrayidx2, align 4
  %call3 = tail call i32 @_Z21async_work_group_copyPU3AS1fPU3AS3Kfjj(float addrspace(1)* %memC, float addrspace(3)* %memA, i32 1, i32 0) nounwind
  tail call void @_Z17wait_group_eventsiPj(i32 1, i32* null) nounwind
  ret void
}

; CHECK: @test_async_strided_copy
; CHECK:      @async_work_group_strided_copy
; CHECK-NEXT: @_Z17wait_group_eventsiPj
; CHECK-NEXT: ret

define void @test_async_strided_copy(float addrspace(1)* %memA, float addrspace(1)* nocapture %memB, float addrspace(3)* %memC, float %s) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float, float addrspace(1)* %memA, i32 %call
  %0 = load float, float addrspace(1)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds float, float addrspace(1)* %memB, i32 %call
  %1 = load float, float addrspace(1)* %arrayidx1, align 4
  %mul = fmul float %1, %s
  %add = fadd float %0, %mul
  %arrayidx2 = getelementptr inbounds float, float addrspace(3)* %memC, i32 %call
  store float %add, float addrspace(3)* %arrayidx2, align 4
  @_Z29async_work_group_strided_copyPU3AS3fPU3AS1Kfjj9ocl_event(float addrspace(3)* %memC, float addrspace(1)* %memA, i32 1, i32 1, i32 0) nounwind
  tail call void @_Z17wait_group_eventsiPj(i32 1, i32* null) nounwind
  ret void
}

define void @test_async_strided_global2local(float addrspace(1)* %memA, float addrspace(1)* nocapture %memB, float addrspace(3)* %memC, float %s) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float, float addrspace(1)* %memA, i32 %call
  %0 = load float, float addrspace(1)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds float, float addrspace(1)* %memB, i32 %call
  %1 = load float, float addrspace(1)* %arrayidx1, align 4
  %mul = fmul float %1, %s
  %add = fadd float %0, %mul
  %arrayidx2 = getelementptr inbounds float, float addrspace(3)* %memC, i32 %call
  store float %add, float addrspace(3)* %arrayidx2, align 4
  %call3 = tail call i32 @_Z29async_work_group_strided_copyPU3AS3fPU3AS1Kfjjj(float addrspace(3)* %memC, float addrspace(1)* %memA, i32 1, i32 1, i32 0) nounwind
  tail call void @_Z17wait_group_eventsiPj(i32 1, i32* null) nounwind
  ret void
}

; CHECK: @test_async_strided_local2global
; CHECK:      @_Z29async_work_group_strided_copyPU3AS1fPU3AS3Kfjjj
; CHECK-NEXT: @_Z17wait_group_eventsiPj
; CHECK-NEXT: ret

define void @test_async_strided_local2global(float addrspace(3)* %memA, float addrspace(1)* nocapture %memB, float addrspace(1)* %memC, float %s) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float, float addrspace(3)* %memA, i32 %call
  %0 = load float, float addrspace(3)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds float, float addrspace(1)* %memB, i32 %call
  %1 = load float, float addrspace(1)* %arrayidx1, align 4
  %mul = fmul float %1, %s
  %add = fadd float %0, %mul
  %arrayidx2 = getelementptr inbounds float, float addrspace(1)* %memC, i32 %call
  store float %add, float addrspace(1)* %arrayidx2, align 4
  %call3 = tail call i32 @_Z29async_work_group_strided_copyPU3AS1fPU3AS3Kfjjj(float addrspace(1)* %memC, float addrspace(3)* %memA, i32 1, i32 1, i32 0) nounwind
  tail call void @_Z17wait_group_eventsiPj(i32 1, i32* null) nounwind
  ret void
}

; CHECK: @test_mem_fence
; CHECK:      @mem_fence
; CHECK-NEXT: ret

define void @test_mem_fence(float addrspace(1)* nocapture %memA, float addrspace(1)* nocapture %memB, float addrspace(3)* nocapture %memC, float %s) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float, float addrspace(1)* %memA, i32 %call
  %0 = load float, float addrspace(1)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds float, float addrspace(1)* %memB, i32 %call
  %1 = load float, float addrspace(1)* %arrayidx1, align 4
  %mul = fmul float %1, %s
  %add = fadd float %0, %mul
  %arrayidx2 = getelementptr inbounds float, float addrspace(3)* %memC, i32 %call
  store float %add, float addrspace(3)* %arrayidx2, align 4
  tail call void @mem_fence(i32 1) nounwind
  ret void
}

; CHECK: @test_read_mem_fence
; CHECK:      @read_mem_fence
; CHECK-NEXT: ret

define void @test_read_mem_fence(float addrspace(1)* nocapture %memA, float addrspace(1)* nocapture %memB, float addrspace(3)* nocapture %memC, float %s) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float, float addrspace(1)* %memA, i32 %call
  %0 = load float, float addrspace(1)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds float, float addrspace(1)* %memB, i32 %call
  %1 = load float, float addrspace(1)* %arrayidx1, align 4
  %mul = fmul float %1, %s
  %add = fadd float %0, %mul
  %arrayidx2 = getelementptr inbounds float, float addrspace(3)* %memC, i32 %call
  store float %add, float addrspace(3)* %arrayidx2, align 4
  tail call void @read_mem_fence(i32 1) nounwind
  ret void
}

; CHECK: @test_write_mem_fence
; CHECK:      @write_mem_fence
; CHECK-NEXT: ret

define void @test_write_mem_fence(float addrspace(1)* nocapture %memA, float addrspace(1)* nocapture %memB, float addrspace(3)* nocapture %memC, float %s) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float, float addrspace(1)* %memA, i32 %call
  %0 = load float, float addrspace(1)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds float, float addrspace(1)* %memB, i32 %call
  %1 = load float, float addrspace(1)* %arrayidx1, align 4
  %mul = fmul float %1, %s
  %add = fadd float %0, %mul
  %arrayidx2 = getelementptr inbounds float, float addrspace(3)* %memC, i32 %call
  store float %add, float addrspace(3)* %arrayidx2, align 4
  tail call void @write_mem_fence(i32 1) nounwind
  ret void
}

; CHECK: @test_get_global_offset
; CHECK:      @get_global_offset
; CHECK-NOT:  @get_global_offset
; CHECK: ret

define void @test_get_global_offset(float addrspace(1)* nocapture %memA, float addrspace(1)* nocapture %memB, float addrspace(3)* nocapture %memC) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %call1 = tail call i32 @get_global_offset(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float, float addrspace(1)* %memA, i32 %call
  %0 = load float, float addrspace(1)* %arrayidx, align 4
  %conv = sitofp i32 %call1 to float
  %arrayidx2 = getelementptr inbounds float, float addrspace(1)* %memB, i32 %call
  %1 = load float, float addrspace(1)* %arrayidx2, align 4
  %mul = fmul float %conv, %1
  %add = fadd float %0, %mul
  %arrayidx3 = getelementptr inbounds float, float addrspace(3)* %memC, i32 %call
  store float %add, float addrspace(3)* %arrayidx3, align 4
  ret void
}

; CHECK: @test_cosD
; CHECK:      @_Z3cosDv4_d
; CHECK-NOT:  @_Z3cosDv4_d
; CHECK: ret

define void @test_cosD(double addrspace(1)* nocapture %memA, double addrspace(1)* nocapture %memB) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds double, double addrspace(1)* %memA, i32 %call
  %0 = load double, double addrspace(1)* %arrayidx, align 8
  %call1 = tail call double @_Z3cosd(double %0) nounwind readnone
  %arrayidx2 = getelementptr inbounds double, double addrspace(1)* %memB, i32 %call
  store double %call1, double addrspace(1)* %arrayidx2, align 8
  ret void
}

; RUN: opt  -runtimelib %p/../Full/runtime.bc -packetize -packet-size=4 -verify %s -S -o - \
; RUN: | FileCheck %s


%struct._image2d_t = type opaque

define void @uniform(%struct._image2d_t* %inputImage, <2 x float> %outCrd, <4 x float> addrspace(1)* nocapture %dst) nounwind {
; CHECK: [[C0:%[a-zA-Z0-9_]+]] = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_f(%struct._image2d_t* %inputImage, i32 17, <2 x float> %outCrd)
; CHECK: [[C1:%[a-zA-Z0-9_]+]] = tail call i32 @_Z13get_global_idj(i32 0)
; CHECK: [[S0:%[a-zA-Z0-9_]+]] = insertelement <4 x i32> undef, i32 [[C1]], i32 0
; CHECK: [[S1:%[a-zA-Z0-9_]+]] = shufflevector <4 x i32> [[S0]], <4 x i32> undef, <4 x i32> zeroinitializer
; CHECK: [[A0:%[a-zA-Z0-9_]+]] = add <4 x i32> [[S1]], <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[E0:%[a-zA-Z0-9_]+]] = extractelement <4 x i32> [[A0]], i32 0
; CHECK: [[E1:%[a-zA-Z0-9_]+]] = extractelement <4 x i32> [[A0]], i32 1
; CHECK: [[E2:%[a-zA-Z0-9_]+]] = extractelement <4 x i32> [[A0]], i32 2
; CHECK: [[E3:%[a-zA-Z0-9_]+]] = extractelement <4 x i32> [[A0]], i32 3
; CHECK: [[G0:%[a-zA-Z0-9_]+]] = getelementptr inbounds <4 x float>, <4 x float> addrspace(1)* %dst, i32 [[E0]]
; CHECK: [[G1:%[a-zA-Z0-9_]+]] = getelementptr inbounds <4 x float>, <4 x float> addrspace(1)* %dst, i32 [[E1]]
; CHECK: [[G2:%[a-zA-Z0-9_]+]] = getelementptr inbounds <4 x float>, <4 x float> addrspace(1)* %dst, i32 [[E2]]
; CHECK: [[G3:%[a-zA-Z0-9_]+]] = getelementptr inbounds <4 x float>, <4 x float> addrspace(1)* %dst, i32 [[E3]]
; CHECK: store <4 x float> [[C0]], <4 x float> addrspace(1)* [[G0]], align 16
; CHECK: store <4 x float> [[C0]], <4 x float> addrspace(1)* [[G1]], align 16
; CHECK: store <4 x float> [[C0]], <4 x float> addrspace(1)* [[G2]], align 16
; CHECK: store <4 x float> [[C0]], <4 x float> addrspace(1)* [[G3]], align 16
; CHECK: ret void
entry:
  %call = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_f(%struct._image2d_t* %inputImage, i32 17, <2 x float> %outCrd) nounwind readnone
  %call1 = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds <4 x float>, <4 x float> addrspace(1)* %dst, i32 %call1
  store <4 x float> %call, <4 x float> addrspace(1)* %arrayidx, align 16
  ret void
}


declare <4 x float> @_Z11read_imagefP10_image2d_tjDv2_f(%struct._image2d_t*, i32, <2 x float>) nounwind readnone
