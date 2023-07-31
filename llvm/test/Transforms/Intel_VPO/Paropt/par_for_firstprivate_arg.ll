; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S %s | FileCheck %s

; This file checks the support of the firstprivate which is a reference.

; CHECK-LABEL: define internal void @"_ZZN9my_struct4workERA12_sENK3$_0clEv"
; CHECK: %{{[a-zA-Z._0-9]+}} = alloca [12 x i16]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }
%"class._ZTSZN9my_struct4workERA12_sE3$_0" = type { ptr, ptr }
%struct.my_struct = type { [12 x i16] }

@_ZSt8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@__num_instances = dso_local global i32 0, align 4
@__num_initializer_calls = dso_local global i32 0, align 4
@_Z1x = internal global [12 x i16] zeroinitializer, align 16
@_Z10y_original = internal global [12 x i16] zeroinitializer, align 16
@_Z10y_expected = internal global [12 x i16] zeroinitializer, align 16
@_Z1z = internal global [12 x i16] zeroinitializer, align 16
@_Z10z_expected = internal global [12 x i16] zeroinitializer, align 16
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @_GLOBAL__sub_I_a.cpp, ptr null }]
@"@tid.addr" = external global i32

define internal void @__cxx_global_var_init() {
entry:
  call void @_ZNSt8ios_base4InitC1Ev(ptr @_ZSt8__ioinit)
  %0 = call i32 @__cxa_atexit(ptr @_ZNSt8ios_base4InitD1Ev, ptr @_ZSt8__ioinit, ptr @__dso_handle)
  ret void
}

declare dso_local void @_ZNSt8ios_base4InitC1Ev(ptr) unnamed_addr

declare dso_local void @_ZNSt8ios_base4InitD1Ev(ptr) unnamed_addr

declare dso_local i32 @__cxa_atexit(ptr, ptr, ptr)

define dso_local void @_Z4initPs(ptr %y) {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp ult i32 %i.0, 12
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %conv = trunc i32 %i.0 to i16
  %idxprom = zext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds [12 x i16], ptr @_Z1x, i64 0, i64 %idxprom
  store i16 %conv, ptr %arrayidx, align 2
  %mul = mul i32 2, %i.0
  %conv1 = trunc i32 %mul to i16
  %arrayidx3 = getelementptr inbounds i16, ptr %y, i64 %idxprom
  store i16 %conv1, ptr %arrayidx3, align 2
  %arrayidx7 = getelementptr inbounds [12 x i16], ptr @_Z10y_original, i64 0, i64 %idxprom
  store i16 %conv1, ptr %arrayidx7, align 2
  %0 = load i16, ptr %arrayidx3, align 2
  %arrayidx11 = getelementptr inbounds [12 x i16], ptr @_Z10y_expected, i64 0, i64 %idxprom
  store i16 %0, ptr %arrayidx11, align 2
  %arrayidx13 = getelementptr inbounds [12 x i16], ptr @_Z1z, i64 0, i64 %idxprom
  store i16 1, ptr %arrayidx13, align 2
  %arrayidx17 = getelementptr inbounds [12 x i16], ptr @_Z10z_expected, i64 0, i64 %idxprom
  store i16 1, ptr %arrayidx17, align 2
  %inc = add i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  br label %for.cond19

for.cond19:                                       ; preds = %for.body21, %for.end
  %i18.0 = phi i32 [ 3, %for.end ], [ %inc27, %for.body21 ]
  %cmp20 = icmp slt i32 %i18.0, 6
  br i1 %cmp20, label %for.body21, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond19
  ret void

for.body21:                                       ; preds = %for.cond19
  %idxprom22 = sext i32 %i18.0 to i64
  %arrayidx23 = getelementptr inbounds i16, ptr %y, i64 %idxprom22
  %1 = load i16, ptr %arrayidx23, align 2
  %arrayidx25 = getelementptr inbounds [12 x i16], ptr @_Z10z_expected, i64 0, i64 %idxprom22
  store i16 %1, ptr %arrayidx25, align 2
  %inc27 = add nsw i32 %i18.0, 1
  br label %for.cond19
}

