; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s

; Test src:
;
; void foo1(long long int n)
; {
;   #pragma omp target
;   #pragma omp teams num_teams(n) thread_limit(42)
;   { }
; }
; void foo2()
; {
;   int local = 2;
;   #pragma omp target
;   #pragma omp teams num_teams(local) thread_limit(local+2)
;   { }
; }
; void foo1c(long long int n)
; {
;   #pragma omp target teams num_teams(n) thread_limit(42)
;   { }
; }
; void foo2c()
; {
;   int local = 2;
;   #pragma omp target teams num_teams(local) thread_limit(local+2)
;   { }
; }

; Check that num_teams and thread_limit passed as Constant or
; by reference work properly.

; CHECK: define dso_local void @_Z4foo1x(i64 noundef %n)
; CHECK: call i32 @__tgt_target_teams({{.*}}, i32 %{{.*}}, i32 42)

; CHECK: define dso_local void @_Z4foo2v()
; CHECK: call i32 @__tgt_target_teams({{.*}}, i32 %{{.*}}, i32 %{{.*}})

; CHECK: define dso_local void @_Z5foo1cx(i64 noundef %n)
; CHECK: call i32 @__tgt_target_teams({{.*}}, i32 %{{.*}}, i32 42)

; CHECK: define dso_local void @_Z5foo2cv()
; CHECK: call i32 @__tgt_target_teams({{.*}}, i32 %{{.*}}, i32 %{{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 0, ptr @.omp_offloading.requires_reg, ptr null }]

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_Z4foo1x(i64 noundef %n) #0 {
entry:
  %n.addr = alloca i64, align 8
  store i64 %n, ptr %n.addr, align 8, !tbaa !8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %n.addr, i64 0, i32 1) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.NUM_TEAMS:TYPED"(ptr %n.addr, i64 0),
    "QUAL.OMP.THREAD_LIMIT"(i32 42) ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_Z4foo2v() #0 {
entry:
  %local = alloca i32, align 4
  %omp.clause.tmp = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %local) #1
  store i32 2, ptr %local, align 4, !tbaa !12
  %0 = load i32, ptr %local, align 4, !tbaa !12
  %add = add nsw i32 %0, 2
  store i32 %add, ptr %omp.clause.tmp, align 4, !tbaa !12
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %local, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %omp.clause.tmp, i32 0, i32 1) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.NUM_TEAMS:TYPED"(ptr %local, i32 0),
    "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr %omp.clause.tmp, i32 0) ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %local) #1
  ret void
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_Z5foo1cx(i64 noundef %n) #0 {
entry:
  %n.addr = alloca i64, align 8
  store i64 %n, ptr %n.addr, align 8, !tbaa !8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %n.addr, i64 0, i32 1) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.NUM_TEAMS:TYPED"(ptr %n.addr, i64 0),
    "QUAL.OMP.THREAD_LIMIT"(i32 42) ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_Z5foo2cv() #0 {
entry:
  %local = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %local) #1
  store i32 2, ptr %local, align 4, !tbaa !12
  call void @llvm.lifetime.start.p0(i64 4, ptr %.capture_expr.0) #1
  %0 = load i32, ptr %local, align 4, !tbaa !12
  %add = add nsw i32 %0, 2
  store i32 %add, ptr %.capture_expr.0, align 4, !tbaa !12
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 3),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %local, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.capture_expr.0, i32 0, i32 1) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.NUM_TEAMS:TYPED"(ptr %local, i32 0),
    "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr %.capture_expr.0, i32 0) ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %.capture_expr.0) #1
  call void @llvm.lifetime.end.p0(i64 4, ptr %local) #1
  ret void
}

; Function Attrs: uwtable
define internal void @.omp_offloading.requires_reg() #3 section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

; Function Attrs: nounwind
declare void @__tgt_register_requires(i64) #1

attributes #0 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #3 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!omp_offload.info = !{!0, !1, !2, !3}
!llvm.module.flags = !{!4, !5, !6}

!0 = !{i32 0, i32 53, i32 -1925528690, !"_Z4foo2v", i32 8, i32 1, i32 0}
!1 = !{i32 0, i32 53, i32 -1925528690, !"_Z4foo1x", i32 2, i32 0, i32 0}
!2 = !{i32 0, i32 53, i32 -1925528690, !"_Z5foo2cv", i32 18, i32 3, i32 0}
!3 = !{i32 0, i32 53, i32 -1925528690, !"_Z5foo1cx", i32 13, i32 2, i32 0}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"openmp", i32 51}
!6 = !{i32 7, !"uwtable", i32 2}
!8 = !{!9, !9, i64 0}
!9 = !{!"long long", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C++ TBAA"}
!12 = !{!13, !13, i64 0}
!13 = !{!"int", !10, i64 0}
