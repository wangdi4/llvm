; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s 2>&1 | FileCheck %s

; #include <stdio.h>
;
; typedef struct {
;     int* a;
;     float b;
; } S;
;
; __attribute__((noinline)) void foo() {
;     S x;
;     int c[10];
;     x.a = &c[0];
;     x.a[1] = 1;
;     x.b = 2;
;
; #pragma omp target map(tofrom:x.a[:10])
;     {
;         x.a[1] = 10;
;         x.b = 20;
;     }
;     printf("a = %d, b = %f\n", x.a[1], x.b);
; }
;
; int main() {
;     foo();
; }

; Check that CodeExtractor does not emit an error for out-of-clause arguments being caputred.
; CHECK-NOT: CodeExtractor captured out-of-clause argument.

; Check that the outlined function only takes one argument (%x).
; CHECK: call void @__omp_offloading_{{.*}}foo{{.*}}(%struct.S* %x)

source_filename = "target_map_struct_and_sec.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%struct.S = type { i32*, float }

@.str = private unnamed_addr constant [16 x i8] c"a = %d, b = %f\0A\00", align 1
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @.omp_offloading.requires_reg, i8* null }]

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %x = alloca %struct.S, align 8
  %c = alloca [10 x i32], align 16
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* %c, i64 0, i64 0
  %a = getelementptr inbounds %struct.S, %struct.S* %x, i32 0, i32 0
  store i32* %arrayidx, i32** %a, align 8
  %a1 = getelementptr inbounds %struct.S, %struct.S* %x, i32 0, i32 0
  %0 = load i32*, i32** %a1, align 8
  %arrayidx2 = getelementptr inbounds i32, i32* %0, i64 1
  store i32 1, i32* %arrayidx2, align 4
  %b = getelementptr inbounds %struct.S, %struct.S* %x, i32 0, i32 1
  store float 2.000000e+00, float* %b, align 8
  %1 = bitcast %struct.S* %x to i8*
  %2 = getelementptr i8, i8* %1, i64 15
  %a3 = getelementptr inbounds %struct.S, %struct.S* %x, i32 0, i32 0
  %3 = bitcast %struct.S* %x to i8*
  %4 = bitcast i32** %a3 to i8*
  %5 = ptrtoint i8* %4 to i64
  %6 = ptrtoint i8* %3 to i64
  %7 = sub i64 %5, %6
  %8 = sdiv exact i64 %7, ptrtoint (i8* getelementptr (i8, i8* null, i32 1) to i64)
  %9 = getelementptr i32*, i32** %a3, i64 1
  %10 = bitcast i32** %9 to i8*
  %11 = getelementptr i8, i8* %2, i64 1
  %12 = ptrtoint i8* %11 to i64
  %13 = ptrtoint i8* %10 to i64
  %14 = sub i64 %12, %13
  %15 = sdiv exact i64 %14, ptrtoint (i8* getelementptr (i8, i8* null, i32 1) to i64)
  %a4 = getelementptr inbounds %struct.S, %struct.S* %x, i32 0, i32 0
  %a5 = getelementptr inbounds %struct.S, %struct.S* %x, i32 0, i32 0
  %16 = load i32*, i32** %a5, align 8
  %arrayidx6 = getelementptr inbounds i32, i32* %16, i64 0
  %17 = getelementptr i8, i8* %2, i32 1
  %18 = bitcast %struct.S* %x to i8*
  %19 = ptrtoint i8* %17 to i64
  %20 = ptrtoint i8* %18 to i64
  %21 = sub i64 %19, %20
  %22 = sdiv exact i64 %21, ptrtoint (i8* getelementptr (i8, i8* null, i32 1) to i64)

  %23 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(%struct.S* %x, %struct.S* %x, i64 %22, i64 32), "QUAL.OMP.MAP.TOFROM:CHAIN"(%struct.S* %x, %struct.S* %x, i64 %8, i64 281474976711171), "QUAL.OMP.MAP.TOFROM:CHAIN"(%struct.S* %x, i32** %9, i64 %15, i64 281474976711171), "QUAL.OMP.MAP.TOFROM:CHAIN"(i32** %a4, i32* %arrayidx6, i64 40, i64 281474976710675) ]

  %a7 = getelementptr inbounds %struct.S, %struct.S* %x, i32 0, i32 0
  %24 = load i32*, i32** %a7, align 8
  %arrayidx8 = getelementptr inbounds i32, i32* %24, i64 1
  store i32 10, i32* %arrayidx8, align 4
  %b9 = getelementptr inbounds %struct.S, %struct.S* %x, i32 0, i32 1
  store float 2.000000e+01, float* %b9, align 8

  call void @llvm.directive.region.exit(token %23) [ "DIR.OMP.END.TARGET"() ]

  %a10 = getelementptr inbounds %struct.S, %struct.S* %x, i32 0, i32 0
  %25 = load i32*, i32** %a10, align 8
  %arrayidx11 = getelementptr inbounds i32, i32* %25, i64 1
  %26 = load i32, i32* %arrayidx11, align 4
  %b12 = getelementptr inbounds %struct.S, %struct.S* %x, i32 0, i32 1
  %27 = load float, float* %b12, align 8
  %conv = fpext float %27 to double
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str, i64 0, i64 0), i32 %26, double %conv)
  ret void
}

; Function Attrs: noinline nounwind uwtable
define internal void @.omp_offloading.requires_reg() #4 section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

declare dso_local void @__tgt_register_requires(i64)

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 2055, i32 156313204, !"foo", i32 15, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 10.0.0"}
