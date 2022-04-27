; REQUIRES: asserts
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug -S %s 2>&1 | FileCheck %s

; Test src:

; a;
; b;
; c() {
;   auto d = [](int, *e, &) {
;     int f;
; #pragma omp for reduction(+e[f])
;   for (int g = 0; g < 2; ++g)
;     ;
;   };
; #pragma omp target
;   for (int h;;)
;     d(h, &a, b);
; }

; Check the debug messages for finding the VLA and for setting a VLA insertion point.
; CHECK: checkIfVLA: '  %e.addr = alloca i32*, align 8' is a VLA clause operand.
; CHECK: setInsertionPtForVlaAllocas: Found a VLA operand. Setting VLA insertion point to

; Check in the IR that the allocas and the stacksave call are inserted before kmpc_reduce and that the stackrestore is inserted after kmpc_end_reduce
; CHECK:  %e.addr.red = alloca i32, i64 1, align 8
; CHECK:  %e.addr.red.minus.offset.addr = alloca i32*, align 8
; CHECK:  [[SS:%[^ ]+]]  = call i8* @llvm.stacksave()
; CHECK:   %{{[^,]+}} = call i32 @__kmpc_reduce
; CHECK:  call void @__kmpc_end_reduce
; CHECK:  call void @llvm.stackrestore(i8* [[SS]])

; ModuleID = 'test3.cpp'
source_filename = "test3.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class._ZTSZ1cvEUliPiRiE_ = type { i8 }

@a = dso_local global i32 0, align 4
@b = dso_local global i32 0, align 4

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local i32 @_Z1cv() #0 {
entry:
  %retval = alloca i32, align 4
  %d = alloca %class._ZTSZ1cvEUliPiRiE_, align 1
  %h = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TO"(%class._ZTSZ1cvEUliPiRiE_* %d, %class._ZTSZ1cvEUliPiRiE_* %d, i64 1, i64 673, i8* null, i8* null), "QUAL.OMP.FIRSTPRIVATE"(i32* @a), "QUAL.OMP.FIRSTPRIVATE"(i32* @b), "QUAL.OMP.PRIVATE"(i32* %h) ]
  br label %for.cond

for.cond:                                         ; preds = %for.cond, %entry
  %1 = load i32, i32* %h, align 4
  call void @_ZZ1cvENKUliPiRiE_clEiS_S0_(%class._ZTSZ1cvEUliPiRiE_* nonnull align 1 dereferenceable(1) %d, i32 %1, i32* @a, i32* nonnull align 4 dereferenceable(4) @b) #1
  br label %for.cond, !llvm.loop !6

dummy:                                            ; No predecessors!
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  %2 = load i32, i32* %retval, align 4
  ret i32 %2
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define internal void @_ZZ1cvENKUliPiRiE_clEiS_S0_(%class._ZTSZ1cvEUliPiRiE_* nonnull align 1 dereferenceable(1) %this, i32 %0, i32* %e, i32* nonnull align 4 dereferenceable(4) %1) #0 align 2 {
entry:
  %this.addr = alloca %class._ZTSZ1cvEUliPiRiE_*, align 8
  %.addr = alloca i32, align 4
  %e.addr = alloca i32*, align 8
  %.addr1 = alloca i32*, align 8
  %f = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %g = alloca i32, align 4
  store %class._ZTSZ1cvEUliPiRiE_* %this, %class._ZTSZ1cvEUliPiRiE_** %this.addr, align 8
  store i32 %0, i32* %.addr, align 4
  store i32* %e, i32** %e.addr, align 8
  store i32* %1, i32** %.addr1, align 8
  %this2 = load %class._ZTSZ1cvEUliPiRiE_*, %class._ZTSZ1cvEUliPiRiE_** %this.addr, align 8
  store i32 0, i32* %.omp.lb, align 4
  store i32 1, i32* %.omp.ub, align 4
  %2 = load i32, i32* %f, align 4
  %conv = sext i32 %2 to i64
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.REDUCTION.ADD:ARRSECT"(i32** %e.addr, i64 1, i64 %conv, i64 1, i64 1), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %g) ]
  %4 = load i32, i32* %.omp.lb, align 4
  store i32 %4, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %5 = load i32, i32* %.omp.iv, align 4
  %6 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %5, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %g, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, i32* %.omp.iv, align 4
  %add3 = add nsw i32 %8, 1
  store i32 %add3, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.LOOP"() ]
  ret void
}

attributes #0 = { mustprogress noinline nounwind optnone uwtable "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 0, i32 66313, i32 160378233, !"_Z1cv", i32 10, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Clang version 9.0.0"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
