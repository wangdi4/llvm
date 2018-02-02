; This IR was generated from the following source code:
; void test1(read_only pipe int __attribute__((blocking)) p, global int *ptr) {
;   read_pipe(p, ptr);
;   local int *ptr_l;
;   read_pipe(p, ptr_l);
;   int *ptr_p;
;   read_pipe(p, ptr_p);
; }
;
; void test2(write_only pipe int __attribute__((blocking)) p, global int *ptr) {
;   write_pipe(p, ptr);
;   local int *ptr_l;
;   write_pipe(p, ptr_l);
;   int *ptr_p;
;   write_pipe(p, ptr_p);
;   constant int *ptr_c;
;   write_pipe(p, ptr_c);
; }

; Compiled by the following command:
; clang -cc1 -triple spir-unknown-unknown-intelfpga -emit-llvm -O0 -cl-std=CL2.0 -o - %s

; REQUIRES: fpga-emulator
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -spir-materializer -verify -S %s | FileCheck %s

%opencl.pipe_t = type opaque

; Function Attrs: convergent noinline nounwind optnone
define spir_func void @test1(%opencl.pipe_t addrspace(1)* %p, i32 addrspace(1)* %ptr) #0 {
entry:
  %p.addr = alloca %opencl.pipe_t addrspace(1)*, align 4
  %ptr.addr = alloca i32 addrspace(1)*, align 4
  %ptr_l = alloca i32 addrspace(3)*, align 4
  %ptr_p = alloca i32 addrspace(4)*, align 4
  store %opencl.pipe_t addrspace(1)* %p, %opencl.pipe_t addrspace(1)** %p.addr, align 4
  store i32 addrspace(1)* %ptr, i32 addrspace(1)** %ptr.addr, align 4
  %0 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)** %p.addr, align 4
  %1 = load i32 addrspace(1)*, i32 addrspace(1)** %ptr.addr, align 4
  %2 = addrspacecast i32 addrspace(1)* %1 to i8 addrspace(4)*
; CHECK: %{{[0-9]+}} = call i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %{{[0-9]+}}, i8 addrspace(4)* %{{[0-9]+}})
  %3 = call i32 @__read_pipe_2_bl(%opencl.pipe_t addrspace(1)* %0, i8 addrspace(4)* %2, i32 4, i32 4)
  %4 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)** %p.addr, align 4
  %5 = load i32 addrspace(3)*, i32 addrspace(3)** %ptr_l, align 4
  %6 = addrspacecast i32 addrspace(3)* %5 to i8 addrspace(4)*
; CHECK: %{{[0-9]+}} = call i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %{{[0-9]+}}, i8 addrspace(4)* %{{[0-9]+}})
  %7 = call i32 @__read_pipe_2_bl(%opencl.pipe_t addrspace(1)* %4, i8 addrspace(4)* %6, i32 4, i32 4)
  %8 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)** %p.addr, align 4
  %9 = load i32 addrspace(4)*, i32 addrspace(4)** %ptr_p, align 4
  %10 = bitcast i32 addrspace(4)* %9 to i8 addrspace(4)*
; CHECK: %{{[0-9]+}} = call i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %{{[0-9]+}}, i8 addrspace(4)* %{{[0-9]+}})
  %11 = call i32 @__read_pipe_2_bl(%opencl.pipe_t addrspace(1)* %8, i8 addrspace(4)* %10, i32 4, i32 4)
  ret void
}

declare i32 @__read_pipe_2_bl(%opencl.pipe_t addrspace(1)*, i8 addrspace(4)*, i32, i32)

; Function Attrs: convergent noinline nounwind optnone
define spir_func void @test2(%opencl.pipe_t addrspace(1)* %p, i32 addrspace(1)* %ptr) #0 {
entry:
  %p.addr = alloca %opencl.pipe_t addrspace(1)*, align 4
  %ptr.addr = alloca i32 addrspace(1)*, align 4
  %ptr_l = alloca i32 addrspace(3)*, align 4
  %ptr_p = alloca i32 addrspace(4)*, align 4
  %ptr_c = alloca i32 addrspace(2)*, align 4
  store %opencl.pipe_t addrspace(1)* %p, %opencl.pipe_t addrspace(1)** %p.addr, align 4
  store i32 addrspace(1)* %ptr, i32 addrspace(1)** %ptr.addr, align 4
  %0 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)** %p.addr, align 4
  %1 = load i32 addrspace(1)*, i32 addrspace(1)** %ptr.addr, align 4
  %2 = addrspacecast i32 addrspace(1)* %1 to i8 addrspace(4)*
; CHECK: %{{[0-9]+}} = call i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %{{[0-9]+}}, i8 addrspace(4)* %{{[0-9]+}})
  %3 = call i32 @__write_pipe_2_bl(%opencl.pipe_t addrspace(1)* %0, i8 addrspace(4)* %2, i32 4, i32 4)
  %4 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)** %p.addr, align 4
  %5 = load i32 addrspace(3)*, i32 addrspace(3)** %ptr_l, align 4
  %6 = addrspacecast i32 addrspace(3)* %5 to i8 addrspace(4)*
; CHECK: %{{[0-9]+}} = call i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %{{[0-9]+}}, i8 addrspace(4)* %{{[0-9]+}})
  %7 = call i32 @__write_pipe_2_bl(%opencl.pipe_t addrspace(1)* %4, i8 addrspace(4)* %6, i32 4, i32 4)
  %8 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)** %p.addr, align 4
  %9 = load i32 addrspace(4)*, i32 addrspace(4)** %ptr_p, align 4
  %10 = bitcast i32 addrspace(4)* %9 to i8 addrspace(4)*
; CHECK: %{{[0-9]+}} = call i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %{{[0-9]+}}, i8 addrspace(4)* %{{[0-9]+}})
  %11 = call i32 @__write_pipe_2_bl(%opencl.pipe_t addrspace(1)* %8, i8 addrspace(4)* %10, i32 4, i32 4)
  %12 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)** %p.addr, align 4
  %13 = load i32 addrspace(2)*, i32 addrspace(2)** %ptr_c, align 4
  %14 = addrspacecast i32 addrspace(2)* %13 to i8 addrspace(4)*
; CHECK: %{{[0-9]+}} = call i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %{{[0-9]+}}, i8 addrspace(4)* %{{[0-9]+}})
  %15 = call i32 @__write_pipe_2_bl(%opencl.pipe_t addrspace(1)* %12, i8 addrspace(4)* %14, i32 4, i32 4)
  ret void
}

declare i32 @__write_pipe_2_bl(%opencl.pipe_t addrspace(1)*, i8 addrspace(4)*, i32, i32)

; CHECK: declare i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)*
; CHECK: declare i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)*

attributes #0 = { convergent noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 2, i32 0}
!2 = !{}
!3 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 470e50aede6d92f4b569e38fc7b2e031a0b82180) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm fcec820f4b80de4ed49d51d31292f09593b932e6)"}
