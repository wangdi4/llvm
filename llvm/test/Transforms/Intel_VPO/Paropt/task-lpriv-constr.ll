; The given TASKLOOP directive has a clause lastprivate(f) where "f" is
; a constructible C++ object. Each task spawned by the taskloop must construct
; its own version of "f". The internal variable should be destructed at the
; end of the task, after any copy-out to the external scope.

; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes="function(vpo-cfg-restructuring),vpo-paropt" -S %s | FileCheck %s

; CHECK: define{{.*}}TASKLOOP
; CHECK-NOT: define
; CHECK: call{{.*}}def_constr
; CHECK: last.done
; CHECK: call{{.*}}omp.destr

; #include <stdio.h>
;
; class foo {
; public:
;   static int kount;
;   __attribute__((noinline)) foo() throw() {  i = kount++; }
;   ~foo() { printf("Destructed %d\n", i); }
;   __attribute__((noinline))  foo (const foo &f) {
;     i = 1000 + f.i;
;   }
;   int i;
; };
;
; int foo::kount = 0;
; int __attribute__ ((noinline)) Compute(void)
; {
;   foo f;
; #pragma omp parallel
;   {
; #pragma omp single
;   {
; #pragma omp taskloop lastprivate(f) num_tasks(4)
;     for (int i = 0; i < 4; i++)
;       f.i = i;
;   }
;   }
;   return f.i;
;   }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.foo = type { i32 }

$_ZN3fooC2Ev = comdat any

$_ZN3fooD2Ev = comdat any

$__clang_call_terminate = comdat any

@_ZN3foo5kountE = dso_local global i32 0, align 4
@.str = private unnamed_addr constant [15 x i8] c"Destructed %d\0A\00", align 1

define dso_local noundef i32 @_Z7Computev() {
entry:
  %f = alloca %class.foo, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  call void @_ZN3fooC2Ev(ptr noundef nonnull align 4 dereferenceable(4) %f) #2
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
     "QUAL.OMP.SHARED:TYPED"(ptr %f, %class.foo zeroinitializer, i32 1),
     "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
     "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
     "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
     "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
     "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"() ]

  fence acquire
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(),
    "QUAL.OMP.IMPLICIT"() ]

  store i32 0, ptr %.omp.lb, align 4
  store i32 3, ptr %.omp.ub, align 4
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(),
     "QUAL.OMP.LASTPRIVATE:NONPOD.TYPED"(ptr %f, %class.foo zeroinitializer, i32 1, ptr @_ZTS3foo.omp.def_constr, ptr @_ZTS3foo.omp.copy_assign, ptr @_ZTS3foo.omp.destr),
     "QUAL.OMP.NUM_TASKS"(i32 4),
     "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
     "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
     "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
     "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  %4 = load i32, ptr %.omp.lb, align 4
  store i32 %4, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %5 = load i32, ptr %.omp.iv, align 4
  %6 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %5, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %8 = load i32, ptr %i, align 4
  %i1 = getelementptr inbounds %class.foo, ptr %f, i32 0, i32 0
  store i32 %8, ptr %i1, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, ptr %.omp.iv, align 4
  %add2 = add nsw i32 %9, 1
  store i32 %add2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TASKLOOP"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TASKGROUP"() ]
  fence release
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SINGLE"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  %i3 = getelementptr inbounds %class.foo, ptr %f, i32 0, i32 0
  %10 = load i32, ptr %i3, align 4
  call void @_ZN3fooD2Ev(ptr noundef nonnull align 4 dereferenceable(4) %f) #2
  ret i32 %10
}

declare  dso_local void @_ZN3fooC2Ev(ptr noundef nonnull align 4 dereferenceable(4) %this)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

define internal noundef ptr @_ZTS3foo.omp.def_constr(ptr noundef %0) #3 section ".text.startup" {
entry:
  %.addr = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  %1 = load ptr, ptr %.addr, align 8
  call void @_ZN3fooC2Ev(ptr noundef nonnull align 4 dereferenceable(4) %1)
  ret ptr %1
}

define internal void @_ZTS3foo.omp.copy_assign(ptr noundef %0, ptr noundef %1) {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  store ptr %1, ptr %.addr1, align 8
  %2 = load ptr, ptr %.addr, align 8
  %3 = load ptr, ptr %.addr1, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %2, ptr align 4 %3, i64 4, i1 false)
  ret void
}

declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg)

define internal void @_ZTS3foo.omp.destr(ptr noundef %0) #3 section ".text.startup" {
entry:
  %.addr = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  %1 = load ptr, ptr %.addr, align 8
  call void @_ZN3fooD2Ev(ptr noundef nonnull align 4 dereferenceable(4) %1)
  ret void
}

declare dso_local void @_ZN3fooD2Ev(ptr noundef nonnull align 4 dereferenceable(4) %this)

declare dso_local i32 @printf(ptr noundef, ...)
