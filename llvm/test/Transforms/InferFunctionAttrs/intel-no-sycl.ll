; CMPLRLLVM-23978: The SPIR target can't have unreachable code. To minimize
; this possibility, we prevent the functions below from being marked "noreturn"
; by the inferattrs pass.

; RUN: opt -passes=inferattrs -S < %s | FileCheck %s
; CHECK-NOT: noreturn

source_filename = "23978.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

; Function Attrs: convergent nounwind
declare dso_local spir_func void @__assert_fail(ptr addrspace(4), ptr addrspace(4), i32, ptr addrspace(4)) #0
declare dso_local spir_func void @error(i32, i32, ptr addrspace(4)) #0
declare dso_local spir_func void @abort() #0

define weak_odr dso_local spir_func void @testattr() {
entry:
  call spir_func void @__assert_fail(ptr addrspace(4) null, ptr addrspace(4) null, i32 0, ptr addrspace(4) null) #1
  call spir_func void @error(i32 0, i32 0, ptr addrspace(4) null) #1
  call spir_func void @abort() #1
  br label %cond.end

cond.end:                                         ; preds = %entry
  ret void
}

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { convergent nounwind }

!llvm.module.flags = !{!0}
!opencl.spir.version = !{!1}
!spirv.Source = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{i32 4, i32 100000}
!3 = !{}
!4 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
