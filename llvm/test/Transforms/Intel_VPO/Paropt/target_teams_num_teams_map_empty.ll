; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Check that the map-type of %a is updated to 33 from 1, and it's
; passed to the kernel.
; 33 (PARAM | TO) is for %a, and 288 (PARAM | LITERAL) is for %no_teams.
; CHECK: @.offload_maptypes = {{.*}} [i64 33, i64 288]
; CHECK: call i32 @__tgt_target_teams_mapper(%struct.ident_t* {{[^,]+}}, i64 {{[^,]+}}, i8* @[[KERNEL:[^,.]+]].region_id, {{.*}})
; CHECK: call void @[[KERNEL]](i8* %a, i64 %no_teams{{.*}})

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

; Function Attrs: noinline nounwind optnone uwtable
declare i32 @bar() #0

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #1 {
entry:
  %a = alloca i8, align 1
  %no_teams = alloca i32, align 4
  %call = call i32 @bar()
  %conv = trunc i32 %call to i8
  store i8 %conv, i8* %a, align 1
  %0 = load i8, i8* %a, align 1
  %conv1 = sext i8 %0 to i32
  %cmp = icmp slt i32 %conv1, 1
  %1 = zext i1 %cmp to i64
  %cond = select i1 %cmp, i32 2, i32 3
  store i32 %cond, i32* %no_teams, align 4

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TO"(i8* %a, i8* %a, i64 1, i64 1, i8* null, i8* null), "QUAL.OMP.FIRSTPRIVATE"(i32* %no_teams) ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS"(i32* %no_teams) ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { noinline nounwind uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}

!0 = !{i32 0, i32 66309, i32 62784765, !"_Z4main", i32 14, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
