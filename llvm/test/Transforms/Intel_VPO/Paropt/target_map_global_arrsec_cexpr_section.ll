; REQUIRES: asserts
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring),vpo-paropt' -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s

; Make sure Paropt can handle const-expr section-ptr operands which
; are also used inside the region.

; #include <stdio.h>
; int a[10];
;
; void foo() {
; #pragma omp target map(a[1:5])
;   printf("a[1] = %d\n", a[1]);
;   return;
; }

; CHECK: createRenamedValueForV : Renamed 'i32* getelementptr inbounds ([10 x i32], [10 x i32]* @a, i64 0, i64 1)' (via launder intrinsic) to: 'i32* %{{.*}}'.
; CHECK: createRenamedValueForV : Renamed '[10 x i32]* @a' (via launder intrinsic) to: '[10 x i32]* %a'.

; CHECK: clearLaunderIntrinBeforeRegion: Number of launder intrinsics for the region is 2.
; CHECK: clearLaunderIntrinBeforeRegion: Replacing launder intrinsic 'i8* %{{.+}}' with its operand.
; CHECK: clearLaunderIntrinBeforeRegion: Replacing launder intrinsic 'i8* %{{.+}}' with its operand.

; CHECK-NOT: call i8* @llvm.launder.invariant.group

; Check that globals @a is not used in the outlined function.
; CHECK: define internal void @__omp_offloading_{{.*}}foov{{.*}}([10 x i32]* %a)
; CHECK: %[[A1:[^ ]+]] = getelementptr inbounds [10 x i32], [10 x i32]* %a, i64 0, i64 1
; CHECK: %[[A1_LOAD:[^ ]+]] = load i32, i32* %0, align 4
; CHECK: call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str, i64 0, i64 0), i32 %[[A1_LOAD]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64"
target device_triples = "x86_64"

@a = external global [10 x i32], align 16
@.str = private unnamed_addr constant [11 x i8] c"a[1] = %d\0A\00", align 1

; Function Attrs: convergent noinline nounwind optnone uwtable mustprogress
define hidden void @_Z3foov() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"([10 x i32]* @a, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @a, i64 0, i64 1), i64 20, i64 35, i8* null, i8* null) ]
  %1 = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @a, i64 0, i64 1), align 4
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str, i64 0, i64 0), i32 %1) #3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: convergent
declare i32 @printf(i8*, ...) #2

attributes #0 = { convergent noinline nounwind optnone uwtable mustprogress "contains-openmp-target"="true" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { convergent "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}

!0 = !{i32 0, i32 66309, i32 64032526, !"_Z3foov", i32 5, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
