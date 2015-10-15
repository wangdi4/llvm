; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -runtime=ocl -print-wia-check -WIAnalysis %t.bc -S -o %t1.ll  | FileCheck %s


; ModuleID = 'c:\work\temp\ashr_add_shl.ll'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

declare i64 @_Z13get_global_idj(i32) nounwind readnone



;CHECK: ashr_shl
;CHECK: WI-RunOnFunction 1 %ashr_gid
;CHECK: ret void
define void @ashr_shl(float addrspace(1)* nocapture %a) nounwind {
  %gid = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %shl_gid = shl i64 %gid, 32
  %ashr_gid = ashr exact i64 %shl_gid, 32
  %gep = getelementptr inbounds float addrspace(1)* %a, i64 %ashr_gid
  store float 0.000000e+00, float addrspace(1)* %gep, align 4
  ret void
}

;CHECK: ashr_add_shl
;CHECK: WI-RunOnFunction 1 %ashr_gid
;CHECK: ret void
define void @ashr_add_shl(float addrspace(1)* nocapture %a) nounwind {
  %gid = tail call i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %shl_gid = shl i64 %gid, 32
  %add_gid = add i64 %shl_gid, 4303574410461184
  %ashr_gid = ashr exact i64 %add_gid, 32
  %gep = getelementptr inbounds float addrspace(1)* %a, i64 %ashr_gid
  store float 0.000000e+00, float addrspace(1)* %gep, align 4
  ret void
}

!opencl.kernels = !{!0}
!opencl.compiler.options = !{!2}

!0 = !{null, !1}
!1 = !{!"image_access_qualifier", i32 3, i32 3}
!2 = !{!"-cl-std=CL1.2"}
