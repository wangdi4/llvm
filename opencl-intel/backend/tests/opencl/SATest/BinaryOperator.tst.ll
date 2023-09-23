; ModuleID = 'BinaryOperator.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @BinaryKernel(ptr addrspace(1) %input, ptr addrspace(1) %output, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca ptr addrspace(1), align 4
  %output.addr = alloca ptr addrspace(1), align 4
  %buffer_size.addr = alloca i32, align 4
  %tid = alloca i32, align 4
  store ptr addrspace(1) %input, ptr addrspace(1) %input.addr, align 4
  store ptr addrspace(1) %output, ptr addrspace(1) %output.addr, align 4
  store i32 %buffer_size, ptr %buffer_size.addr, align 4
  %call = call i32 @_Z13get_global_idj(i32 0)
  store i32 %call, ptr %tid, align 4
  %tmp = load ptr %tid, align 4
  %tmp1 = load ptr addrspace(1) %input.addr, align 4
  %arrayidx = getelementptr inbounds float, ptr addrspace(1) %tmp1, i32 %tmp
  %tmp2 = load ptr addrspace(1) %arrayidx
  %add = fadd float %tmp2, 1.000000e+000
  %tmp3 = load ptr %tid, align 4
  %tmp4 = load ptr addrspace(1) %output.addr, align 4
  %arrayidx5 = getelementptr inbounds float, ptr addrspace(1) %tmp4, i32 %tmp3
  store float %add, ptr addrspace(1) %arrayidx5
  %tmp6 = load ptr %tid, align 4
  %tmp7 = load ptr addrspace(1) %output.addr, align 4
  %arrayidx8 = getelementptr inbounds float, ptr addrspace(1) %tmp7, i32 %tmp6
  %tmp9 = load ptr addrspace(1) %arrayidx8
  %div = fdiv float %tmp9, 5.000000e+000
  %tmp10 = load ptr %tid, align 4
  %tmp11 = load ptr addrspace(1) %output.addr, align 4
  %arrayidx12 = getelementptr inbounds float, ptr addrspace(1) %tmp11, i32 %tmp10
  store float %div, ptr addrspace(1) %arrayidx12
  %tmp13 = load ptr %tid, align 4
  %tmp14 = load ptr addrspace(1) %output.addr, align 4
  %arrayidx15 = getelementptr inbounds float, ptr addrspace(1) %tmp14, i32 %tmp13
  %tmp16 = load ptr addrspace(1) %arrayidx15
  %sub = fsub float 2.000000e+000, %tmp16
  %tmp17 = load ptr %tid, align 4
  %tmp18 = load ptr addrspace(1) %output.addr, align 4
  %arrayidx19 = getelementptr inbounds float, ptr addrspace(1) %tmp18, i32 %tmp17
  store float %sub, ptr addrspace(1) %arrayidx19
  %tmp20 = load ptr %tid, align 4
  %tmp21 = load ptr addrspace(1) %output.addr, align 4
  %arrayidx22 = getelementptr inbounds float, ptr addrspace(1) %tmp21, i32 %tmp20
  %tmp23 = load ptr addrspace(1) %arrayidx22
  %tmp24 = load ptr %tid, align 4
  %tmp25 = load ptr addrspace(1) %input.addr, align 4
  %arrayidx26 = getelementptr inbounds float, ptr addrspace(1) %tmp25, i32 %tmp24
  %tmp27 = load ptr addrspace(1) %arrayidx26
  %mul = fmul float %tmp23, %tmp27
  %tmp28 = load ptr %tid, align 4
  %tmp29 = load ptr addrspace(1) %output.addr, align 4
  %arrayidx30 = getelementptr inbounds float, ptr addrspace(1) %tmp29, i32 %tmp28
  store float %mul, ptr addrspace(1) %arrayidx30
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @BinaryKernel}
