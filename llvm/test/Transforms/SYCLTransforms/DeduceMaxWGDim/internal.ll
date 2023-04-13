; RUN: opt -passes=sycl-kernel-deduce-max-dim -S %s | FileCheck %s
; RUN: opt -passes=sycl-kernel-deduce-max-dim -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

; Call to an internal function (which was not inlined)

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK-DAG: define void @A{{.*}} !max_wg_dimensions ![[WG_DIM1:[0-9]+]]
; CHECK-DAG: define void @B{{.*}} !max_wg_dimensions ![[WG_DIM2:[0-9]+]]

; CHECK-DAG: ![[WG_DIM1]] = !{i32 1}
; CHECK-DAG: ![[WG_DIM2]] = !{i32 2}

; Function Attrs: nounwind readnone
define i32 @bar(i32 %x) #0 {
entry:
  %gid = tail call i64 @_Z13get_global_idj(i32 0) #0
  ret i32 %x
}

; Function Attrs: nounwind readnone
define i32 @foo(i32 %x) #0 {
entry:
  %call = call i32 @bar(i32 %x) #0
  ret i32 %x
}

; Function Attrs: nounwind
define void @A() #1 !no_barrier_path !4 !vectorized_width !5 {
entry:
  %call = call i32 @foo(i32 0) #0
  ret void
}

; Function Attrs: nounwind
define void @B() #1 !no_barrier_path !4 !vectorized_width !5 {
entry:
  %gid = tail call i64 @_Z13get_global_idj(i32 1) #0
  %call = call i32 @foo(i32 0) #0
  ret void
}

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) #0

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }

!sycl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!1}
!opencl.ocl.version = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.functions_info = !{}

!0 = !{void ()* @A, void ()* @B}
!1 = !{i32 1, i32 0}
!2 = !{i32 0, i32 0}
!3 = !{}
!4 = !{i1 true}
!5 = !{i32 1}

; DEBUGIFY-NOT: WARNING
