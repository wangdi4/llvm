; RUN: llvm-as %s -o %t.bc
; RUN: %oclopt  -runtimelib %p/../Full/runtime.bc -packetize -packet-size=4 -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

declare i32 @_Z13get_global_idj(i32) nounwind readnone

; CHECK: @test_fptrunc
; CHECK:      fptrunc
; CHECK-NEXT: fadd
; CHECK: ret

define void @test_fptrunc(float addrspace(3)* %memA, double addrspace(1)* nocapture %memB, float addrspace(1)* %memC) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float, float addrspace(3)* %memA, i32 %call
  %0 = load float, float addrspace(3)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds double, double addrspace(1)* %memB, i32 %call
  %1 = load double, double addrspace(1)* %arrayidx1, align 4
  %trunc = fptrunc double %1 to float
  %add = fadd float %0, %trunc
  %arrayidx2 = getelementptr inbounds float, float addrspace(1)* %memC, i32 %call
  store float %add, float addrspace(1)* %arrayidx2, align 4
  ret void
}

; CHECK: @test_fpext
; CHECK:      fpext
; CHECK-NEXT: fadd
; CHECK: ret

define void @test_fpext(double addrspace(3)* %memA, float addrspace(1)* nocapture %memB, double addrspace(1)* %memC) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds double, double addrspace(3)* %memA, i32 %call
  %0 = load double, double addrspace(3)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds float, float addrspace(1)* %memB, i32 %call
  %1 = load float, float addrspace(1)* %arrayidx1, align 4
  %ext = fpext float %1 to double
  %add = fadd double %0, %ext
  %arrayidx2 = getelementptr inbounds double, double addrspace(1)* %memC, i32 %call
  store double %add, double addrspace(1)* %arrayidx2, align 4
  ret void
}

; CHECK: @test_trunc
; CHECK:      trunc
; CHECK-NEXT: add
; CHECK: ret

define void @test_trunc(i32 addrspace(3)* %memA, i64 addrspace(1)* nocapture %memB, i32 addrspace(1)* %memC) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds i32, i32 addrspace(3)* %memA, i32 %call
  %0 = load i32, i32 addrspace(3)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds i64, i64 addrspace(1)* %memB, i32 %call
  %1 = load i64, i64 addrspace(1)* %arrayidx1, align 4
  %tr = trunc i64 %1 to i32
  %add = add i32 %0, %tr
  %arrayidx2 = getelementptr inbounds i32, i32 addrspace(1)* %memC, i32 %call
  store i32 %add, i32 addrspace(1)* %arrayidx2, align 4
  ret void
}

; CHECK: @test_sext
; CHECK:      sext
; CHECK-NEXT: add
; CHECK: ret

define void @test_sext(i64 addrspace(3)* %memA, i32 addrspace(1)* nocapture %memB, i64 addrspace(1)* %memC) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds i64, i64 addrspace(3)* %memA, i32 %call
  %0 = load i64, i64 addrspace(3)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds i32, i32 addrspace(1)* %memB, i32 %call
  %1 = load i32, i32 addrspace(1)* %arrayidx1, align 4
  %ext = sext i32 %1 to i64
  %add = add i64 %0, %ext
  %arrayidx2 = getelementptr inbounds i64, i64 addrspace(1)* %memC, i32 %call
  store i64 %add, i64 addrspace(1)* %arrayidx2, align 4
  ret void
}

; CHECK: @test_zext
; CHECK:      zext
; CHECK-NEXT: add
; CHECK: ret

define void @test_zext(i64 addrspace(3)* %memA, i32 addrspace(1)* nocapture %memB, i64 addrspace(1)* %memC) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds i64, i64 addrspace(3)* %memA, i32 %call
  %0 = load i64, i64 addrspace(3)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds i32, i32 addrspace(1)* %memB, i32 %call
  %1 = load i32, i32 addrspace(1)* %arrayidx1, align 4
  %ext = zext i32 %1 to i64
  %add = add i64 %0, %ext
  %arrayidx2 = getelementptr inbounds i64, i64 addrspace(1)* %memC, i32 %call
  store i64 %add, i64 addrspace(1)* %arrayidx2, align 4
  ret void
}

