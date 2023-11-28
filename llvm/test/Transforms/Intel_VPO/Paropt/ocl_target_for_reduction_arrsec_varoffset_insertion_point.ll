; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -switch-to-offload=true -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload=true -debug -S %s 2>&1 | FileCheck %s

; Test src:
;
; int a;
; int b;
; int c() {
;   auto d = [](int, int *e, int&h) {
;     int f;
; #pragma omp for reduction(+: e[f])
;   for (int g = 0; g < 2; ++g)
;     ;
;   };
; #pragma omp target
;   for (int h;;)
;     d(h, &a, b);
; }

; The test IR was hand-modified to use a simplified reduction section offset.
; CFE currently generates IR instructions to compute it.

; Check the debug messages for finding the VLA and for setting a VLA insertion point.
; CHECK: checkIfVLA: '  %e.addr.ascast = addrspacecast ptr %e.addr to ptr addrspace(4)' is a VLA clause operand.
; CHECK: setInsertionPtForVlaAllocas: Found a VLA operand. Setting VLA insertion point to

; Check in the IR that the allocas and the stacksave call are inserted before red.init.body  and that the stackrestore is inserted after red.init.done
; CHECK: %e.addr.ascast.red.ascast = addrspacecast ptr %e.addr.ascast.red to ptr addrspace(4)
; CHECK: %e.addr.ascast.red.ascast.minus.offset.addr = alloca ptr addrspace(4), align 8
; CHECK: [[SS:%[^ ]+]] = call ptr @llvm.stacksave.p0()
; CHECK: red.init.body:
; CHECK: red.init.done:
; CHECK: call void @llvm.stackrestore.p0(ptr [[SS]])

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%class._ZTSZ1cvEUliPiRiE_ = type { i8 }

@a = external dso_local addrspace(1) global i32, align 4
@b = external dso_local addrspace(1) global i32, align 4

; Function Attrs: convergent mustprogress noinline nounwind optnone uwtable
define hidden spir_func i32 @_Z1cv() #0 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %d = alloca %class._ZTSZ1cvEUliPiRiE_, align 1
  %d.ascast = addrspacecast ptr %d to ptr addrspace(4)
  %h = alloca i32, align 4
  %h.ascast = addrspacecast ptr %h to ptr addrspace(4)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TO"(ptr addrspace(4) %d.ascast, ptr addrspace(4) %d.ascast, i64 1, i64 673, ptr null, ptr null), ; MAP type: 673 = 0x2a1 = IMPLICIT (0x200) | PRIVATE (0x80) | TARGET_PARAM (0x20) | TO (0x1)
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @a to ptr addrspace(4)), i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @b to ptr addrspace(4)), i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %h.ascast, i32 0, i32 1) ]

  br label %for.cond

for.cond:                                         ; preds = %for.cond, %entry
  %1 = load i32, ptr addrspace(4) %h.ascast, align 4
  call spir_func void @_ZZ1cvENKUliPiRiE_clEiS_S0_(ptr addrspace(4) align 1 dereferenceable_or_null(1) %d.ascast, i32 %1, ptr addrspace(4) addrspacecast (ptr addrspace(1) @a to ptr addrspace(4)), ptr addrspace(4) align 4 dereferenceable(4) addrspacecast (ptr addrspace(1) @b to ptr addrspace(4))) #3
  br label %for.cond, !llvm.loop !8

dummy:                                            ; No predecessors!
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  %2 = load i32, ptr addrspace(4) %retval.ascast, align 4
  ret i32 %2
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: convergent mustprogress noinline nounwind optnone uwtable
define internal spir_func void @_ZZ1cvENKUliPiRiE_clEiS_S0_(ptr addrspace(4) align 1 dereferenceable_or_null(1) %this, i32 %0, ptr addrspace(4) %e, ptr addrspace(4) align 4 dereferenceable(4) %1) #2 align 2 {
entry:
  %this.addr = alloca ptr addrspace(4), align 8
  %this.addr.ascast = addrspacecast ptr %this.addr to ptr addrspace(4)
  %.addr = alloca i32, align 4
  %.addr.ascast = addrspacecast ptr %.addr to ptr addrspace(4)
  %e.addr = alloca ptr addrspace(4), align 8
  %e.addr.ascast = addrspacecast ptr %e.addr to ptr addrspace(4)
  %.addr1 = alloca ptr addrspace(4), align 8
  %.addr1.ascast = addrspacecast ptr %.addr1 to ptr addrspace(4)
  %f = alloca i32, align 4
  %f.ascast = addrspacecast ptr %f to ptr addrspace(4)
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %g = alloca i32, align 4
  %g.ascast = addrspacecast ptr %g to ptr addrspace(4)
  store ptr addrspace(4) %this, ptr addrspace(4) %this.addr.ascast, align 8
  store i32 %0, ptr addrspace(4) %.addr.ascast, align 4
  store ptr addrspace(4) %e, ptr addrspace(4) %e.addr.ascast, align 8
  store ptr addrspace(4) %1, ptr addrspace(4) %.addr1.ascast, align 8
  %this2 = load ptr addrspace(4), ptr addrspace(4) %this.addr.ascast, align 8
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 1, ptr addrspace(4) %.omp.ub.ascast, align 4
  %2 = load i32, ptr addrspace(4) %f.ascast, align 4
  %conv = sext i32 %2 to i64
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.PTR_TO_PTR.TYPED"(ptr addrspace(4) %e.addr.ascast, i32 0, i64 1, i64 %conv),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %g.ascast, i32 0, i32 1) ]

  %4 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %4, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %5 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %6 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %5, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %g.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add3 = add nsw i32 %8, 1
  store i32 %add3, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.LOOP"() ]

  ret void
}

attributes #0 = { convergent mustprogress noinline nounwind optnone uwtable "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #1 = { nounwind }
attributes #2 = { convergent mustprogress noinline nounwind optnone uwtable "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" }
attributes #3 = { convergent nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!6}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}

!0 = !{i32 0, i32 66313, i32 160378236, !"_Z1cv", i32 10, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"uwtable", i32 1}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!7 = !{!"clang version 9.0.0"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
