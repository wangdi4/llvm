; RUN: sycl-post-link -symbols -S %s -o - 2>&1 | FileCheck %s

; Check that there are no warnings for a phony test:
; CHECK-NOT: warning:

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-sycldevice"

!opencl.spir.version = !{!0}
!spirv.Source = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!llvm.module.flags = !{!4}

!0 = !{i32 1, i32 2}
!1 = !{i32 4, i32 100000}
!2 = !{}
!3 = !{!"clang version 9.0.0"}
!4 = !{i32 1, !"wchar_size", i32 4}
