; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; This test checks that no extra alloca is created within the simd
; region as it is combined with a loop one hence a proper single
; destructor for that private is generated.
;
; Original code:
; struct ClassA {
;  ClassA() {}
;  ~ClassA() {}
;  int m_value;
;  void inc(int a);
; };
;
; void func (int* dst, int* src, int n)
; {
;    int i;
;    ClassA value;
;    #pragma omp for simd private (value)
;    for (i = 0; i < n; ++i) {
;        value.inc(i);
;        dst[i] += src[i] + value.m_value;
;    }
; }

; CHECK-LABEL: void @func
; CHECK:   [[PRIV_VAL:%.+\.priv[0-9]*]] = alloca %struct.ClassA, align 4
; CHECK-NOT: {{%.+\.priv[0-9]*}} = alloca %struct.ClassA, align 4
; CHECK:   @llvm.directive.region.exit(token {{.*}}) [ "DIR.OMP.END.SIMD"() ]
; CHECK:   call void @_ZTS6ClassA.omp.destr(ptr [[PRIV_VAL]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ClassA = type { i32 }

declare dso_local void @_ZN6ClassA3incEi(ptr noundef nonnull align 4 dereferenceable(4), i32 noundef)

declare dso_local void @_ZN6ClassAC2Ev(ptr noundef nonnull align 4 dereferenceable(4) %this)
declare dso_local void @_ZN6ClassAD2Ev(ptr noundef nonnull align 4 dereferenceable(4) %this)

declare nonnull ptr @_ZTS6ClassA.omp.def_constr(ptr noundef %0)
declare void @_ZTS6ClassA.omp.destr(ptr noundef %0)

define dso_local void @func(ptr noundef %dst, ptr noundef %src, i32 noundef %n) {
entry:
  %dst.addr = alloca ptr, align 8
  %src.addr = alloca ptr, align 8
  %n.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %value = alloca %struct.ClassA, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store ptr %dst, ptr %dst.addr, align 8
  store ptr %src, ptr %src.addr, align 8
  store i32 %n, ptr %n.addr, align 4
  call void @_ZN6ClassAC2Ev(ptr noundef nonnull align 4 dereferenceable(4) %value)
  %0 = load i32, ptr %n.addr, align 4
  store i32 %0, ptr %.capture_expr.0, align 4
  %1 = load i32, ptr %.capture_expr.0, align 4
  %sub = sub nsw i32 %1, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.1, align 4
  %2 = load i32, ptr %.capture_expr.0, align 4
  %cmp = icmp slt i32 0, %2
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, ptr %.omp.lb, align 4
  %3 = load i32, ptr %.capture_expr.1, align 4
  store i32 %3, ptr %.omp.ub, align 4
  br label %DIR.OMP.LOOP.1

DIR.OMP.LOOP.1:                                   ; preds = %omp.precond.then
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr %value, %struct.ClassA zeroinitializer, i32 1, ptr @_ZTS6ClassA.omp.def_constr, ptr @_ZTS6ClassA.omp.destr),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0) ]

  br label %DIR.OMP.LOOP.2

DIR.OMP.LOOP.2:                                   ; preds = %DIR.OMP.LOOP.1
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr %value, %struct.ClassA zeroinitializer, i32 1, ptr @_ZTS6ClassA.omp.def_constr, ptr @_ZTS6ClassA.omp.destr),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i, i32 0, i32 1, i32 1) ]

  br label %DIR.OMP.SIMD.3

DIR.OMP.SIMD.3:                                   ; preds = %DIR.OMP.LOOP.2
  %6 = load i32, ptr %.omp.lb, align 4
  store i32 %6, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %DIR.OMP.SIMD.3
  %7 = load i32, ptr %.omp.iv, align 4
  %8 = load i32, ptr %.omp.ub, align 4
  %cmp3 = icmp sle i32 %7, %8
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %9, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, ptr %i, align 4
  %10 = load i32, ptr %i, align 4
  call void @_ZN6ClassA3incEi(ptr noundef nonnull align 4 dereferenceable(4) %value, i32 noundef %10) #2
  %11 = load ptr, ptr %src.addr, align 8
  %12 = load i32, ptr %i, align 4
  %idxprom = sext i32 %12 to i64
  %arrayidx = getelementptr inbounds i32, ptr %11, i64 %idxprom
  %13 = load i32, ptr %arrayidx, align 4
  %m_value = getelementptr inbounds %struct.ClassA, ptr %value, i32 0, i32 0
  %14 = load i32, ptr %m_value, align 4
  %add5 = add nsw i32 %13, %14
  %15 = load ptr, ptr %dst.addr, align 8
  %16 = load i32, ptr %i, align 4
  %idxprom6 = sext i32 %16 to i64
  %arrayidx7 = getelementptr inbounds i32, ptr %15, i64 %idxprom6
  %17 = load i32, ptr %arrayidx7, align 4
  %add8 = add nsw i32 %17, %add5
  store i32 %add8, ptr %arrayidx7, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %18 = load i32, ptr %.omp.iv, align 4
  %add9 = add nsw i32 %18, 1
  store i32 %add9, ptr %.omp.iv, align 4
  %19 = load i32, ptr %i, align 4
  %add10 = add nsw i32 %19, 1
  store i32 %add10, ptr %i, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.SIMD"() ]

  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %omp.loop.exit
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.LOOP"() ]

  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.4, %entry
  call void @_ZN6ClassAD2Ev(ptr noundef nonnull align 4 dereferenceable(4) %value) #2
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

