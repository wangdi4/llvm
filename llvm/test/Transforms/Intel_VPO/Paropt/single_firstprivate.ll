; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s
;
; Test src:
;
; #include <stdio.h>
;
; int main() {
;   int x = 1;
; #pragma omp parallel num_threads(1)
;   {
;     printf("parallel (before): x = %d (expected 1)\n", x);
; #pragma omp single firstprivate(x)
;     {
;       printf("single (before): x = %d (expected 1)\n", x);
;       x = 2;
;       printf("single (after): x = %d (expected 2)\n", x);
;     }
;     printf("parallel (after): x = %d (expected 1)\n", x);
;   }
;   return 0;
; }
;
; ModuleID = 'single_firstprivate.c'
source_filename = "single_firstprivate.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [40 x i8] c"parallel (before): x = %d (expected 1)\0A\00", align 1
@.str.1 = private unnamed_addr constant [38 x i8] c"single (before): x = %d (expected 1)\0A\00", align 1
@.str.2 = private unnamed_addr constant [37 x i8] c"single (after): x = %d (expected 2)\0A\00", align 1
@.str.3 = private unnamed_addr constant [39 x i8] c"parallel (after): x = %d (expected 1)\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 1, i32* %x, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.NUM_THREADS"(i32 1), "QUAL.OMP.SHARED"(i32* %x) ]
  %1 = load i32, i32* %x, align 4
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([40 x i8], [40 x i8]* @.str, i64 0, i64 0), i32 %1)

; Check that %x was privatized for the single region
; CHECK: "DIR.OMP.PARALLEL"
; CHECK: [[X_FPRIV:%[a-zA-Z._0-9]+]] = alloca i32
; CHECK: kmpc_single
; CHECK: store i32 2, i32* [[X_FPRIV]]
; CHECK: kmpc_end_single

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %x) ]
  fence acquire
  %3 = load i32, i32* %x, align 4
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([38 x i8], [38 x i8]* @.str.1, i64 0, i64 0), i32 %3)
  store i32 2, i32* %x, align 4
  %4 = load i32, i32* %x, align 4
  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([37 x i8], [37 x i8]* @.str.2, i64 0, i64 0), i32 %4)
  fence release
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SINGLE"() ]


  %5 = load i32, i32* %x, align 4
  %call3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @.str.3, i64 0, i64 0), i32 %5)
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
