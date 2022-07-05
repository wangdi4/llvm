; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes="vpo-cfg-restructuring,vpo-paropt-prepare" -S %s | FileCheck %s
;
; Test Src:
;
; #include <stdint.h>
;
; uint8_t u1;
; int8_t i1;
; uint16_t u2;
; int16_t i2;
; uint32_t u4;
; int32_t i4;
; uint64_t u8;
; int64_t i8;
; int64_t v;
;
; void atomics_sdiv_vs_udiv(int64_t rhs) {
;
; #pragma omp atomic capture
;     { v = i1; i1 /= rhs; };
;
; #pragma omp atomic capture
;     { v = u1; u1 /= rhs; };
;
; #pragma omp atomic capture
;     { v = i2; i2 /= rhs; };
;
; #pragma omp atomic capture
;     { v = u2; u2 /= rhs; };
;
; #pragma omp atomic capture
;     { v = i4; i4 /= rhs; };
;
; #pragma omp atomic capture
;     { v = u4; u4 /= rhs; };
;
; #pragma omp atomic capture
;     { v = i8; i8 /= rhs; };
;
; #pragma omp atomic capture
;     { v = u8; u8 /= rhs; };
; }
;
; void atomics_sdiv_vs_udiv_f128(_Quad rhs) {
;
; #pragma omp atomic capture
;     { v = i1; i1 /= rhs; };
;
; #pragma omp atomic capture
;     { v = u1; u1 /= rhs; };
;
; #pragma omp atomic capture
;     { v = i2; i2 /= rhs; };
;
; #pragma omp atomic capture
;     { v = u2; u2 /= rhs; };
;
; #pragma omp atomic capture
;     { v = i4; i4 /= rhs; };
;
; #pragma omp atomic capture
;     { v = u4; u4 /= rhs; };
;
; #pragma omp atomic capture
;     { v = i8; i8 /= rhs; };
;
; #pragma omp atomic capture
;     { v = u8; u8 /= rhs; };
; }
;
; void atomics_smul_vs_umul_f128(_Quad rhs) {
;
; #pragma omp atomic capture
;     { v = i1; i1 *= rhs; };
;
; #pragma omp atomic capture
;     { v = u1; u1 *= rhs; };
;
; #pragma omp atomic capture
;     { v = i2; i2 *= rhs; };
;
; #pragma omp atomic capture
;     { v = u2; u2 *= rhs; };
;
; #pragma omp atomic capture
;     { v = i4; i4 *= rhs; };
;
; #pragma omp atomic capture
;     { v = u4; u4 *= rhs; };
;
; #pragma omp atomic capture
;     { v = i8; i8 *= rhs; };
;
; #pragma omp atomic capture
;     { v = u8; u8 *= rhs; };
; }
;
; void atomic_ashr_vs_lshr(int64_t rhs) {
;
; #pragma omp atomic capture
;     { v = i1; i1 >>= rhs; };
;
; #pragma omp atomic capture
;     { v = u1; u1 >>= rhs; };
;
; #pragma omp atomic capture
;     { v = i2; i2 >>= rhs; };
;
; #pragma omp atomic capture
;     { v = u2; u2 >>= rhs; };
;
; #pragma omp atomic capture
;     { v = i4; i4 >>= rhs; };
;
; #pragma omp atomic capture
;     { v = u4; u4 >>= rhs; };
;
; #pragma omp atomic capture
;     { v = i8; i8 >>= rhs; };
;
; #pragma omp atomic capture
;     { v = u8; u8 >>= rhs; };
; }

