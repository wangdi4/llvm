; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes="vpo-cfg-restructuring,vpo-paropt-prepare" -S %s | FileCheck %s
;
; Test src:
;
; #include <float.h>
; short x = 10;
; long v = 2;
; __float128 e = 1.0;
;
; void atomic_swap (void) {
; #pragma omp atomic capture
;   { v = x; x = e; }
; }
;
; void atomic_capture_before (void) {
; #pragma omp atomic capture
;   { v = x; x = x + e; }
; }
;
; void atomic_capture_after (void) {
; #pragma omp atomic capture
;   { x = x + e; v = x;}
; }
;
; void atomic_update (void) {
; #pragma omp atomic
;   x = x + e;
; }
;
; void atomic_update_reverse (void) {
; #pragma omp atomic
;   x = e - x;
; }
;
; void atomic_read (void) {
; #pragma omp atomic read
;   e = x;
; }
;
; void atomic_write (void) {
; #pragma omp atomic write
;   x = e;
; }

; ModuleID = 'atomic.c'
source_filename = "atomic.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = dso_local global i16 10, align 2
@v = dso_local global i64 2, align 8
@e = dso_local global fp128 0xL00000000000000003FFF000000000000, align 16

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @atomic_swap() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
  fence acquire
  %1 = load i16, i16* @x, align 2
  %conv = sext i16 %1 to i64
  store i64 %conv, i64* @v, align 8
  %2 = load fp128, fp128* @e, align 16
  %conv1 = fptosi fp128 %2 to i16
  store i16 %conv1, i16* @x, align 2
; CHECK:  %{{[a-zA-Z._0-9]+}} = call i16 @__kmpc_atomic_fixed2_swp({{[^,]+}}, i32 %{{[a-zA-Z._0-9]+}}, i16* @x, i16 %{{[a-zA-Z._0-9]+}})
  fence release
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.ATOMIC"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @atomic_capture_before() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
  fence acquire
  %1 = load i16, i16* @x, align 2
  %conv = sext i16 %1 to i64
  store i64 %conv, i64* @v, align 8
  %2 = load i16, i16* @x, align 2
  %conv1 = sext i16 %2 to i32
  %conv2 = sitofp i32 %conv1 to fp128
  %3 = load fp128, fp128* @e, align 16
  %add = fadd fp128 %conv2, %3
  %conv3 = fptosi fp128 %add to i16
  store i16 %conv3, i16* @x, align 2
; CHECK:  %{{[a-zA-Z._0-9]+}} = call i16 @__kmpc_atomic_fixed2_add_cpt_fp({{[^,]+}}, i32 %{{[a-zA-Z._0-9]+}}, i16* @x, fp128 %{{[a-zA-Z._0-9]+}}, i32 0)
  fence release
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.ATOMIC"() ]
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @atomic_capture_after() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
  fence acquire
  %1 = load i16, i16* @x, align 2
  %conv = sext i16 %1 to i32
  %conv1 = sitofp i32 %conv to fp128
  %2 = load fp128, fp128* @e, align 16
  %add = fadd fp128 %conv1, %2
  %conv2 = fptosi fp128 %add to i16
  store i16 %conv2, i16* @x, align 2
  %3 = load i16, i16* @x, align 2
  %conv3 = sext i16 %3 to i64
  store i64 %conv3, i64* @v, align 8
; CHECK:  %{{[a-zA-Z._0-9]+}} = call i16 @__kmpc_atomic_fixed2_add_cpt_fp({{[^,]+}}, i32 %{{[a-zA-Z._0-9]+}}, i16* @x, fp128 %{{[a-zA-Z._0-9]+}}, i32 1)
  fence release
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.ATOMIC"() ]
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @atomic_update() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
  fence acquire
  %1 = load i16, i16* @x, align 2
  %conv = sext i16 %1 to i32
  %conv1 = sitofp i32 %conv to fp128
  %2 = load fp128, fp128* @e, align 16
  %add = fadd fp128 %conv1, %2
  %conv2 = fptosi fp128 %add to i16
  store i16 %conv2, i16* @x, align 2
; CHECK: call void @__kmpc_atomic_fixed2_add_fp({{[^,]+}}, i32 %{{[a-zA-Z._0-9]+}}, i16* @x, fp128 %{{[a-zA-Z._0-9]+}})
  fence release
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.ATOMIC"() ]
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @atomic_update_reverse() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
  fence acquire
  %1 = load fp128, fp128* @e, align 16
  %2 = load i16, i16* @x, align 2
  %conv = sext i16 %2 to i32
  %conv1 = sitofp i32 %conv to fp128
  %sub = fsub fp128 %1, %conv1
  %conv2 = fptosi fp128 %sub to i16
  store i16 %conv2, i16* @x, align 2
; CHECK: call void @__kmpc_atomic_fixed2_sub_rev_fp({{[^,]+}}, i32 %{{[a-zA-Z._0-9]+}}, i16* @x, fp128 %{{[a-zA-Z._0-9]+}})
  fence release
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.ATOMIC"() ]
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @atomic_read() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.READ"() ]
  fence acquire
  %1 = load i16, i16* @x, align 2
  %conv = sitofp i16 %1 to fp128
  store fp128 %conv, fp128* @e, align 16
