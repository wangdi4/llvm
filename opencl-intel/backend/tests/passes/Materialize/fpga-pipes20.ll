; Compiled from:
; ----------------------------------------------------
; void test1(read_only pipe int p, global int *ptr) {
;   read_pipe(p, ptr);
;   local int *ptr_l;
;   read_pipe(p, ptr_l);
;   int *ptr_p;
;   read_pipe(p, ptr_p);
; }
;
; void test2(write_only pipe int p, global int *ptr) {
;   write_pipe(p, ptr);
;   local int *ptr_l;
;   write_pipe(p, ptr_l);
;   int *ptr_p;
;   write_pipe(p, ptr_p);
;   constant int *ptr_c;
;   write_pipe(p, ptr_c);
; }
; ----------------------------------------------------
; Compile options:
;   -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -x cl -cl-std=CL2.0 -disable-llvm-passes -finclude-default-header
; ----------------------------------------------------
; REQUIRES: fpga-emulator
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -demangle-fpga-pipes -spir-materializer -verify -S %s -o %t.1
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -spir-materializer -verify -S %s -o %t.2
; RUN: diff %t.1 %t.2

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.pipe_ro_t = type opaque
%opencl.pipe_wo_t = type opaque

; Function Attrs: convergent nounwind
define spir_func void @test1(%opencl.pipe_ro_t addrspace(1)* %p, i32 addrspace(1)* %ptr) #0 {
entry:
  %p.addr = alloca %opencl.pipe_ro_t addrspace(1)*, align 8
  %ptr.addr = alloca i32 addrspace(1)*, align 8
  %ptr_l = alloca i32 addrspace(3)*, align 8
  %ptr_p = alloca i32 addrspace(4)*, align 8
  store %opencl.pipe_ro_t addrspace(1)* %p, %opencl.pipe_ro_t addrspace(1)** %p.addr, align 8, !tbaa !4
  store i32 addrspace(1)* %ptr, i32 addrspace(1)** %ptr.addr, align 8, !tbaa !7
  %0 = load %opencl.pipe_ro_t addrspace(1)*, %opencl.pipe_ro_t addrspace(1)** %p.addr, align 8, !tbaa !4
  %1 = load i32 addrspace(1)*, i32 addrspace(1)** %ptr.addr, align 8, !tbaa !7
  %2 = addrspacecast i32 addrspace(1)* %1 to i8 addrspace(4)*
  %3 = call i32 @__read_pipe_2(%opencl.pipe_ro_t addrspace(1)* %0, i8 addrspace(4)* %2, i32 4, i32 4)
  %4 = bitcast i32 addrspace(3)** %ptr_l to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %4) #2
  %5 = load %opencl.pipe_ro_t addrspace(1)*, %opencl.pipe_ro_t addrspace(1)** %p.addr, align 8, !tbaa !4
  %6 = load i32 addrspace(3)*, i32 addrspace(3)** %ptr_l, align 8, !tbaa !7
  %7 = addrspacecast i32 addrspace(3)* %6 to i8 addrspace(4)*
  %8 = call i32 @__read_pipe_2(%opencl.pipe_ro_t addrspace(1)* %5, i8 addrspace(4)* %7, i32 4, i32 4)
  %9 = bitcast i32 addrspace(4)** %ptr_p to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %9) #2
  %10 = load %opencl.pipe_ro_t addrspace(1)*, %opencl.pipe_ro_t addrspace(1)** %p.addr, align 8, !tbaa !4
  %11 = load i32 addrspace(4)*, i32 addrspace(4)** %ptr_p, align 8, !tbaa !7
  %12 = bitcast i32 addrspace(4)* %11 to i8 addrspace(4)*
  %13 = call i32 @__read_pipe_2(%opencl.pipe_ro_t addrspace(1)* %10, i8 addrspace(4)* %12, i32 4, i32 4)
  %14 = bitcast i32 addrspace(4)** %ptr_p to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %14) #2
  %15 = bitcast i32 addrspace(3)** %ptr_l to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %15) #2
  ret void
}