; CHECK: @test_fptosi
; CHECK:      fptosi
; CHECK-NEXT: add
; CHECK: ret

define void @test_fptosi(i32 addrspace(3)* %memA, float addrspace(1)* nocapture %memB, i32 addrspace(1)* %memC) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds i32, i32 addrspace(3)* %memA, i32 %call
  %0 = load i32, i32 addrspace(3)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds float, float addrspace(1)* %memB, i32 %call
  %1 = load float, float addrspace(1)* %arrayidx1, align 4
  %to = fptosi float %1 to i32
  %add = add i32 %0, %to
  %arrayidx2 = getelementptr inbounds i32, i32 addrspace(1)* %memC, i32 %call
  store i32 %add, i32 addrspace(1)* %arrayidx2, align 4
  ret void
}

; CHECK: @test_fptoui
; CHECK:      fptoui
; CHECK-NEXT: add
; CHECK: ret

define void @test_fptoui(i32 addrspace(3)* %memA, float addrspace(1)* nocapture %memB, i32 addrspace(1)* %memC) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds i32, i32 addrspace(3)* %memA, i32 %call
  %0 = load i32, i32 addrspace(3)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds float, float addrspace(1)* %memB, i32 %call
  %1 = load float, float addrspace(1)* %arrayidx1, align 4
  %to = fptoui float %1 to i32
  %add = add i32 %0, %to
  %arrayidx2 = getelementptr inbounds i32, i32 addrspace(1)* %memC, i32 %call
  store i32 %add, i32 addrspace(1)* %arrayidx2, align 4
  ret void
}

; CHECK: @test_uitofp
; CHECK:      uitofp
; CHECK-NEXT: fadd
; CHECK: ret

define void @test_uitofp(float addrspace(3)* %memA, i32 addrspace(1)* nocapture %memB, float addrspace(1)* %memC) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float, float addrspace(3)* %memA, i32 %call
  %0 = load float, float addrspace(3)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds i32, i32 addrspace(1)* %memB, i32 %call
  %1 = load i32, i32 addrspace(1)* %arrayidx1, align 4
  %to = uitofp i32 %1 to float
  %add = fadd float %0, %to
  %arrayidx2 = getelementptr inbounds float, float addrspace(1)* %memC, i32 %call
  store float %add, float addrspace(1)* %arrayidx2, align 4
  ret void
}

; CHECK: @test_sitofp
; CHECK:      sitofp
; CHECK-NEXT: fadd
; CHECK: ret

define void @test_sitofp(float addrspace(3)* %memA, i32 addrspace(1)* nocapture %memB, float addrspace(1)* %memC) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float, float addrspace(3)* %memA, i32 %call
  %0 = load float, float addrspace(3)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds i32, i32 addrspace(1)* %memB, i32 %call
  %1 = load i32, i32 addrspace(1)* %arrayidx1, align 4
  %to = sitofp i32 %1 to float
  %add = fadd float %0, %to
  %arrayidx2 = getelementptr inbounds float, float addrspace(1)* %memC, i32 %call
  store float %add, float addrspace(1)* %arrayidx2, align 4
  ret void
}

; CHECK: @test_bitcast
; CHECK:      bitcast
; CHECK:      load
; CHECK:      bitcast
; CHECK:      load
; CHECK:      bitcast
; CHECK-NEXT: fadd
; CHECK: ret

define void @test_bitcast(float addrspace(3)* %memA, i32 addrspace(1)* nocapture %memB, float addrspace(1)* %memC) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float, float addrspace(3)* %memA, i32 %call
  %0 = load float, float addrspace(3)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds i32, i32 addrspace(1)* %memB, i32 %call
  %1 = load i32, i32 addrspace(1)* %arrayidx1, align 4
  %to = bitcast i32 %1 to float
  %add = fadd float %0, %to
  %arrayidx2 = getelementptr inbounds float, float addrspace(1)* %memC, i32 %call
  store float %add, float addrspace(1)* %arrayidx2, align 4
  ret void
}
