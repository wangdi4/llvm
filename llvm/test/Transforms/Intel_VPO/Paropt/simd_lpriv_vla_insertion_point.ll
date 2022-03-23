;REQUIRES: asserts
;RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug -S %s 2>&1 | FileCheck %s
;RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug -S %s 2>&1 | FileCheck %s
;
;test src:
; void foo(int n) {
; int e[n];
; #pragma omp simd lastprivate(e)
;   for(int d=0; d<100; d++);
; }
;
;check the debug messages for finding the VLA and for setting a VLA insertion point.
;CHECK: checkIfVLA: '  %{{[^,]+}} = alloca i32, i64 %{{[^,]+}}, align 16' is a VLA clause operand.
;CHECK: setInsertionPtForVlaAllocas: Found a VLA operand. Setting VLA insertion point to
;CHECK: insertStackSaveRestore: Inserted stacksave' %{{[^,]+}} = call i8* @llvm.stacksave()'.
;
;check in the IR that the allocas and the stacksave call are inserted before the region entry and that the stackrestore is inserted after the region exit
;CHECK:  [[SS1:%[^ ]+]] = call i8* @llvm.stacksave()
;CHECK:  store i8* [[SS1]], i8** %saved_stack, align 8
;CHECK:  %vla.lpriv = alloca i32, i64 %{{[^,]}}, align 16
;CHECK:  [[SS2:%[^ ]+]]  = call i8* @llvm.stacksave()
;CHECK:  %{{[^,]}} = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE"(i32* %vla.lpriv)
;CHECK:  call void @llvm.directive.region.exit(token %{{[^,]+}}) [ "DIR.OMP.END.SIMD"() ]
;CHECK:  call void @llvm.stackrestore(i8* [[SS2]])
;CHECK:  [[READSS1:%[^ ]+]] = load i8*, i8** %saved_stack, align 8
;CHECK:  call void @llvm.stackrestore(i8* [[READSS1]])
;
; ModuleID = 'test3.c'
source_filename = "test3.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(i32 %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %saved_stack = alloca i8*, align 8
  %__vla_expr0 = alloca i64, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %omp.vla.tmp = alloca i64, align 8
  %d = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4
  %0 = load i32, i32* %n.addr, align 4
  %1 = zext i32 %0 to i64
  %2 = call i8* @llvm.stacksave()
  store i8* %2, i8** %saved_stack, align 8
  %vla = alloca i32, i64 %1, align 16
  store i64 %1, i64* %__vla_expr0, align 8
  store i32 99, i32* %.omp.ub, align 4
  store i64 %1, i64* %omp.vla.tmp, align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE"(i32* %vla), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.LINEAR:IV"(i32* %d, i32 1) ]
  %4 = load i64, i64* %omp.vla.tmp, align 8
  store i32 0, i32* %.omp.iv, align 4
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
  store i32 %add, i32* %d, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, i32* %.omp.iv, align 4
  %add1 = add nsw i32 %8, 1
  store i32 %add1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SIMD"() ]
  %9 = load i8*, i8** %saved_stack, align 8
  call void @llvm.stackrestore(i8* %9)
  ret void
}

; Function Attrs: nofree nosync nounwind willreturn
declare i8* @llvm.stacksave() #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: nofree nosync nounwind willreturn
declare void @llvm.stackrestore(i8*) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 7, !"frame-pointer", i32 2}
!4 = !{!"Clang version 9.0.0"}
