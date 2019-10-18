; RUN: opt < %s -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg  -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S | FileCheck %s
; RUN: opt < %s -passes='function(loop(rotate),vpo-cfg-restructuring,vpo-paropt-prepare,simplify-cfg,loop(simplify-cfg),sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt'  -S | FileCheck %s

; Check that num_teams and thread_limit passed as Constant or
; by reference work properly.

; Original code:
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

; CHECK: define dso_local void @_Z4foo1x(i64 %n)
; CHECK: call i32 @__tgt_target_teams({{.*}}, i32 %{{.*}}, i32 42)

; CHECK: define dso_local void @_Z4foo2v()
; CHECK: call i32 @__tgt_target_teams({{.*}}, i32 %{{.*}}, i32 %{{.*}})

; CHECK: define dso_local void @_Z5foo1cx(i64 %n)
; CHECK: call i32 @__tgt_target_teams({{.*}}, i32 %{{.*}}, i32 42)

; CHECK: define dso_local void @_Z5foo2cv()
; CHECK: call i32 @__tgt_target_teams({{.*}}, i32 %{{.*}}, i32 %{{.*}})

; ModuleID = 'num_teams_thread_limit_by_ref.cpp'
source_filename = "num_teams_thread_limit_by_ref.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: nounwind uwtable
define dso_local void @_Z4foo1x(i64 %n) #0 {
entry:
  %n.addr = alloca i64, align 8
  store i64 %n, i64* %n.addr, align 8, !tbaa !6
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(i64* %n.addr) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS"(i64* %n.addr), "QUAL.OMP.THREAD_LIMIT"(i32 42) ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nounwind uwtable
define dso_local void @_Z4foo2v() #0 {
entry:
  %local = alloca i32, align 4
  %omp.clause.tmp = alloca i32, align 4
  %0 = bitcast i32* %local to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #1
  store i32 2, i32* %local, align 4, !tbaa !10
  %1 = load i32, i32* %local, align 4, !tbaa !10
  %add = add nsw i32 %1, 2
  store i32 %add, i32* %omp.clause.tmp, align 4, !tbaa !10
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1), "QUAL.OMP.FIRSTPRIVATE"(i32* %local), "QUAL.OMP.FIRSTPRIVATE"(i32* %omp.clause.tmp) ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS"(i32* %local), "QUAL.OMP.THREAD_LIMIT"(i32* %omp.clause.tmp) ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  %4 = bitcast i32* %local to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %4) #1
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #2

; Function Attrs: nounwind uwtable
define dso_local void @_Z5foo1cx(i64 %n) #0 {
entry:
  %n.addr = alloca i64, align 8
  %.capture_expr. = alloca i64, align 8
  store i64 %n, i64* %n.addr, align 8, !tbaa !6
  %0 = bitcast i64* %.capture_expr. to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %0) #1
  %1 = load i64, i64* %n.addr, align 8, !tbaa !6
  store i64 %1, i64* %.capture_expr., align 8, !tbaa !6
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2), "QUAL.OMP.FIRSTPRIVATE"(i64* %.capture_expr.) ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS"(i64* %.capture_expr.), "QUAL.OMP.THREAD_LIMIT"(i32 42) ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  %4 = bitcast i64* %.capture_expr. to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %4) #1
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @_Z5foo2cv() #0 {
entry:
  %local = alloca i32, align 4
  %.capture_expr. = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %0 = bitcast i32* %local to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #1
  store i32 2, i32* %local, align 4, !tbaa !10
  %1 = bitcast i32* %.capture_expr. to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #1
  %2 = load i32, i32* %local, align 4, !tbaa !10
  store i32 %2, i32* %.capture_expr., align 4, !tbaa !10
  %3 = bitcast i32* %.capture_expr.1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #1
  %4 = load i32, i32* %local, align 4, !tbaa !10
  %add = add nsw i32 %4, 2
  store i32 %add, i32* %.capture_expr.1, align 4, !tbaa !10
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 3), "QUAL.OMP.FIRSTPRIVATE"(i32* %.capture_expr.), "QUAL.OMP.FIRSTPRIVATE"(i32* %.capture_expr.1) ]
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS"(i32* %.capture_expr.), "QUAL.OMP.THREAD_LIMIT"(i32* %.capture_expr.1) ]
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TARGET"() ]
  %7 = bitcast i32* %.capture_expr.1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %7) #1
  %8 = bitcast i32* %.capture_expr. to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %8) #1
  %9 = bitcast i32* %local to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %9) #1
  ret void
}

attributes #0 = { nounwind uwtable "may-have-openmp-directive"="true" }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nounwind }

!omp_offload.info = !{!0, !1, !2, !3}
!llvm.module.flags = !{!4}

!0 = !{i32 0, i32 2052, i32 85986341, !"_Z4foo2v", i32 10, i32 1, i32 0}
!1 = !{i32 0, i32 2052, i32 85986341, !"_Z4foo1x", i32 3, i32 0, i32 0}
!2 = !{i32 0, i32 2052, i32 85986341, !"_Z5foo2cv", i32 22, i32 3, i32 0}
!3 = !{i32 0, i32 2052, i32 85986341, !"_Z5foo1cx", i32 16, i32 2, i32 0}
!4 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!7, !7, i64 0}
!7 = !{!"long long", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C++ TBAA"}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !8, i64 0}
