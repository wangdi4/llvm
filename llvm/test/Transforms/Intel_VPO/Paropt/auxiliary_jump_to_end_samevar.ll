; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg -S < %s | FileCheck %s -check-prefix=SIMPL
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,simplify-cfg,loop(simplify-cfg))'  -S | FileCheck %s -check-prefix=SIMPL

; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S < %s | FileCheck %s -check-prefix=TFORM
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,simplify-cfg,loop(simplify-cfg),vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S | FileCheck %s -check-prefix=TFORM
;
; Test src:
;
; void bar(int x) {
; #pragma omp parallel
;   if (x) {
; #pragma omp parallel
;     ;
;   } else {
; #pragma omp parallel
;     ;
;   }
; }

; ModuleID = 'jump_to_end.c'
source_filename = "jump_to_end.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @bar(i32 %x) #0 {
entry:
  %x.addr = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(i32* %x.addr) ]
  %1 = load i32, i32* %x.addr, align 4
  %tobool = icmp ne i32 %1, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]
  br label %if.end


; Check that after simplifycfg, both inner parallel directives use the same vars
; for the auxiliary "jump.to.end.if" clause inserted in paropt-prepare.
; SIMPL: call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.JUMP.TO.END.IF"(i1* [[VAR1:%[^ ]+]]) ]
; SIMPL: call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.JUMP.TO.END.IF"(i1* [[VAR1]]) ]

; Check that after paropt transform, three fork_calls are present.
; TFORM: call {{.*}} @__kmpc_fork_call{{.*}}
; TFORM: call {{.*}} @__kmpc_fork_call{{.*}}
; TFORM: call {{.*}} @__kmpc_fork_call{{.*}}

if.else:                                          ; preds = %entry
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL"() ]
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
