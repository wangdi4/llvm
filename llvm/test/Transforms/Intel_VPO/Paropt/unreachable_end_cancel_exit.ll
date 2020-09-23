; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S < %s | FileCheck %s -check-prefix=PREPR
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)'  -S | FileCheck %s -check-prefix=PREPR

; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg -S < %s | FileCheck %s -check-prefix=SIMPL
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplifycfg,loop(loop-simplifycfg))'  -S | FileCheck %s -check-prefix=SIMPL

; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S < %s | FileCheck %s -check-prefix=TFORM
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplifycfg,loop(loop-simplifycfg),vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S | FileCheck %s -check-prefix=TFORM

; Test src:
;
; #include <stdio.h>
; #include <omp.h>
;
; void foo() {
; #pragma omp parallel num_threads(4)
;   {
;
;     int i = omp_get_thread_num();
;   //  printf("%d: before\n", i);
;
; #pragma omp cancel parallel
;     exit(1);
;
;     printf("%d: after\n", i);
;   }
; }
;
; /*int main() {
;   foo();
;   return 0;
; }*/
;
; ModuleID = 'cancel_par_exit.c'
source_filename = "cancel_par_exit.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [11 x i8] c"%d: after\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %i = alloca i32, align 4

; Check for a branch emitted from begin to end directive after vpo-paropt-prepare phase.
; PREPR: call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"()
; PREPR-SAME: "QUAL.OMP.JUMP.TO.END.IF"(i1* [[BRANCH_TEMP:%[^ ,]+]])
; PREPR-NEXT: [[TEMP_LOAD:%[^ ]+]] = load volatile i1, i1* [[BRANCH_TEMP]]
; PREPR-NEXT: [[CMP:%[^ ]+]] = icmp ne i1 [[TEMP_LOAD]], false
; PREPR-NEXT: br i1 [[CMP]], label %[[END_LABEL:[^ ,]+]], label %{{.*}}
; PREPR: [[END_LABEL]]:
; PREPR-NEXT: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.PARALLEL"() ]


; After simplifycfg, there should be no printf left in the IR as it's unreachable,
; but the end region directive should still be present.
; SIMPL-NOT: call i32 (i8*, ...) @printf
; SIMPL: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.PARALLEL"() ]

; After transform, there should be no directives left, and if we don't comp-fail,
; then it means we're fine.
; TFORM-NOT:  call token @llvm.directive.region.entry()
; TFORM: call void {{.+}} @__kmpc_fork_call
; TFORM: call i32 @__kmpc_cancel

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.NUM_THREADS"(i32 4), "QUAL.OMP.PRIVATE"(i32* %i) ]

  %call = call i32 @omp_get_thread_num() #1
  store i32 %call, i32* %i, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCEL"(), "QUAL.OMP.CANCEL.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.CANCEL"() ]
  call void @exit(i32 1) #5
  %2 = load i32, i32* %i, align 4
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str, i64 0, i64 0), i32 %2)

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nounwind
declare dso_local i32 @omp_get_thread_num() #2

; Function Attrs: noreturn nounwind
declare dso_local void @exit(i32) #3

declare dso_local i32 @printf(i8*, ...) #4

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noreturn nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { noreturn nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
