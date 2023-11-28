; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=true -S %s | FileCheck %s
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
; CHECK: define{{.*}}void @__omp_offloading{{.*}}foo{{.*}}(i64{{[^*,]*}}, i64{{[^*,]*}}, i64{{[^*,]*}}, i64{{[^*,]*}}, i64{{[^*,]*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 0, ptr @.omp_offloading.requires_reg, ptr null }]

define dso_local void @foo() {
entry:
  %x = alloca float, align 4
  %y = alloca double, align 8
  %i = alloca i32, align 4
  %j = alloca i64, align 8
  %c = alloca i8, align 1
  %0 = bitcast ptr %x to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %0)
  %1 = bitcast ptr %y to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr %1)
  %2 = bitcast ptr %i to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %2)
  %3 = bitcast ptr %j to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr %3)
  call void @llvm.lifetime.start.p0(i64 1, ptr %c)
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %x, float 0.000000e+00, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y, double 0.000000e+00, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %j, i64 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %c, i8 0, i32 1) ]

  %5 = load float, ptr %x, align 4, !tbaa !3
  %6 = load double, ptr %y, align 8, !tbaa !7
  %7 = load i32, ptr %i, align 4, !tbaa !9
  %8 = load i64, ptr %j, align 8, !tbaa !11
  %9 = load i8, ptr %c, align 1, !tbaa !13
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TARGET"() ]

  call void @llvm.lifetime.end.p0(i64 1, ptr %c)
  %10 = bitcast ptr %j to ptr
  call void @llvm.lifetime.end.p0(i64 8, ptr %10)
  %11 = bitcast ptr %i to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %11)
  %12 = bitcast ptr %y to ptr
  call void @llvm.lifetime.end.p0(i64 8, ptr %12)
  %13 = bitcast ptr %x to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %13)
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @.omp_offloading.requires_reg()
declare void @__tgt_register_requires(i64)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)


!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2054, i32 1856547, !"_Z3foo", i32 7, i32 0, i32 0}
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
