target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024"
target triple = "spir64-unknown-unknown"

; Function Attrs: nounwind
define spir_kernel void @foo(i32 addrspace(1)* %subdevice_id, i32 addrspace(1)* %hwthread_id) #0 {
entry:
  %0 = call spir_func i32 @_Z31__spirv_BuiltInSubDeviceIDINTELv() #1
  %1 = call spir_func i32 @_Z36__spirv_BuiltInGlobalHWThreadIDINTELv() #1
  store i32 %0, i32 addrspace(1)* %subdevice_id
  store i32 %1, i32 addrspace(1)* %hwthread_id
  ret void
}

; Function Attrs: nounwind readnone willreturn
declare spir_func i32 @_Z31__spirv_BuiltInSubDeviceIDINTELv() #1

; Function Attrs: nounwind readnone willreturn
declare spir_func i32 @_Z36__spirv_BuiltInGlobalHWThreadIDINTELv() #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone willreturn }

!spirv.MemoryModel = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!spirv.Source = !{!1}
!opencl.spir.version = !{!2}
!opencl.ocl.version = !{!3}
!opencl.used.extensions = !{!4}
!opencl.used.optional.core.features = !{!4}
!spirv.Generator = !{!5}

!0 = !{i32 2, i32 2}
!1 = !{i32 4, i32 100000}
!2 = !{i32 1, i32 2}
!3 = !{i32 1, i32 0}
!4 = !{}
!5 = !{i16 6, i16 14}
!6 = !{!"none", !"none"}
!7 = !{!"int*", !"class.cl::sycl::id"}
!8 = !{!"", !""}
!9 = !{i32 8}
