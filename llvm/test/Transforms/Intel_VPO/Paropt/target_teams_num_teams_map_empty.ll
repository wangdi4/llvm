; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Check that the map-type of %a is updated to 33 from 1, and it's
; passed to the kernel.
; 33 (PARAM | TO) is for %a, and 288 (PARAM | LITERAL) is for %no_teams.
; CHECK: @.offload_maptypes = {{.*}} [i64 33, i64 288]
; CHECK: call i32 @__tgt_target_teams_mapper(ptr {{[^,]+}}, i64 {{[^,]+}}, ptr @[[KERNEL:[^,.]+]].region_id, {{.*}})
; CHECK: call void @[[KERNEL]](ptr %a, i64 %no_teams{{.*}})

; Test src:
;
; #include <stdio.h>
; #include <string.h>
; int  b = 5;
; int bar()
; {
;   return b;
; }
; int main()
; {
;   char a;
;   a = bar();
;   const int no_teams = a  < 1 ? 2 : 3;
; #pragma omp target teams num_teams(no_teams) map(to:a)  // fails
; // #pragma omp target teams num_teams(5) map(to:a)      // works
; // #pragma omp target teams num_teams(no_teams)         // works
; // #pragma omp target map(to:a)                         // works
; // #pragma omp target teams map(to:a)                   // works
;   {}
; //  printf("a = %c\n", a);
; }
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

@b = dso_local global i32 5, align 4

define dso_local i32 @bar() {
entry:
  %0 = load i32, ptr @b, align 4
  ret i32 %0
}

define dso_local i32 @main() {
entry:
  %a = alloca i8, align 1
  %no_teams = alloca i32, align 4
  %call = call i32 @bar()
  %conv = trunc i32 %call to i8
  store i8 %conv, ptr %a, align 1
  %0 = load i8, ptr %a, align 1
  %conv1 = sext i8 %0 to i32
  %cmp = icmp slt i32 %conv1, 1
  %1 = zext i1 %cmp to i64
  %cond = select i1 %cmp, i32 2, i32 3
  store i32 %cond, ptr %no_teams, align 4
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TO"(ptr %a, ptr %a, i64 1, i64 1, ptr null, ptr null), ; MAP type: 1 = 0x1 = TO (0x1)
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %no_teams, i32 0, i32 1) ]

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.NUM_TEAMS:TYPED"(ptr %no_teams, i32 0) ]

  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry() 

declare void @llvm.directive.region.exit(token) 

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 64773, i32 3825455, !"_Z4main", i32 13, i32 0, i32 0, i32 0}
