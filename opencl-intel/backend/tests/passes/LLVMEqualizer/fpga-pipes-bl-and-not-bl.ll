; Compiled from:
; ----------------------------------------------------
; void test1(read_only pipe int p, global int *ptr) {
;   read_pipe(p, ptr);
; }
;
; void test2(read_only pipe int __attribute__((blocking)) p, global int *ptr) {
;   read_pipe(p, ptr);
; }
; ----------------------------------------------------
; Compile options:
;   -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -x cl -cl-std=CL1.2 -disable-llvm-passes -finclude-default-header
; ----------------------------------------------------
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -demangle-fpga-pipes -llvm-equalizer -verify -S %s | FileCheck %s

; CHECK-LABEL: define void @test1
; CHECK-NOT: call i32 @__read_pipe_2_AS1
; CHECK: call i32 @__read_pipe_2_fpga(%opencl.pipe_ro_t {{.*}}, i8 addrspace(4)* %{{.*}}, i32 4, i32 4)
; CHECK-NOT: call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t {{.*}}, i8 addrspace(4)* %{{.*}}, i32 4, i32 4)

; CHECK-LABEL: define void @test2
; CHECK-NOT: call i32 @__read_pipe_2_bl_AS1
; CHECK: call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t {{.*}}, i8 addrspace(4)* %{{.*}}, i32 4, i32 4)

; CHECK-NOT: declare i32 @__read_pipe_2_bl_AS1(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(1)*, i32, i32)
; CHECK: declare i32 @__read_pipe_2_fpga
; CHECK: declare i32 @__read_pipe_2_bl_fpga

; ModuleID = 'test.cl'
source_filename = "test.cl"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.pipe_ro_t = type opaque

; Function Attrs: convergent nounwind
define spir_func void @test1(%opencl.pipe_ro_t addrspace(1)* %p, i32 addrspace(1)* %ptr) #0 {
entry:
  %p.addr = alloca %opencl.pipe_ro_t addrspace(1)*, align 8
  %ptr.addr = alloca i32 addrspace(1)*, align 8
  store %opencl.pipe_ro_t addrspace(1)* %p, %opencl.pipe_ro_t addrspace(1)** %p.addr, align 8, !tbaa !4
  store i32 addrspace(1)* %ptr, i32 addrspace(1)** %ptr.addr, align 8, !tbaa !7
  %0 = load %opencl.pipe_ro_t addrspace(1)*, %opencl.pipe_ro_t addrspace(1)** %p.addr, align 8, !tbaa !4
  %1 = load i32 addrspace(1)*, i32 addrspace(1)** %ptr.addr, align 8, !tbaa !7
  %2 = bitcast i32 addrspace(1)* %1 to i8 addrspace(1)*
  %3 = call i32 @__read_pipe_2_AS1(%opencl.pipe_ro_t addrspace(1)* %0, i8 addrspace(1)* %2, i32 4, i32 4)
  ret void
}

declare i32 @__read_pipe_2_AS1(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(1)*, i32, i32)

; Function Attrs: convergent nounwind
define spir_func void @test2(%opencl.pipe_ro_t addrspace(1)* %p, i32 addrspace(1)* %ptr) #0 {
entry:
  %p.addr = alloca %opencl.pipe_ro_t addrspace(1)*, align 8
  %ptr.addr = alloca i32 addrspace(1)*, align 8
  store %opencl.pipe_ro_t addrspace(1)* %p, %opencl.pipe_ro_t addrspace(1)** %p.addr, align 8, !tbaa !4
  store i32 addrspace(1)* %ptr, i32 addrspace(1)** %ptr.addr, align 8, !tbaa !7
  %0 = load %opencl.pipe_ro_t addrspace(1)*, %opencl.pipe_ro_t addrspace(1)** %p.addr, align 8, !tbaa !4
  %1 = load i32 addrspace(1)*, i32 addrspace(1)** %ptr.addr, align 8, !tbaa !7
  %2 = bitcast i32 addrspace(1)* %1 to i8 addrspace(1)*
  %3 = call i32 @__read_pipe_2_bl_AS1(%opencl.pipe_ro_t addrspace(1)* %0, i8 addrspace(1)* %2, i32 4, i32 4)
  ret void
}

declare i32 @__read_pipe_2_bl_AS1(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(1)*, i32, i32)

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"icx (ICX) dev.8.x.0"}
!4 = !{!5, !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"any pointer", !5, i64 0}
