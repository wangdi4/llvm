; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; The test is to make sure paropt does not crash when it
; encounters an indirect call.
;
; #include <stdio.h>
;
; extern int bar();
; void foo() {
;   int (*bar_ptr)(void) = bar;
;
; #pragma omp critical
;   printf("%d\n", bar_ptr());
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ident_t = type { i32, i32, i32, i32, ptr }

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@"@tid.addr" = external global i32
@.gomp_critical_user_.AS0.var = common global [8 x i32] zeroinitializer
@.source.0.0 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, ptr @.source.0.0 }
@.source.0.0.1 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0.2 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, ptr @.source.0.0.1 }

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %bar_ptr = alloca ptr, align 8
  store ptr @bar, ptr %bar_ptr, align 8
  br label %DIR.OMP.CRITICAL.1

DIR.OMP.CRITICAL.1:                               ; preds = %entry
  %my.tid = load i32, ptr @"@tid.addr", align 4
  call void @__kmpc_critical(ptr @.kmpc_loc.0.0, i32 %my.tid, ptr @.gomp_critical_user_.AS0.var)
; CHECK: call void @__kmpc_critical
  br label %DIR.OMP.CRITICAL.2

DIR.OMP.CRITICAL.2:                               ; preds = %DIR.OMP.CRITICAL.1
  fence acquire
  %0 = load ptr, ptr %bar_ptr, align 8
  %call = call i32 %0()
  %call1 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %call)
  fence release
  br label %DIR.OMP.END.CRITICAL.3

DIR.OMP.END.CRITICAL.3:                           ; preds = %DIR.OMP.CRITICAL.2
  %my.tid2 = load i32, ptr @"@tid.addr", align 4
  call void @__kmpc_end_critical(ptr @.kmpc_loc.0.0.2, i32 %my.tid2, ptr @.gomp_critical_user_.AS0.var)
; CHECK: call void @__kmpc_end_critical
  br label %DIR.OMP.END.CRITICAL.4

DIR.OMP.END.CRITICAL.4:                           ; preds = %DIR.OMP.END.CRITICAL.3
  ret void
}

declare dso_local i32 @bar(...) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare dso_local i32 @printf(ptr noundef, ...) #1

declare void @__kmpc_critical(ptr, i32, ptr)

declare void @__kmpc_end_critical(ptr, i32, ptr)

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
