; REQUIRES: asserts
; RUN: opt -debugify -vpo-cfg-restructuring -check-debugify -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='module(debugify),function(vpo-cfg-restructuring),module(check-debugify)' -S %s 2>&1 | FileCheck %s
; RUN: opt -debugify -vpo-cfg-restructuring -vpo-paropt-prepare -check-debugify -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='module(debugify),function(vpo-cfg-restructuring),vpo-paropt-prepare,check-debugify' -S %s 2>&1 | FileCheck %s
; RUN: opt -debugify -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -check-debugify -S %s 2>&1 | FileCheck %s --check-prefixes=CHECK,PAROPT
; RUN: opt -passes='module(debugify),function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,module(check-debugify)' -S %s 2>&1 | FileCheck %s --check-prefixes=CHECK,PAROPT
;
; Test src:
; int test_add()
; {
;   int r0 = 0;
; #pragma omp parallel for reduction(+: r0)
;   for (int i = 0; i < 128; i++);
;   return r0;
; }
;
;
; -----------------------------------------------------------------------------
; The warnings below are introduced by vpo-paropt. They should eventually be
; fixed. For now, make sure noone adds more.
;
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add --  br label %DIR.OMP.END.PARALLEL.LOOP.4
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add_tree_reduce_2 --  %dst.cast = bitcast i8* %dst to %struct.fast_red_t*
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add_tree_reduce_2 --  %src.cast = bitcast i8* %src to %struct.fast_red_t*
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add_tree_reduce_2 --  %dst.r0 = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* %dst.cast, i32 0, i32 0
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add_tree_reduce_2 --  %src.r0 = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* %src.cast, i32 0, i32 0
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add_tree_reduce_2 --  %0 = load i32, i32* %src.r0, align 4
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add_tree_reduce_2 --  %1 = load i32, i32* %dst.r0, align 4
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add_tree_reduce_2 --  %2 = add i32 %1, %0
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add_tree_reduce_2 --  store i32 %2, i32* %dst.r0, align 4
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add_tree_reduce_2 --  ret void
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add.DIR.OMP.PARALLEL.LOOP.25.split10 --  %my.tid15 = load i32, i32* %tid, align 4
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add.DIR.OMP.PARALLEL.LOOP.25.split10 --  %my.tid16 = load i32, i32* %tid, align 4
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add.DIR.OMP.PARALLEL.LOOP.25.split10 --  call void @__kmpc_end_reduce(%struct.ident_t* @.kmpc_loc.10.29.8, i32 %my.tid16, [8 x i32]* @.gomp_critical_user_.fast_reduction.AS0.var)
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add.DIR.OMP.PARALLEL.LOOP.25.split10 --  %my.tid14 = load i32, i32* %tid, align 4
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add.DIR.OMP.PARALLEL.LOOP.25.split10 --  %my.tid19 = load i32, i32* %tid, align 4
; PAROPT: WARNING: Instruction with empty DebugLoc in function test_add.DIR.OMP.PARALLEL.LOOP.25.split10 --  call void @__kmpc_end_reduce(%struct.ident_t* @.kmpc_loc.10.29.10, i32 %my.tid19, [8 x i32]* @.gomp_critical_user_.fast_reduction.AS0.var)
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
;
; CHECK-NOT: WARNING
; CHECK: CheckModuleDebugify: PASS
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @test_add() #0 {
entry:
  %r0 = alloca i32, align 4
; Check that alloca instructions result in call to @llvm.dbg.value, example:
; call void @llvm.dbg.value(metadata i32* %r0, metadata !11, metadata !DIExpression()), !dbg !32
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, i32* %r0, align 4
  store i32 0, i32* %.omp.lb, align 4
  store i32 127, i32* %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD"(i32* %r0), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
; PREP:   [[ENTRY:%[a-zA-Z._0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD"(i32* %r0), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.OPERAND.ADDR"(i32* %i, i32** %i.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %.omp.lb, i32** %.omp.lb.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %r0, i32** %r0.addr), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ], !dbg [[ENTRYDBG:![0-9]+]]
; ALL-NEXT:call void @llvm.dbg.value(metadata i32 [[LBLOAD]], metadata !{{.*}}, metadata !DIExpression()), !dbg [[LBLOADDBG]]
  %1 = load i32, i32* %.omp.lb, align 4
  store i32 %1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
; checking the loaded value in %2
; PREP: [[IVLOAD:%[a-zA-Z._0-9]+]] = load volatile i32, i32* %.omp.iv, align 4, !dbg [[IVLOADDBG:![0-9]+]]
  %2 = load i32, i32* %.omp.iv, align 4
; PREP:  [[UBLOAD:%[a-zA-Z._0-9]+]] = load volatile i32, i32* %.omp.ub, align 4, !dbg [[UBLOADDBG:![0-9]+]]
  %3 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
; checking the loaded value in %4
; PREP: [[IVLOAD2:%[a-zA-Z._0-9]+]] = load volatile i32, i32* %.omp.iv, align 4, !dbg [[IVLOAD2DBG:![0-9]+]]
  %4 = load i32, i32* %.omp.iv, align 4
; checking the result of multiply in %mul
  %mul = mul nsw i32 %4, 1

; checking the result of add in %add
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
; PREP: [[IVLOAD3:%[a-zA-Z._0-9]+]] = load volatile i32, i32* %.omp.iv, align 4, !dbg [[IVLOAD3DBG:![0-9]+]]
  %5 = load i32, i32* %.omp.iv, align 4
; checking the result of add in %add1
  %add1 = add nsw i32 %5, 1
  store i32 %add1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
; checking the loaded value in %6
  %6 = load i32, i32* %r0, align 4
  ret i32 %6
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
