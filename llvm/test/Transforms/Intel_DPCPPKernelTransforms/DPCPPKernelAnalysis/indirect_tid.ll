; RUN: opt -analyze -dpcpp-kernel-analysis < %s -S -o - | FileCheck %s

; CHECK: KernelAnalysis
; CHECK: kernel_call_func_with_gid no
; CHECK: kernel_call_func_with_lid no
; CHECK: kernel_call_func_call_func_with_gid no
; CHECK: kernel_call_func_call_func_with_lid no
; CHECK: kernel_call_func_without_tid yes

define void @func_with_gid(i32 addrspace(1)* %out) nounwind alwaysinline {
  %id = call i32 @_Z13get_global_idj(i32 1) nounwind
  %outptr = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %id
  store i32 0, i32 addrspace(1)* %outptr
  ret void
}

define void @func_with_lid(i32 addrspace(1)* %out) nounwind alwaysinline {
  %id = call i32 @_Z12get_local_idj(i32 1) nounwind
  %outptr = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %id
  store i32 0, i32 addrspace(1)* %outptr
  ret void
}

define void @func_call_func_with_gid(i32 addrspace(1)* %out) nounwind alwaysinline {
  tail call void @func_with_gid(i32 addrspace(1)* %out)
  ret void
}


define void @func_call_func_with_lid(i32 addrspace(1)* %out) nounwind alwaysinline {
  tail call void @func_with_lid(i32 addrspace(1)* %out)
  ret void
}

define void @func_without_tid(i32 addrspace(1)* %out) nounwind alwaysinline {
  store i32 0, i32 addrspace(1)* %out
  ret void
}

define void @kernel_call_func_with_gid(i32 addrspace(1)* %out) #0 {
  tail call void @func_with_gid(i32 addrspace(1)* %out)
  ret void
}

define void @kernel_call_func_with_lid(i32 addrspace(1)* %out) #0 {
  tail call void @func_with_lid(i32 addrspace(1)* %out)
  ret void
}

define void @kernel_call_func_call_func_with_gid(i32 addrspace(1)* %out) #0 {
  tail call void @func_call_func_with_gid(i32 addrspace(1)* %out)
  ret void
}

define void @kernel_call_func_call_func_with_lid(i32 addrspace(1)* %out) #0 {
  tail call void @func_call_func_with_lid(i32 addrspace(1)* %out)
  ret void
}

define void @kernel_call_func_without_tid(i32 addrspace(1)* %out) #0 {
  tail call void @func_without_tid(i32 addrspace(1)* %out)
  ret void
}

declare dso_local i32 @_Z13get_global_idj(i32)

declare dso_local i32 @_Z12get_local_idj(i32)

attributes #0 = { "sycl_kernel" }
