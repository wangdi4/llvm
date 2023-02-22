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

; RUN: opt -opaque-pointers=0 -passes="transform-fpga-reg" -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

%struct._ZTS2st.st = type { i32, float }
%"class._ZTSZ4mainE3$_0.anon" = type { i8 }

@.str = private unnamed_addr constant [25 x i8] c"__builtin_intel_fpga_reg\00", section "llvm.metadata"
@.str.1 = private unnamed_addr constant [37 x i8] c"../../../tests/fpga_reg/fpga_reg.cpp\00", section "llvm.metadata"
@__const._Z3foov.i = private unnamed_addr constant %struct._ZTS2st.st { i32 1, float 5.000000e+00 }, align 4

; Function Attrs: noinline nounwind optnone
define spir_kernel void @_ZTSZ4mainE11fake_kernel() #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 {
entry:
  %0 = alloca %"class._ZTSZ4mainE3$_0.anon", align 1
  call spir_func void @"_ZZ4mainENK3$_0clEv"(%"class._ZTSZ4mainE3$_0.anon"* %0)
  ret void
}

; Function Attrs: noinline nounwind optnone
define internal spir_func void @"_ZZ4mainENK3$_0clEv"(%"class._ZTSZ4mainE3$_0.anon"* %this) #1 align 2 {
entry:
  %this.addr = alloca %"class._ZTSZ4mainE3$_0.anon"*, align 4
  store %"class._ZTSZ4mainE3$_0.anon"* %this, %"class._ZTSZ4mainE3$_0.anon"** %this.addr, align 4
  %this1 = load %"class._ZTSZ4mainE3$_0.anon"*, %"class._ZTSZ4mainE3$_0.anon"** %this.addr, align 4
  call spir_func void @_Z3foov()
  ret void
}

; Function Attrs: noinline nounwind optnone
define spir_func void @_Z3foov() #1 {
entry:
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  %c = alloca i32, align 4
  %d = alloca float, align 4
  %i = alloca %struct._ZTS2st.st, align 4
  %ii = alloca %struct._ZTS2st.st, align 4
  %agg-temp = alloca %struct._ZTS2st.st, align 4
  store i32 123, i32* %a, align 4
  %0 = load i32, i32* %a, align 4
  ; CHECK: %1 = call i32 @llvm.fpga.reg.i32(i32 %0)
  %1 = call i32 @llvm.annotation.i32(i32 %0, i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([37 x i8], [37 x i8]* @.str.1, i32 0, i32 0), i32 9)
  store i32 %1, i32* %b, align 4
  %2 = load i32, i32* %a, align 4
  ; CHECK: %3 = call i32 @llvm.fpga.reg.i32(i32 %2)
  %3 = call i32 @llvm.annotation.i32(i32 %2, i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([37 x i8], [37 x i8]* @.str.1, i32 0, i32 0), i32 10)
  ; CHECK: %4 = call i32 @llvm.fpga.reg.i32(i32 %3)
  %4 = call i32 @llvm.annotation.i32(i32 %3, i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([37 x i8], [37 x i8]* @.str.1, i32 0, i32 0), i32 10)
  store i32 %4, i32* %c, align 4
  ; CHECK: %5 = call i32 @llvm.fpga.reg.i32(i32 1074580685)
  %5 = call i32 @llvm.annotation.i32(i32 1074580685, i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([37 x i8], [37 x i8]* @.str.1, i32 0, i32 0), i32 11)
  %6 = bitcast i32 %5 to float
  store float %6, float* %d, align 4
  %7 = bitcast %struct._ZTS2st.st* %i to i8*
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* align 4 %7, i8* align 4 bitcast (%struct._ZTS2st.st* @__const._Z3foov.i to i8*), i32 8, i1 false)
  %8 = bitcast %struct._ZTS2st.st* %agg-temp to i8*
  %9 = bitcast %struct._ZTS2st.st* %i to i8*
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* align 4 %8, i8* align 4 %9, i32 8, i1 false)
  %10 = bitcast %struct._ZTS2st.st* %agg-temp to i8*
  ; CHECK: %11 = call i8* @llvm.fpga.reg.p0i8(i8* %10)
  %11 = call i8* @llvm.ptr.annotation.p0i8(i8* %10, i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([37 x i8], [37 x i8]* @.str.1, i32 0, i32 0), i32 14, i8* null)
  %12 = bitcast i8* %11 to %struct._ZTS2st.st*
  %13 = bitcast %struct._ZTS2st.st* %ii to i8*
  %14 = bitcast %struct._ZTS2st.st* %12 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %13, i8* align 4 %14, i64 4, i1 false)
  ret void
}

; CHECK-DAG: declare i32 @llvm.fpga.reg.i32(i32)
; CHECK-DAG: declare i8* @llvm.fpga.reg.p0i8(i8*)

; Function Attrs: nounwind
declare i32 @llvm.annotation.i32(i32, i8*, i8*, i32) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i32(i8* nocapture writeonly, i8* nocapture readonly, i32, i1 immarg) #3

; Function Attrs: nounwind
declare i8* @llvm.ptr.annotation.p0i8(i8*, i8*, i8*, i32, i8*) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1 immarg) #3

attributes #0 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { argmemonly nounwind }

!llvm.module.flags = !{!0}
!opencl.spir.version = !{!1}
!spirv.Source = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{i32 4, i32 100000}
!3 = !{!"clang version 9.0.0 (https://github.com/otcshare/llvm.git 32e3dfdd4c74062ed42db73ba8e43810078bfec9)"}
!4 = !{}