; ModuleID = 'atomic_capture_unsigned.c'
source_filename = "atomic_capture_unsigned.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@i1 = common dso_local global i8 0, align 1
@v = common dso_local global i64 0, align 8
@u1 = common dso_local global i8 0, align 1
@i2 = common dso_local global i16 0, align 2
@u2 = common dso_local global i16 0, align 2
@i4 = common dso_local global i32 0, align 4
@u4 = common dso_local global i32 0, align 4
@i8 = common dso_local global i64 0, align 8
@u8 = common dso_local global i64 0, align 8

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @atomics_sdiv_vs_udiv(i64 %rhs) #0 {
entry:
  %rhs.addr = alloca i64, align 8
  store i64 %rhs, i64* %rhs.addr, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed1_div_cpt
  fence acquire
  %1 = load i8, i8* @i1, align 1
  %conv = sext i8 %1 to i64
  store i64 %conv, i64* @v, align 8
  %2 = load i64, i64* %rhs.addr, align 8
  %3 = load i8, i8* @i1, align 1
  %conv1 = sext i8 %3 to i64
  %div = sdiv i64 %conv1, %2
  %conv2 = trunc i64 %div to i8
  store i8 %conv2, i8* @i1, align 1
  fence release
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.ATOMIC"() ]
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed1u_div_cpt
  fence acquire
  %5 = load i8, i8* @u1, align 1
  %conv3 = zext i8 %5 to i64
  store i64 %conv3, i64* @v, align 8
  %6 = load i64, i64* %rhs.addr, align 8
  %7 = load i8, i8* @u1, align 1
  %conv4 = zext i8 %7 to i64
  %div5 = sdiv i64 %conv4, %6
  %conv6 = trunc i64 %div5 to i8
  store i8 %conv6, i8* @u1, align 1
  fence release
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.ATOMIC"() ]
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed2_div_cpt
  fence acquire
  %9 = load i16, i16* @i2, align 2
  %conv7 = sext i16 %9 to i64
  store i64 %conv7, i64* @v, align 8
  %10 = load i64, i64* %rhs.addr, align 8
  %11 = load i16, i16* @i2, align 2
  %conv8 = sext i16 %11 to i64
  %div9 = sdiv i64 %conv8, %10
  %conv10 = trunc i64 %div9 to i16
  store i16 %conv10, i16* @i2, align 2
  fence release
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.ATOMIC"() ]
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed2u_div_cpt
  fence acquire
  %13 = load i16, i16* @u2, align 2
  %conv11 = zext i16 %13 to i64
  store i64 %conv11, i64* @v, align 8
  %14 = load i64, i64* %rhs.addr, align 8
  %15 = load i16, i16* @u2, align 2
  %conv12 = zext i16 %15 to i64
  %div13 = sdiv i64 %conv12, %14
  %conv14 = trunc i64 %div13 to i16
  store i16 %conv14, i16* @u2, align 2
  fence release
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.ATOMIC"() ]
  %16 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed4_div_cpt
  fence acquire
  %17 = load i32, i32* @i4, align 4
  %conv15 = sext i32 %17 to i64
  store i64 %conv15, i64* @v, align 8
  %18 = load i64, i64* %rhs.addr, align 8
  %19 = load i32, i32* @i4, align 4
  %conv16 = sext i32 %19 to i64
  %div17 = sdiv i64 %conv16, %18
  %conv18 = trunc i64 %div17 to i32
  store i32 %conv18, i32* @i4, align 4
  fence release
  call void @llvm.directive.region.exit(token %16) [ "DIR.OMP.END.ATOMIC"() ]
  %20 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed4u_div_cpt
  fence acquire
  %21 = load i32, i32* @u4, align 4
  %conv19 = zext i32 %21 to i64
  store i64 %conv19, i64* @v, align 8
  %22 = load i64, i64* %rhs.addr, align 8
  %23 = load i32, i32* @u4, align 4
  %conv20 = zext i32 %23 to i64
  %div21 = sdiv i64 %conv20, %22
  %conv22 = trunc i64 %div21 to i32
  store i32 %conv22, i32* @u4, align 4
  fence release
  call void @llvm.directive.region.exit(token %20) [ "DIR.OMP.END.ATOMIC"() ]
  %24 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed8_div_cpt
  fence acquire
  %25 = load i64, i64* @i8, align 8
  store i64 %25, i64* @v, align 8
  %26 = load i64, i64* %rhs.addr, align 8
  %27 = load i64, i64* @i8, align 8
  %div23 = sdiv i64 %27, %26
  store i64 %div23, i64* @i8, align 8
  fence release
  call void @llvm.directive.region.exit(token %24) [ "DIR.OMP.END.ATOMIC"() ]
  %28 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed8u_div_cpt
  fence acquire
  %29 = load i64, i64* @u8, align 8
  store i64 %29, i64* @v, align 8
  %30 = load i64, i64* %rhs.addr, align 8
  %31 = load i64, i64* @u8, align 8
  %div24 = udiv i64 %31, %30
  store i64 %div24, i64* @u8, align 8
  fence release
  call void @llvm.directive.region.exit(token %28) [ "DIR.OMP.END.ATOMIC"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @atomics_sdiv_vs_udiv_f128(fp128 %rhs) #0 {
entry:
  %rhs.addr = alloca fp128, align 16
  store fp128 %rhs, fp128* %rhs.addr, align 16
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed1_div_cpt_fp
  fence acquire
  %1 = load i8, i8* @i1, align 1
  %conv = sext i8 %1 to i64
  store i64 %conv, i64* @v, align 8
  %2 = load fp128, fp128* %rhs.addr, align 16
  %3 = load i8, i8* @i1, align 1
  %conv1 = sitofp i8 %3 to fp128
  %div = fdiv fp128 %conv1, %2
  %conv2 = fptosi fp128 %div to i8
  store i8 %conv2, i8* @i1, align 1
  fence release
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.ATOMIC"() ]
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed1u_div_cpt_fp
  fence acquire
  %5 = load i8, i8* @u1, align 1
  %conv3 = zext i8 %5 to i64
  store i64 %conv3, i64* @v, align 8
  %6 = load fp128, fp128* %rhs.addr, align 16
  %7 = load i8, i8* @u1, align 1
  %conv4 = uitofp i8 %7 to fp128
  %div5 = fdiv fp128 %conv4, %6
  %conv6 = fptoui fp128 %div5 to i8
  store i8 %conv6, i8* @u1, align 1
  fence release
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.ATOMIC"() ]
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed2_div_cpt_fp
  fence acquire
  %9 = load i16, i16* @i2, align 2
  %conv7 = sext i16 %9 to i64
  store i64 %conv7, i64* @v, align 8
  %10 = load fp128, fp128* %rhs.addr, align 16
  %11 = load i16, i16* @i2, align 2
  %conv8 = sitofp i16 %11 to fp128
  %div9 = fdiv fp128 %conv8, %10
  %conv10 = fptosi fp128 %div9 to i16
  store i16 %conv10, i16* @i2, align 2
  fence release
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.ATOMIC"() ]
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed2u_div_cpt_fp
  fence acquire
  %13 = load i16, i16* @u2, align 2
  %conv11 = zext i16 %13 to i64
  store i64 %conv11, i64* @v, align 8
  %14 = load fp128, fp128* %rhs.addr, align 16
  %15 = load i16, i16* @u2, align 2
  %conv12 = uitofp i16 %15 to fp128
  %div13 = fdiv fp128 %conv12, %14
  %conv14 = fptoui fp128 %div13 to i16
  store i16 %conv14, i16* @u2, align 2
  fence release
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.ATOMIC"() ]
  %16 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed4_div_cpt_fp
  fence acquire
  %17 = load i32, i32* @i4, align 4
  %conv15 = sext i32 %17 to i64
  store i64 %conv15, i64* @v, align 8
  %18 = load fp128, fp128* %rhs.addr, align 16
  %19 = load i32, i32* @i4, align 4
  %conv16 = sitofp i32 %19 to fp128
  %div17 = fdiv fp128 %conv16, %18
  %conv18 = fptosi fp128 %div17 to i32
  store i32 %conv18, i32* @i4, align 4
  fence release
  call void @llvm.directive.region.exit(token %16) [ "DIR.OMP.END.ATOMIC"() ]
  %20 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed4u_div_cpt_fp
  fence acquire
  %21 = load i32, i32* @u4, align 4
  %conv19 = zext i32 %21 to i64
  store i64 %conv19, i64* @v, align 8
  %22 = load fp128, fp128* %rhs.addr, align 16
  %23 = load i32, i32* @u4, align 4
  %conv20 = uitofp i32 %23 to fp128
  %div21 = fdiv fp128 %conv20, %22
  %conv22 = fptoui fp128 %div21 to i32
  store i32 %conv22, i32* @u4, align 4
  fence release
  call void @llvm.directive.region.exit(token %20) [ "DIR.OMP.END.ATOMIC"() ]
  %24 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed8_div_cpt_fp
  fence acquire
  %25 = load i64, i64* @i8, align 8
  store i64 %25, i64* @v, align 8
  %26 = load fp128, fp128* %rhs.addr, align 16
  %27 = load i64, i64* @i8, align 8
  %conv23 = sitofp i64 %27 to fp128
  %div24 = fdiv fp128 %conv23, %26
  %conv25 = fptosi fp128 %div24 to i64
  store i64 %conv25, i64* @i8, align 8
  fence release
  call void @llvm.directive.region.exit(token %24) [ "DIR.OMP.END.ATOMIC"() ]
  %28 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed8u_div_cpt_fp
  fence acquire
  %29 = load i64, i64* @u8, align 8
  store i64 %29, i64* @v, align 8
  %30 = load fp128, fp128* %rhs.addr, align 16
  %31 = load i64, i64* @u8, align 8
  %conv26 = uitofp i64 %31 to fp128
  %div27 = fdiv fp128 %conv26, %30
  %conv28 = fptoui fp128 %div27 to i64
  store i64 %conv28, i64* @u8, align 8
  fence release
  call void @llvm.directive.region.exit(token %28) [ "DIR.OMP.END.ATOMIC"() ]
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @atomics_smul_vs_umul_f128(fp128 %rhs) #0 {
entry:
  %rhs.addr = alloca fp128, align 16
  store fp128 %rhs, fp128* %rhs.addr, align 16
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed1_mul_cpt_fp
  fence acquire
  %1 = load i8, i8* @i1, align 1
  %conv = sext i8 %1 to i64
  store i64 %conv, i64* @v, align 8
  %2 = load fp128, fp128* %rhs.addr, align 16
  %3 = load i8, i8* @i1, align 1
  %conv1 = sitofp i8 %3 to fp128
  %mul = fmul fp128 %conv1, %2
  %conv2 = fptosi fp128 %mul to i8
  store i8 %conv2, i8* @i1, align 1
  fence release
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.ATOMIC"() ]
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed1u_mul_cpt_fp
  fence acquire
  %5 = load i8, i8* @u1, align 1
  %conv3 = zext i8 %5 to i64
  store i64 %conv3, i64* @v, align 8
  %6 = load fp128, fp128* %rhs.addr, align 16
  %7 = load i8, i8* @u1, align 1
  %conv4 = uitofp i8 %7 to fp128
  %mul5 = fmul fp128 %conv4, %6
  %conv6 = fptoui fp128 %mul5 to i8
  store i8 %conv6, i8* @u1, align 1
  fence release
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.ATOMIC"() ]
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed2_mul_cpt_fp
  fence acquire
  %9 = load i16, i16* @i2, align 2
  %conv7 = sext i16 %9 to i64
  store i64 %conv7, i64* @v, align 8
  %10 = load fp128, fp128* %rhs.addr, align 16
  %11 = load i16, i16* @i2, align 2
  %conv8 = sitofp i16 %11 to fp128
  %mul9 = fmul fp128 %conv8, %10
  %conv10 = fptosi fp128 %mul9 to i16
  store i16 %conv10, i16* @i2, align 2
  fence release
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.ATOMIC"() ]
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed2u_mul_cpt_fp
  fence acquire
  %13 = load i16, i16* @u2, align 2
  %conv11 = zext i16 %13 to i64
  store i64 %conv11, i64* @v, align 8
  %14 = load fp128, fp128* %rhs.addr, align 16
  %15 = load i16, i16* @u2, align 2
  %conv12 = uitofp i16 %15 to fp128
  %mul13 = fmul fp128 %conv12, %14
  %conv14 = fptoui fp128 %mul13 to i16
  store i16 %conv14, i16* @u2, align 2
  fence release
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.ATOMIC"() ]
  %16 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed4_mul_cpt_fp
  fence acquire
  %17 = load i32, i32* @i4, align 4
  %conv15 = sext i32 %17 to i64
  store i64 %conv15, i64* @v, align 8
  %18 = load fp128, fp128* %rhs.addr, align 16
  %19 = load i32, i32* @i4, align 4
  %conv16 = sitofp i32 %19 to fp128
  %mul17 = fmul fp128 %conv16, %18
  %conv18 = fptosi fp128 %mul17 to i32
  store i32 %conv18, i32* @i4, align 4
  fence release
  call void @llvm.directive.region.exit(token %16) [ "DIR.OMP.END.ATOMIC"() ]
  %20 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed4u_mul_cpt_fp
  fence acquire
  %21 = load i32, i32* @u4, align 4
  %conv19 = zext i32 %21 to i64
  store i64 %conv19, i64* @v, align 8
  %22 = load fp128, fp128* %rhs.addr, align 16
  %23 = load i32, i32* @u4, align 4
  %conv20 = uitofp i32 %23 to fp128
  %mul21 = fmul fp128 %conv20, %22
  %conv22 = fptoui fp128 %mul21 to i32
  store i32 %conv22, i32* @u4, align 4
  fence release
  call void @llvm.directive.region.exit(token %20) [ "DIR.OMP.END.ATOMIC"() ]
  %24 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed8_mul_cpt_fp
  fence acquire
  %25 = load i64, i64* @i8, align 8
  store i64 %25, i64* @v, align 8
  %26 = load fp128, fp128* %rhs.addr, align 16
  %27 = load i64, i64* @i8, align 8
  %conv23 = sitofp i64 %27 to fp128
  %mul24 = fmul fp128 %conv23, %26
  %conv25 = fptosi fp128 %mul24 to i64
  store i64 %conv25, i64* @i8, align 8
  fence release
  call void @llvm.directive.region.exit(token %24) [ "DIR.OMP.END.ATOMIC"() ]
  %28 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed8u_mul_cpt_fp
  fence acquire
  %29 = load i64, i64* @u8, align 8
  store i64 %29, i64* @v, align 8
  %30 = load fp128, fp128* %rhs.addr, align 16
  %31 = load i64, i64* @u8, align 8
  %conv26 = uitofp i64 %31 to fp128
  %mul27 = fmul fp128 %conv26, %30
  %conv28 = fptoui fp128 %mul27 to i64
  store i64 %conv28, i64* @u8, align 8
  fence release
  call void @llvm.directive.region.exit(token %28) [ "DIR.OMP.END.ATOMIC"() ]
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @atomic_ashr_vs_lshr(i64 %rhs) #0 {
entry:
  %rhs.addr = alloca i64, align 8
  store i64 %rhs, i64* %rhs.addr, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed1_shr_cpt
  fence acquire
  %1 = load i8, i8* @i1, align 1
  %conv = sext i8 %1 to i64
  store i64 %conv, i64* @v, align 8
  %2 = load i64, i64* %rhs.addr, align 8
  %3 = load i8, i8* @i1, align 1
  %conv1 = sext i8 %3 to i32
  %sh_prom = trunc i64 %2 to i32
  %shl.mask = and i32 %sh_prom, 31
  %shr = ashr i32 %conv1, %shl.mask
  %conv2 = trunc i32 %shr to i8
  store i8 %conv2, i8* @i1, align 1
  fence release
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.ATOMIC"() ]
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed1u_shr_cpt
  fence acquire
  %5 = load i8, i8* @u1, align 1
  %conv3 = zext i8 %5 to i64
  store i64 %conv3, i64* @v, align 8
  %6 = load i64, i64* %rhs.addr, align 8
  %7 = load i8, i8* @u1, align 1
  %conv4 = zext i8 %7 to i32
  %sh_prom5 = trunc i64 %6 to i32
  %shl.mask6 = and i32 %sh_prom5, 31
  %shr7 = ashr i32 %conv4, %shl.mask6
  %conv8 = trunc i32 %shr7 to i8
  store i8 %conv8, i8* @u1, align 1
  fence release
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.ATOMIC"() ]
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed2_shr_cpt
  fence acquire
  %9 = load i16, i16* @i2, align 2
  %conv9 = sext i16 %9 to i64
  store i64 %conv9, i64* @v, align 8
  %10 = load i64, i64* %rhs.addr, align 8
  %11 = load i16, i16* @i2, align 2
  %conv10 = sext i16 %11 to i32
  %sh_prom11 = trunc i64 %10 to i32
  %shl.mask12 = and i32 %sh_prom11, 31
  %shr13 = ashr i32 %conv10, %shl.mask12
  %conv14 = trunc i32 %shr13 to i16
  store i16 %conv14, i16* @i2, align 2
  fence release
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.ATOMIC"() ]
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed2u_shr_cpt
  fence acquire
  %13 = load i16, i16* @u2, align 2
  %conv15 = zext i16 %13 to i64
  store i64 %conv15, i64* @v, align 8
  %14 = load i64, i64* %rhs.addr, align 8
  %15 = load i16, i16* @u2, align 2
  %conv16 = zext i16 %15 to i32
  %sh_prom17 = trunc i64 %14 to i32
  %shl.mask18 = and i32 %sh_prom17, 31
  %shr19 = ashr i32 %conv16, %shl.mask18
  %conv20 = trunc i32 %shr19 to i16
  store i16 %conv20, i16* @u2, align 2
  fence release
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.ATOMIC"() ]
  %16 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed4_shr_cpt
  fence acquire
  %17 = load i32, i32* @i4, align 4
  %conv21 = sext i32 %17 to i64
  store i64 %conv21, i64* @v, align 8
  %18 = load i64, i64* %rhs.addr, align 8
  %19 = load i32, i32* @i4, align 4
  %sh_prom22 = trunc i64 %18 to i32
  %shl.mask23 = and i32 %sh_prom22, 31
  %shr24 = ashr i32 %19, %shl.mask23
  store i32 %shr24, i32* @i4, align 4
  fence release
  call void @llvm.directive.region.exit(token %16) [ "DIR.OMP.END.ATOMIC"() ]
  %20 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed4u_shr_cpt
  fence acquire
  %21 = load i32, i32* @u4, align 4
  %conv25 = zext i32 %21 to i64
  store i64 %conv25, i64* @v, align 8
  %22 = load i64, i64* %rhs.addr, align 8
  %23 = load i32, i32* @u4, align 4
  %sh_prom26 = trunc i64 %22 to i32
  %shl.mask27 = and i32 %sh_prom26, 31
  %shr28 = lshr i32 %23, %shl.mask27
  store i32 %shr28, i32* @u4, align 4
  fence release
  call void @llvm.directive.region.exit(token %20) [ "DIR.OMP.END.ATOMIC"() ]
  %24 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed8_shr_cpt
  fence acquire
  %25 = load i64, i64* @i8, align 8
  store i64 %25, i64* @v, align 8
  %26 = load i64, i64* %rhs.addr, align 8
  %27 = load i64, i64* @i8, align 8
  %shl.mask29 = and i64 %26, 63
  %shr30 = ashr i64 %27, %shl.mask29
  store i64 %shr30, i64* @i8, align 8
  fence release
  call void @llvm.directive.region.exit(token %24) [ "DIR.OMP.END.ATOMIC"() ]
  %28 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
; CHECK: call {{i[0-9]+}} @__kmpc_atomic_fixed8u_shr_cpt
  fence acquire
  %29 = load i64, i64* @u8, align 8
  store i64 %29, i64* @v, align 8
  %30 = load i64, i64* %rhs.addr, align 8
  %31 = load i64, i64* @u8, align 8
  %shl.mask31 = and i64 %30, 63
  %shr32 = lshr i64 %31, %shl.mask31
  store i64 %shr32, i64* @u8, align 8
  fence release
  call void @llvm.directive.region.exit(token %28) [ "DIR.OMP.END.ATOMIC"() ]
  ret void
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
