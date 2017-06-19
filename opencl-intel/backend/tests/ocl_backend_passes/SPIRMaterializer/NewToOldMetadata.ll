; RUN: opt -spir-materializer -S %s -o - | FileCheck %s

; CHECK: !{{.*}} = !{void ()* @k, ![[AS:[0-9]+]], ![[AQ:[0-9]*]], ![[AT:[0-9]*]], ![[BT:[0-9]*]], ![[TQ:[0-9]*]]}
; CHECK: ![[AS]] = !{!"kernel_arg_addr_space"}
; CHECK: ![[AQ]] = !{!"kernel_arg_access_qual"}
; CHECK: ![[AT]] = !{!"kernel_arg_type"}
; CHECK: ![[BT]] = !{!"kernel_arg_base_type"}
; CHECK: ![[TQ]] = !{!"kernel_arg_type_qual"}

target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir"

; Function Attrs: noinline nounwind
define spir_kernel void @k() #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !1 !kernel_arg_type !1 !kernel_arg_base_type !1 !kernel_arg_type_qual !1 {
entry:
  ret void
}

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!1}
!opencl.compiler.options = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 2, i32 0}
!1 = !{}
!2 = !{!"clang version 4.0.1 (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-clang 7035f2d8cf714dda16ba4019fcfd0eabee7e8c67) (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-llvm 8456c1092144484dcae55966238f9b9a09d740fe)"}