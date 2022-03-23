; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes="vpo-cfg-restructuring,vpo-paropt-prepare" -S %s | FileCheck %s
;
; Test src:
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
;
; void atomics_sdiv_vs_udiv(int64_t rhs) {
;
; #pragma omp atomic
;     i1 /=rhs;
;
; #pragma omp atomic
;     u1 /=rhs;
;
; #pragma omp atomic
;     i2 /=rhs;
;
; #pragma omp atomic
;     u2 /=rhs;
;
; #pragma omp atomic
;     i4 /=rhs;
;
; #pragma omp atomic
;     u4 /=rhs;
;
; #pragma omp atomic
;     i8 /=rhs;
;
; #pragma omp atomic
;     u8 /=rhs;
; }
;
; void atomics_sdiv_vs_udiv_f128(_Quad rhs) {
;
; #pragma omp atomic
;     i1 /=rhs;
;
; #pragma omp atomic
;     u1 /=rhs;
;
; #pragma omp atomic
;     i2 /=rhs;
;
; #pragma omp atomic
;     u2 /=rhs;
;
; #pragma omp atomic
;     i4 /=rhs;
;
; #pragma omp atomic
;     u4 /=rhs;
;
; #pragma omp atomic
;     i8 /=rhs;
;
; #pragma omp atomic
;     u8 /=rhs;
; }
;
; void atomics_smul_vs_umul_f128(_Quad rhs) {
;
; #pragma omp atomic
;     i1 *=rhs;
;
; #pragma omp atomic
;     u1 *=rhs;
;
; #pragma omp atomic
;     i2 *=rhs;
;
; #pragma omp atomic
;     u2 *=rhs;
;
; #pragma omp atomic
;     i4 *=rhs;
;
; #pragma omp atomic
;     u4 *=rhs;
;
; #pragma omp atomic
;     i8 *=rhs;
;
; #pragma omp atomic
;     u8 *=rhs;
; }
;
; void atomic_ashr_vs_lshr(int64_t rhs) {
;
; #pragma omp atomic
;     i1 >>= rhs;
;
; #pragma omp atomic
;     u1 >>= rhs;
;
; #pragma omp atomic
;     i2 >>= rhs;
;
; #pragma omp atomic
;     u2 >>= rhs;
;
; #pragma omp atomic
;     i4 >>= rhs;
;
; #pragma omp atomic
;     u4 >>= rhs;
;
; #pragma omp atomic
;     i8 >>= rhs;
;
; #pragma omp atomic
;     u8 >>= rhs;
; }
;
; ModuleID = 'atomic_unsigned.c'
source_filename = "atomic_unsigned.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@i1 = common dso_local global i8 0, align 1
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
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed1_div
  fence acquire
  %1 = load i64, i64* %rhs.addr, align 8
  %2 = load i8, i8* @i1, align 1
  %conv = sext i8 %2 to i64
  %div = sdiv i64 %conv, %1
  %conv1 = trunc i64 %div to i8
  store i8 %conv1, i8* @i1, align 1
  fence release
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.ATOMIC"() ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed1u_div
  fence acquire
  %4 = load i64, i64* %rhs.addr, align 8
  %5 = load i8, i8* @u1, align 1
  %conv2 = zext i8 %5 to i64
  %div3 = sdiv i64 %conv2, %4
  %conv4 = trunc i64 %div3 to i8
  store i8 %conv4, i8* @u1, align 1
  fence release
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.ATOMIC"() ]
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed2_div
  fence acquire
  %7 = load i64, i64* %rhs.addr, align 8
  %8 = load i16, i16* @i2, align 2
  %conv5 = sext i16 %8 to i64
  %div6 = sdiv i64 %conv5, %7
  %conv7 = trunc i64 %div6 to i16
  store i16 %conv7, i16* @i2, align 2
  fence release
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.ATOMIC"() ]
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed2u_div
  fence acquire
  %10 = load i64, i64* %rhs.addr, align 8
  %11 = load i16, i16* @u2, align 2
  %conv8 = zext i16 %11 to i64
  %div9 = sdiv i64 %conv8, %10
  %conv10 = trunc i64 %div9 to i16
  store i16 %conv10, i16* @u2, align 2
  fence release
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.ATOMIC"() ]
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed4_div
  fence acquire
  %13 = load i64, i64* %rhs.addr, align 8
  %14 = load i32, i32* @i4, align 4
  %conv11 = sext i32 %14 to i64
  %div12 = sdiv i64 %conv11, %13
  %conv13 = trunc i64 %div12 to i32
  store i32 %conv13, i32* @i4, align 4
  fence release
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.ATOMIC"() ]
  %15 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed4u_div
  fence acquire
  %16 = load i64, i64* %rhs.addr, align 8
  %17 = load i32, i32* @u4, align 4
  %conv14 = zext i32 %17 to i64
  %div15 = sdiv i64 %conv14, %16
  %conv16 = trunc i64 %div15 to i32
  store i32 %conv16, i32* @u4, align 4
  fence release
  call void @llvm.directive.region.exit(token %15) [ "DIR.OMP.END.ATOMIC"() ]
  %18 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed8_div
  fence acquire
  %19 = load i64, i64* %rhs.addr, align 8
  %20 = load i64, i64* @i8, align 8
  %div17 = sdiv i64 %20, %19
  store i64 %div17, i64* @i8, align 8
  fence release
  call void @llvm.directive.region.exit(token %18) [ "DIR.OMP.END.ATOMIC"() ]
  %21 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed8u_div
  fence acquire
  %22 = load i64, i64* %rhs.addr, align 8
  %23 = load i64, i64* @u8, align 8
  %div18 = udiv i64 %23, %22
  store i64 %div18, i64* @u8, align 8
  fence release
  call void @llvm.directive.region.exit(token %21) [ "DIR.OMP.END.ATOMIC"() ]
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
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed1_div_fp
  fence acquire
  %1 = load fp128, fp128* %rhs.addr, align 16
  %2 = load i8, i8* @i1, align 1
  %conv = sitofp i8 %2 to fp128
  %div = fdiv fp128 %conv, %1
  %conv1 = fptosi fp128 %div to i8
  store i8 %conv1, i8* @i1, align 1
  fence release
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.ATOMIC"() ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed1u_div_fp
  fence acquire
  %4 = load fp128, fp128* %rhs.addr, align 16
  %5 = load i8, i8* @u1, align 1
  %conv2 = uitofp i8 %5 to fp128
  %div3 = fdiv fp128 %conv2, %4
  %conv4 = fptoui fp128 %div3 to i8
  store i8 %conv4, i8* @u1, align 1
  fence release
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.ATOMIC"() ]
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed2_div_fp
  fence acquire
  %7 = load fp128, fp128* %rhs.addr, align 16
  %8 = load i16, i16* @i2, align 2
  %conv5 = sitofp i16 %8 to fp128
  %div6 = fdiv fp128 %conv5, %7
  %conv7 = fptosi fp128 %div6 to i16
  store i16 %conv7, i16* @i2, align 2
  fence release
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.ATOMIC"() ]
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed2u_div_fp
  fence acquire
  %10 = load fp128, fp128* %rhs.addr, align 16
  %11 = load i16, i16* @u2, align 2
  %conv8 = uitofp i16 %11 to fp128
  %div9 = fdiv fp128 %conv8, %10
  %conv10 = fptoui fp128 %div9 to i16
  store i16 %conv10, i16* @u2, align 2
  fence release
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.ATOMIC"() ]
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed4_div_fp
  fence acquire
  %13 = load fp128, fp128* %rhs.addr, align 16
  %14 = load i32, i32* @i4, align 4
  %conv11 = sitofp i32 %14 to fp128
  %div12 = fdiv fp128 %conv11, %13
  %conv13 = fptosi fp128 %div12 to i32
  store i32 %conv13, i32* @i4, align 4
  fence release
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.ATOMIC"() ]
  %15 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed4u_div_fp
  fence acquire
  %16 = load fp128, fp128* %rhs.addr, align 16
  %17 = load i32, i32* @u4, align 4
  %conv14 = uitofp i32 %17 to fp128
  %div15 = fdiv fp128 %conv14, %16
  %conv16 = fptoui fp128 %div15 to i32
  store i32 %conv16, i32* @u4, align 4
  fence release
  call void @llvm.directive.region.exit(token %15) [ "DIR.OMP.END.ATOMIC"() ]
  %18 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed8_div_fp
  fence acquire
  %19 = load fp128, fp128* %rhs.addr, align 16
  %20 = load i64, i64* @i8, align 8
  %conv17 = sitofp i64 %20 to fp128
  %div18 = fdiv fp128 %conv17, %19
  %conv19 = fptosi fp128 %div18 to i64
  store i64 %conv19, i64* @i8, align 8
  fence release
  call void @llvm.directive.region.exit(token %18) [ "DIR.OMP.END.ATOMIC"() ]
  %21 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed8u_div_fp
  fence acquire
  %22 = load fp128, fp128* %rhs.addr, align 16
  %23 = load i64, i64* @u8, align 8
  %conv20 = uitofp i64 %23 to fp128
  %div21 = fdiv fp128 %conv20, %22
  %conv22 = fptoui fp128 %div21 to i64
  store i64 %conv22, i64* @u8, align 8
  fence release
  call void @llvm.directive.region.exit(token %21) [ "DIR.OMP.END.ATOMIC"() ]
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @atomics_smul_vs_umul_f128(fp128 %rhs) #0 {
entry:
  %rhs.addr = alloca fp128, align 16
  store fp128 %rhs, fp128* %rhs.addr, align 16
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed1_mul_fp
  fence acquire
  %1 = load fp128, fp128* %rhs.addr, align 16
  %2 = load i8, i8* @i1, align 1
  %conv = sitofp i8 %2 to fp128
  %mul = fmul fp128 %conv, %1
  %conv1 = fptosi fp128 %mul to i8
  store i8 %conv1, i8* @i1, align 1
  fence release
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.ATOMIC"() ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed1u_mul_fp
  fence acquire
  %4 = load fp128, fp128* %rhs.addr, align 16
  %5 = load i8, i8* @u1, align 1
  %conv2 = uitofp i8 %5 to fp128
  %mul3 = fmul fp128 %conv2, %4
  %conv4 = fptoui fp128 %mul3 to i8
  store i8 %conv4, i8* @u1, align 1
  fence release
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.ATOMIC"() ]
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed2_mul_fp
  fence acquire
  %7 = load fp128, fp128* %rhs.addr, align 16
  %8 = load i16, i16* @i2, align 2
  %conv5 = sitofp i16 %8 to fp128
  %mul6 = fmul fp128 %conv5, %7
  %conv7 = fptosi fp128 %mul6 to i16
  store i16 %conv7, i16* @i2, align 2
  fence release
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.ATOMIC"() ]
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed2u_mul_fp
  fence acquire
  %10 = load fp128, fp128* %rhs.addr, align 16
  %11 = load i16, i16* @u2, align 2
  %conv8 = uitofp i16 %11 to fp128
  %mul9 = fmul fp128 %conv8, %10
  %conv10 = fptoui fp128 %mul9 to i16
  store i16 %conv10, i16* @u2, align 2
  fence release
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.ATOMIC"() ]
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed4_mul_fp
  fence acquire
  %13 = load fp128, fp128* %rhs.addr, align 16
  %14 = load i32, i32* @i4, align 4
  %conv11 = sitofp i32 %14 to fp128
  %mul12 = fmul fp128 %conv11, %13
  %conv13 = fptosi fp128 %mul12 to i32
  store i32 %conv13, i32* @i4, align 4
  fence release
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.ATOMIC"() ]
  %15 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed4u_mul_fp
  fence acquire
  %16 = load fp128, fp128* %rhs.addr, align 16
  %17 = load i32, i32* @u4, align 4
  %conv14 = uitofp i32 %17 to fp128
  %mul15 = fmul fp128 %conv14, %16
  %conv16 = fptoui fp128 %mul15 to i32
  store i32 %conv16, i32* @u4, align 4
  fence release
  call void @llvm.directive.region.exit(token %15) [ "DIR.OMP.END.ATOMIC"() ]
  %18 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed8_mul_fp
  fence acquire
  %19 = load fp128, fp128* %rhs.addr, align 16
  %20 = load i64, i64* @i8, align 8
  %conv17 = sitofp i64 %20 to fp128
  %mul18 = fmul fp128 %conv17, %19
  %conv19 = fptosi fp128 %mul18 to i64
  store i64 %conv19, i64* @i8, align 8
  fence release
  call void @llvm.directive.region.exit(token %18) [ "DIR.OMP.END.ATOMIC"() ]
  %21 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed8u_mul_fp
  fence acquire
  %22 = load fp128, fp128* %rhs.addr, align 16
  %23 = load i64, i64* @u8, align 8
  %conv20 = uitofp i64 %23 to fp128
  %mul21 = fmul fp128 %conv20, %22
  %conv22 = fptoui fp128 %mul21 to i64
  store i64 %conv22, i64* @u8, align 8
  fence release
  call void @llvm.directive.region.exit(token %21) [ "DIR.OMP.END.ATOMIC"() ]
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @atomic_ashr_vs_lshr(i64 %rhs) #0 {
entry:
  %rhs.addr = alloca i64, align 8
  store i64 %rhs, i64* %rhs.addr, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed1_shr
  fence acquire
  %1 = load i64, i64* %rhs.addr, align 8
  %2 = load i8, i8* @i1, align 1
  %conv = sext i8 %2 to i32
  %sh_prom = trunc i64 %1 to i32
  %shl.mask = and i32 %sh_prom, 31
  %shr = ashr i32 %conv, %shl.mask
  %conv1 = trunc i32 %shr to i8
  store i8 %conv1, i8* @i1, align 1
  fence release
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.ATOMIC"() ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed1u_shr
  fence acquire
  %4 = load i64, i64* %rhs.addr, align 8
  %5 = load i8, i8* @u1, align 1
  %conv2 = zext i8 %5 to i32
  %sh_prom3 = trunc i64 %4 to i32
  %shl.mask4 = and i32 %sh_prom3, 31
  %shr5 = ashr i32 %conv2, %shl.mask4
  %conv6 = trunc i32 %shr5 to i8
  store i8 %conv6, i8* @u1, align 1
  fence release
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.ATOMIC"() ]
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed2_shr
  fence acquire
  %7 = load i64, i64* %rhs.addr, align 8
  %8 = load i16, i16* @i2, align 2
  %conv7 = sext i16 %8 to i32
  %sh_prom8 = trunc i64 %7 to i32
  %shl.mask9 = and i32 %sh_prom8, 31
  %shr10 = ashr i32 %conv7, %shl.mask9
  %conv11 = trunc i32 %shr10 to i16
  store i16 %conv11, i16* @i2, align 2
  fence release
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.ATOMIC"() ]
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed2u_shr
  fence acquire
  %10 = load i64, i64* %rhs.addr, align 8
  %11 = load i16, i16* @u2, align 2
  %conv12 = zext i16 %11 to i32
  %sh_prom13 = trunc i64 %10 to i32
  %shl.mask14 = and i32 %sh_prom13, 31
  %shr15 = ashr i32 %conv12, %shl.mask14
  %conv16 = trunc i32 %shr15 to i16
  store i16 %conv16, i16* @u2, align 2
  fence release
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.ATOMIC"() ]
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed4_shr
  fence acquire
  %13 = load i64, i64* %rhs.addr, align 8
  %14 = load i32, i32* @i4, align 4
  %sh_prom17 = trunc i64 %13 to i32
  %shl.mask18 = and i32 %sh_prom17, 31
  %shr19 = ashr i32 %14, %shl.mask18
  store i32 %shr19, i32* @i4, align 4
  fence release
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.ATOMIC"() ]
  %15 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed4u_shr
  fence acquire
  %16 = load i64, i64* %rhs.addr, align 8
  %17 = load i32, i32* @u4, align 4
  %sh_prom20 = trunc i64 %16 to i32
  %shl.mask21 = and i32 %sh_prom20, 31
  %shr22 = lshr i32 %17, %shl.mask21
  store i32 %shr22, i32* @u4, align 4
  fence release
  call void @llvm.directive.region.exit(token %15) [ "DIR.OMP.END.ATOMIC"() ]
  %18 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed8_shr
  fence acquire
  %19 = load i64, i64* %rhs.addr, align 8
  %20 = load i64, i64* @i8, align 8
  %shl.mask23 = and i64 %19, 63
  %shr24 = ashr i64 %20, %shl.mask23
  store i64 %shr24, i64* @i8, align 8
  fence release
  call void @llvm.directive.region.exit(token %18) [ "DIR.OMP.END.ATOMIC"() ]
  %21 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
; CHECK: call void @__kmpc_atomic_fixed8u_shr
  fence acquire
  %22 = load i64, i64* %rhs.addr, align 8
  %23 = load i64, i64* @u8, align 8
  %shl.mask25 = and i64 %22, 63
  %shr26 = lshr i64 %23, %shl.mask25
  store i64 %shr26, i64* @u8, align 8
  fence release
  call void @llvm.directive.region.exit(token %21) [ "DIR.OMP.END.ATOMIC"() ]
  ret void
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
