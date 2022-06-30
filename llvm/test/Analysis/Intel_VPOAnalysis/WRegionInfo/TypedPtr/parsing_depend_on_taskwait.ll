; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-wrncollection -analyze -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(print<vpo-wrncollection>)' -debug -S %s 2>&1 | FileCheck %s
;
; SRC:
; #include <omp.h>
; #include <stdio.h>
;
; int main() {
;  int Result[10];
;  int a;
;  #pragma omp taskwait
;   printf("Result = %d .... \n", Result[0]);
;}
;
;This test checks that we are parsing the depend clause on taskwait
;The input IR was hand-modified because front end doesn't yet handle the Depend clause on taskwait
;CHECK: DEPEND clause (size=1): (i32* %{{.*}})
;
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [19 x i8] c"Result = %d .... \0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %Result = alloca [10 x i32], align 16
  %a = alloca i32, align 4
  br label %DIR.OMP.TASKWAIT.1

DIR.OMP.TASKWAIT.1:                               ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKWAIT"(), "QUAL.OMP.DEPEND.IN"(i32* %a) ]
  br label %DIR.OMP.TASKWAIT.2

DIR.OMP.TASKWAIT.2:                               ; preds = %DIR.OMP.TASKWAIT.1
  fence acq_rel
  br label %DIR.OMP.END.TASKWAIT.3

DIR.OMP.END.TASKWAIT.3:                           ; preds = %DIR.OMP.TASKWAIT.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASKWAIT"() ]
  br label %DIR.OMP.END.TASKWAIT.4

DIR.OMP.END.TASKWAIT.4:                           ; preds = %DIR.OMP.END.TASKWAIT.3
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* %Result, i64 0, i64 0
  %1 = load i32, i32* %arrayidx, align 16
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([19 x i8], [19 x i8]* @.str, i64 0, i64 0), i32 %1)
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

attributes #0 = { noinline nounwind optnone uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"clang version 9.0.0"}
