; SYCL source code of the program can be found below:
;
; struct st {
;   int a;
;   float b;
; };
;
; void foo() {
; int a = 123;
; int b = __builtin_intel_fpga_reg(a);
; int c = __builtin_intel_fpga_reg(__builtin_intel_fpga_reg(a));
; float d = __builtin_intel_fpga_reg(2.2f);
;
; struct st i = {1, 5.0f};
; struct st ii = __builtin_intel_fpga_reg(i);
; }
;
; template <typename name, typename Func>
; __attribute__((sycl_kernel)) void kernel_single_task(Func kernelFunc) {
;   kernelFunc();
; }
;
; int main() {
;   kernel_single_task<class fake_kernel>([]() { foo(); });
;   return 0;
; }

; Compile options: clang -cc1 -O0 -fsycl-is-device -triple spir-unknown-unknown-intelfpga test.cpp -emit-llvm -o test.ll

; RUN: opt -passes="transform-fpga-reg" -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

%struct.st = type { i32, float }
%class.anon = type { i8 }

@.str = private unnamed_addr addrspace(1) constant [25 x i8] c"__builtin_intel_fpga_reg\00", section "llvm.metadata"
@.str.1 = private unnamed_addr addrspace(1) constant [9 x i8] c"test.cpp\00", section "llvm.metadata"
@__const._Z3foov.i = private unnamed_addr addrspace(1) constant %struct.st { i32 1, float 5.000000e+00 }, align 4

; Function Attrs: convergent mustprogress noinline norecurse nounwind optnone
define dso_local spir_kernel void @_ZTSZ4mainE11fake_kernel() #0 !srcloc !5 !kernel_arg_buffer_location !3 {
entry:
  %this.addr.i = alloca ptr addrspace(4), align 4
  %__SYCLKernel = alloca %class.anon, align 1
  %__SYCLKernel.ascast = addrspacecast ptr %__SYCLKernel to ptr addrspace(4)
  %this.addr.ascast.i = addrspacecast ptr %this.addr.i to ptr addrspace(4)
  store ptr addrspace(4) %__SYCLKernel.ascast, ptr addrspace(4) %this.addr.ascast.i, align 4
  %this1.i = load ptr addrspace(4), ptr addrspace(4) %this.addr.ascast.i, align 4
  call spir_func void @_Z3foov() #4
  ret void
}

; Function Attrs: convergent mustprogress noinline norecurse nounwind optnone
define dso_local spir_func void @_Z3foov() #1 !srcloc !6 {
entry:
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  %c = alloca i32, align 4
  %d = alloca float, align 4
  %i = alloca %struct.st, align 4
  %ii = alloca %struct.st, align 4
  %a.ascast = addrspacecast ptr %a to ptr addrspace(4)
  %b.ascast = addrspacecast ptr %b to ptr addrspace(4)
  %c.ascast = addrspacecast ptr %c to ptr addrspace(4)
  %d.ascast = addrspacecast ptr %d to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %ii.ascast = addrspacecast ptr %ii to ptr addrspace(4)
  store i32 123, ptr addrspace(4) %a.ascast, align 4
  %0 = load i32, ptr addrspace(4) %a.ascast, align 4
  ; CHECK: %1 = call i32 @llvm.fpga.reg.i32(i32 %0)
  %1 = call i32 @llvm.annotation.i32.p1(i32 %0, ptr addrspace(1) @.str, ptr addrspace(1) @.str.1, i32 8)
  store i32 %1, ptr addrspace(4) %b.ascast, align 4
  %2 = load i32, ptr addrspace(4) %a.ascast, align 4
  ; CHECK: %3 = call i32 @llvm.fpga.reg.i32(i32 %2)
  %3 = call i32 @llvm.annotation.i32.p1(i32 %2, ptr addrspace(1) @.str, ptr addrspace(1) @.str.1, i32 9)
  ; CHECK: %4 = call i32 @llvm.fpga.reg.i32(i32 %3)
  %4 = call i32 @llvm.annotation.i32.p1(i32 %3, ptr addrspace(1) @.str, ptr addrspace(1) @.str.1, i32 9)
  store i32 %4, ptr addrspace(4) %c.ascast, align 4
  ; CHECK: %5 = call i32 @llvm.fpga.reg.i32(i32 1074580685)
  %5 = call i32 @llvm.annotation.i32.p1(i32 1074580685, ptr addrspace(1) @.str, ptr addrspace(1) @.str.1, i32 10)
  %6 = bitcast i32 %5 to float
  store float %6, ptr addrspace(4) %d.ascast, align 4
  call void @llvm.memcpy.p4.p1.i32(ptr addrspace(4) align 4 %i.ascast, ptr addrspace(1) align 4 @__const._Z3foov.i, i32 8, i1 false)
  call void @llvm.memcpy.p4.p4.i32(ptr addrspace(4) align 4 %ii.ascast, ptr addrspace(4) align 4 %i.ascast, i32 8, i1 false)
  ; CHECK: %7 = call ptr addrspace(4) @llvm.fpga.reg.p4(ptr addrspace(4) %ii.ascast)
  %7 = call ptr addrspace(4) @llvm.ptr.annotation.p4.p1(ptr addrspace(4) %ii.ascast, ptr addrspace(1) @.str, ptr addrspace(1) @.str.1, i32 13, ptr addrspace(1) null)
  ret void
}

; CHECK-DAG: declare i32 @llvm.fpga.reg.i32(i32)
; CHECK-DAG: declare ptr addrspace(4) @llvm.fpga.reg.p4(ptr addrspace(4))

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare i32 @llvm.annotation.i32.p1(i32, ptr addrspace(1), ptr addrspace(1), i32) #2

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p4.p1.i32(ptr addrspace(4) noalias nocapture writeonly, ptr addrspace(1) noalias nocapture readonly, i32, i1 immarg) #3

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p4.p4.i32(ptr addrspace(4) noalias nocapture writeonly, ptr addrspace(4) noalias nocapture readonly, i32, i1 immarg) #3

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare ptr addrspace(4) @llvm.ptr.annotation.p4.p1(ptr addrspace(4), ptr addrspace(1), ptr addrspace(1), i32, ptr addrspace(1)) #2

declare dso_local spir_func i32 @_Z18__spirv_ocl_printfPU3AS2Kcz(ptr addrspace(2), ...)

attributes #0 = { convergent mustprogress noinline norecurse nounwind optnone "no-trapping-math"="true" "stack-protector-buffer-size"="8" "sycl-module-id"="test.cpp" "sycl-optlevel"="0" "uniform-work-group-size"="true" }
attributes #1 = { convergent mustprogress noinline norecurse nounwind optnone "no-trapping-math"="true" "stack-protector-buffer-size"="8" "sycl-optlevel"="0" }
attributes #2 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }
attributes #3 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #4 = { convergent nounwind }

!llvm.module.flags = !{!0}
!opencl.spir.version = !{!1}
!spirv.Source = !{!2}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{i32 4, i32 100000}
!3 = !{}
!4 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!5 = !{i32 463}
!6 = !{i32 43}