declare i32 @__read_pipe_2(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(4)*, i32, i32)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: convergent nounwind
define spir_func void @test2(%opencl.pipe_wo_t addrspace(1)* %p, i32 addrspace(1)* %ptr) #0 {
entry:
  %p.addr = alloca %opencl.pipe_wo_t addrspace(1)*, align 8
  %ptr.addr = alloca i32 addrspace(1)*, align 8
  %ptr_l = alloca i32 addrspace(3)*, align 8
  %ptr_p = alloca i32 addrspace(4)*, align 8
  %ptr_c = alloca i32 addrspace(2)*, align 8
  store %opencl.pipe_wo_t addrspace(1)* %p, %opencl.pipe_wo_t addrspace(1)** %p.addr, align 8, !tbaa !4
  store i32 addrspace(1)* %ptr, i32 addrspace(1)** %ptr.addr, align 8, !tbaa !7
  %0 = load %opencl.pipe_wo_t addrspace(1)*, %opencl.pipe_wo_t addrspace(1)** %p.addr, align 8, !tbaa !4
  %1 = load i32 addrspace(1)*, i32 addrspace(1)** %ptr.addr, align 8, !tbaa !7
  %2 = addrspacecast i32 addrspace(1)* %1 to i8 addrspace(4)*
  %3 = call i32 @__write_pipe_2(%opencl.pipe_wo_t addrspace(1)* %0, i8 addrspace(4)* %2, i32 4, i32 4)
  %4 = bitcast i32 addrspace(3)** %ptr_l to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %4) #2
  %5 = load %opencl.pipe_wo_t addrspace(1)*, %opencl.pipe_wo_t addrspace(1)** %p.addr, align 8, !tbaa !4
  %6 = load i32 addrspace(3)*, i32 addrspace(3)** %ptr_l, align 8, !tbaa !7
  %7 = addrspacecast i32 addrspace(3)* %6 to i8 addrspace(4)*
  %8 = call i32 @__write_pipe_2(%opencl.pipe_wo_t addrspace(1)* %5, i8 addrspace(4)* %7, i32 4, i32 4)
  %9 = bitcast i32 addrspace(4)** %ptr_p to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %9) #2
  %10 = load %opencl.pipe_wo_t addrspace(1)*, %opencl.pipe_wo_t addrspace(1)** %p.addr, align 8, !tbaa !4
  %11 = load i32 addrspace(4)*, i32 addrspace(4)** %ptr_p, align 8, !tbaa !7
  %12 = bitcast i32 addrspace(4)* %11 to i8 addrspace(4)*
  %13 = call i32 @__write_pipe_2(%opencl.pipe_wo_t addrspace(1)* %10, i8 addrspace(4)* %12, i32 4, i32 4)
  %14 = bitcast i32 addrspace(2)** %ptr_c to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %14) #2
  %15 = load %opencl.pipe_wo_t addrspace(1)*, %opencl.pipe_wo_t addrspace(1)** %p.addr, align 8, !tbaa !4
  %16 = load i32 addrspace(2)*, i32 addrspace(2)** %ptr_c, align 8, !tbaa !7
  %17 = addrspacecast i32 addrspace(2)* %16 to i8 addrspace(4)*
  %18 = call i32 @__write_pipe_2(%opencl.pipe_wo_t addrspace(1)* %15, i8 addrspace(4)* %17, i32 4, i32 4)
  %19 = bitcast i32 addrspace(2)** %ptr_c to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %19) #2
  %20 = bitcast i32 addrspace(4)** %ptr_p to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %20) #2
  %21 = bitcast i32 addrspace(3)** %ptr_l to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %21) #2
  ret void
}

declare i32 @__write_pipe_2(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(4)*, i32, i32)

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

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
!3 = !{!"clang version 7.0.0 "}
!4 = !{!5, !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"any pointer", !5, i64 0}