define dso_local void @_ZN9my_struct4workERA12_s(ptr %this, ptr dereferenceable(24) %x) align 2 {
entry:
  %foo = alloca %"class._ZTSZN9my_struct4workERA12_sE3$_0", align 8
  %0 = bitcast ptr %foo to ptr
  call void @llvm.lifetime.start.p0(i64 16, ptr %0)
  %1 = getelementptr inbounds %"class._ZTSZN9my_struct4workERA12_sE3$_0", ptr %foo, i32 0, i32 0
  store ptr %this, ptr %1, align 8
  %2 = getelementptr inbounds %"class._ZTSZN9my_struct4workERA12_sE3$_0", ptr %foo, i32 0, i32 1
  store ptr %x, ptr %2, align 8
  call void @"_ZZN9my_struct4workERA12_sENK3$_0clEv"(ptr %foo)
  call void @llvm.lifetime.end.p0(i64 16, ptr %0)
  ret void
}

define internal void @"_ZZN9my_struct4workERA12_sENK3$_0clEv"(ptr %this) align 2 {
entry:
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %0 = getelementptr inbounds %"class._ZTSZN9my_struct4workERA12_sE3$_0", ptr %this, i32 0, i32 0
  %1 = load ptr, ptr %0, align 8
  %y2 = getelementptr inbounds %struct.my_struct, ptr %1, i32 0, i32 0
  %2 = getelementptr inbounds %"class._ZTSZN9my_struct4workERA12_sE3$_0", ptr %this, i32 0, i32 1
  %3 = bitcast ptr %.omp.iv to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %3)
  %4 = bitcast ptr %.omp.lb to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %4)
  store i32 0, ptr %.omp.lb, align 4
  %5 = bitcast ptr %.omp.ub to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %5)
  store i32 2, ptr %.omp.ub, align 4
  br label %DIR.OMP.LOOP.1

DIR.OMP.LOOP.1:                                   ; preds = %entry
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y2, [12 x i16] zeroinitializer, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  br label %DIR.OMP.LOOP.11

DIR.OMP.LOOP.11:                                  ; preds = %DIR.OMP.LOOP.1
  %7 = load i32, ptr %.omp.lb, align 4
  store volatile i32 %7, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.LOOP.11
  %8 = load volatile i32, ptr %.omp.iv, align 4
  %9 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %8, %9
  br i1 %cmp, label %omp.inner.for.body, label %omp.loop.exit

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = bitcast ptr %i to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %10)
  %11 = load volatile i32, ptr %.omp.iv, align 4
  %add = add nsw i32 3, %11
  store i32 %add, ptr %i, align 4
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds [12 x i16], ptr %y2, i64 0, i64 %idxprom
  %12 = load i16, ptr %arrayidx, align 2
  %arrayidx6 = getelementptr inbounds [12 x i16], ptr @_Z1z, i64 0, i64 %idxprom
  store i16 %12, ptr %arrayidx6, align 2
  %13 = load ptr, ptr %2, align 8
  %14 = load i32, ptr %i, align 4
  %idxprom7 = sext i32 %14 to i64
  %arrayidx8 = getelementptr inbounds [12 x i16], ptr %13, i64 0, i64 %idxprom7
  %15 = load i16, ptr %arrayidx8, align 2
  %arrayidx10 = getelementptr inbounds [12 x i16], ptr %y2, i64 0, i64 %idxprom7
  store i16 %15, ptr %arrayidx10, align 2
  call void @llvm.lifetime.end.p0(i64 4, ptr %10)
  %16 = load volatile i32, ptr %.omp.iv, align 4
  %add11 = add nsw i32 %16, 1
  store volatile i32 %add11, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.LOOP"() ]

  br label %DIR.OMP.END.LOOP.2

DIR.OMP.END.LOOP.2:                               ; preds = %omp.loop.exit
  call void @llvm.lifetime.end.p0(i64 4, ptr %5)
  call void @llvm.lifetime.end.p0(i64 4, ptr %4)
  call void @llvm.lifetime.end.p0(i64 4, ptr %3)
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @_GLOBAL__sub_I_a.cpp()

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)

declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)
