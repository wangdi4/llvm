target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64_x86_64-unknown-unknown"

; Function Attrs: convergent norecurse nounwind
define dso_local spir_kernel void @test() #0 !kernel_arg_addr_space !0 !kernel_arg_access_qual !0 !kernel_arg_type !0 !kernel_arg_base_type !0 !kernel_arg_type_qual !0 !kernel_arg_name !0 !kernel_arg_host_accessible !0 !kernel_arg_pipe_depth !0 !kernel_arg_pipe_io !0 !kernel_arg_buffer_location !0 {
entry:
  ret void
}

attributes #0 = { convergent norecurse nounwind }

!0 = !{}
