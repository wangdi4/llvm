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

; REQUIRES: fpga-emulator
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -remove-fpga-reg -spir-materializer -verify -S %s | FileCheck %s

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
  %iiii = alloca %struct.st*, align 4
  %u1 = alloca %union.un, align 4
  %u2 = alloca %union.un, align 4
  %u3 = alloca %union.un*, align 4
  %tmp3 = alloca %union.un, align 4
  %agg-temp4 = alloca %union.un, align 4
  %ap = alloca i32*, align 4
  %bp = alloca i32*, align 4
  store i32 123, i32* %a, align 4
  store i32 321, i32* %myA, align 4
  %0 = load i32, i32* %a, align 4
; CHECK: %[[LOADA1:[0-9]+]] = load i32, i32* %a, align 4
  %1 = call i32 @llvm.fpga.reg.i32(i32 %0)
; CHECK-NOT: %{{[0-9]+}} = call i32 @llvm.fpga.reg.i32(i32 %[[LOADA1]])
  store i32 %1, i32* %b, align 4
; CHECK: store i32 %[[LOADA1]], i32* %b, align 4
  %2 = load i32, i32* %myA, align 4
; CHECK: %[[LOADMYA:[0-9]+]] = load i32, i32* %myA, align 4
  %3 = call i32 @llvm.fpga.reg.i32(i32 %2)
; CHECK-NOT: %{{[0-9]+}} = call i32 @llvm.fpga.reg.i32(i32 %[[LOADMYA]])
  store i32 %3, i32* %myB, align 4
; CHECK: store i32 %[[LOADMYA]], i32* %myB, align 4
  %4 = call float @llvm.fpga.reg.f32(float 2.000000e+00)
; CHECK-NOT: %{{[0-9]+}} = call float @llvm.fpga.reg.f32(float 2.000000e+00)
  %conv = fptosi float %4 to i32
; CHECK: %conv = fptosi float 2.000000e+00 to i32
  store i32 %conv, i32* %c, align 4
  %5 = load i32, i32* %b, align 4
  %add = add nsw i32 %5, 12
  %6 = call i32 @llvm.fpga.reg.i32(i32 %add)
; CHECK-NOT: %[[RET1:[0-9]+]] = call i32 @llvm.fpga.reg.i32(i32 %add)
  %7 = call i32 @llvm.fpga.reg.i32(i32 %6)
; CHECK-NOT: %{{[0-9]+}} = call i32 @llvm.fpga.reg.i32(i32 %[[RET1]])
  store i32 %7, i32* %d, align 4
; CHECK: store i32 %add, i32* %d, align 4
  %8 = load i32, i32* %a, align 4
  %9 = load i32, i32* %b, align 4
  %add1 = add nsw i32 %8, %9
  %10 = call i32 @llvm.fpga.reg.i32(i32 %add1)
; CHECK-NOT: %[[RET2:[0-9]+]] = call i32 @llvm.fpga.reg.i32(i32 %add1)
  %11 = call i32 @llvm.fpga.reg.i32(i32 %10)
; CHECK-NOT: %{{[0-9]+}} = call i32 @llvm.fpga.reg.i32(i32 %[[RET2]])
  store i32 %11, i32* %e, align 4
; CHECK: store i32 %add1, i32* %e, align 4
  %12 = load i32, i32* %a, align 4
; CHECK: %[[LOADA2:[0-9]+]] = load i32, i32* %a, align 4
  %13 = call i32 @llvm.fpga.reg.i32(i32 %12)
; CHECK-NOT: %{{[0-9]+}} = call i32 @llvm.fpga.reg.i32(i32 %LOADA2)
  store i32 %13, i32* %f, align 4
; CHECK: store i32 %[[LOADA2]], i32* %f, align 4
  %14 = bitcast %struct.st* %i to i8*
  call void @llvm.memcpy.p0i8.p2i8.i32(i8* align 4 %14, i8 addrspace(2)* align 4 bitcast (%struct.st addrspace(2)* @foo.i to i8 addrspace(2)*), i32 8, i1 false)
  %15 = bitcast %struct.st* %agg-temp to i8*
  %16 = bitcast %struct.st* %i to i8*
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* align 4 %15, i8* align 4 %16, i32 8, i1 false)
  call void @llvm.fpga.reg.struct.p0s_struct.sts(%struct.st* %ii, %struct.st* %agg-temp)
; CHECK-NOT: call void @llvm.fpga.reg.struct.p0s_struct.sts(%struct.st* %ii, %struct.st* %agg-temp)
  %17 = bitcast %struct.st* %agg-temp2 to i8*
  %18 = bitcast %struct.st* %ii to i8*
