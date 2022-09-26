; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -pass-remarks-output=%t -S %s
; RUN: FileCheck --input-file %t %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -pass-remarks-output=%t -S %s
; RUN: FileCheck --input-file %t %s

; Test src:
;
; #include <stdio.h>
; #include <omp.h>
;
; void foo(int *a, int *b, int *c) {
;   int i;
; #pragma omp parallel
;   {
;     a[i] = b[i] + c[i];
;
; #pragma omp master
;     printf("tid = %d\n", omp_get_thread_num());
;   }
; }

; Test to check that opt-report messages are printed for VPO OpenMP.

; CHECK: Pass:{{[ ]*}}openmp
; CHECK: Construct:{{[ ]*}}masked
; CHECK: String:{{[ ]*}}' construct transformed'
; CHECK: Pass:{{[ ]*}}openmp
; CHECK: Construct:{{[ ]*}}parallel
; CHECK: String:{{[ ]*}}' construct transformed'

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [10 x i8] c"tid = %d\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(ptr noundef %a, ptr noundef %b, ptr noundef %c) #0 {
entry:
  %a.addr = alloca ptr, align 8
  %b.addr = alloca ptr, align 8
  %c.addr = alloca ptr, align 8
  %i = alloca i32, align 4
  store ptr %a, ptr %a.addr, align 8
  store ptr %b, ptr %b.addr, align 8
  store ptr %c, ptr %c.addr, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %a.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %b.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %c.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %i, i32 0, i32 1) ]
  %1 = load ptr, ptr %b.addr, align 8
  %2 = load i32, ptr %i, align 4
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds i32, ptr %1, i64 %idxprom
  %3 = load i32, ptr %arrayidx, align 4
  %4 = load ptr, ptr %c.addr, align 8
  %5 = load i32, ptr %i, align 4
  %idxprom1 = sext i32 %5 to i64
  %arrayidx2 = getelementptr inbounds i32, ptr %4, i64 %idxprom1
  %6 = load i32, ptr %arrayidx2, align 4
  %add = add nsw i32 %3, %6
  %7 = load ptr, ptr %a.addr, align 8
  %8 = load i32, ptr %i, align 4
  %idxprom3 = sext i32 %8 to i64
  %arrayidx4 = getelementptr inbounds i32, ptr %7, i64 %idxprom3
  store i32 %add, ptr %arrayidx4, align 4
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASTER"() ]
  fence acquire
  %call = call i32 @omp_get_thread_num() #1
  %call5 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %call) #1
  fence release
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.MASTER"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(ptr noundef, ...) #2

; Function Attrs: nounwind
declare dso_local i32 @omp_get_thread_num() #3

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
