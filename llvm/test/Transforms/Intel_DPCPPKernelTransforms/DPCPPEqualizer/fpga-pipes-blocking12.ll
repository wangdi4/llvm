; Compiled from:
; ----------------------------------------------------
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
; ----------------------------------------------------
; Compile options:
;   -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -x cl -cl-std=CL1.2 -disable-llvm-passes -finclude-default-header
; ----------------------------------------------------

; RUN: llvm-as %p/../Inputs/fpga-pipes.rtl -o %t.rtl.bc
; RUN: opt -dpcpp-kernel-builtin-lib=%t.rtl.bc -dpcpp-demangle-fpga-pipes -passes=dpcpp-kernel-equalizer -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-builtin-lib=%t.rtl.bc -dpcpp-demangle-fpga-pipes -passes=dpcpp-kernel-equalizer -S %s | FileCheck %s

; CHECK-LABEL: define void @test1
; CHECK: %[[GL2GEN:[0-9]+]] = addrspacecast i8 addrspace(1)* %{{[0-9]+}} to i8 addrspace(4)*
; CHECK-NOT: call i32 @__read_pipe_2_bl_AS1
; CHECK: call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t {{.*}}, i8 addrspace(4)* %[[GL2GEN]], i32 4, i32 4)
; CHECK: %[[LO2GEN:[0-9]+]] = addrspacecast i8 addrspace(3)* %{{[0-9]+}} to i8 addrspace(4)*
; CHECK-NOT: call i32 @__read_pipe_2_bl_AS3
; CHECK: call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t {{.*}}, i8 addrspace(4)* %[[LO2GEN]], i32 4, i32 4)
; CHECK: %[[PR2GEN:[0-9]+]] = addrspacecast i8* %{{[0-9]+}} to i8 addrspace(4)*
; CHECK-NOT: call i32 @__read_pipe_2_bl_AS0
; CHECK: call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t {{.*}}, i8 addrspace(4)* %[[PR2GEN]], i32 4, i32 4)
; CHECK-NOT: declare i32 @__read_pipe_2_bl_AS1(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(1)*, i32, i32)
; CHECK-NOT: declare i32 @__read_pipe_2_bl_AS3(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(3)*, i32, i32)
; CHECK-NOT: declare i32 @__read_pipe_2_bl_AS0(%opencl.pipe_ro_t addrspace(1)*, i8*, i32, i32)
;
; CHECK-LABEL: define void @test2
; CHECK: %[[GL2GEN:[0-9]+]] = addrspacecast i8 addrspace(1)* %{{[0-9]+}} to i8 addrspace(4)*
; CHECK-NOT: call i32 @__write_pipe_2_bl_AS1
; CHECK: call i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t {{.*}}, i8 addrspace(4)* %[[GL2GEN]], i32 4, i32 4)
; CHECK: %[[LO2GEN:[0-9]+]] = addrspacecast i8 addrspace(3)* %{{[0-9]+}} to i8 addrspace(4)*
; CHECK-NOT: call i32 @__write_pipe_2_bl_AS3
; CHECK: call i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t {{.*}}, i8 addrspace(4)* %[[LO2GEN]], i32 4, i32 4)
; CHECK: %[[PR2GEN:[0-9]+]] = addrspacecast i8* %{{[0-9]+}} to i8 addrspace(4)*
; CHECK-NOT: call i32 @__write_pipe_2_bl_AS0
; CHECK: call i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t {{.*}}, i8 addrspace(4)* %[[PR2GEN]], i32 4, i32 4)
; CHECK: %[[CO2GEN:[0-9]+]] = addrspacecast i8 addrspace(2)* %{{[0-9]+}} to i8 addrspace(4)*
; CHECK-NOT: call i32 @__write_pipe_2_bl_AS2
; CHECK: call i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t {{.*}}, i8 addrspace(4)* %[[CO2GEN]], i32 4, i32 4)
; CHECK-NOT: declare i32 @__write_pipe_2_bl_AS2(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(2)*, i32, i32)
; CHECK-NOT: declare i32 @__write_pipe_2_bl_AS3(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(3)*, i32, i32)
; CHECK-NOT: declare i32 @__write_pipe_2_bl_AS0(%opencl.pipe_wo_t addrspace(1)*, i8*, i32, i32)
; CHECK-NOT: declare i32 @__write_pipe_2_bl_AS2(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(2)*, i32, i32)
;
; CHECK: declare i32 @__read_pipe_2_bl_fpga
; CHECK: declare i32 @__write_pipe_2_bl_fpga

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
  %ptr_p = alloca i32*, align 8
  store %opencl.pipe_ro_t addrspace(1)* %p, %opencl.pipe_ro_t addrspace(1)** %p.addr, align 8, !tbaa !4
  store i32 addrspace(1)* %ptr, i32 addrspace(1)** %ptr.addr, align 8, !tbaa !7
  %0 = load %opencl.pipe_ro_t addrspace(1)*, %opencl.pipe_ro_t addrspace(1)** %p.addr, align 8, !tbaa !4
  %1 = load i32 addrspace(1)*, i32 addrspace(1)** %ptr.addr, align 8, !tbaa !7
  %2 = bitcast i32 addrspace(1)* %1 to i8 addrspace(1)*
  %3 = call i32 @__read_pipe_2_bl_AS1(%opencl.pipe_ro_t addrspace(1)* %0, i8 addrspace(1)* %2, i32 4, i32 4)
  %4 = bitcast i32 addrspace(3)** %ptr_l to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %4) #2
  %5 = load %opencl.pipe_ro_t addrspace(1)*, %opencl.pipe_ro_t addrspace(1)** %p.addr, align 8, !tbaa !4
  %6 = load i32 addrspace(3)*, i32 addrspace(3)** %ptr_l, align 8, !tbaa !7
  %7 = bitcast i32 addrspace(3)* %6 to i8 addrspace(3)*
  %8 = call i32 @__read_pipe_2_bl_AS3(%opencl.pipe_ro_t addrspace(1)* %5, i8 addrspace(3)* %7, i32 4, i32 4)
  %9 = bitcast i32** %ptr_p to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %9) #2
  %10 = load %opencl.pipe_ro_t addrspace(1)*, %opencl.pipe_ro_t addrspace(1)** %p.addr, align 8, !tbaa !4
  %11 = load i32*, i32** %ptr_p, align 8, !tbaa !7
  %12 = bitcast i32* %11 to i8*
  %13 = call i32 @__read_pipe_2_bl_AS0(%opencl.pipe_ro_t addrspace(1)* %10, i8* %12, i32 4, i32 4)
  %14 = bitcast i32** %ptr_p to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %14) #2
  %15 = bitcast i32 addrspace(3)** %ptr_l to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %15) #2
  ret void
}

