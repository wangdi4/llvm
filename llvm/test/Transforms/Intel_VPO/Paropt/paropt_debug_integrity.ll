; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -debugify -vpo-cfg-restructuring -check-debugify -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='module(debugify),function(vpo-cfg-restructuring),module(check-debugify)' -S %s 2>&1 | FileCheck %s
; RUN: opt -bugpoint-enable-legacy-pm -debugify -vpo-cfg-restructuring -vpo-paropt-prepare -check-debugify -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='module(debugify),function(vpo-cfg-restructuring),vpo-paropt-prepare,check-debugify' -S %s 2>&1 | FileCheck %s
; RUN: opt -bugpoint-enable-legacy-pm -debugify -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -check-debugify -S %s 2>&1 | FileCheck %s --check-prefixes=CHECK,PAROPT
; RUN: opt -passes='module(debugify),function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,module(check-debugify)' -S %s 2>&1 | FileCheck %s --check-prefixes=CHECK,PAROPT

; Test src:
;
; int test_add()
; {
;   int r0 = 0;
; #pragma omp parallel for reduction(+: r0)
;   for (int i = 0; i < 128; i++);
;   return r0;
; }

; -----------------------------------------------------------------------------
; The warnings below are introduced by vpo-paropt. They should eventually be
; fixed. For now, make sure noone adds more.

; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add --  br label %DIR.OMP.END.PARALLEL.LOOP.4
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add_tree_reduce_2 --  %dst.r0 = getelementptr inbounds %struct.fast_red_t, ptr %dst, i32 0, i32 0
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add_tree_reduce_2 --  %src.r0 = getelementptr inbounds %struct.fast_red_t, ptr %src, i32 0, i32 0
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add_tree_reduce_2 --  %0 = load i32, ptr %src.r0, align 4
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add_tree_reduce_2 --  %1 = load i32, ptr %dst.r0, align 4
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add_tree_reduce_2 --  %2 = add i32 %1, %0
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add_tree_reduce_2 --  store i32 %2, ptr %dst.r0, align 4
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add_tree_reduce_2 --  ret void
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add.DIR.OMP.PARALLEL.LOOP.25.split10 --  %my.tid15 = load i32, ptr %tid, align 4
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add.DIR.OMP.PARALLEL.LOOP.25.split10 --  %my.tid16 = load i32, ptr %tid, align 4
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add.DIR.OMP.PARALLEL.LOOP.25.split10 --  call void @__kmpc_end_reduce(ptr @.kmpc_loc.10.29.8, i32 %my.tid16, ptr @.gomp_critical_user_.fast_reduction.AS0.var)
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add.DIR.OMP.PARALLEL.LOOP.25.split10 --  %my.tid14 = load i32, ptr %tid, align 4
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add.DIR.OMP.PARALLEL.LOOP.25.split10 --  %my.tid19 = load i32, ptr %tid, align 4
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add.DIR.OMP.PARALLEL.LOOP.25.split10 --  call void @__kmpc_end_reduce(ptr @.kmpc_loc.10.29.10, i32 %my.tid19, ptr @.gomp_critical_user_.fast_reduction.AS0.var)
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add.DIR.OMP.PARALLEL.LOOP.25.split10 --  ret void
; PAROPT: WARNING: Missing line 12
; PAROPT: WARNING: Missing line 13
; PAROPT: WARNING: Missing line 14
; PAROPT: WARNING: Missing line 15
; PAROPT: WARNING: Missing line 18
; PAROPT: WARNING: Missing line 24
; PAROPT: WARNING: Missing line 26
; PAROPT: WARNING: Missing line 27
; -----------------------------------------------------------------------------

; CHECK-NOT: WARNING
; CHECK: CheckModuleDebugify: PASS

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @test_add() {
entry:
  %r0 = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, ptr %r0, align 4
  store i32 0, ptr %.omp.lb, align 4
  store i32 127, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %r0, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %5 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %5, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  %6 = load i32, ptr %r0, align 4
  ret i32 %6
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