; CHECK:  %{{[a-zA-Z._0-9]+}} = call i16 @__kmpc_atomic_fixed2_rd({{[^,]+}}, i32 %{{[a-zA-Z._0-9]+}}, i16* @x)
  fence release
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.ATOMIC"() ]
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @atomic_write() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.WRITE"() ]
  fence acquire
  %1 = load fp128, fp128* @e, align 16
  %conv = fptosi fp128 %1 to i16
  store i16 %conv, i16* @x, align 2
  fence release
; CHECK: call void @__kmpc_atomic_fixed2_wr({{[^,]+}}, i32 %{{[a-zA-Z._0-9]+}}, i16* @x, i16 %{{[a-zA-Z._0-9]+}})
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.ATOMIC"() ]
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @atomic_cpt_f10(x86_fp80 %x) #0 {
entry:
  %x.addr = alloca x86_fp80, align 16
  store x86_fp80 %x, x86_fp80* %x.addr, align 16
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
  fence acquire
  %1 = load x86_fp80, x86_fp80* %x.addr, align 16
  %conv = fptosi x86_fp80 %1 to i64
  store i64 %conv, i64* @v, align 8
  %2 = load fp128, fp128* @e, align 16
  %3 = load x86_fp80, x86_fp80* %x.addr, align 16
  %conv1 = fpext x86_fp80 %3 to fp128
  %add = fadd fp128 %conv1, %2
  %conv2 = fptrunc fp128 %add to x86_fp80
  store x86_fp80 %conv2, x86_fp80* %x.addr, align 16
  fence release
; CHECK: %{{[a-zA-Z._0-9]+}} = call x86_fp80 @__kmpc_atomic_float10_add_cpt({{[^,]+}}, i32 %{{[a-zA-Z._0-9]+}}, x86_fp80* %x.addr, x86_fp80 %{{[a-zA-Z._0-9]+}}, i32 0)
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.ATOMIC"() ]
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
  fence acquire
  %5 = load x86_fp80, x86_fp80* %x.addr, align 16
  %conv3 = fptosi x86_fp80 %5 to i64
  store i64 %conv3, i64* @v, align 8
  %6 = load fp128, fp128* @e, align 16
  %7 = load x86_fp80, x86_fp80* %x.addr, align 16
  %conv4 = fpext x86_fp80 %7 to fp128
  %mul = fmul fp128 %conv4, %6
  %conv5 = fptrunc fp128 %mul to x86_fp80
  store x86_fp80 %conv5, x86_fp80* %x.addr, align 16
  fence release
; CHECK: %{{[a-zA-Z._0-9]+}} = call x86_fp80 @__kmpc_atomic_float10_mul_cpt({{[^,]+}}, i32 %{{[a-zA-Z._0-9]+}}, x86_fp80* %x.addr, x86_fp80 %{{[a-zA-Z._0-9]+}}, i32 0)
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.ATOMIC"() ]
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
  fence acquire
  %9 = load x86_fp80, x86_fp80* %x.addr, align 16
  %conv6 = fptosi x86_fp80 %9 to i64
  store i64 %conv6, i64* @v, align 8
  %10 = load fp128, fp128* @e, align 16
  %11 = load x86_fp80, x86_fp80* %x.addr, align 16
  %conv7 = fpext x86_fp80 %11 to fp128
  %sub = fsub fp128 %conv7, %10
  %conv8 = fptrunc fp128 %sub to x86_fp80
  store x86_fp80 %conv8, x86_fp80* %x.addr, align 16
  fence release
; CHECK: %{{[a-zA-Z._0-9]+}} = call x86_fp80 @__kmpc_atomic_float10_sub_cpt({{[^,]+}}, i32 %{{[a-zA-Z._0-9]+}}, x86_fp80* %x.addr, x86_fp80 %{{[a-zA-Z._0-9]+}}, i32 0)
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.ATOMIC"() ]
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
  fence acquire
  %13 = load x86_fp80, x86_fp80* %x.addr, align 16
  %conv9 = fptosi x86_fp80 %13 to i64
  store i64 %conv9, i64* @v, align 8
  %14 = load fp128, fp128* @e, align 16
  %15 = load x86_fp80, x86_fp80* %x.addr, align 16
  %conv10 = fpext x86_fp80 %15 to fp128
  %div = fdiv fp128 %conv10, %14
  %conv11 = fptrunc fp128 %div to x86_fp80
  store x86_fp80 %conv11, x86_fp80* %x.addr, align 16
  fence release
; CHECK: %{{[a-zA-Z._0-9]+}} = call x86_fp80 @__kmpc_atomic_float10_div_cpt({{[^,]+}}, i32 %{{[a-zA-Z._0-9]+}}, x86_fp80* %x.addr, x86_fp80 %{{[a-zA-Z._0-9]+}}, i32 0)
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.ATOMIC"() ]
  ret void
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0"}
