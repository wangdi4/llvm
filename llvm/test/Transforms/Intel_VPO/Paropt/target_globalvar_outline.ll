; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test Src:
;
; unsigned int NY = 100;
;
; void lbm_1() {
; #pragma omp target teams
;   {
;     NY += 2;
;     printf("NY= %d \n", NY); // should print 102
;   }
;   return;
; }
;
; int main() {
;   lbm_1();
;   return 0;
; }

; Teams Construct
; CHECK: define internal void @_Z5lbm_1v.DIR.OMP.TEAMS.{{.*}}(ptr %tid, ptr %bid, ptr %NY)
; CHECK: [[LOAD:%[0-9]+]] = load i32, ptr %NY, align 4
; CHECK: %add = add i32 [[LOAD]], 2
; CHECK: store i32 %add, ptr %NY, align 4

; Target Construct
; CHECK: define internal void @__omp_offloading_{{.*}}__Z5lbm_1v_l7(i64 {{.*}})
; CHECK: {{.*}}.fpriv = alloca i32, align 4

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64"
target device_triples = "x86_64"

@NY = external global i32, align 4
@.str = private unnamed_addr constant [9 x i8] c"NY= %d \0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define hidden void @_Z5lbm_1v() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @NY, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr @NY, i32 0, i32 1) ]

  %2 = load i32, ptr @NY, align 4, !tbaa !5
  %add = add i32 %2, 2
  store i32 %add, ptr @NY, align 4, !tbaa !5
  %3 = load i32, ptr @NY, align 4, !tbaa !5
  %call = call i32 (ptr, ...) @printf(ptr @.str, i32 %3) #1
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

declare i32 @printf(ptr, ...)


!omp_offload.info = !{!0, !1}

!0 = !{i32 0, i32 64768, i32 11346413, !"_Z5lbm_2v", i32 18, i32 1, i32 0}
!1 = !{i32 0, i32 64768, i32 11346413, !"_Z5lbm_1v", i32 7, i32 0, i32 0}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}
