; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck --check-prefix=CHECK %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s 2>&1 | FileCheck --check-prefix=CHECK %s

; Original code:
;subroutine foo(a)
;    integer :: a(10)
;    integer :: b(10)
;    integer :: i
;
;    !$omp parallel do firstprivate(b)
;    do i = 1,10
;        a(i) = b(i)
;    end do
;    !$omp end parallel do
;end subroutine

; CHECK: define internal void @foo_.DIR.OMP.PARALLEL.LOOP
; CHECK: [[FPRIV:%[A-Za-z0-9$._"]+]] = alloca [10 x i32]
; CHECK: [[FPRIV_GEP:%[A-Za-z0-9$._"]+]] = getelementptr inbounds [10 x i32], ptr [[FPRIV]], i32 0, i32 0
; CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 4 [[FPRIV_GEP]], ptr align 4 %"foo_$B", i64 40, i1 false)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo_(ptr dereferenceable(4) %"foo_$A") {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 16
  %"foo_$I" = alloca i32, align 8
  %"foo_$B" = alloca [10 x i32], align 16
  %omp.pdo.start = alloca i32, align 4
  store i32 1, ptr %omp.pdo.start, align 1
  %omp.pdo.end = alloca i32, align 4
  store i32 10, ptr %omp.pdo.end, align 1
  %omp.pdo.step = alloca i32, align 4
  store i32 1, ptr %omp.pdo.step, align 1
  %omp.pdo.norm.iv = alloca i64, align 8
  %omp.pdo.norm.lb = alloca i64, align 8
  store i64 0, ptr %omp.pdo.norm.lb, align 1
  %omp.pdo.norm.ub = alloca i64, align 8
  %omp.pdo.end_fetch.1 = load i32, ptr %omp.pdo.end, align 1
  %omp.pdo.start_fetch.2 = load i32, ptr %omp.pdo.start, align 1
  %sub.1 = sub nsw i32 %omp.pdo.end_fetch.1, %omp.pdo.start_fetch.2
  %omp.pdo.step_fetch.3 = load i32, ptr %omp.pdo.step, align 1
  %div.1 = sdiv i32 %sub.1, %omp.pdo.step_fetch.3
  %int_sext3 = sext i32 %div.1 to i64
  store i64 %int_sext3, ptr %omp.pdo.norm.ub, align 1
  br label %bb_new6

omp.pdo.cond3:                                    ; preds = %omp.pdo.body4, %bb_new6
  %omp.pdo.norm.iv_fetch.5 = load i64, ptr %omp.pdo.norm.iv, align 1
  %omp.pdo.norm.ub_fetch.6 = load i64, ptr %omp.pdo.norm.ub, align 1
  %rel.1 = icmp sle i64 %omp.pdo.norm.iv_fetch.5, %omp.pdo.norm.ub_fetch.6
  br i1 %rel.1, label %omp.pdo.body4, label %omp.pdo.epilog5

omp.pdo.body4:                                    ; preds = %omp.pdo.cond3
  %omp.pdo.norm.iv_fetch.7 = load i64, ptr %omp.pdo.norm.iv, align 1
  %int_sext = trunc i64 %omp.pdo.norm.iv_fetch.7 to i32
  %omp.pdo.step_fetch.8 = load i32, ptr %omp.pdo.step, align 1
  %mul.1 = mul nsw i32 %int_sext, %omp.pdo.step_fetch.8
  %omp.pdo.start_fetch.9 = load i32, ptr %omp.pdo.start, align 1
  %add.1 = add nsw i32 %mul.1, %omp.pdo.start_fetch.9
  store i32 %add.1, ptr %"foo_$I", align 1
  %"foo_$I_fetch.10" = load i32, ptr %"foo_$I", align 1
  %int_sext1 = sext i32 %"foo_$I_fetch.10" to i64
  %"foo_$B[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %"foo_$B", i64 %int_sext1)
  %"foo_$B[]_fetch.11" = load i32, ptr %"foo_$B[]", align 1
  %"foo_$I_fetch.12" = load i32, ptr %"foo_$I", align 1
  %int_sext2 = sext i32 %"foo_$I_fetch.12" to i64
  %"foo_$A_entry[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %"foo_$A", i64 %int_sext2)
  store i32 %"foo_$B[]_fetch.11", ptr %"foo_$A_entry[]", align 1
  %omp.pdo.norm.iv_fetch.13 = load i64, ptr %omp.pdo.norm.iv, align 1
  %add.2 = add nsw i64 %omp.pdo.norm.iv_fetch.13, 1
  store i64 %add.2, ptr %omp.pdo.norm.iv, align 1
  br label %omp.pdo.cond3

bb_new6:                                          ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %"foo_$A", i32 0, i64 10),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"foo_$I", i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %"foo_$B", i32 0, i32 10),
    "QUAL.OMP.SHARED:TYPED"(ptr %omp.pdo.step, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %omp.pdo.end, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %omp.pdo.start, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %omp.pdo.norm.lb, i64 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %omp.pdo.norm.iv, i64 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %omp.pdo.norm.ub, i64 0) ]

  %omp.pdo.norm.lb_fetch.4 = load i64, ptr %omp.pdo.norm.lb, align 1
  store i64 %omp.pdo.norm.lb_fetch.4, ptr %omp.pdo.norm.iv, align 1
  br label %omp.pdo.cond3

omp.pdo.epilog5:                                  ; preds = %omp.pdo.cond3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 %0, i64 %1, i64 %2, ptr %3, i64 %4)
declare void @llvm.directive.region.exit(token %0)
