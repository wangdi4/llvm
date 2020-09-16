; REQUIRES: asserts
; RUN: opt -debugify -vpo-cfg-restructuring -S < %s | FileCheck %s --check-prefix=CFGRES --check-prefix=CFGRES-PREP --check-prefix=ALL
; RUN: opt -passes='module(debugify),function(vpo-cfg-restructuring)' -S < %s | FileCheck %s --check-prefix=CFGRES --check-prefix=CFGRES-PREP --check-prefix=ALL
; RUN: opt -debugify -vpo-cfg-restructuring -vpo-paropt-prepare -S < %s | FileCheck %s --check-prefix=PREP --check-prefix=CFGRES-PREP --check-prefix=ALL
; RUN: opt -passes='module(debugify),function(vpo-cfg-restructuring)',vpo-paropt-prepare -S < %s | FileCheck %s --check-prefix=PREP --check-prefix=CFGRES-PREP --check-prefix=ALL
; RUN: opt -debugify -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S < %s | FileCheck %s --check-prefix=PAROPT --check-prefix=ALL
; RUN: opt -passes='module(debugify),function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S < %s | FileCheck %s  --check-prefix=PAROPT --check-prefix=ALL
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
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @test_add() #0 {
entry:
  %r0 = alloca i32, align 4
; Check that alloca instructions result in call to @llvm.dbg.value, example:
; call void @llvm.dbg.value(metadata i32* %r0, metadata !11, metadata !DIExpression()), !dbg !32
; ALL:  [[R0:%[a-zA-Z._0-9]+]] = alloca i32, align 4, !dbg [[R0DBG:![0-9]+]]
; ALL:  call void @llvm.dbg.value(metadata i32* [[R0]], metadata !{{.*}}, metadata !DIExpression()), !dbg [[R0DBG]]
  %tmp = alloca i32, align 4
; ALL:  [[TMP:%[a-zA-Z._0-9]+]] = alloca i32, align 4, !dbg [[TMPDBG:![0-9]+]]
; ALL:  call void @llvm.dbg.value(metadata i32* [[TMP]], metadata !{{.*}}, metadata !DIExpression()), !dbg [[TMPDBG]]
  %.omp.iv = alloca i32, align 4
; ALL:  [[IV:%[a-zA-Z._0-9]+]] = alloca i32, align 4, !dbg [[IVDBG:![0-9]+]]
; ALL:  call void @llvm.dbg.value(metadata i32* [[IV]], metadata !{{.*}}, metadata !DIExpression()), !dbg [[IVDBG]]
  %.omp.lb = alloca i32, align 4
; ALL:  [[LB:%[a-zA-Z._0-9]+]] = alloca i32, align 4, !dbg [[LBDBG:![0-9]+]]
; ALL:  call void @llvm.dbg.value(metadata i32* [[LB]], metadata !{{.*}}, metadata !DIExpression()), !dbg [[LBDBG]]
  %.omp.ub = alloca i32, align 4
; ALL:  [[UB:%[a-zA-Z._0-9]+]] = alloca i32, align 4, !dbg [[UBDBG:![0-9]+]]
; ALL:  call void @llvm.dbg.value(metadata i32* [[UB]], metadata !{{.*}}, metadata !DIExpression()), !dbg [[UBDBG]]
  %i = alloca i32, align 4
; ALL:  [[I:%[a-zA-Z._0-9]+]] = alloca i32, align 4, !dbg [[IDBG:![0-9]+]]
; ALL:  call void @llvm.dbg.value(metadata i32* [[I]], metadata !{{.*}}, metadata !DIExpression()), !dbg [[IDBG]]
  store i32 0, i32* %r0, align 4
  store i32 0, i32* %.omp.lb, align 4
  store i32 127, i32* %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD"(i32* %r0), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
