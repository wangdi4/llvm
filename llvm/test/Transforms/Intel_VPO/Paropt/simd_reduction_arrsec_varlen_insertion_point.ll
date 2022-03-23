;REQUIRES: asserts
;RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug -S %s 2>&1 | FileCheck %s
;RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug -S %s 2>&1 | FileCheck %s
;
;test src:
;void reduction(int x[2][4])
;{
; int i,m;
; int a[4][4] = {0,0,0,0};
;#pragma omp simd reduction(+:a[0:i][0:0])
;    for (i = 0; i < m; i++);
;}
;
;check the debug messages for finding the variable length array section and for setting a VLA insertion point.
;CHECK: checkIfVLA: '  [[Array:%[a-zA-Z._0-9]+]] = alloca [4 x [4 x i32]], align 16' is a VLA clause operand.
;CHECK: setInsertionPtForVlaAllocas: Found a VLA operand. Setting VLA insertion point to

;check in the IR that the allocas and the stacksave call are inserted before the region entry and that the stackrestore is inserted after the region exit
;CHECK:  %a.fast_red.alloca = alloca i32, i64 %size, align 16
;CHECK:  %a.red = alloca i32, i64 %size, align 16
;CHECK:  %{{[^,]+}} = call i8* @llvm.stacksave()
;CHECK:  %{{[^,]+}} = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()
;CHECK:  call void @llvm.directive.region.exit(token %{{[^,]+}}) [ "DIR.OMP.END.SIMD"() ]
;CHECK:  call void @llvm.stackrestore(i8* %{{[^,]+}})

source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @reduction([4 x i32]* %x) #0 {
entry:
  %x.addr = alloca [4 x i32]*, align 8
  %i = alloca i32, align 4
  %m = alloca i32, align 4
  %a = alloca [4 x [4 x i32]], align 16
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store [4 x i32]* %x, [4 x i32]** %x.addr, align 8
  %0 = bitcast [4 x [4 x i32]]* %a to i8*
  call void @llvm.memset.p0i8.i64(i8* align 16 %0, i8 0, i64 64, i1 false)
  %1 = load i32, i32* %m, align 4
  store i32 %1, i32* %.capture_expr.0, align 4
  %2 = load i32, i32* %.capture_expr.0, align 4
  %sub = sub nsw i32 %2, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, i32* %.capture_expr.1, align 4
  %3 = load i32, i32* %.capture_expr.0, align 4
  %cmp = icmp slt i32 0, %3
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %4 = load i32, i32* %.capture_expr.1, align 4
  store i32 %4, i32* %.omp.ub, align 4
  %5 = load i32, i32* %i, align 4
  %conv = sext i32 %5 to i64
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:ARRSECT"([4 x [4 x i32]]* %a, i64 2, i64 0, i64 %conv, i64 1, i64 0, i64 0, i64 1), "QUAL.OMP.LINEAR:IV"(i32* %i, i32 1), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub) ]
  store i32 0, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %7 = load i32, i32* %.omp.iv, align 4
  %8 = load i32, i32* %.omp.ub, align 4
  %cmp3 = icmp sle i32 %7, %8
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %9, 1
  %add5 = add nsw i32 0, %mul
  store i32 %add5, i32* %i, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, i32* %.omp.iv, align 4
  %add6 = add nsw i32 %10, 1
  store i32 %add6, i32* %.omp.iv, align 4
  %11 = load i32, i32* %i, align 4
  %add7 = add nsw i32 %11, 1
  store i32 %add7, i32* %i, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  ret void
}

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline nounwind optnone uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nounwind willreturn writeonly }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 7, !"frame-pointer", i32 2}
!4 = !{!"Clang version 9.0.0"}
