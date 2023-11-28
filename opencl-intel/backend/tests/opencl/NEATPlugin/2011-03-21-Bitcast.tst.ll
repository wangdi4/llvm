; ModuleID = '2011-03-21-Bitcast.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @bitcast(ptr addrspace(1) %input, ptr addrspace(1) %output, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca ptr addrspace(1), align 4
  %output.addr = alloca ptr addrspace(1), align 4
  %buffer_size.addr = alloca i32, align 4
  %tid = alloca i32, align 4
  %f = alloca float, align 4
  store ptr addrspace(1) %input, ptr addrspace(1) %input.addr, align 4
  store ptr addrspace(1) %output, ptr addrspace(1) %output.addr, align 4
  store i32 %buffer_size, ptr %buffer_size.addr, align 4
  %call = call i32 @_Z13get_global_idj(i32 0)
  store i32 %call, ptr %tid, align 4
  %tmp = load i32, ptr %tid, align 4
  %as_typen = bitcast i32 %tmp to float
  store float %as_typen, ptr %f, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @bitcast}
