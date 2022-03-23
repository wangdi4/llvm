; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck --check-prefix=CHECK %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s 2>&1 | FileCheck --check-prefix=CHECK %s
; RUN: opt -opaque-pointers -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck --check-prefix=OPQPTR %s
; RUN: opt -opaque-pointers -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s 2>&1 | FileCheck --check-prefix=OPQPTR %s

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
; CHECK: [[FPRIV_GEP:%[A-Za-z0-9$._"]+]] = getelementptr inbounds [10 x i32], [10 x i32]* [[FPRIV]], i32 0, i32 0
; CHECK: [[FPRIV_GEP_CAST:%[A-Za-z0-9$._"]+]] = bitcast i32* [[FPRIV_GEP]] to [10 x i32]*
; CHECK: [[CAST:%[A-Za-z0-9$._"]+]] = bitcast i32* [[FPRIV_GEP]] to i8*
; CHECK: call void @llvm.memcpy{{.*}}(i8*{{.*}}[[CAST]],

; OPQPTR: define internal void @foo_.DIR.OMP.PARALLEL.LOOP
; OPQPTR: [[FPRIV:%[A-Za-z0-9$._"]+]] = alloca [10 x i32]
; OPQPTR: [[FPRIV_GEP:%[A-Za-z0-9$._"]+]] = getelementptr inbounds [10 x i32], ptr [[FPRIV]], i32 0, i32 0
; OPQPTR: call void @llvm.memcpy{{.*}}(ptr{{.*}}[[FPRIV_GEP]],


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo_(i32* dereferenceable(4) %"foo_$A") #0 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 16
  %"foo_$I" = alloca i32, align 8
  %"foo_$B" = alloca [10 x i32], align 16
  %"foo_$A_entry" = bitcast i32* %"foo_$A" to [10 x i32]*
  %omp.pdo.start = alloca i32, align 4
  store i32 1, i32* %omp.pdo.start, align 1, !tbaa !1
  %omp.pdo.end = alloca i32, align 4
  store i32 10, i32* %omp.pdo.end, align 1, !tbaa !1
  %omp.pdo.step = alloca i32, align 4
  store i32 1, i32* %omp.pdo.step, align 1, !tbaa !1
  %omp.pdo.norm.iv = alloca i64, align 8
  %omp.pdo.norm.lb = alloca i64, align 8
  store i64 0, i64* %omp.pdo.norm.lb, align 1, !tbaa !1
  %omp.pdo.norm.ub = alloca i64, align 8
  %omp.pdo.end_fetch.1 = load i32, i32* %omp.pdo.end, align 1, !tbaa !1
  %omp.pdo.start_fetch.2 = load i32, i32* %omp.pdo.start, align 1, !tbaa !1
  %sub.1 = sub nsw i32 %omp.pdo.end_fetch.1, %omp.pdo.start_fetch.2
  %omp.pdo.step_fetch.3 = load i32, i32* %omp.pdo.step, align 1, !tbaa !1
  %div.1 = sdiv i32 %sub.1, %omp.pdo.step_fetch.3
  %int_sext3 = sext i32 %div.1 to i64
  store i64 %int_sext3, i64* %omp.pdo.norm.ub, align 1, !tbaa !1
  br label %bb_new6

omp.pdo.cond3:                                    ; preds = %omp.pdo.body4, %bb_new6
  %omp.pdo.norm.iv_fetch.5 = load i64, i64* %omp.pdo.norm.iv, align 1, !tbaa !1
  %omp.pdo.norm.ub_fetch.6 = load i64, i64* %omp.pdo.norm.ub, align 1, !tbaa !1
  %rel.1 = icmp sle i64 %omp.pdo.norm.iv_fetch.5, %omp.pdo.norm.ub_fetch.6
  br i1 %rel.1, label %omp.pdo.body4, label %omp.pdo.epilog5

omp.pdo.body4:                                    ; preds = %omp.pdo.cond3
  %omp.pdo.norm.iv_fetch.7 = load i64, i64* %omp.pdo.norm.iv, align 1, !tbaa !1
  %int_sext = trunc i64 %omp.pdo.norm.iv_fetch.7 to i32
  %omp.pdo.step_fetch.8 = load i32, i32* %omp.pdo.step, align 1, !tbaa !1
  %mul.1 = mul nsw i32 %int_sext, %omp.pdo.step_fetch.8
  %omp.pdo.start_fetch.9 = load i32, i32* %omp.pdo.start, align 1, !tbaa !1
  %add.1 = add nsw i32 %mul.1, %omp.pdo.start_fetch.9
  store i32 %add.1, i32* %"foo_$I", align 1, !tbaa !4
  %"foo_$I_fetch.10" = load i32, i32* %"foo_$I", align 1, !tbaa !4
  %int_sext1 = sext i32 %"foo_$I_fetch.10" to i64
  %"(i32*)foo_$B$" = bitcast [10 x i32]* %"foo_$B" to i32*
  %"foo_$B[]" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %"(i32*)foo_$B$", i64 %int_sext1)
  %"foo_$B[]_fetch.11" = load i32, i32* %"foo_$B[]", align 1, !tbaa !6
  %"foo_$I_fetch.12" = load i32, i32* %"foo_$I", align 1, !tbaa !4
  %int_sext2 = sext i32 %"foo_$I_fetch.12" to i64
  %"(i32*)foo_$A_entry$" = bitcast [10 x i32]* %"foo_$A_entry" to i32*
  %"foo_$A_entry[]" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %"(i32*)foo_$A_entry$", i64 %int_sext2)
  store i32 %"foo_$B[]_fetch.11", i32* %"foo_$A_entry[]", align 1, !tbaa !8
  %omp.pdo.norm.iv_fetch.13 = load i64, i64* %omp.pdo.norm.iv, align 1, !tbaa !1
  %add.2 = add nsw i64 %omp.pdo.norm.iv_fetch.13, 1
  store i64 %add.2, i64* %omp.pdo.norm.iv, align 1, !tbaa !1
  br label %omp.pdo.cond3

bb_new6:                                          ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SHARED"([10 x i32]* %"foo_$A_entry"), "QUAL.OMP.PRIVATE:TYPED"(i32* %"foo_$I", i32 0, i32 1), "QUAL.OMP.FIRSTPRIVATE:TYPED"([10 x i32]* %"foo_$B", i32 0, i32 10), "QUAL.OMP.SHARED"(i32* %omp.pdo.step), "QUAL.OMP.SHARED"(i32* %omp.pdo.end), "QUAL.OMP.SHARED"(i32* %omp.pdo.start), "QUAL.OMP.FIRSTPRIVATE:TYPED"(i64* %omp.pdo.norm.lb, i64 0, i32 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(i64* %omp.pdo.norm.iv, i64 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(i64* %omp.pdo.norm.ub, i64 0) ]
  %omp.pdo.norm.lb_fetch.4 = load i64, i64* %omp.pdo.norm.lb, align 1, !tbaa !1
  store i64 %omp.pdo.norm.lb_fetch.4, i64* %omp.pdo.norm.iv, align 1, !tbaa !1
  br label %omp.pdo.cond3

omp.pdo.epilog5:                                  ; preds = %omp.pdo.cond3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 %0, i64 %1, i64 %2, i32* %3, i64 %4) #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #1

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
attributes #2 = { nounwind readnone speculatable }

!omp_offload.info = !{}
!llvm.module.flags = !{!0}

!0 = !{i32 7, !"openmp", i32 50}
!1 = !{!2, !2, i64 0}
!2 = !{!"Generic Fortran Symbol", !3, i64 0}
!3 = !{!"ifx$root$1$foo_"}
!4 = !{!5, !5, i64 0}
!5 = !{!"ifx$unique_sym$1", !2, i64 0}
!6 = !{!7, !7, i64 0}
!7 = !{!"ifx$unique_sym$2", !2, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"ifx$unique_sym$3", !2, i64 0}
