; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; This test checks that no extra alloca is created within the simd
; region as it is combined with a loop one hence a proper single
; destructor for that private is generated.
;
; Original code:
;void func (int* dst, int* src, int n)
;{
;    int i;
;    ClassA value;
;    #pragma omp for simd private (value)
;    for (i = 0; i < n; ++i) {
;        value.inc(i);
;        dst[i] += src[i] + value.m_value;
;    }
;}


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ClassA = type { i32 }

@_ZN6ClassA5a_cntE = dso_local local_unnamed_addr global i32 0, align 4
@_ZN6ClassA5d_cntE = dso_local local_unnamed_addr global i32 0, align 4

declare dso_local void @_ZN6ClassA3incEi(%struct.ClassA* nocapture nonnull dereferenceable(4) %this, i32 %a)

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)

declare dso_local void @_ZN6ClassAC1Ev(%struct.ClassA* nocapture nonnull dereferenceable(4) %this)
declare dso_local void @_ZN6ClassAD1Ev(%struct.ClassA* nocapture readnone %this)

declare nonnull %struct.ClassA* @_ZTS6ClassA.omp.def_constr(%struct.ClassA* nonnull returned %0)
declare void @_ZTS6ClassA.omp.destr(%struct.ClassA* nocapture readnone %0)

; CHECK-LABEL: void @func
; CHECK:   [[PRIV_VAL:%.+\.priv[0-9]*]] = alloca %struct.ClassA, align 4
; CHECK-NOT: {{%.+\.priv[0-9]*}} = alloca %struct.ClassA, align 4
; CHECK:   @llvm.directive.region.exit(token {{.*}}) [ "DIR.OMP.END.SIMD"() ]
; CHECK:   call void @_ZTS6ClassA.omp.destr(%struct.ClassA* [[PRIV_VAL]])

define dso_local void @func(i32* %dst, i32* %src, i32 %n) #2 {
entry:
  %dst.addr = alloca i32*, align 8
  %src.addr = alloca i32*, align 8
  %n.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %value = alloca %struct.ClassA, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32* %dst, i32** %dst.addr, align 8
  store i32* %src, i32** %src.addr, align 8
  store i32 %n, i32* %n.addr, align 4
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #4
  %1 = bitcast %struct.ClassA* %value to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #4
  call void @_ZN6ClassAC1Ev(%struct.ClassA* nonnull dereferenceable(4) %value)
  %2 = bitcast i32* %.capture_expr.0 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #4
  %3 = load i32, i32* %n.addr, align 4
  store i32 %3, i32* %.capture_expr.0, align 4
  %4 = bitcast i32* %.capture_expr.1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #4
  %5 = load i32, i32* %.capture_expr.0, align 4
  %sub = sub nsw i32 %5, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, i32* %.capture_expr.1, align 4
  %6 = load i32, i32* %.capture_expr.0, align 4
  %cmp = icmp slt i32 0, %6
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %7 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %7) #4
  %8 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %8) #4
  store i32 0, i32* %.omp.lb, align 4
  %9 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %9) #4
  %10 = load i32, i32* %.capture_expr.1, align 4
  store i32 %10, i32* %.omp.ub, align 4
  br label %DIR.OMP.LOOP.1

DIR.OMP.LOOP.1:                                   ; preds = %omp.precond.then
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.PRIVATE:NONPOD"(%struct.ClassA* %value, %struct.ClassA* (%struct.ClassA*)* @_ZTS6ClassA.omp.def_constr, void (%struct.ClassA*)* @_ZTS6ClassA.omp.destr), "QUAL.OMP.LASTPRIVATE"(i32* %i), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub) ]
  br label %DIR.OMP.LOOP.2

DIR.OMP.LOOP.2:                                   ; preds = %DIR.OMP.LOOP.1
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:NONPOD"(%struct.ClassA* %value, %struct.ClassA* (%struct.ClassA*)* @_ZTS6ClassA.omp.def_constr, void (%struct.ClassA*)* @_ZTS6ClassA.omp.destr), "QUAL.OMP.LINEAR:IV"(i32* %i, i32 1) ]
  br label %DIR.OMP.SIMD.3

DIR.OMP.SIMD.3:                                   ; preds = %DIR.OMP.LOOP.2
  %13 = load i32, i32* %.omp.lb, align 4
  store i32 %13, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %DIR.OMP.SIMD.3
  %14 = load i32, i32* %.omp.iv, align 4
  %15 = load i32, i32* %.omp.ub, align 4
  %cmp3 = icmp sle i32 %14, %15
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %16 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %16, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, i32* %i, align 4
  %17 = load i32, i32* %i, align 4
  call void @_ZN6ClassA3incEi(%struct.ClassA* nonnull dereferenceable(4) %value, i32 %17) #4
  %18 = load i32*, i32** %src.addr, align 8
  %19 = load i32, i32* %i, align 4
  %idxprom = sext i32 %19 to i64
  %ptridx = getelementptr inbounds i32, i32* %18, i64 %idxprom
  %20 = load i32, i32* %ptridx, align 4
  %m_value = getelementptr inbounds %struct.ClassA, %struct.ClassA* %value, i32 0, i32 0
  %21 = load i32, i32* %m_value, align 4
  %add5 = add nsw i32 %20, %21
  %22 = load i32*, i32** %dst.addr, align 8
  %23 = load i32, i32* %i, align 4
  %idxprom6 = sext i32 %23 to i64
  %ptridx7 = getelementptr inbounds i32, i32* %22, i64 %idxprom6
  %24 = load i32, i32* %ptridx7, align 4
  %add8 = add nsw i32 %24, %add5
  store i32 %add8, i32* %ptridx7, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %25 = load i32, i32* %.omp.iv, align 4
  %add9 = add nsw i32 %25, 1
  store i32 %add9, i32* %.omp.iv, align 4
  %26 = load i32, i32* %i, align 4
  %add10 = add nsw i32 %26, 1
  store i32 %add10, i32* %i, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %omp.loop.exit
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.4, %entry
  %27 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %27) #4
  %28 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %28) #4
  %29 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %29) #4
  %30 = bitcast i32* %.capture_expr.1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %30) #4
  %31 = bitcast i32* %.capture_expr.0 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %31) #4
  call void @_ZN6ClassAD1Ev(%struct.ClassA* nonnull dereferenceable(4) %value) #4
  %32 = bitcast %struct.ClassA* %value to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %32) #4
  %33 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %33) #4
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.0"}
