; REQUIRES: asserts
; RUN: opt -vpo-paropt -debug-only=vpo-paropt-transform -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='vpo-paropt' -debug-only=vpo-paropt-transform -S %s 2>&1 | FileCheck %s

; Test Src:
; Compilation command:
; icpx -D__cdecl=" " -O0 -fiopenmp -std=c++11 test2.cpp -mllvm -print-after-all -mllvm -print-module-scope
; #include <stdio.h>
; int main(int argc, char *argv[]) {
;  double A[1000];
;
; #pragma omp parallel
;   {
; #pragma omp master
;     {
;       unsigned UB = 1000;
; #pragma omp taskloop
;       for (unsigned i = 0; i < UB; i++) {
;         A[i] = 1.0;
;       }
;     }
;   }
;   return 0;
; }
;

; ModuleID = 'test2.cpp'
source_filename = "test2.cpp"
target datalayout = "e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

%struct.ident_t = type { i32, i32, i32, i32, i8* }

@"@tid.addr" = external global i32
@.source.0.0 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0, i32 0, i32 0) }
@.source.0.0.1 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0.2 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0.1, i32 0, i32 0) }

; Function Attrs: noinline norecurse nounwind optnone
define dso_local i32 @main(i32 %argc, i8** %argv) #0 {
entry:
  %retval = alloca i32, align 4
  %argc.addr = alloca i32, align 4
  %argv.addr = alloca i8**, align 4
  %A = alloca [1000 x double], align 8
  %UB = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %i = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 %argc, i32* %argc.addr, align 4
  store i8** %argv, i8*** %argv.addr, align 4
  %end.dir.temp24 = alloca i1, align 1
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %entry
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"([1000 x double]* %A), "QUAL.OMP.PRIVATE"(i32* %UB), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.1), "QUAL.OMP.PRIVATE"(i32* %.omp.iv), "QUAL.OMP.PRIVATE"(i64* %.omp.lb), "QUAL.OMP.PRIVATE"(i64* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.0), "QUAL.OMP.PRIVATE"(i32* %tmp), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp24) ]
  br label %DIR.OMP.PARALLEL.327

DIR.OMP.PARALLEL.327:                             ; preds = %DIR.OMP.PARALLEL.2
  %temp.load25 = load volatile i1, i1* %end.dir.temp24, align 1
  %cmp26 = icmp ne i1 %temp.load25, false
  br i1 %cmp26, label %DIR.OMP.END.MASTER.9.split, label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %DIR.OMP.PARALLEL.327
  %my.tid = load i32, i32* @"@tid.addr", align 4
  %1 = call i32 @__kmpc_master(%struct.ident_t* @.kmpc_loc.0.0, i32 %my.tid)
  %2 = icmp eq i32 %1, 1
  br i1 %2, label %if.then.master.5, label %DIR.OMP.END.MASTER.9.split

if.then.master.5:                                 ; preds = %DIR.OMP.PARALLEL.3
  fence acquire
  store i32 1000, i32* %UB, align 4
  %3 = load i32, i32* %UB, align 4
  store i32 %3, i32* %.capture_expr.0, align 4
  %4 = load i32, i32* %.capture_expr.0, align 4
  %sub = sub i32 %4, 0
  %div = udiv i32 %sub, 1
  %sub1 = sub i32 %div, 1
  store i32 %sub1, i32* %.capture_expr.1, align 4
  %5 = load i32, i32* %.capture_expr.0, align 4
  %cmp = icmp ult i32 0, %5
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %if.then.master.5
  store i64 0, i64* %.omp.lb, align 8
  %6 = load i32, i32* %.capture_expr.1, align 4
  %conv = zext i32 %6 to i64
  store volatile i64 %conv, i64* %.omp.ub, align 8
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.TASKLOOP.4

DIR.OMP.TASKLOOP.4:                               ; preds = %omp.precond.then
  br label %DIR.OMP.TASKLOOP.5

DIR.OMP.TASKLOOP.5:                               ; preds = %DIR.OMP.TASKLOOP.4
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(), "QUAL.OMP.SHARED"([1000 x double]* %A), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i64* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]
  br label %DIR.OMP.TASKLOOP.6

; CHECK: addFirstprivateForNormalizedUB: Created Firstprivate Clause for NormalizedUB Var:   %.omp.ub = alloca i64, align 8
; CHECK: define internal void @main.DIR.OMP.TASKLOOP.5.split4.split(i32 %tid, %__struct.kmp_task_t_with_privates* %taskt.withprivates)

DIR.OMP.TASKLOOP.6:                               ; preds = %DIR.OMP.TASKLOOP.5
  %temp.load = load volatile i1, i1* %end.dir.temp, align 1
  %cmp11 = icmp ne i1 %temp.load, false
  br i1 %cmp11, label %omp.loop.exit.split, label %DIR.OMP.TASKLOOP.7

DIR.OMP.TASKLOOP.7:                               ; preds = %DIR.OMP.TASKLOOP.6
  %8 = load i64, i64* %.omp.lb, align 8
  %conv2 = trunc i64 %8 to i32
  store volatile i32 %conv2, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.TASKLOOP.7
  %9 = load volatile i32, i32* %.omp.iv, align 4
  %conv3 = zext i32 %9 to i64
  %10 = load volatile i64, i64* %.omp.ub, align 8
  %add = add i64 %10, 1
  %cmp4 = icmp ult i64 %conv3, %add
  br i1 %cmp4, label %omp.inner.for.body, label %omp.loop.exit.split.loopexit

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %11 = load volatile i32, i32* %.omp.iv, align 4
  %mul = mul i32 %11, 1
  %add5 = add i32 0, %mul
  store i32 %add5, i32* %i, align 4
  %12 = load i32, i32* %i, align 4
  %arrayidx = getelementptr inbounds [1000 x double], [1000 x double]* %A, i32 0, i32 %12
  store double 1.000000e+00, double* %arrayidx, align 8
  %13 = load volatile i32, i32* %.omp.iv, align 4
  %add6 = add nuw i32 %13, 1
  store volatile i32 %add6, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.loop.exit.split.loopexit:                     ; preds = %omp.inner.for.cond
  br label %omp.loop.exit.split

omp.loop.exit.split:                              ; preds = %omp.loop.exit.split.loopexit, %DIR.OMP.TASKLOOP.6
  br label %DIR.OMP.END.TASKLOOP.7

DIR.OMP.END.TASKLOOP.7:                           ; preds = %omp.loop.exit.split
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TASKLOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.TASKLOOP.7, %if.then.master.5
  fence release
  %my.tid12 = load i32, i32* @"@tid.addr", align 4
  call void @__kmpc_end_master(%struct.ident_t* @.kmpc_loc.0.0.2, i32 %my.tid12)
  br label %DIR.OMP.END.MASTER.9.split

DIR.OMP.END.MASTER.9.split:                       ; preds = %omp.precond.end, %DIR.OMP.PARALLEL.3, %DIR.OMP.PARALLEL.327
  br label %DIR.OMP.END.PARALLEL.8

DIR.OMP.END.PARALLEL.8:                           ; preds = %DIR.OMP.END.MASTER.9.split
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.9

DIR.OMP.END.PARALLEL.9:                           ; preds = %DIR.OMP.END.PARALLEL.8
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare i32 @__kmpc_master(%struct.ident_t*, i32)

declare void @__kmpc_end_master(%struct.ident_t*, i32)

attributes #0 = { noinline norecurse nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"NumRegisterParameters", i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 10.0.0"}
