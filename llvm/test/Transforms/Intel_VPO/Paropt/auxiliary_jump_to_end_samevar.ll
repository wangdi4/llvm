; RUN: opt -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
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

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; The IR for this test is the output of:
; opt -vpo-cfg-restructuring -vpo-paropt-ptrpare -simplifycfg.
; Note that after simplifycfg, both inner parallel directives use the same
; var "%end.dir.temp1" for the jump.to.end branch.

; Check that after paropt transform, three fork_calls are present.
; CHECK: call {{.*}} @__kmpc_fork_call{{.*}}
; CHECK: call {{.*}} @__kmpc_fork_call{{.*}}
; CHECK: call {{.*}} @__kmpc_fork_call{{.*}}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @bar(i32 %x) #0 {
entry:
  %x.addr = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  %x.addr.addr = alloca i32*, align 8
  store i32* %x.addr, i32** %x.addr.addr, align 8
  %end.dir.temp5 = alloca i1, align 1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(i32* %x.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %x.addr, i32** %x.addr.addr), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp5) ]
  %temp.load6 = load volatile i1, i1* %end.dir.temp5, align 1
  %cmp7 = icmp ne i1 %temp.load6, false
  br i1 %cmp7, label %DIR.OMP.END.PARALLEL.8.split, label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %entry
  %x.addr4 = load volatile i32*, i32** %x.addr.addr, align 8
  %1 = load i32, i32* %x.addr4, align 4
  %tobool = icmp ne i32 %1, 0
  %end.dir.temp1 = alloca i1, align 1
  br i1 %tobool, label %DIR.OMP.PARALLEL.4.split, label %DIR.OMP.PARALLEL.6.split

DIR.OMP.PARALLEL.4.split:                         ; preds = %DIR.OMP.PARALLEL.3
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp1) ]
  %temp.load2 = load volatile i1, i1* %end.dir.temp1, align 1
  br label %DIR.OMP.PARALLEL.5.split

DIR.OMP.PARALLEL.5.split:                         ; preds = %DIR.OMP.PARALLEL.4.split
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.8.split

DIR.OMP.PARALLEL.6.split:                         ; preds = %DIR.OMP.PARALLEL.3
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp1) ]
  %temp.load = load volatile i1, i1* %end.dir.temp1, align 1
  br label %DIR.OMP.PARALLEL.7.split

DIR.OMP.PARALLEL.7.split:                         ; preds = %DIR.OMP.PARALLEL.6.split
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.8.split

DIR.OMP.END.PARALLEL.8.split:                     ; preds = %DIR.OMP.PARALLEL.7.split, %DIR.OMP.PARALLEL.5.split, %entry
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
