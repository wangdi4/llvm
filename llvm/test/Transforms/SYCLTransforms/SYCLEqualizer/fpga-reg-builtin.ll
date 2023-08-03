; This IR was generated from the following source code:
;
; struct st {
;   int a;
;   float b;
; };
;
; union un {
;   int a;
;   char c[4];
; };
;
; typedef int myInt;
;
; void foo() {
;   int a=123;
;   myInt myA = 321;
;   int b = __builtin_fpga_reg(a);
;   int myB = __builtin_fpga_reg(myA);
;   int c = __builtin_fpga_reg(2.0f);
;   int d = __builtin_fpga_reg( __builtin_fpga_reg( b+12 ));
;   int e = __builtin_fpga_reg( __builtin_fpga_reg( a+b ));
;   int f;
;   f = __builtin_fpga_reg(a);
;   struct st i = {1, 5.0f};
;   struct st ii = __builtin_fpga_reg(i);
;   struct st iii;
;   iii = __builtin_fpga_reg(ii);
;   struct st *iiii = __builtin_fpga_reg(&iii);
;   union un u1 = {1};
;   union un u2, *u3;
;   u2 = __builtin_fpga_reg(u1);
;   u3 = __builtin_fpga_reg(&u2);
;   int *ap = &a;
;   int *bp = __builtin_fpga_reg(ap);
; }
;
; Compiled by the following command: clang -cc1 -triple spir-unknown-unknown-intelfpga %s -emit-llvm -o -

; RUN: llvm-as %p/../Inputs/fpga-pipes.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -sycl-remove-fpga-reg -passes=sycl-kernel-equalizer -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -sycl-remove-fpga-reg -passes=sycl-kernel-equalizer -S %s | FileCheck %s

%struct.st = type { i32, float }
%union.un = type { i32 }

@foo.i = private unnamed_addr addrspace(2) constant %struct.st { i32 1, float 5.000000e+00 }, align 4
@foo.u1 = private unnamed_addr addrspace(2) constant %union.un { i32 1 }, align 4

; Function Attrs: convergent noinline nounwind optnone
define spir_func void @foo() #0 {
entry:
; CHECK: define void @foo
; CHECK: entry:
  %a = alloca i32, align 4
  %myA = alloca i32, align 4
  %b = alloca i32, align 4
  %myB = alloca i32, align 4
  %c = alloca i32, align 4
  %d = alloca i32, align 4
  %e = alloca i32, align 4
  %f = alloca i32, align 4
  %i = alloca %struct.st, align 4
  %ii = alloca %struct.st, align 4
  %agg-temp = alloca %struct.st, align 4
  %iii = alloca %struct.st, align 4
  %tmp = alloca %struct.st, align 4
  %agg-temp2 = alloca %struct.st, align 4
  %iiii = alloca ptr, align 4
  %u1 = alloca %union.un, align 4
  %u2 = alloca %union.un, align 4
  %u3 = alloca ptr, align 4
  %tmp3 = alloca %union.un, align 4
  %agg-temp4 = alloca %union.un, align 4
  %ap = alloca ptr, align 4
  %bp = alloca ptr, align 4
  store i32 123, ptr %a, align 4
  store i32 321, ptr %myA, align 4
  %0 = load i32, ptr %a, align 4
; CHECK: %[[LOADA1:[0-9]+]] = load i32, ptr %a, align 4
  %1 = call i32 @llvm.fpga.reg.i32(i32 %0)
; CHECK-NOT: %{{[0-9]+}} = call i32 @llvm.fpga.reg.i32
  store i32 %1, ptr %b, align 4
; CHECK: store i32 %[[LOADA1]], ptr %b, align 4
  %2 = load i32, ptr %myA, align 4
; CHECK: %[[LOADMYA:[0-9]+]] = load i32, ptr %myA, align 4
  %3 = call i32 @llvm.fpga.reg.i32(i32 %2)
; CHECK-NOT: %{{[0-9]+}} = call i32 @llvm.fpga.reg.i32
  store i32 %3, ptr %myB, align 4
; CHECK: store i32 %[[LOADMYA]], ptr %myB, align 4
  %4 = call float @llvm.fpga.reg.f32(float 2.000000e+00)
; CHECK-NOT: %{{[0-9]+}} = call float @llvm.fpga.reg.f32(float 2.000000e+00)
  %conv = fptosi float %4 to i32
; CHECK: %conv = fptosi float 2.000000e+00 to i32
  store i32 %conv, ptr %c, align 4
  %5 = load i32, ptr %b, align 4
  %add = add nsw i32 %5, 12
  %6 = call i32 @llvm.fpga.reg.i32(i32 %add)
; CHECK-NOT: %[[RET1:[0-9]+]] = call i32 @llvm.fpga.reg.i32(i32 %add)
  %7 = call i32 @llvm.fpga.reg.i32(i32 %6)
; CHECK-NOT: %{{[0-9]+}} = call i32 @llvm.fpga.reg.i32
  store i32 %7, ptr %d, align 4
; CHECK: store i32 %add, ptr %d, align 4
  %8 = load i32, ptr %a, align 4
  %9 = load i32, ptr %b, align 4
  %add1 = add nsw i32 %8, %9
  %10 = call i32 @llvm.fpga.reg.i32(i32 %add1)
; CHECK-NOT: %[[RET2:[0-9]+]] = call i32 @llvm.fpga.reg.i32(i32 %add1)
  %11 = call i32 @llvm.fpga.reg.i32(i32 %10)
; CHECK-NOT: %{{[0-9]+}} = call i32 @llvm.fpga.reg.i32
  store i32 %11, ptr %e, align 4
