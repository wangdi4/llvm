; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-reduction=true -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-reduction=true -S %s | FileCheck %s

; Original code:
;void foo() {
;  int s = 0;
;#pragma omp target teams distribute parallel for reduction(+: s)
;  for (int i = 0; i < 100; ++i)
;    s += i;
;}

; Verify that we use private linkage for Windows, because MSVC linker
; does not support duplicate extern_weak symbols with the same name
; in different modules:
; CHECK: @red_buf = private global i32 0

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.16.27045"
target device_triples = "spir64"

@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @.omp_offloading.requires_reg, i8* null }]

; Function Attrs: nounwind uwtable
define dso_local void @foo() #0 {
entry:
  %s = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, i32* %s, align 4
  store i32 0, i32* %.omp.lb, align 4
  store i32 99, i32* %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(i32* %s, i32* %s, i64 4, i64 33315, i32* null, i32* null), ; MAP type: 33315 = 0x8223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1) | UNKNOWN (0x8000)
    "QUAL.OMP.PRIVATE:TYPED"(i32* %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32* %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32* %tmp, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(i32* %s, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32* %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(i32* %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(i32* %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32* %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32* %tmp, i32 0, i32 1) ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(i32* %s, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(i32* %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(i32* %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(i32* %i, i32 0, i32 1) ]

  %3 = load i32, i32* %.omp.lb, align 4
  store i32 %3, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, i32* %.omp.iv, align 4
  %5 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %7 = load i32, i32* %i, align 4
  %8 = load i32, i32* %s, align 4
  %add1 = add nsw i32 %8, %7
  store i32 %add1, i32* %s, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, i32* %.omp.iv, align 4
  %add2 = add nsw i32 %9, 1
  store i32 %add2, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nounwind uwtable
define internal void @.omp_offloading.requires_reg() #2 {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

; Function Attrs: nounwind
declare void @__tgt_register_requires(i64) #1

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 1754006398, i32 1300412, !"_Z3foo", i32 3, i32 0, i32 0}
