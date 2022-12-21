; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; SIMD loops should not be multiversioned.

; BEGIN REGION { }
;       %2 = bitcast.i32*.i8*(&((%.omp.ub)[0]));
;       @llvm.lifetime.start.p0i8(4,  &((i8*)(%.omp.ub)[0]));
;       (%.omp.ub)[0] = %n + -1;
;       %3 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.LINEAR(&((%ptr.addr)[0])1),  QUAL.OMP.REDUCTION.ADD(&((%s)[0])),  QUAL.OMP.NORMALIZED.IV(&((%.omp.iv)[0])),  QUAL.OMP.NORMALIZED.UB(&((%.omp.ub)[0])) ]
;       (%.omp.iv)[0] = 0;
;       %4 = (%.omp.ub)[0];
;
;          %ptr.addr.promoted = (%ptr.addr)[0];
;          %.pre = (%s)[0];
;          %5 = %.pre;
;       + DO i1 = 0, %4, 1   <DO_LOOP>  <MAX_TC_EST = 2147483648>
;       |   %6 = (%ptr.addr.promoted)[i1];
;       |   %5 = %5  +  %6 * i1;
;       |   (%s)[0] = %5;
;       |   %incdec.ptr = &((%ptr.addr.promoted)[i1 + 1]);
;       |   (%.omp.iv)[0] = i1 + 1;
;       + END LOOP
;          (%ptr.addr)[0] = &((%incdec.ptr)[0]);
;
;        @llvm.directive.region.exit(%3); [ DIR.OMP.END.SIMD() ]
;       %.pre16 = (%s)[0];
; END REGION

; CHECK: BEGIN REGION
; CHECK-NOT: if (
; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @_Z3fooPii(i32* %ptr, i32 %n) {
entry:
  %ptr.addr = alloca i32*, align 8
  %s = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32* %ptr, i32** %ptr.addr, align 8
  %0 = bitcast i32* %s to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0)
  store i32 0, i32* %s, align 4
  %1 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1)
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %omp.precond.then, label %entry.omp.precond.end_crit_edge

entry.omp.precond.end_crit_edge:                  ; preds = %entry
  %.pre17 = bitcast i32* %.omp.ub to i8*
  br label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %sub2 = add nsw i32 %n, -1
  %2 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2)
  store i32 %sub2, i32* %.omp.ub, align 4
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR"(i32** %ptr.addr, i32 1), "QUAL.OMP.REDUCTION.ADD"(i32* %s), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub) ]
  store i32 0, i32* %.omp.iv, align 4
  %4 = load i32, i32* %.omp.ub, align 4
  %cmp413 = icmp slt i32 %4, 0
  br i1 %cmp413, label %omp.loop.exit, label %omp.inner.for.body.lr.ph

omp.inner.for.body.lr.ph:                         ; preds = %omp.precond.then
  %ptr.addr.promoted = load i32*, i32** %ptr.addr, align 8
  %.pre = load i32, i32* %s, align 4
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.inner.for.body.lr.ph
  %5 = phi i32 [ %.pre, %omp.inner.for.body.lr.ph ], [ %add7, %omp.inner.for.body ]
  %incdec.ptr15 = phi i32* [ %ptr.addr.promoted, %omp.inner.for.body.lr.ph ], [ %incdec.ptr, %omp.inner.for.body ]
  %storemerge14 = phi i32 [ 0, %omp.inner.for.body.lr.ph ], [ %add8, %omp.inner.for.body ]
  %6 = load i32, i32* %incdec.ptr15, align 4
  %mul6 = mul nsw i32 %6, %storemerge14
  %add7 = add nsw i32 %5, %mul6
  store i32 %add7, i32* %s, align 4
  %incdec.ptr = getelementptr inbounds i32, i32* %incdec.ptr15, i64 1
  %add8 = add nuw nsw i32 %storemerge14, 1
  store i32 %add8, i32* %.omp.iv, align 4
  %cmp4 = icmp slt i32 %storemerge14, %4
  br i1 %cmp4, label %omp.inner.for.body, label %omp.inner.for.cond.omp.loop.exit_crit_edge

omp.inner.for.cond.omp.loop.exit_crit_edge:       ; preds = %omp.inner.for.body
  %incdec.ptr.lcssa = phi i32* [ %incdec.ptr, %omp.inner.for.body ]
  store i32* %incdec.ptr.lcssa, i32** %ptr.addr, align 8
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.cond.omp.loop.exit_crit_edge, %omp.precond.then
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SIMD"() ]
  %.pre16 = load i32, i32* %s, align 4
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %entry.omp.precond.end_crit_edge, %omp.loop.exit
  %.pre-phi = phi i8* [ %.pre17, %entry.omp.precond.end_crit_edge ], [ %2, %omp.loop.exit ]
  %7 = phi i32 [ 0, %entry.omp.precond.end_crit_edge ], [ %.pre16, %omp.loop.exit ]
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %.pre-phi)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0)
  ret i32 %7
}

declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture)

