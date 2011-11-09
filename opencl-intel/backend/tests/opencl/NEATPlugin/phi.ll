; RUN: llvm-as %s -o %s.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1

; ModuleID = 'phi.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%0 = type { float, float }
%struct.anon = type { float, float, float, float, float }

define void @phi(%0 addrspace(1)* nocapture %pOptionValue, %struct.anon addrspace(1)* nocapture %pOptionData, i32 %pathN) nounwind {
; <label>:0
  %1 = sitofp i32 %pathN to float
  %2 = fmul float %1, 0x3FB47AE140000000
  %3 = fptosi float %2 to i32
  %4 = icmp sgt i32 %3, 2
  %.op1 = add i32 %3, 1
  %5 = icmp sgt i32 %.op1, 1
  %6 = or i1 %4, %5
  br i1 %6, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %0
  %tmp4 = icmp slt i32 %3, 2
  %tmp5 = select i1 %tmp4, i32 %.op1, i32 3
  %tmp6 = icmp sgt i32 %tmp5, 2
  %tmp5.op = add i32 %tmp5, -1
  %tmp8 = select i1 %tmp6, i32 %tmp5.op, i32 1
  br label %7

; <label>:7                                       ; preds = %7, %bb.nph
  %indvar = phi i32 [ 0, %bb.nph ], [ %pos.03, %7 ]
  %sum.02 = phi float [ 3.000000e+000, %bb.nph ], [ %9, %7 ]
  %pos.03 = add i32 %indvar, 1
  %8 = sitofp i32 %pos.03 to float
  %9 = fadd float %sum.02, %8
  %exitcond = icmp eq i32 %pos.03, %tmp8
  br i1 %exitcond, label %._crit_edge, label %7

._crit_edge:                                      ; preds = %7, %0
  %sum.0.lcssa = phi float [ 3.000000e+000, %0 ], [ %9, %7 ]
  %10 = fsub float %1, %sum.0.lcssa
  %11 = tail call float @_Z4sqrtf(float %10) nounwind
  %12 = getelementptr inbounds %0 addrspace(1)* %pOptionValue, i64 0, i32 0
  store float %sum.0.lcssa, float addrspace(1)* %12, align 4
  %13 = getelementptr inbounds %0 addrspace(1)* %pOptionValue, i64 0, i32 1
  store float %11, float addrspace(1)* %13, align 4
  ret void
}

declare float @_Z4sqrtf(float)

!opencl.kernels = !{!0}

!0 = metadata !{void (%0 addrspace(1)*, %struct.anon addrspace(1)*, i32)* @phi, metadata !1, metadata !1, metadata !"", metadata !"TOptionValue __attribute__((address_space(1))) *, TOptionData __attribute__((address_space(1))) *, int const", metadata !"opencl_phi_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}
