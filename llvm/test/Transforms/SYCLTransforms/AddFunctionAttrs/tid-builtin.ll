; RUN: opt -passes=sycl-kernel-add-function-attrs -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-function-attrs -S < %s | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

; CHECK-NOT: memory(none)

define void @foo() #0 {
entry:
  %0 = call spir_func i64 @_Z13get_global_idj(i32 noundef 0) #1
  %1 = call spir_func i64 @_Z12get_local_idj(i32 noundef 0) #1
  %2 = call spir_func i64 @_Z20get_global_linear_idv() #1
  %3 = call spir_func i64 @_Z19get_local_linear_idv() #1
  %4 = call spir_func i32 @_Z16get_sub_group_idv() #1
  %5 = call spir_func i32 @_Z22get_sub_group_local_idv() #1
  ret void
}

declare spir_func i64 @_Z13get_global_idj(i32 noundef) #1

declare spir_func i64 @_Z12get_local_idj(i32 noundef) #1

declare spir_func i64 @_Z20get_global_linear_idv() #1

declare spir_func i64 @_Z19get_local_linear_idv() #1

declare spir_func i32 @_Z16get_sub_group_idv() #1

declare spir_func i32 @_Z22get_sub_group_local_idv() #1

define dso_local void @test() {
entry:
  call void @foo()
  ret void
}

attributes #0 = { norecurse nounwind willreturn memory(none) }
attributes #1 = { nounwind willreturn memory(none) }

!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}

!0 = !{i32 3, i32 0}

; DEBUGIFY-NOT: WARNING
