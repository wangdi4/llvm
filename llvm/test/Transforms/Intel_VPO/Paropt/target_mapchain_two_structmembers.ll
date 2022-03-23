; REQUIRES: asserts
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S -vpo-paropt-force-member-of-to-point-to-base=true %s | FileCheck %s -check-prefix=TFORM
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S -vpo-paropt-force-member-of-to-point-to-base=true %s | FileCheck %s -check-prefix=TFORM
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=vpo-paropt-target -S -vpo-paropt-force-member-of-to-point-to-base=true %s 2>&1 | FileCheck %s -check-prefix=DBG
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug-only=vpo-paropt-target -S -vpo-paropt-force-member-of-to-point-to-base=true %s 2>&1 | FileCheck %s -check-prefix=DBG

; Test src:
;
; #include <stdio.h>
;
; typedef struct {
;   int a;
;   float b;
; } S;
;
; __attribute__((noinline)) void foo() {
;   S x, y;
;   x.a = 0; y.a = 2;
;
; #pragma omp target map(tofrom:x.a, y.b)
;   {
;     x.a = 10;
;     y.b = 20;
;   }
;   printf("x = %d, y = %f\n", x.a, y.b);
; }
;
; int main() {
;   foo();
; }

; Check that while processing MapTypes from the directive intrinsics,
; the MemberOf fields are updated based on the index of the starting
; link of the map-chain.
; DBG: Updated MemberOf Flag from '3' to '1'
; DBG: MapType changed from '844424930131971 (0x0003000000000003)' to '281474976710659 (0x0001000000000003)'
; DBG: Updated MemberOf Flag from '1' to '3'
; DBG: MapType changed from '281474976710659 (0x0001000000000003)' to '844424930131971 (0x0003000000000003)'


; Check that the maptypes struct has the value in the correct order. If the memberof
; fields were not updated, the 2nd and 4th elements would be interchanged.
; TFORM: @{{[^ ]+}} = private unnamed_addr constant [4 x i64] [i64 32, i64 281474976710659, i64 32, i64 844424930131971]


; ModuleID = '/tmp/icxFB6v30.bc'
source_filename = "target_map_twostructs.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%struct.S = type { i32, float }

@.str = private unnamed_addr constant [16 x i8] c"x = %d, y = %f\0A\00", align 1
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @.omp_offloading.requires_reg, i8* null }]
; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %x = alloca %struct.S, align 4
  %y = alloca %struct.S, align 4
  %a = getelementptr inbounds %struct.S, %struct.S* %x, i32 0, i32 0
  store i32 0, i32* %a, align 4
  %a1 = getelementptr inbounds %struct.S, %struct.S* %y, i32 0, i32 0
  store i32 2, i32* %a1, align 4
  %a2 = getelementptr inbounds %struct.S, %struct.S* %x, i32 0, i32 0
  %0 = getelementptr i32, i32* %a2, i32 1
  %1 = bitcast i32* %a2 to i8*
  %2 = bitcast i32* %0 to i8*
  %3 = ptrtoint i8* %2 to i64
  %4 = ptrtoint i8* %1 to i64
  %5 = sub i64 %3, %4
  %6 = sdiv exact i64 %5, ptrtoint (i8* getelementptr (i8, i8* null, i32 1) to i64)
  %b = getelementptr inbounds %struct.S, %struct.S* %y, i32 0, i32 1
  %7 = getelementptr float, float* %b, i32 1
  %8 = bitcast float* %b to i8*
  %9 = bitcast float* %7 to i8*
  %10 = ptrtoint i8* %9 to i64
  %11 = ptrtoint i8* %8 to i64
  %12 = sub i64 %10, %11
  %13 = sdiv exact i64 %12, ptrtoint (i8* getelementptr (i8, i8* null, i32 1) to i64)

  %14 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(%struct.S* %y, float* %b, i64 %13, i64 32), "QUAL.OMP.MAP.TOFROM:CHAIN"(%struct.S* %y, float* %b, i64 4, i64 844424930131971), "QUAL.OMP.MAP.TOFROM"(%struct.S* %x, i32* %a2, i64 %6, i64 32), "QUAL.OMP.MAP.TOFROM:CHAIN"(%struct.S* %x, i32* %a2, i64 4, i64 281474976710659) ]


  %a3 = getelementptr inbounds %struct.S, %struct.S* %x, i32 0, i32 0
  store i32 10, i32* %a3, align 4
  %b4 = getelementptr inbounds %struct.S, %struct.S* %y, i32 0, i32 1
  store float 2.000000e+01, float* %b4, align 4

  call void @llvm.directive.region.exit(token %14) [ "DIR.OMP.END.TARGET"() ]

  %a5 = getelementptr inbounds %struct.S, %struct.S* %x, i32 0, i32 0
  %15 = load i32, i32* %a5, align 4
  %b6 = getelementptr inbounds %struct.S, %struct.S* %y, i32 0, i32 1
  %16 = load float, float* %b6, align 4
  %conv = fpext float %16 to double
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str, i64 0, i64 0), i32 %15, double %conv)
  ret void
}
; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1
; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1
declare dso_local i32 @printf(i8*, ...) #2
; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #3 {
entry:
  call void @foo()
  ret i32 0
}
; Function Attrs: noinline nounwind uwtable
define internal void @.omp_offloading.requires_reg() #4 section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}
declare dso_local void @__tgt_register_requires(i64)

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 2055, i32 156315161, !"foo", i32 12, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 10.0.0"}
