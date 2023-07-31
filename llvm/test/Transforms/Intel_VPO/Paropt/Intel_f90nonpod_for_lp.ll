; INTEL_CUSTOMIZATION
; RUN: opt -passes='function(vpo-cfg-restructuring,loop-simplify,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; The IR was modified from this source to use F90_NONPOD
; instead of NONPOD, and copy-constructor instead of a constructor,
; for the lastprivate operand c.

; Test src:

; #include <stdio.h>
; #include <stdlib.h>
; #include <cstring>
;
; struct C {
;   int a;
;   int *p;
;   // Constructor
;   C() {
;     p = (int *)calloc(5, sizeof(int));
;     printf("allocating %p\n", this);
;   }
;   // Copy constructor
;   C(const C& c1) {
;       a = c1.a;
;       p = (int*) calloc(5, sizeof(int));
;       printf("copy constructing %p\n", this);
;       std::memcpy(p, c1.p, 5 * sizeof(int));
;   }
;   // Copy assignment operator
;   C& operator=(const C& c1) {
;      a = c1.a;
;      std::memcpy(p, c1.p, 5 * sizeof(int));
;      printf("copy assigning to %p\n", this);
;      return *this;
;   }
;   // Destructor
;   ~C() {
;     free(p);
;     printf("freeing %p\n", this);
;   }
; };
;
;   C c;
; int main() {
;   c.p[1] = 100;
; //#pragma omp parallel num_threads(1)
; #pragma omp for lastprivate(c)
;   for (int i = 0; i < 1; i++) {
; //    printf("inside parallel: %d, %p, %p.\n", c.p[1], &c, &c.p[1]);
;     c.p[1] = 20;
; //    printf("inside parallel: %d, %p, %p.\n", c.p[1], &c, &c.p[1]);
;   }
;
; //  printf("outside parallel: %d, %p, %p.\n", c.p[1], &c, &c.p[1]);
; }

; CHECK: %[[CPRIV:c[^ ]+]] = alloca %struct.C, align 8
; CHECK: call void @_ZTS1C.omp.copy_constr(ptr %[[CPRIV]], ptr @c)
; CHECK: call void @__kmpc_for_static_init_4({{.*}})
; CHECK: call void @__kmpc_for_static_fini({{.*}})
; CHECK: call void @_ZTS1C.omp.copy_assign(ptr @c, ptr %[[CPRIV]])
; CHECK: call void @_ZTS1C.omp.destr(ptr %[[CPRIV]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.C = type { i32, ptr }

@c = dso_local global %struct.C zeroinitializer, align 8

define dso_local noundef i32 @main() {
entry:
  %retval = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  %0 = load ptr, ptr getelementptr inbounds (%struct.C, ptr @c, i32 0, i32 1), align 8
  %arrayidx = getelementptr inbounds i32, ptr %0, i64 1
  store i32 100, ptr %arrayidx, align 4
  store i32 0, ptr %.omp.lb, align 4
  store i32 0, ptr %.omp.ub, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.LASTPRIVATE:F90_NONPOD.TYPED"(ptr @c, %struct.C zeroinitializer, i32 1, ptr @_ZTS1C.omp.copy_constr, ptr @_ZTS1C.omp.copy_assign, ptr @_ZTS1C.omp.destr),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  %2 = load i32, ptr %.omp.lb, align 4
  store i32 %2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, ptr %.omp.iv, align 4
  %4 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %6 = load ptr, ptr getelementptr inbounds (%struct.C, ptr @c, i32 0, i32 1), align 8
  %arrayidx1 = getelementptr inbounds i32, ptr %6, i64 1
  store i32 20, ptr %arrayidx1, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, ptr %.omp.iv, align 4
  %add2 = add nsw i32 %7, 1
  store i32 %add2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.LOOP"() ]

  %8 = load i32, ptr %retval, align 4
  ret i32 %8
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @_ZTS1C.omp.copy_constr(ptr noundef %0, ptr noundef %1)

declare void @_ZTS1C.omp.copy_assign(ptr noundef %0, ptr noundef %1)

declare void @_ZTS1C.omp.destr(ptr noundef %0)
; end INTEL_CUSTOMIZATION
