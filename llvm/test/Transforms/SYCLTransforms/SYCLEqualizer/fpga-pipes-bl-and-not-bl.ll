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

; RUN: llvm-as %p/../Inputs/fpga-pipes.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -sycl-demangle-fpga-pipes -passes=sycl-kernel-equalizer -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -sycl-demangle-fpga-pipes -passes=sycl-kernel-equalizer -S %s | FileCheck %s

; CHECK-LABEL: define dso_local void @test1
; CHECK-NOT: call i32 @__read_pipe_2_AS1
; CHECK: call i32 @__read_pipe_2_fpga({{.*}}, ptr addrspace(4) %{{.*}}, i32 4, i32 4)
; CHECK-NOT: call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t {{.*}}, ptr addrspace(4) %{{.*}}, i32 4, i32 4)

; CHECK-LABEL: define dso_local void @test2
; CHECK-NOT: call i32 @__read_pipe_2_bl_AS1
; CHECK: call i32 @__read_pipe_2_bl_fpga({{.*}}, ptr addrspace(4) %{{.*}}, i32 4, i32 4)

; CHECK-NOT: declare i32 @__read_pipe_2_bl_AS1(ptr addrspace(1), ptr addrspace(1), i32, i32)
; CHECK: declare i32 @__read_pipe_2_fpga
; CHECK: declare i32 @__read_pipe_2_bl_fpga

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

; Function Attrs: convergent norecurse nounwind
define dso_local spir_func void @test1(ptr addrspace(1) %p, ptr addrspace(1) noundef %ptr) #0 !arg_type_null_val !3 {
entry:
  %p.addr = alloca ptr addrspace(1), align 8
  %ptr.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %p, ptr %p.addr, align 8, !tbaa !4
  store ptr addrspace(1) %ptr, ptr %ptr.addr, align 8, !tbaa !7
  %0 = load ptr addrspace(1), ptr %p.addr, align 8, !tbaa !4
  %1 = load ptr addrspace(1), ptr %ptr.addr, align 8, !tbaa !7
  %2 = call spir_func i32 @__read_pipe_2_AS1(ptr addrspace(1) %0, ptr addrspace(1) %1, i32 4, i32 4)
  ret void
}

declare spir_func i32 @__read_pipe_2_AS1(ptr addrspace(1), ptr addrspace(1), i32, i32)

; Function Attrs: convergent norecurse nounwind
define dso_local spir_func void @test2(ptr addrspace(1) %p, ptr addrspace(1) noundef %ptr) #0 !arg_type_null_val !3 {
entry:
  %p.addr = alloca ptr addrspace(1), align 8
  %ptr.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %p, ptr %p.addr, align 8, !tbaa !4
  store ptr addrspace(1) %ptr, ptr %ptr.addr, align 8, !tbaa !7
  %0 = load ptr addrspace(1), ptr %p.addr, align 8, !tbaa !4
  %1 = load ptr addrspace(1), ptr %ptr.addr, align 8, !tbaa !7
  %2 = call spir_func i32 @__read_pipe_2_bl_AS1(ptr addrspace(1) %0, ptr addrspace(1) %1, i32 4, i32 4)
  ret void
}

declare spir_func i32 @__read_pipe_2_bl_AS1(ptr addrspace(1), ptr addrspace(1), i32, i32)

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }

!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.compiler.options = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 1, i32 2}
!1 = !{}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!3 = !{target("spirv.Pipe", 0) zeroinitializer, ptr addrspace(1) null}
!4 = !{!5, !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"any pointer", !5, i64 0}

; DEBUGIFY-NOT: WARNING
