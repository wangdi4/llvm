; Checks that objects in the private clauses of tasks and taskloops,
; have their constructors and destructors called.

; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes="vpo-paropt" -S %s | FileCheck %s

; CHECK: define{{.*}}DIR.OMP.TASK.
; CHECK: call{{.*}}def_con{{.*}}
; CHECK: OMP.END.TASK.
; CHECK: call{{.*}}omp.destr{{.*}}

; CHECK: define{{.*}}TASKLOOP{{.*}}split
; CHECK: call{{.*}}def_con{{.*}}
; CHECK: for.body:
; CHECK: for.end
; CHECK: call{{.*}}omp.destr{{.*}}

; class c {
; public:
;   __attribute__((noinline)) c() { i = 5; }
;   __attribute__((noinline)) ~c() { i = -1; }
;   int i;
; };
;
; int foo() {
;   c c1;
; #pragma omp parallel
; #pragma omp single
;   {
; #pragma omp task private(c1)
;     c1.i = 4;
;   }
;
; #pragma omp taskloop private(c1)
;   for (int i = 0; i < 10; i++)
;     c1.i = 4;
;   return c1.i;
; }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.c = type { i32 }

@.str = private unnamed_addr constant [2 x i8] c"c\00", align 1
@.str.1 = private unnamed_addr constant [2 x i8] c"d\00", align 1

define dso_local i32 @_Z3foov() {
entry:
  %c1 = alloca %class.c, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %i2 = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %c1)
  call void @_ZN1cC2Ev(ptr %c1)
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %c1, %class.c zeroinitializer, i32 1) ]
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"() ]
  br label %DIR.OMP.SINGLE.3

DIR.OMP.SINGLE.3:                                 ; preds = %DIR.OMP.PARALLEL.2
  fence acquire
  br label %DIR.OMP.TASK.4

DIR.OMP.TASK.4:                                   ; preds = %DIR.OMP.SINGLE.3
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr %c1, %class.c zeroinitializer, i32 1, ptr @_ZTS1c.omp.def_constr, ptr @_ZTS1c.omp.destr) ]
  br label %DIR.OMP.TASK.5

DIR.OMP.TASK.5:                                   ; preds = %DIR.OMP.TASK.4
  store i32 4, ptr %c1, align 4
  br label %DIR.OMP.END.TASK.6

DIR.OMP.END.TASK.6:                               ; preds = %DIR.OMP.TASK.5
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TASK"() ]
  br label %DIR.OMP.END.TASK.7

DIR.OMP.END.TASK.7:                               ; preds = %DIR.OMP.END.TASK.6
  fence release
  br label %DIR.OMP.END.SINGLE.8

DIR.OMP.END.SINGLE.8:                             ; preds = %DIR.OMP.END.TASK.7
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SINGLE"() ]
  br label %DIR.OMP.END.SINGLE.9

DIR.OMP.END.SINGLE.9:                             ; preds = %DIR.OMP.END.SINGLE.8
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.10

DIR.OMP.END.PARALLEL.10:                          ; preds = %DIR.OMP.END.SINGLE.9
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv)
  call void @llvm.lifetime.start.p0(i64 8, ptr %.omp.lb)
  store i64 0, ptr %.omp.lb, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %.omp.ub)
  store i64 9, ptr %.omp.ub, align 8
  br label %DIR.OMP.TASKLOOP.11

DIR.OMP.TASKLOOP.11:                              ; preds = %DIR.OMP.END.PARALLEL.10
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(),
    "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr %c1, %class.c zeroinitializer, i32 1, ptr @_ZTS1c.omp.def_constr, ptr @_ZTS1c.omp.destr),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i64 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i64 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i2, i32 0, i32 1) ]
  br label %DIR.OMP.TASKLOOP.12

DIR.OMP.TASKLOOP.12:                              ; preds = %DIR.OMP.TASKLOOP.11
  %4 = load i64, ptr %.omp.lb, align 8
  %conv = trunc i64 %4 to i32
  store i32 %conv, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %DIR.OMP.TASKLOOP.12
  %5 = load i32, ptr %.omp.iv, align 4
  %conv1 = sext i32 %5 to i64
  %6 = load i64, ptr %.omp.ub, align 8
  %cmp = icmp ule i64 %conv1, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0(i64 4, ptr %i2)
  %7 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i2, align 4
  store i32 4, ptr %c1, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  call void @llvm.lifetime.end.p0(i64 4, ptr %i2)
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4
  %add4 = add nsw i32 %8, 1
  store i32 %add4, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TASKLOOP"() ]
  br label %DIR.OMP.END.TASKLOOP.13

DIR.OMP.END.TASKLOOP.13:                          ; preds = %omp.loop.exit
  call void @llvm.lifetime.end.p0(i64 8, ptr %.omp.ub)
  call void @llvm.lifetime.end.p0(i64 8, ptr %.omp.lb)
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv)
  %9 = load i32, ptr %c1, align 4
  call void @_ZN1cD2Ev(ptr %c1)
  call void @llvm.lifetime.end.p0(i64 4, ptr %c1)
  ret i32 %9
}

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)
declare void @_ZN1cC2Ev(ptr %this)
declare ptr @_ZTS1c.omp.def_constr(ptr)
declare void @_ZTS1c.omp.destr(ptr)
declare void @_ZN1cD2Ev(ptr %this)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)
declare dso_local i32 @printf(ptr, ...)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
