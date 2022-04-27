; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=true -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-opt-scalar-fp=true -S %s | FileCheck %s

; Original code:
; void foo() {
;   float x;
;   double y;
;   int i;
;   long long int j;
;   char c;
; #pragma omp target firstprivate(x, y, i, j, c)
;   {
;     (void)x;(void)y;(void)i;(void)j;(void)c;
;   }
; }

; Check that all firstprivate values are passed by value:
; CHECK: define{{.*}}void @__omp_offloading_806_1c5423__Z3foo_l7(i64{{[^*,]*}}, i64{{[^*,]*}}, i64{{[^*,]*}}, i64{{[^*,]*}}, i64{{[^*,]*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @.omp_offloading.requires_reg, i8* null }]

; Function Attrs: nounwind uwtable
define dso_local void @foo() #0 {
entry:
  %x = alloca float, align 4
  %y = alloca double, align 8
  %i = alloca i32, align 4
  %j = alloca i64, align 8
  %c = alloca i8, align 1
  %0 = bitcast float* %x to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = bitcast double* %y to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %1) #2
  %2 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  %3 = bitcast i64* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %3) #2
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %c) #2
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(float* %x), "QUAL.OMP.FIRSTPRIVATE"(double* %y), "QUAL.OMP.FIRSTPRIVATE"(i32* %i), "QUAL.OMP.FIRSTPRIVATE"(i64* %j), "QUAL.OMP.FIRSTPRIVATE"(i8* %c) ]
  %5 = load float, float* %x, align 4, !tbaa !3
  %6 = load double, double* %y, align 8, !tbaa !7
  %7 = load i32, i32* %i, align 4, !tbaa !9
  %8 = load i64, i64* %j, align 8, !tbaa !11
  %9 = load i8, i8* %c, align 1, !tbaa !13
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %c) #2
  %10 = bitcast i64* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %10) #2
  %11 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %11) #2
  %12 = bitcast double* %y to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %12) #2
  %13 = bitcast float* %x to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %13) #2
  ret void
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind uwtable
define internal void @.omp_offloading.requires_reg() #3 section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

; Function Attrs: nounwind
declare void @__tgt_register_requires(i64) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind }
attributes #3 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 2054, i32 1856547, !"_Z3foo", i32 7, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 10.0.0"}
!3 = !{!4, !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"double", !5, i64 0}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !5, i64 0}
!11 = !{!12, !12, i64 0}
!12 = !{!"long long", !5, i64 0}
!13 = !{!5, !5, i64 0}