; PAROPT: [[RESULT:%[a-zA-Z._0-9]+]] = load i32, i32* %r0, align 4, !dbg [[RESULTDBG:![0-9]+]]
; PAROPT: call void @llvm.dbg.value(metadata i32 [[RESULT]], metadata !31, metadata !DIExpression()), !dbg [[RESULTDBG]]
; PAROPT:  declare void @llvm.dbg.value(metadata, metadata, metadata) #2
; PAROPT:  call void @llvm.dbg.value(metadata i32* [[R0]], metadata !{{.*}}, metadata !DIExpression(DW_OP_deref)), !dbg !{{.*}}
; PAROPT:  call void @llvm.dbg.value(metadata i32* [[UB]], metadata !{{.*}}, metadata !DIExpression(DW_OP_deref)), !dbg !{{.*}}
; PAROPT: call void @llvm.dbg.value(metadata token undef, metadata !{{.*}}, metadata !DIExpression()), !dbg !{{.*}}
; PREP:   [[ENTRY:%[a-zA-Z._0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD"(i32* %r0), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.OPERAND.ADDR"(i32* %i, i32** %i.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %.omp.lb, i32** %.omp.lb.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %r0, i32** %r0.addr), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ], !dbg [[ENTRYDBG:![0-9]+]]
; CFGRES: [[ENTRY:%[a-zA-Z._0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD"(i32* %r0), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ], !dbg [[ENTRYDBG:![0-9]+]]
; CFGRES-PREP: call void @llvm.dbg.value(metadata token [[ENTRY]], metadata !{{.*}}, metadata !DIExpression()), !dbg [[ENTRYDBG]]
; CFGRES-PREP: [[LBLOAD:%[a-zA-Z._0-9]+]] = load i32, i32* %.omp.lb{{.*}}, align 4, !dbg [[LBLOADDBG:![0-9]+]]
; PAROPT: [[LBLOAD:%[a-zA-Z._0-9]+]] = load i32, i32* %.omp.lb{{.*}}, align 4, !dbg [[LBLOADDBG:![0-9]+]], !{{.*}}
; ALL-NEXT:call void @llvm.dbg.value(metadata i32 [[LBLOAD]], metadata !{{.*}}, metadata !DIExpression()), !dbg [[LBLOADDBG]]
; PAROPT:call void @llvm.dbg.value(metadata i32 [[LBLOAD]], metadata !{{.*}}, metadata !DIExpression()), !dbg !{{.*}}
; PAROPT: call void @llvm.dbg.value(metadata i32 %{{.*}}, metadata !{{.*}}, metadata !DIExpression()), !dbg !{{.*}}
  %1 = load i32, i32* %.omp.lb, align 4
  store i32 %1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
; checking the loaded value in %2
; CFGRES: [[IVLOAD:%[a-zA-Z._0-9]+]] = load i32, i32* %.omp.iv, align 4, !dbg [[IVLOADDBG:![0-9]+]]
; PREP: [[IVLOAD:%[a-zA-Z._0-9]+]] = load volatile i32, i32* %.omp.iv, align 4, !dbg [[IVLOADDBG:![0-9]+]]
; CFGRES-PREP-NEXT:  call void @llvm.dbg.value(metadata i32 [[IVLOAD]], metadata !{{.*}}, metadata !DIExpression()), !dbg [[IVLOADDBG]]
  %2 = load i32, i32* %.omp.iv, align 4
; CFGRES: [[UBLOAD:%[a-zA-Z._0-9]+]] = load i32, i32* %.omp.ub, align 4, !dbg [[UBLOADDBG:![0-9]+]]
; PREP:  [[UBLOAD:%[a-zA-Z._0-9]+]] = load volatile i32, i32* %.omp.ub, align 4, !dbg [[UBLOADDBG:![0-9]+]]
; CFGRES-PREP-NEXT:  call void @llvm.dbg.value(metadata i32 [[UBLOAD]], metadata !{{.*}}, metadata !DIExpression()), !dbg [[UBLOADDBG]]
  %3 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
; ALL:  [[CMP:%[a-zA-Z._0-9]+]] = icmp sle i32 {{.*}}, {{.*}}, !dbg [[CMPDBG:![0-9]+]]
; ALL:  call void @llvm.dbg.value(metadata i1 [[CMP]], metadata !{{.*}}, metadata !DIExpression()), !dbg [[CMPDBG]]
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
; checking the loaded value in %4
; PAROPT:  [[LOCALIV:%[a-zA-Z._0-9]+]] = phi i32 [ %add1, %omp.inner.for.inc ], [ %lb.new, %omp.inner.for.body.lr.ph ]
; PAROPT:  call void @llvm.dbg.value(metadata i32 [[LOCALIV]], metadata !{{.*}}, metadata !DIExpression()), !dbg !{{.*}}
; PAROPT-NEXT:  call void @llvm.dbg.value(metadata i32 [[LOCALIV]], metadata !{{.*}}, metadata !DIExpression()), !dbg !{{.*}}
; CFGRES: [[IVLOAD2:%[a-zA-Z._0-9]+]] = load i32, i32* %.omp.iv, align 4, !dbg [[IVLOAD2DBG:![0-9]+]]
; PREP: [[IVLOAD2:%[a-zA-Z._0-9]+]] = load volatile i32, i32* %.omp.iv, align 4, !dbg [[IVLOAD2DBG:![0-9]+]]
; CFGRES-PREP-NEXT:  call void @llvm.dbg.value(metadata i32 [[IVLOAD2]], metadata !{{.*}}, metadata !DIExpression()), !dbg [[IVLOAD2DBG]]
  %4 = load i32, i32* %.omp.iv, align 4
; checking the result of multiply in %mul
  %mul = mul nsw i32 %4, 1
; ALL:  [[MUL:%[a-zA-Z._0-9]+]] = mul nsw i32 {{.*}}, 1, !dbg [[MULDBG:![0-9]+]]
; ALL:  call void @llvm.dbg.value(metadata i32 [[MUL]], metadata !{{.*}}, metadata !DIExpression()), !dbg [[MULDBG]]

; checking the result of add in %add
  %add = add nsw i32 0, %mul
; ALL:  [[ADD:%[a-zA-Z._0-9]+]] = add nsw i32 0, %mul, !dbg [[ADDDBG:![0-9]+]]
; ALL:  call void @llvm.dbg.value(metadata i32 [[ADD]], metadata !{{.*}}, metadata !DIExpression()), !dbg [[ADDDBG]]
  store i32 %add, i32* %i, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
; CFGRES: [[IVLOAD3:%[a-zA-Z._0-9]+]] = load i32, i32* %.omp.iv, align 4, !dbg [[IVLOAD3DBG:![0-9]+]]
; PREP: [[IVLOAD3:%[a-zA-Z._0-9]+]] = load volatile i32, i32* %.omp.iv, align 4, !dbg [[IVLOAD3DBG:![0-9]+]]
; PAROPT:  call void @llvm.dbg.value(metadata i32 [[LOCALIV]], metadata !{{.*}}, metadata !DIExpression()), !dbg !{{.*}}
; CFGRES-PREP-NEXT:  call void @llvm.dbg.value(metadata i32 [[IVLOAD3]], metadata !{{.*}}, metadata !DIExpression()), !dbg [[IVLOAD3DBG]]
  %5 = load i32, i32* %.omp.iv, align 4
; checking the result of add in %add1
  %add1 = add nsw i32 %5, 1
; ALL:  [[ADD1:%[a-zA-Z._0-9]+]] = add nsw i32 {{.*}}, 1, !dbg [[ADD1DBG:![0-9]+]]
; ALL:  call void @llvm.dbg.value(metadata i32 [[ADD1]], metadata !{{.*}}, metadata !DIExpression()), !dbg [[ADD1DBG]]
; PAROPT:  call void @llvm.dbg.value(metadata i32 [[ADD1]], metadata !{{.*}}, metadata !DIExpression()), !dbg !{{.*}}
; PAROPT:  call void @llvm.dbg.value(metadata i32 %{{.*}}, metadata !{{.*}}, metadata !DIExpression()), !dbg !{{.*}}
; PAROPT:  call void @llvm.dbg.value(metadata i1 %{{.*}}, metadata !{{.*}}, metadata !DIExpression()), !dbg !{{.*}}
  store i32 %add1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
; checking the loaded value in %6
; CFGRES-PREP:  [[RESULT:%[a-zA-Z._0-9]+]] = load i32, i32* %r0, align 4, !dbg [[RESULTDBG:![0-9]+]]
; CFGRES-PREP-NEXT:  call void @llvm.dbg.value(metadata i32 [[RESULT]], metadata !{{.*}}, metadata !DIExpression()), !dbg [[RESULTDBG]]
  %6 = load i32, i32* %r0, align 4
  ret i32 %6
}

; CFGRES-PREP:  declare void @llvm.dbg.value(metadata, metadata, metadata) #2

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
