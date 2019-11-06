; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S < %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S < %s  | FileCheck %s
;
; Test src:
;
; #include <stdio.h>
;
; void foo(int &a) {
; #pragma omp taskloop lastprivate(a)
;   for (int i = 0; i < 10; i++) {
;     a = i;
;   }
; }
;
; //int main() {
; //  int a;
; //  a = 0;
; //  foo(a);
; //  printf("%d\n", a);
; //  return 0;
; //}

; ModuleID = 'taskloop_scalar_byref_lp.cpp'
source_filename = "taskloop_scalar_byref_lp.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check for the space allocated for the private copy. { i64, i32, i32 }
; i64 - %.omp.lb,
; i32 - %a.addr,
; i32 - %i
; CHECK: %__struct.kmp_privates.t = type { i64, i32, i32 }

; Check shared thunk for space allocated for pointer to the original %a.addr for lastprivate copyout.
; i64 - %.omp.lb  (redundant. remove eventually)
; i32** - %a.addr
; CHECK: %__struct.shared.t = type { i64, i32** }

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z3fooRi(i32* dereferenceable(4) %a) #0 {
entry:
  %a.addr = alloca i32*, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %i = alloca i32, align 4
  store i32* %a, i32** %a.addr, align 8
  store i64 0, i64* %.omp.lb, align 8
  store i64 9, i64* %.omp.ub, align 8
  %0 = load i32*, i32** %a.addr, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(), "QUAL.OMP.LASTPRIVATE:BYREF"(i32** %a.addr), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i64* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]

; For the outlined function for the task, check that the address of the local
; copy for '%a.addr' is stored to an i32**, and then that is used instead of
; '%a.addr' in the region.
; CHECK: define internal void @{{.*}}DIR.OMP.TASK{{.*}}
; CHECK: [[A_PRIVATE:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, %__struct.kmp_privates.t* %4, i32 0, i32 1
; CHECK: [[A_SHR_ADDR:%[^ ]+]] = getelementptr inbounds %__struct.shared.t, %__struct.shared.t* %3, i32 0, i32 1
; CHECK: [[A_SHR:%[^ ]+]] = load i32**, i32*** [[A_SHR_ADDR]]
; CHECK: store i32* [[A_PRIVATE]], i32** {{[^ ]+}}

; Check for the lastprivate copyout code
; CHECK: [[A_SHR_LOAD:%[^ ]+]] = load i32*, i32** [[A_SHR]]
; CHECK: [[A_PRIVATE_LOAD:%[^ ]+]] = load i32, i32* [[A_PRIVATE]]
; CHECK: store i32 [[A_PRIVATE_LOAD]], i32* [[A_SHR_LOAD]]

  %2 = load i64, i64* %.omp.lb, align 8
  %conv = trunc i64 %2 to i32
  store i32 %conv, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, i32* %.omp.iv, align 4
  %conv1 = sext i32 %3 to i64
  %4 = load i64, i64* %.omp.ub, align 8
  %cmp = icmp ule i64 %conv1, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %6 = load i32, i32* %i, align 4
  %7 = load i32*, i32** %a.addr, align 8
  store i32 %6, i32* %7, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, i32* %.omp.iv, align 4
  %add2 = add nsw i32 %8, 1
  store i32 %add2, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASKLOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
