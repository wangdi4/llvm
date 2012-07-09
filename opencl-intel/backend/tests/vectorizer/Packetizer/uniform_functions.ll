; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -packetize -packet-size=4 -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

declare i32 @get_global_id(i32) nounwind readnone
declare i32 @async_work_group_copy(float addrspace(3)*, float addrspace(1)*, i32, i32)
declare i32 @async_work_group_strided_copy(float addrspace(3)*, float addrspace(1)*, i32, i32, i32)
declare i32 @_Z21async_work_group_copyPU3AS3fPKU3AS1fjj(float addrspace(3)*, float addrspace(1)*, i32, i32)
declare i32 @_Z21async_work_group_copyPU3AS1fPKU3AS3fjj(float addrspace(1)*, float addrspace(3)*, i32, i32)
declare i32 @_Z29async_work_group_strided_copyPU3AS3fPKU3AS1fjjj(float addrspace(3)*, float addrspace(1)*, i32, i32, i32)
declare i32 @_Z29async_work_group_strided_copyPU3AS1fPKU3AS3fjjj(float addrspace(1)*, float addrspace(3)*, i32, i32, i32)
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
  %call = tail call i32 @get_global_id(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float addrspace(1)* %memA, i32 %call
  %0 = load float addrspace(1)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds float addrspace(1)* %memB, i32 %call
  %1 = load float addrspace(1)* %arrayidx1, align 4
  %mul = fmul float %1, %s
  %add = fadd float %0, %mul
  %arrayidx2 = getelementptr inbounds float addrspace(3)* %memC, i32 %call
  store float %add, float addrspace(3)* %arrayidx2, align 4
  %call3 = tail call i32 @async_work_group_copy(float addrspace(3)* %memC, float addrspace(1)* %memA, i32 1, i32 0) nounwind
  tail call void @_Z17wait_group_eventsiPj(i32 1, i32* null) nounwind
  ret void
}

; CHECK: @test_async_global2local
; CHECK:      @_Z21async_work_group_copyPU3AS3fPKU3AS1fjj
; CHECK-NEXT: @_Z17wait_group_eventsiPj
; CHECK-NEXT: ret

