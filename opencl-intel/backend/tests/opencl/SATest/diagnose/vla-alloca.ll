target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

; Function Attrs: convergent norecurse nounwind
define dso_local spir_kernel void @test(i32 noundef %x) #0 !kernel_arg_addr_space !1 !kernel_arg_type !2 !kernel_arg_base_type !2 {
entry:
  %x.addr = alloca i32, i32 %x, align 4
  call spir_func void @_Z7barrierj(i32 0) #1
  %arrayidx = getelementptr inbounds i32, ptr %x.addr, i64 0
  store i32 0, ptr %arrayidx, align 4
  ret void
}

; Function Attrs: convergent nounwind
declare spir_func void @_Z7barrierj(i32 noundef) #1

attributes #0 = { convergent norecurse nounwind }
attributes #1 = { convergent nounwind }

!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}

!0 = !{i32 3, i32 0}
!1 = !{i32 0}
!2 = !{!"int"}
