; RUN: opt -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:

; #include <omp.h>
; #include <stdio.h>
;
; typedef struct {
;   int a;
; } S;
;
; S s1;
;
; int main() {
;   s1.a = 111;
; //#pragma omp parallel
;   {
;     int tid = omp_get_thread_num();
; #pragma omp target firstprivate(s1) firstprivate(tid)
;     {
;       s1.a = tid;
; //#pragma omp parallel
;       { printf("a = %d, &a = %p\n", s1.a, &s1.a); }
;     }
;   }
; }

; Make sure that a local copy of %s1 is created within the outlined function
; for the target construct, even though there's an explicit map on it.

; CHECK:     define internal void @__omp_offloading_{{.*}}_Z4main{{.*}}(%struct.S* %s1{{.*}})
; CHECK:       [[S1FPRIV:%s1.fpriv]] = alloca %struct.S, align 1

; CHECK-DAG:   call i32 (i8*, ...) @printf({{.*}}, i32* noundef [[S1A:%[^ ]+]])
; CHECK-DAG:   [[S1A]] = getelementptr inbounds %struct.S, %struct.S* [[S1FPRIV]], i32 0, i32 0

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%struct.S = type { i32 }

@s1 = dso_local global %struct.S zeroinitializer, align 4
@.str = private unnamed_addr constant [17 x i8] c"a = %d, &a = %p\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %tid = alloca i32, align 4
  store i32 111, i32* getelementptr inbounds (%struct.S, %struct.S* @s1, i32 0, i32 0), align 4
  %call = call i32 @omp_get_thread_num() #2
  store i32 %call, i32* %tid, align 4

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
  "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
  "QUAL.OMP.FIRSTPRIVATE"(%struct.S* @s1),
  "QUAL.OMP.FIRSTPRIVATE"(i32* %tid),
  "QUAL.OMP.MAP.TO"(%struct.S* @s1, %struct.S* @s1, i64 4, i64 161, i8* null, i8* null) ]

  %1 = load i32, i32* %tid, align 4
  store i32 %1, i32* getelementptr inbounds (%struct.S, %struct.S* @s1, i32 0, i32 0), align 4
  %2 = load i32, i32* getelementptr inbounds (%struct.S, %struct.S* @s1, i32 0, i32 0), align 4
  %call1 = call i32 (i8*, ...) @printf(i8* noundef getelementptr inbounds ([17 x i8], [17 x i8]* @.str, i64 0, i64 0), i32 noundef %2, i32* noundef getelementptr inbounds (%struct.S, %struct.S* @s1, i32 0, i32 0)) #2

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local i32 @omp_get_thread_num() #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare dso_local i32 @printf(i8* noundef, ...) #3

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nounwind }
attributes #3 = { "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #4 = { noinline nounwind uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{i32 0, i32 57, i32 -698712660, !"_Z4main", i32 14, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 7, !"frame-pointer", i32 2}
