target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @BitcastPointer(ptr addrspace(1) %input, ptr addrspace(1) %output, i32 %buffer_size) nounwind {
entry:
  %f_vec = alloca <3 x float>, align 4
  %0 = bitcast ptr %f_vec to ptr

  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @BitcastPointer}