; CHECK-NOT: %{{[0-9]+}} = bitcast %struct.st* %ii to i8*
; CHECK: %{{[0-9]+}} = bitcast %struct.st* %agg-temp to i8*
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* align 4 %17, i8* align 4 %18, i32 8, i1 false)
  call void @llvm.fpga.reg.struct.p0s_struct.sts(%struct.st* %tmp, %struct.st* %agg-temp2)
; CHECK-NOT: call void @llvm.fpga.reg.struct.p0s_struct.sts(%struct.st* %tmp, %struct.st* %agg-temp2)
  %19 = bitcast %struct.st* %iii to i8*
  %20 = bitcast %struct.st* %tmp to i8*
; CHECK-NOT: %{{[0-9]+}} = bitcast %struct.st* %tmp to i8*
; CHECK: %{{[0-9]+}} = bitcast %struct.st* %agg-temp2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* align 4 %19, i8* align 4 %20, i32 8, i1 false)
  %21 = call %struct.st* @llvm.fpga.reg.p0s_struct.sts(%struct.st* %iii)
; CHECK-NOT: %[[RET3:[0-9]+]] = call %struct.st* @llvm.fpga.reg.p0s_struct.sts(%struct.st* %iii)
  store %struct.st* %21, %struct.st** %iiii, align 4
; CHECK-NOT: store %struct.st* %[[RET3]], %struct.st** %iiii, align 4
; CHECK: store %struct.st* %iii, %struct.st** %iiii, align 4
  %22 = bitcast %union.un* %u1 to i8*
  call void @llvm.memcpy.p0i8.p2i8.i32(i8* align 4 %22, i8 addrspace(2)* align 4 bitcast (%union.un addrspace(2)* @foo.u1 to i8 addrspace(2)*), i32 4, i1 false)
  %23 = bitcast %union.un* %agg-temp4 to i8*
  %24 = bitcast %union.un* %u1 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* align 4 %23, i8* align 4 %24, i32 4, i1 false)
  call void @llvm.fpga.reg.struct.p0s_union.uns(%union.un* %tmp3, %union.un* %agg-temp4)
; CHECK-NOT: call void @llvm.fpga.reg.struct.p0s_union.uns(%union.un* %tmp3, %union.un* %agg-temp4)
  %25 = bitcast %union.un* %u2 to i8*
  %26 = bitcast %union.un* %tmp3 to i8*
; CHECK-NOT: %{{[0-9]+}} = bitcast %union.un* %tmp3 to i8*
; CHECK: %{{[0-9]+}} = bitcast %union.un* %agg-temp4 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* align 4 %25, i8* align 4 %26, i32 4, i1 false)
  %27 = call %union.un* @llvm.fpga.reg.p0s_union.uns(%union.un* %u2)
; CHECK-NOT: %[[RET4:[0-9]+]] = call %union.un* @llvm.fpga.reg.p0s_union.uns(%union.un* %u2)
  store %union.un* %27, %union.un** %u3, align 4
; CHECK-NOT: store %union.un* %[[RET4]], %union.un** %u3, align 4
; CHECK: store %union.un* %u2, %union.un** %u3, align 4
  store i32* %a, i32** %ap, align 4
  %28 = load i32*, i32** %ap, align 4
; CHECK: %[[LOADAP:[0-9]+]] = load i32*, i32** %ap, align 4
  %29 = call i32* @llvm.fpga.reg.p0i32(i32* %28)
; CHECK-NOT: %[[RET5:[0-9]+]] = call i32* @llvm.fpga.reg.p0i32(i32* %[[LOADAP]])
  store i32* %29, i32** %bp, align 4
; CHECK-NOT: store i32* %[[RET5]], i32** %bp, align 4
; CHECK: store i32* %[[LOADAP]], i32** %bp, align 4
  ret void
}

; Function Attrs: nounwind
declare i32 @llvm.fpga.reg.i32(i32) #1

; Function Attrs: nounwind
declare float @llvm.fpga.reg.f32(float) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p2i8.i32(i8* nocapture writeonly, i8 addrspace(2)* nocapture readonly, i32, i1) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i32(i8* nocapture writeonly, i8* nocapture readonly, i32, i1) #2

; Function Attrs: nounwind
declare void @llvm.fpga.reg.struct.p0s_struct.sts(%struct.st*, %struct.st*) #1

; Function Attrs: nounwind
declare %struct.st* @llvm.fpga.reg.p0s_struct.sts(%struct.st*) #1

; Function Attrs: nounwind
declare void @llvm.fpga.reg.struct.p0s_union.uns(%union.un*, %union.un*) #1

; Function Attrs: nounwind
declare %union.un* @llvm.fpga.reg.p0s_union.uns(%union.un*) #1

; Function Attrs: nounwind
declare i32* @llvm.fpga.reg.p0i32(i32*) #1

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
