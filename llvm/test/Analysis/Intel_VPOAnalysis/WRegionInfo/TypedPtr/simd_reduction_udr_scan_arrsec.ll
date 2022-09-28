; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s

; Test src:

; #include <cstdio>
;
; class C {
; public:
;   int a = 0;
; };
;
; void my_comb(C &out, C &in) { out.a += in.a; };
; void my_init(C &priv, C &orig) { priv.a = orig.a; }
;
; #pragma omp declare reduction(my_add:C                                         \
;                               : my_comb(omp_out, omp_in))                      \
;     initializer(my_init(omp_priv, omp_orig))
;
; C p[10];
; C q[10];
; int y;
;
; void foo(C (&x)[10]) {
; #pragma omp simd reduction(inscan, my_add : x[2:5]) reduction(inscan,+: y)
;   for (int i = 0; i < 10; i++) {
;     my_comb(x[5], p[i]);
;     y += p[i].a;
;
; #pragma omp scan inclusive(x[2:5], y)
;     my_init(q[i], x[5]);
;     q[i].a = y;
;   }
; }

; The IR is a hand-modified version of the above test without scan/incan.

; CHECK: BEGIN SIMD ID=1 {
; ...
; CHECK:   REDUCTION-INSCAN maps: (1: INCLUSIVE) (2: INCLUSIVE)
; CHECK:   REDUCTION clause (size=2): (UDR: BYREF([10 x %class.C]** %x.addr) INSCAN<1> ARRAY SECTION INFO: ( Dims: ( i64 2, i64 5, i64 1 ))) (ADD: i32* @y INSCAN<2>)
; CHECK:   LINEAR clause (size=1): IV(i32* %i, i32 1)
; ...
; CHECK:   BEGIN SCAN ID=2 {
; ...
; CHECK:    INSCAN-REDUCTION maps: (2: ADD) (1: UDR)
; CHECK:    INCLUSIVE clause (size=2): (i32* @y INSCAN<2>) ([10 x %class.C]** %x.addr INSCAN<1>)
; CHECK:    EXCLUSIVE clause: UNSPECIFIED
; ...
; CHECK:  } END SCAN ID=2
;
; CHECK: } END SIMD ID=1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.C = type { i32 }

@p = dso_local global [10 x %class.C] zeroinitializer, align 16
@q = dso_local global [10 x %class.C] zeroinitializer, align 16
@y = dso_local global i32 0, align 4

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z3fooRA10_1C([10 x %class.C]* noundef nonnull align 4 dereferenceable(40) %x) #1 {
entry:
  %x.addr = alloca [10 x %class.C]*, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store [10 x %class.C]* %x, [10 x %class.C]** %x.addr, align 8
  store i32 9, i32* %.omp.ub, align 4
  %0 = load [10 x %class.C]*, [10 x %class.C]** %x.addr, align 8

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
  "QUAL.OMP.REDUCTION.UDR:BYREF.ARRSECT.INSCAN"([10 x %class.C]** %x.addr, i64 1, i64 2, i64 5, i64 1, i8* null, i8* null, void (%class.C*, %class.C*)* @.omp_combiner., void (%class.C*, %class.C*)* @.omp_initializer., i64 1), ;inscan idx = 1
  "QUAL.OMP.REDUCTION.ADD:INSCAN"(i32* @y, i64 2), ; inscan idx = 2
  "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv),
  "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub),
  "QUAL.OMP.LINEAR:IV"(i32* %i, i32 1) ]

  store i32 0, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, i32* %.omp.iv, align 4
  %3 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %5 = load [10 x %class.C]*, [10 x %class.C]** %x.addr, align 8
  %arrayidx = getelementptr inbounds [10 x %class.C], [10 x %class.C]* %5, i64 0, i64 5
  %6 = load i32, i32* %i, align 4
  %idxprom = sext i32 %6 to i64
  %arrayidx1 = getelementptr inbounds [10 x %class.C], [10 x %class.C]* @p, i64 0, i64 %idxprom
  call void @_Z7my_combR1CS0_(%class.C* noundef nonnull align 4 dereferenceable(4) %arrayidx, %class.C* noundef nonnull align 4 dereferenceable(4) %arrayidx1) #2
  %7 = load i32, i32* %i, align 4
  %idxprom2 = sext i32 %7 to i64
  %arrayidx3 = getelementptr inbounds [10 x %class.C], [10 x %class.C]* @p, i64 0, i64 %idxprom2
  %a = getelementptr inbounds %class.C, %class.C* %arrayidx3, i32 0, i32 0
  %8 = load i32, i32* %a, align 4
  %9 = load i32, i32* @y, align 4
  %add4 = add nsw i32 %9, %8
  store i32 %add4, i32* @y, align 4
  %10 = load i32, i32* %i, align 4
  %idxprom5 = sext i32 %10 to i64
  %arrayidx6 = getelementptr inbounds [10 x %class.C], [10 x %class.C]* @q, i64 0, i64 %idxprom5
  %11 = load [10 x %class.C]*, [10 x %class.C]** %x.addr, align 8
  %arrayidx7 = getelementptr inbounds [10 x %class.C], [10 x %class.C]* %11, i64 0, i64 5
  call void @_Z7my_initR1CS0_(%class.C* noundef nonnull align 4 dereferenceable(4) %arrayidx6, %class.C* noundef nonnull align 4 dereferenceable(4) %arrayidx7) #2

  %scan = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(),
  "QUAL.OMP.INCLUSIVE"(i32* @y, i64 2), ; inscan idx = 2
  "QUAL.OMP.INCLUSIVE"([10 x %class.C]** %x.addr, i64 1) ] ; inscan idx = 1
  fence acq_rel
  call void @llvm.directive.region.exit(token %scan) [ "DIR.OMP.END.SCAN"() ]

  %12 = load i32, i32* @y, align 4
  %13 = load i32, i32* %i, align 4
  %idxprom8 = sext i32 %13 to i64
  %arrayidx9 = getelementptr inbounds [10 x %class.C], [10 x %class.C]* @q, i64 0, i64 %idxprom8
  %a10 = getelementptr inbounds %class.C, %class.C* %arrayidx9, i32 0, i32 0
  store i32 %12, i32* %a10, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %14 = load i32, i32* %.omp.iv, align 4
  %add11 = add nsw i32 %14, 1
  store i32 %add11, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond, !llvm.loop !5

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: mustprogress noinline nounwind optnone uwtable
declare void @_Z7my_combR1CS0_(%class.C* noundef nonnull align 4 dereferenceable(4) %out, %class.C* noundef nonnull align 4 dereferenceable(4) %in) #0

; Function Attrs: mustprogress noinline nounwind optnone uwtable
declare void @_Z7my_initR1CS0_(%class.C* noundef nonnull align 4 dereferenceable(4) %priv, %class.C* noundef nonnull align 4 dereferenceable(4) %orig) #0

; Function Attrs: noinline uwtable
declare void @.omp_combiner.(%class.C* noalias noundef %0, %class.C* noalias noundef %1) #3

; Function Attrs: noinline uwtable
declare void @.omp_initializer.(%class.C* noalias noundef %0, %class.C* noalias noundef %1) #3

attributes #0 = { mustprogress noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress noinline nounwind optnone uwtable "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nounwind }
attributes #3 = { noinline uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 7, !"frame-pointer", i32 2}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.vectorize.enable", i1 true}
