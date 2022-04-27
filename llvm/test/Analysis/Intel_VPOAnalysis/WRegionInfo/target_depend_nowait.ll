; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -disable-output %s 2>&1 | FileCheck %s

; Test src:
;
; #include <stdio.h>
;
; __attribute__((noinline)) void foo() {
;   double w = 1.0;
;   long x = 2;
;   short y = 3;
;
; #pragma omp target private(w) firstprivate(x) map(tofrom:y) depend(out:y) nowait
;       {
;         y = 10;
;       }
;   printf("y = %d\n", y);
; }
;
; CHECK: BEGIN TASK ID=1
; CHECK: TARGET_TASK: true
; CHECK: BEGIN TARGET ID=2
; CHECK: END TARGET ID=2
; CHECK: END TASK ID=1

; ModuleID = 'target_nowait.c'
source_filename = "target_nowait.c"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr constant [8 x i8] c"y = %d\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @foo() #0 {
entry:
  %w = alloca double, align 8
  %x = alloca i64, align 8
  %y = alloca i16, align 2
  store double 1.000000e+00, double* %w, align 8
  store i64 2, i64* %x, align 8
  store i16 3, i16* %y, align 2
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.TARGET.TASK"(), "QUAL.OMP.PRIVATE"(double* %w), "QUAL.OMP.FIRSTPRIVATE"(i64* %x), "QUAL.OMP.DEPEND.OUT"(i16* %y), "QUAL.OMP.SHARED"(i16* %y) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(double* %w), "QUAL.OMP.FIRSTPRIVATE"(i64* %x), "QUAL.OMP.MAP.TOFROM"(i16* %y), "QUAL.OMP.NOWAIT"() ]
  store i16 10, i16* %y, align 2
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]
  %2 = load i16, i16* %y, align 2
  %conv = sext i16 %2 to i32
  %call = call spir_func i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0), i32 %conv)
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local spir_func i32 @printf(i8*, ...) #2

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!4}

!0 = !{i32 0, i32 2051, i32 146390039, !"foo", i32 8, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"cl_doubles"}
!4 = !{!"clang version 9.0.0"}
