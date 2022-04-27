; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-wrncollection -analyze -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(print<vpo-wrncollection>)' -debug -S %s 2>&1 | FileCheck %s
;
; SRC code :
;
;#include<omp.h>
; void test() {
; #pragma omp critical(xaxis) hint(omp_lock_hint_contended)
;     ;
; }
;
;check that we are parsing the Hint clause
;CHECK: HINT: 2

; ModuleID = 'test.ll'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @test() #0 {
entry:
  br label %DIR.OMP.CRITICAL.1

DIR.OMP.CRITICAL.1:                               ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.CRITICAL"(), "QUAL.OMP.NAME"([5 x i8] c"xaxis"), "QUAL.OMP.HINT"(i32 2) ]
  br label %DIR.OMP.CRITICAL.2

DIR.OMP.CRITICAL.2:                               ; preds = %DIR.OMP.CRITICAL.1
  fence acquire
  fence release
  br label %DIR.OMP.END.CRITICAL.3

DIR.OMP.END.CRITICAL.3:                           ; preds = %DIR.OMP.CRITICAL.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.CRITICAL"() ]
  br label %DIR.OMP.END.CRITICAL.4

DIR.OMP.END.CRITICAL.4:                           ; preds = %DIR.OMP.END.CRITICAL.3
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"clang version 9.0.0"}
