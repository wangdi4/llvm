;REQUIRES: asserts
;RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug -S %s 2>&1 | FileCheck --check-prefixes=OPQPTR,ALL %s
;RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug -S %s 2>&1 | FileCheck --check-prefixes=OPQPTR,ALL %s

;test src:
;void foo(int n) {
;int e[n];
;#pragma omp simd reduction (+:e)
;   for(int d=0; d<100; d++);
; }
;
;IR was hand modified to add TYPED modifer to the Reduction clause.
;check the debug messages for finding the VLA and for setting a VLA insertion point.
;ALL: checkIfVLA: '  %{{[^,]+}} = alloca i32, i64 %{{[^,]+}}, align 16' is a VLA clause operand.
;OPQPTR: insertStackSaveRestore: Inserted stacksave' %{{[^,]+}} = call ptr @llvm.stacksave.p0()'.

;check in the IR that the allocas and the stacksave call are inserted before the region entry and that the stackrestore is inserted after the region exit
;OPQPTR:  [[SS1:%[^ ]+]] = call ptr @llvm.stacksave.p0()
;OPQPTR:  store ptr [[SS1]], ptr %saved_stack, align 8
;OPQPTR:  %vla.fast_red.alloca = alloca i32, i64 %{{[^,]}}, align 16
;OPQPTR:  %vla.red = alloca i32, i64 %{{[^,]}}, align 16
;OPQPTR:  [[SS2:%[^ ]+]]  = call ptr @llvm.stacksave.p0()
;OPQPTR:  %{{[^,]+}} = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()
;OPQPTR:  call void @llvm.directive.region.exit(token %{{[^,]+}}) [ "DIR.OMP.END.SIMD"() ]
;OPQPTR:  call void @llvm.stackrestore.p0(ptr [[SS2]])
;OPQPTR:  [[READSS1:%[^ ]+]] = load ptr, ptr %saved_stack, align 8
;OPQPTR:  call void @llvm.stackrestore.p0(ptr [[READSS1]])

; ModuleID = 'simd_reduction_typed_vla_insertion_point.c'
source_filename = "simd_reduction_typed_vla_insertion_point.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32 noundef %n) {
entry:
  %n.addr = alloca i32, align 4
  %saved_stack = alloca ptr, align 8
  %__vla_expr0 = alloca i64, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %omp.vla.tmp = alloca i64, align 8
  %d = alloca i32, align 4
  store i32 %n, ptr %n.addr, align 4
  %0 = load i32, ptr %n.addr, align 4
  %1 = zext i32 %0 to i64
  %2 = call ptr @llvm.stacksave()
  store ptr %2, ptr %saved_stack, align 8
  %vla = alloca i32, i64 %1, align 16
  store i64 %1, ptr %__vla_expr0, align 8
  store i32 99, ptr %.omp.ub, align 4
  store i64 %1, ptr %omp.vla.tmp, align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %vla, i32 0, i64 %1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %d, i32 0, i32 1, i32 1) ]

  %4 = load i64, ptr %omp.vla.tmp, align 8
  store i32 0, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %5 = load i32, ptr %.omp.iv, align 4
  %6 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %5, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %d, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %8, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SIMD"() ]

  %9 = load ptr, ptr %saved_stack, align 8
  call void @llvm.stackrestore(ptr %9)
  ret void
}

declare ptr @llvm.stacksave()

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @llvm.stackrestore(ptr)

