; RUN: opt -vpo-paropt-tpv -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt-tpv' -S %s | FileCheck %s
;
; Make sure that vpo-paropt-tpv does not generate incorrect code
; for a test containing two threadprivate variables.
;
; Test src:
;
; #include <stdio.h>
;
; int i, j;
; #pragma omp threadprivate(i,j)
;
; int main() {
; #pragma omp parallel num_threads(1)
;   printf("i = %d, j = %d\n", i, j);
;
;   return 0;
; }
;
; CHECK: [[BC_I:%[0-9]+]] = bitcast i32* @i to i8*
; CHECK: [[TPV_CALL_I:%[0-9]+]] = call i8* @__kmpc_threadprivate_cached({{.*}}, i8* [[BC_I]], {{.*}})
; CHECK: store i8* [[TPV_CALL_I]], i8** [[TPV_ALLOCA_I:%[0-9]+]]

; CHECK: [[BC_J:%[0-9]+]] = bitcast i32* @j to i8*
; CHECK: [[TPV_CALL_J:%[0-9]+]] = call i8* @__kmpc_threadprivate_cached({{.*}}, i8* [[BC_J]], {{.*}})
; CHECK: store i8* [[TPV_CALL_J]], i8** [[TPV_ALLOCA_J:%[0-9]+]]

; CHECK: [[TPV_LOAD_I:%[0-9]+]] = load i8*, i8** [[TPV_ALLOCA_I]]
; CHECK: [[TPV_LOAD_CAST_I:%[0-9]+]] = bitcast i8* [[TPV_LOAD_I]] to i32*

; CHECK: [[TPV_LOAD_J:%[0-9]+]] = load i8*, i8** [[TPV_ALLOCA_J]]
; CHECK: [[TPV_LOAD_CAST_J:%[0-9]+]] = bitcast i8* [[TPV_LOAD_J]] to i32*

; CHECK: "DIR.OMP.PARALLEL"()
; CHECK-SAME: "QUAL.OMP.SHARED"(i32* [[TPV_LOAD_CAST_I]])
; CHECK-SAME: "QUAL.OMP.SHARED"(i32* [[TPV_LOAD_CAST_J]])

; ModuleID = 'thread_private_twovars.c'
source_filename = "thread_private_twovars.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@i = common dso_local thread_private global i32 0, align 4
@j = common dso_local thread_private global i32 0, align 4
@.str = private unnamed_addr constant [16 x i8] c"i = %d, j = %d\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.NUM_THREADS"(i32 1), "QUAL.OMP.SHARED"(i32* @i), "QUAL.OMP.SHARED"(i32* @j) ]
  %1 = load i32, i32* @i, align 4
  %2 = load i32, i32* @j, align 4
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str, i64 0, i64 0), i32 %1, i32 %2)
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
