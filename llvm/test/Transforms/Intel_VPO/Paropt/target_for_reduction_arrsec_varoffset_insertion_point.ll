; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug -S %s 2>&1 | FileCheck %s

; Test src:

; int a;
; int b;
; int c() {
;   auto d = [](int, int *e, int &) {
;     int f;
; #pragma omp for reduction(+:e[f])
;   for (int g = 0; g < 2; ++g)
;     ;
;   };
; #pragma omp target
;   for (int h;;)
;     d(h, &a, b);
; }

; Check the debug messages for finding the VLA and for setting a VLA insertion point.
; CHECK: checkIfVLA: '  %e.addr = alloca ptr, align 8' is a VLA clause operand.
; CHECK: setInsertionPtForVlaAllocas: Found a VLA operand. Setting VLA insertion point to

; Check in the IR that the allocas and the stacksave call are inserted before kmpc_reduce and that the stackrestore is inserted after kmpc_end_reduce
; CHECK:  %e.addr.red = alloca i32, i64 1, align 8
; CHECK:  %e.addr.red.minus.offset.addr = alloca ptr, align 8
; CHECK:  [[SS:%[^ ]+]]  = call ptr @llvm.stacksave.p0()
; CHECK:   %{{[^,]+}} = call i32 @__kmpc_reduce
; CHECK:  call void @__kmpc_end_reduce
; CHECK:  call void @llvm.stackrestore.p0(ptr [[SS]])

; ModuleID = 'test3.cpp'
source_filename = "test3.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class._ZTSZ1cvEUliPiRiE_ = type { i8 }

@a = dso_local global i32 0, align 4
@b = dso_local global i32 0, align 4

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef i32 @_Z1cv() #0 {
entry:
  %retval = alloca i32, align 4
  %d = alloca %class._ZTSZ1cvEUliPiRiE_, align 1
  %h = alloca i32, align 4

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TO"(ptr %d, ptr %d, i64 1, i64 673, ptr null, ptr null), ; MAP type: 673 = 0x2a1 = IMPLICIT (0x200) | PRIVATE (0x80) | TARGET_PARAM (0x20) | TO (0x1)
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @a, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @b, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %h, i32 0, i32 1) ]

  br label %for.cond

for.cond:                                         ; preds = %for.cond, %entry
  %1 = load i32, ptr %h, align 4
  call void @_ZZ1cvENKUliPiRiE_clEiS_S0_(ptr noundef nonnull align 1 dereferenceable(1) %d, i32 noundef %1, ptr noundef @a, ptr noundef nonnull align 4 dereferenceable(4) @b) #1
  br label %for.cond, !llvm.loop !5

dummy:                                            ; No predecessors!
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  %2 = load i32, ptr %retval, align 4
  ret i32 %2
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define internal void @_ZZ1cvENKUliPiRiE_clEiS_S0_(ptr noundef nonnull align 1 dereferenceable(1) %this, i32 noundef %0, ptr noundef %e, ptr noundef nonnull align 4 dereferenceable(4) %1) #0 align 2 {
entry:
  %this.addr = alloca ptr, align 8
  %.addr = alloca i32, align 4
  %e.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  %f = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %g = alloca i32, align 4
  store ptr %this, ptr %this.addr, align 8
  store i32 %0, ptr %.addr, align 4
  store ptr %e, ptr %e.addr, align 8
  store ptr %1, ptr %.addr1, align 8
  %this2 = load ptr, ptr %this.addr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 1, ptr %.omp.ub, align 4
  %2 = load i32, ptr %f, align 4
  %conv = sext i32 %2 to i64
  %3 = load ptr, ptr %e.addr, align 8
  %4 = load i32, ptr %f, align 4
  %idxprom = sext i32 %4 to i64
  %arrayidx = getelementptr inbounds i32, ptr %3, i64 %idxprom
  %5 = load ptr, ptr %e.addr, align 8
  %sec.base.cast = ptrtoint ptr %5 to i64
  %sec.lower.cast = ptrtoint ptr %arrayidx to i64
  %6 = sub i64 %sec.lower.cast, %sec.base.cast
  %sec.offset_in_elements = sdiv exact i64 %6, 4

  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.PTR_TO_PTR.TYPED"(ptr %e.addr, i32 0, i64 1, i64 %sec.offset_in_elements),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %g, i32 0, i32 1) ]

  %8 = load i32, ptr %.omp.lb, align 4
  store i32 %8, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %9 = load i32, ptr %.omp.iv, align 4
  %10 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %9, %10
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %11 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %11, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %g, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, ptr %.omp.iv, align 4
  %add3 = add nsw i32 %12, 1
  store i32 %add3, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.LOOP"() ]

  ret void
}

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{i32 0, i32 66313, i32 186593307, !"_Z1cv", i32 10, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
