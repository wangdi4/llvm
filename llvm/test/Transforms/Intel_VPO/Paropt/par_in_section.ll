; RUN: opt -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Pass condition: successful compilation. The test would fail if
; vpo-cfg-restructuring doesn't add an extra empty BBlock before the entry
; directive for the parallel construct, as loop recognition for the
; outer sections construct fails.
;
; CHECK: call void {{.*}} @__kmpc_fork_call

; Test src:
;
; #include <stdio.h>
; void foo() {
; #pragma omp sections
;   {
; #pragma omp section
;     {
; #pragma omp parallel
;       {
;         printf("a\n");
;       }
;     }
;
; #pragma omp section
;     {
;       printf("b\n");
;     }
;   }
; }
; ModuleID = 'par_in_section.c'
source_filename = "par_in_section.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [3 x i8] c"a\0A\00", align 1
@.str.1 = private unnamed_addr constant [3 x i8] c"b\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local void @foo() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTIONS"() ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i64 0, i64 0))
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SECTION"() ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.1, i64 0, i64 0))
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SECTION"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SECTIONS"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