; CHECK: store i32 %add1, ptr %e, align 4
  %12 = load i32, ptr %a, align 4
; CHECK: %[[LOADA2:[0-9]+]] = load i32, ptr %a, align 4
  %13 = call i32 @llvm.fpga.reg.i32(i32 %12)
; CHECK-NOT: %{{[0-9]+}} = call i32 @llvm.fpga.reg.i32
  store i32 %13, ptr %f, align 4
; CHECK: store i32 %[[LOADA2]], ptr %f, align 4
  call void @llvm.memcpy.p0.p2.i32(ptr align 4 %i, ptr addrspace(2) align 4 @foo.i, i32 8, i1 false)
  call void @llvm.memcpy.p0.p0.i32(ptr align 4 %agg-temp, ptr align 4 %i, i32 8, i1 false)
  call void @llvm.fpga.reg.struct.p0(ptr %ii, ptr %agg-temp)
; CHECK-NOT: call void @llvm.fpga.reg.struct.p0(ptr %ii, ptr %agg-temp)
  call void @llvm.memcpy.p0.p0.i32(ptr align 4 %agg-temp2, ptr align 4 %ii, i32 8, i1 false)
  call void @llvm.fpga.reg.struct.p0(ptr %tmp, ptr %agg-temp2)
; CHECK-NOT: call void @llvm.fpga.reg.struct.p0(ptr %tmp, ptr %agg-temp2)
  call void @llvm.memcpy.p0.p0.i32(ptr align 4 %iii, ptr align 4 %tmp, i32 8, i1 false)
  %14 = call ptr @llvm.fpga.reg.p0(ptr %iii)
; CHECK-NOT: %{{[0-9]+}} = call ptr @llvm.fpga.reg.p0(ptr %iii)
  store ptr %14, ptr %iiii, align 4
; CHECK-NOT: store ptr %{{[0-9]+}}, ptr %iiii, align 4
; CHECK: store ptr %iii, ptr %iiii, align 4
  call void @llvm.memcpy.p0.p2.i32(ptr align 4 %u1, ptr addrspace(2) align 4 @foo.u1, i32 4, i1 false)
  call void @llvm.memcpy.p0.p0.i32(ptr align 4 %agg-temp4, ptr align 4 %u1, i32 4, i1 false)
  call void @llvm.fpga.reg.struct.p0(ptr %tmp3, ptr %agg-temp4)
; CHECK-NOT: call void @llvm.fpga.reg.struct.p0(ptr %tmp3, ptr %agg-temp4)
  call void @llvm.memcpy.p0.p0.i32(ptr align 4 %u2, ptr align 4 %tmp3, i32 4, i1 false)
  %15 = call ptr @llvm.fpga.reg.p0(ptr %u2)
; CHECK-NOT: %{{[0-9]+}} = call ptr @llvm.fpga.reg.p0(ptr %u2)
  store ptr %15, ptr %u3, align 4
; CHECK-NOT: store ptr %{{[0-9]+}}, ptr %u3, align 4
; CHECK: store ptr %u2, ptr %u3, align 4
  store ptr %a, ptr %ap, align 4
  %16 = load ptr, ptr %ap, align 4
; CHECK: %[[LOADAP:[0-9]+]] = load ptr, ptr %ap, align 4
  %17 = call ptr @llvm.fpga.reg.p0(ptr %16)
; CHECK-NOT: %{{[0-9]+}} = call ptr @llvm.fpga.reg.p0
  store ptr %17, ptr %bp, align 4
; CHECK-NOT: store ptr %{{[0-9]+}}, ptr %bp, align 4
; CHECK: store ptr %[[LOADAP]], ptr %bp, align 4
  ret void
}

; Function Attrs: nounwind
declare i32 @llvm.fpga.reg.i32(i32) #1
; CHECK-NOT:  declare i32 @llvm.fpga.reg.i32(i32)

; Function Attrs: nounwind
declare float @llvm.fpga.reg.f32(float) #1
; CHECK-NOT: declare float @llvm.fpga.reg.f32(float)

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0.p2.i32(ptr nocapture writeonly, ptr addrspace(2) nocapture readonly, i32, i1) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0.p0.i32(ptr nocapture writeonly, ptr nocapture readonly, i32, i1) #2

; Function Attrs: nounwind
declare void @llvm.fpga.reg.struct.p0(ptr, ptr) #1
; CHECK-NOT: declare void @llvm.fpga.reg.struct.p0(ptr, ptr)

; Function Attrs: nounwind
declare ptr @llvm.fpga.reg.p0(ptr) #1
; CHECK-NOT: ptr @llvm.fpga.reg.p0(ptr)

; Function Attrs: nounwind
; CHECK-NOT: declare void @llvm.fpga.reg.struct.p0(ptr, ptr)

; Function Attrs: nounwind
; CHECK-NOT: ptr @llvm.fpga.reg.p0(ptr)

; Function Attrs: nounwind
; CHECK-NOT: declare ptr @llvm.fpga.reg.p0(ptr)

attributes #0 = { convergent noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nounwind }

!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 0}
!2 = !{i32 1, i32 2}
!3 = !{}
!4 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 785365fbd2965b6d4b84ca2e99d66bf6935bbcd4)"}

; 14 @llvm.fpga.reg.* instructions have been removed.
; So debugify would report 14 warnings of "Missing line"
; Other warnings are not expected.

; DEBUGIFY-NOT: WARNING:
; DEBUGIFY-COUNT-14: WARNING: Missing line
; DEBUGIFY-NOT: WARNING:
