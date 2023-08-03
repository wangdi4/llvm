; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
;
; It checks whether the construt omp target team is supported in the OMP
; codegen.
; void bar();
; void foo(int n) {
;   #pragma omp target
;   #pragma omp teams num_teams(n)
;   {
;     bar();
;   }
; }
; CHECK:  call i32 @__tgt_target_teams({{.*}})

target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local void @foo(i32 noundef %n) {
entry:
  %n.addr = alloca i32, align 4
  store i32 %n, ptr %n.addr, align 4
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %n.addr, i32 0, i32 1) ]

  br label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %DIR.OMP.TARGET.2
  br label %DIR.OMP.TEAMS.4

DIR.OMP.TEAMS.4:                                  ; preds = %DIR.OMP.TARGET.3
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.NUM_TEAMS:TYPED"(ptr %n.addr, i32 0) ]

  br label %DIR.OMP.TEAMS.5

DIR.OMP.TEAMS.5:                                  ; preds = %DIR.OMP.TEAMS.4
  call void (...) @bar() #1
  br label %DIR.OMP.END.TEAMS.6

DIR.OMP.END.TEAMS.6:                              ; preds = %DIR.OMP.TEAMS.5
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  br label %DIR.OMP.END.TEAMS.7

DIR.OMP.END.TEAMS.7:                              ; preds = %DIR.OMP.END.TEAMS.6
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  br label %DIR.OMP.END.TARGET.8

DIR.OMP.END.TARGET.8:                             ; preds = %DIR.OMP.END.TEAMS.7
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local void @bar(...) 

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 64773, i32 3825464, !"_Z3foo", i32 3, i32 0, i32 0, i32 0}