declare i32 @__read_pipe_2_bl_AS1(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(1)*, i32, i32)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

declare i32 @__read_pipe_2_bl_AS3(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(3)*, i32, i32)

declare i32 @__read_pipe_2_bl_AS0(%opencl.pipe_ro_t addrspace(1)*, i8*, i32, i32)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: convergent nounwind
define spir_func void @test2(%opencl.pipe_wo_t addrspace(1)* %p, i32 addrspace(1)* %ptr) #0 {
entry:
  %p.addr = alloca %opencl.pipe_wo_t addrspace(1)*, align 8
  %ptr.addr = alloca i32 addrspace(1)*, align 8
  %ptr_l = alloca i32 addrspace(3)*, align 8
  %ptr_p = alloca i32*, align 8
  %ptr_c = alloca i32 addrspace(2)*, align 8
  store %opencl.pipe_wo_t addrspace(1)* %p, %opencl.pipe_wo_t addrspace(1)** %p.addr, align 8, !tbaa !4
  store i32 addrspace(1)* %ptr, i32 addrspace(1)** %ptr.addr, align 8, !tbaa !7
  %0 = load %opencl.pipe_wo_t addrspace(1)*, %opencl.pipe_wo_t addrspace(1)** %p.addr, align 8, !tbaa !4
  %1 = load i32 addrspace(1)*, i32 addrspace(1)** %ptr.addr, align 8, !tbaa !7
  %2 = bitcast i32 addrspace(1)* %1 to i8 addrspace(1)*
  %3 = call i32 @__write_pipe_2_bl_AS1(%opencl.pipe_wo_t addrspace(1)* %0, i8 addrspace(1)* %2, i32 4, i32 4)
  %4 = bitcast i32 addrspace(3)** %ptr_l to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %4) #2
  %5 = load %opencl.pipe_wo_t addrspace(1)*, %opencl.pipe_wo_t addrspace(1)** %p.addr, align 8, !tbaa !4
  %6 = load i32 addrspace(3)*, i32 addrspace(3)** %ptr_l, align 8, !tbaa !7
  %7 = bitcast i32 addrspace(3)* %6 to i8 addrspace(3)*
  %8 = call i32 @__write_pipe_2_bl_AS3(%opencl.pipe_wo_t addrspace(1)* %5, i8 addrspace(3)* %7, i32 4, i32 4)
  %9 = bitcast i32** %ptr_p to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %9) #2
  %10 = load %opencl.pipe_wo_t addrspace(1)*, %opencl.pipe_wo_t addrspace(1)** %p.addr, align 8, !tbaa !4
  %11 = load i32*, i32** %ptr_p, align 8, !tbaa !7
  %12 = bitcast i32* %11 to i8*
  %13 = call i32 @__write_pipe_2_bl_AS0(%opencl.pipe_wo_t addrspace(1)* %10, i8* %12, i32 4, i32 4)
  %14 = bitcast i32 addrspace(2)** %ptr_c to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %14) #2
  %15 = load %opencl.pipe_wo_t addrspace(1)*, %opencl.pipe_wo_t addrspace(1)** %p.addr, align 8, !tbaa !4
  %16 = load i32 addrspace(2)*, i32 addrspace(2)** %ptr_c, align 8, !tbaa !7
  %17 = bitcast i32 addrspace(2)* %16 to i8 addrspace(2)*
  %18 = call i32 @__write_pipe_2_bl_AS2(%opencl.pipe_wo_t addrspace(1)* %15, i8 addrspace(2)* %17, i32 4, i32 4)
  %19 = bitcast i32 addrspace(2)** %ptr_c to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %19) #2
  %20 = bitcast i32** %ptr_p to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %20) #2
  %21 = bitcast i32 addrspace(3)** %ptr_l to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %21) #2
  ret void
}

declare i32 @__write_pipe_2_bl_AS1(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(1)*, i32, i32)

declare i32 @__write_pipe_2_bl_AS3(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(3)*, i32, i32)

declare i32 @__write_pipe_2_bl_AS0(%opencl.pipe_wo_t addrspace(1)*, i8*, i32, i32)

declare i32 @__write_pipe_2_bl_AS2(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(2)*, i32, i32)

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
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"clang version 7.0.0 "}
!4 = !{!5, !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"any pointer", !5, i64 0}

; DEBUGIFY-NOT: WARNING