define void @test_async_global2local(float addrspace(1)* %memA, float addrspace(1)* nocapture %memB, float addrspace(3)* %memC, float %s) nounwind {
entry:
  %call = tail call i32 @get_global_id(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float addrspace(1)* %memA, i32 %call
  %0 = load float addrspace(1)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds float addrspace(1)* %memB, i32 %call
  %1 = load float addrspace(1)* %arrayidx1, align 4
  %mul = fmul float %1, %s
  %add = fadd float %0, %mul
  %arrayidx2 = getelementptr inbounds float addrspace(3)* %memC, i32 %call
  store float %add, float addrspace(3)* %arrayidx2, align 4
  %call3 = tail call i32 @_Z21async_work_group_copyPU3AS3fPKU3AS1fjj(float addrspace(3)* %memC, float addrspace(1)* %memA, i32 1, i32 0) nounwind
  tail call void @_Z17wait_group_eventsiPj(i32 1, i32* null) nounwind
  ret void
}

; CHECK: @test_async_local2global
; CHECK:      @_Z21async_work_group_copyPU3AS1fPKU3AS3fjj
; CHECK-NEXT: @_Z17wait_group_eventsiPj
; CHECK-NEXT: ret

define void @test_async_local2global(float addrspace(3)* %memA, float addrspace(1)* nocapture %memB, float addrspace(1)* %memC, float %s) nounwind {
entry:
  %call = tail call i32 @get_global_id(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float addrspace(3)* %memA, i32 %call
  %0 = load float addrspace(3)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds float addrspace(1)* %memB, i32 %call
  %1 = load float addrspace(1)* %arrayidx1, align 4
  %mul = fmul float %1, %s
  %add = fadd float %0, %mul
  %arrayidx2 = getelementptr inbounds float addrspace(1)* %memC, i32 %call
  store float %add, float addrspace(1)* %arrayidx2, align 4
  %call3 = tail call i32 @_Z21async_work_group_copyPU3AS1fPKU3AS3fjj(float addrspace(1)* %memC, float addrspace(3)* %memA, i32 1, i32 0) nounwind
  tail call void @_Z17wait_group_eventsiPj(i32 1, i32* null) nounwind
  ret void
}

; CHECK: @test_async_strided_copy
; CHECK:      @async_work_group_strided_copy
; CHECK-NEXT: @_Z17wait_group_eventsiPj
; CHECK-NEXT: ret

define void @test_async_strided_copy(float addrspace(1)* %memA, float addrspace(1)* nocapture %memB, float addrspace(3)* %memC, float %s) nounwind {
entry:
  %call = tail call i32 @get_global_id(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float addrspace(1)* %memA, i32 %call
  %0 = load float addrspace(1)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds float addrspace(1)* %memB, i32 %call
  %1 = load float addrspace(1)* %arrayidx1, align 4
  %mul = fmul float %1, %s
  %add = fadd float %0, %mul
  %arrayidx2 = getelementptr inbounds float addrspace(3)* %memC, i32 %call
  store float %add, float addrspace(3)* %arrayidx2, align 4
  %call3 = tail call i32 @async_work_group_strided_copy(float addrspace(3)* %memC, float addrspace(1)* %memA, i32 1, i32 1, i32 0) nounwind
  tail call void @_Z17wait_group_eventsiPj(i32 1, i32* null) nounwind
  ret void
}

define void @test_async_strided_global2local(float addrspace(1)* %memA, float addrspace(1)* nocapture %memB, float addrspace(3)* %memC, float %s) nounwind {
entry:
  %call = tail call i32 @get_global_id(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float addrspace(1)* %memA, i32 %call
  %0 = load float addrspace(1)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds float addrspace(1)* %memB, i32 %call
  %1 = load float addrspace(1)* %arrayidx1, align 4
  %mul = fmul float %1, %s
  %add = fadd float %0, %mul
  %arrayidx2 = getelementptr inbounds float addrspace(3)* %memC, i32 %call
  store float %add, float addrspace(3)* %arrayidx2, align 4
  %call3 = tail call i32 @_Z29async_work_group_strided_copyPU3AS3fPKU3AS1fjjj(float addrspace(3)* %memC, float addrspace(1)* %memA, i32 1, i32 1, i32 0) nounwind
  tail call void @_Z17wait_group_eventsiPj(i32 1, i32* null) nounwind
  ret void
}

; CHECK: @test_async_strided_local2global
; CHECK:      @_Z29async_work_group_strided_copyPU3AS1fPKU3AS3fjjj
; CHECK-NEXT: @_Z17wait_group_eventsiPj
; CHECK-NEXT: ret

define void @test_async_strided_local2global(float addrspace(3)* %memA, float addrspace(1)* nocapture %memB, float addrspace(1)* %memC, float %s) nounwind {
entry:
  %call = tail call i32 @get_global_id(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float addrspace(3)* %memA, i32 %call
  %0 = load float addrspace(3)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds float addrspace(1)* %memB, i32 %call
  %1 = load float addrspace(1)* %arrayidx1, align 4
  %mul = fmul float %1, %s
  %add = fadd float %0, %mul
  %arrayidx2 = getelementptr inbounds float addrspace(1)* %memC, i32 %call
  store float %add, float addrspace(1)* %arrayidx2, align 4
  %call3 = tail call i32 @_Z29async_work_group_strided_copyPU3AS1fPKU3AS3fjjj(float addrspace(1)* %memC, float addrspace(3)* %memA, i32 1, i32 1, i32 0) nounwind
  tail call void @_Z17wait_group_eventsiPj(i32 1, i32* null) nounwind
  ret void
}

; CHECK: @test_mem_fence
; CHECK:      @mem_fence
; CHECK-NEXT: ret

define void @test_mem_fence(float addrspace(1)* nocapture %memA, float addrspace(1)* nocapture %memB, float addrspace(3)* nocapture %memC, float %s) nounwind {
entry:
  %call = tail call i32 @get_global_id(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float addrspace(1)* %memA, i32 %call
  %0 = load float addrspace(1)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds float addrspace(1)* %memB, i32 %call
  %1 = load float addrspace(1)* %arrayidx1, align 4
  %mul = fmul float %1, %s
  %add = fadd float %0, %mul
  %arrayidx2 = getelementptr inbounds float addrspace(3)* %memC, i32 %call
  store float %add, float addrspace(3)* %arrayidx2, align 4
  tail call void @mem_fence(i32 1) nounwind
  ret void
}

; CHECK: @test_read_mem_fence
; CHECK:      @read_mem_fence
; CHECK-NEXT: ret

define void @test_read_mem_fence(float addrspace(1)* nocapture %memA, float addrspace(1)* nocapture %memB, float addrspace(3)* nocapture %memC, float %s) nounwind {
entry:
  %call = tail call i32 @get_global_id(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float addrspace(1)* %memA, i32 %call
  %0 = load float addrspace(1)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds float addrspace(1)* %memB, i32 %call
  %1 = load float addrspace(1)* %arrayidx1, align 4
  %mul = fmul float %1, %s
  %add = fadd float %0, %mul
  %arrayidx2 = getelementptr inbounds float addrspace(3)* %memC, i32 %call
  store float %add, float addrspace(3)* %arrayidx2, align 4
  tail call void @read_mem_fence(i32 1) nounwind
  ret void
}

; CHECK: @test_write_mem_fence
; CHECK:      @write_mem_fence
; CHECK-NEXT: ret

define void @test_write_mem_fence(float addrspace(1)* nocapture %memA, float addrspace(1)* nocapture %memB, float addrspace(3)* nocapture %memC, float %s) nounwind {
entry:
  %call = tail call i32 @get_global_id(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float addrspace(1)* %memA, i32 %call
  %0 = load float addrspace(1)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds float addrspace(1)* %memB, i32 %call
  %1 = load float addrspace(1)* %arrayidx1, align 4
  %mul = fmul float %1, %s
  %add = fadd float %0, %mul
  %arrayidx2 = getelementptr inbounds float addrspace(3)* %memC, i32 %call
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
  %call = tail call i32 @get_global_id(i32 0) nounwind readnone
  %call1 = tail call i32 @get_global_offset(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float addrspace(1)* %memA, i32 %call
  %0 = load float addrspace(1)* %arrayidx, align 4
  %conv = sitofp i32 %call1 to float
  %arrayidx2 = getelementptr inbounds float addrspace(1)* %memB, i32 %call
  %1 = load float addrspace(1)* %arrayidx2, align 4
  %mul = fmul float %conv, %1
  %add = fadd float %0, %mul
  %arrayidx3 = getelementptr inbounds float addrspace(3)* %memC, i32 %call
  store float %add, float addrspace(3)* %arrayidx3, align 4
  ret void
}

; CHECK: @test_cosD
; CHECK:      @_Z3cosDv4_d
; CHECK-NOT:  @_Z3cosDv4_d
; CHECK: ret

define void @test_cosD(double addrspace(1)* nocapture %memA, double addrspace(1)* nocapture %memB) nounwind {
entry:
  %call = tail call i32 @get_global_id(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds double addrspace(1)* %memA, i32 %call
  %0 = load double addrspace(1)* %arrayidx, align 8
  %call1 = tail call double @_Z3cosd(double %0) nounwind readnone
  %arrayidx2 = getelementptr inbounds double addrspace(1)* %memB, i32 %call
  store double %call1, double addrspace(1)* %arrayidx2, align 8
  ret void
}
