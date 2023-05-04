; RUN: opt -passes='vplan-vec' -disable-output -vplan-print-after-vpentity-instrs -vplan-entities-dump -vplan-enable-f90-dv < %s 2>&1 | FileCheck %s
; RUN: opt -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vplan-vec' -disable-output -vplan-print-after-vpentity-instrs -vplan-entities-dump -vplan-enable-f90-dv < %s 2>&1 | FileCheck %s

; Fortran sourcecode used for this test
; integer function sum(c1, c2, n, m)
;      integer :: c1(:)
;      integer :: c2, n, m
;      sum = 1
;        !$omp simd private(c1)
;        do j=1, n
;          c1(j) = m*n + c2
;        enddo
; end function sum

; CHECK:     Private list
; CHECK:       Private tag: F90DV
; CHECK-NEXT:  Linked values: ptr %"sum_$C1.priv", ptr [[VP_TMP:%.*]], 
; CHECK-NEXT:  Memory: ptr %"sum_$C1.priv"

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$i32*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

; Function Attrs: nounwind
define i32 @sum_(ptr noalias dereferenceable(72) "assumed_shape" "ptrnoalias" %"sum_$C1", ptr noalias nocapture dereferenceable(4) %"sum_$C2", ptr noalias nocapture dereferenceable(4) %"sum_$N", ptr noalias nocapture dereferenceable(4) %"sum_$M") {
DIR.OMP.SIMD.2:
  %"sum_$J.linear.iv" = alloca i32, align 4
  %"sum_$C1.priv" = alloca %"QNCA_a0$i32*$rank1$", align 8
  %"sum_$N_fetch.1" = load i32, ptr %"sum_$N", align 1
  %.dv.init = call i64 @_f90_dope_vector_init2(ptr nonnull %"sum_$C1.priv", ptr nonnull %"sum_$C1")
  %is.allocated = icmp sgt i64 %.dv.init, 0
  br i1 %is.allocated, label %allocated.then, label %DIR.OMP.SIMD.1

allocated.then:                                   ; preds = %DIR.OMP.SIMD.2
  %"sum_$C1.priv.alloc.num_elements16" = lshr i64 %.dv.init, 2
  %"sum_$C1.priv.addr0" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %"sum_$C1.priv", i64 0, i32 0
  %"sum_$C1.priv.data" = alloca i32, i64 %"sum_$C1.priv.alloc.num_elements16", align 4
  store ptr %"sum_$C1.priv.data", ptr %"sum_$C1.priv.addr0", align 8
  br label %DIR.OMP.SIMD.1

omp.pdo.body6:                                    ; preds = %DIR.OMP.SIMD.119, %omp.pdo.body6
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.119 ], [ %indvars.iv.next, %omp.pdo.body6 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %"sum_$C1.addr_a0$_fetch.10[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %"sum_$C1.dim_info$.spacing$[]_fetch.11", ptr elementtype(i32) %"sum_$C1.addr_a0$_fetch.10", i64 %indvars.iv.next)
  store i32 %add.3, ptr %"sum_$C1.addr_a0$_fetch.10[]", align 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.pdo.body6

DIR.OMP.SIMD.1:                                   ; preds = %allocated.then, %DIR.OMP.SIMD.2
  %rel.1.not13 = icmp slt i32 %"sum_$N_fetch.1", 1
  br i1 %rel.1.not13, label %DIR.OMP.END.SIMD.412, label %DIR.OMP.SIMD.118

DIR.OMP.SIMD.118:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:TYPED.IV"(ptr %"sum_$J.linear.iv", i32 0, i64 1, i32 1), "QUAL.OMP.PRIVATE:F90_DV.TYPED"(ptr %"sum_$C1.priv", %"QNCA_a0$i32*$rank1$" zeroinitializer, i32 0), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0), "QUAL.OMP.LIVEIN"(ptr %"sum_$C2"), "QUAL.OMP.LIVEIN"(ptr %"sum_$M"), "QUAL.OMP.LIVEIN"(ptr %"sum_$N") ]
  br label %DIR.OMP.SIMD.119

DIR.OMP.SIMD.119:                                 ; preds = %DIR.OMP.SIMD.118
  %"sum_$M_fetch.7" = load i32, ptr %"sum_$M", align 1
  %"sum_$N_fetch.8" = load i32, ptr %"sum_$N", align 1
  %mul.1 = mul nsw i32 %"sum_$N_fetch.8", %"sum_$M_fetch.7"
  %"sum_$C2_fetch.9" = load i32, ptr %"sum_$C2", align 1
  %add.3 = add nsw i32 %mul.1, %"sum_$C2_fetch.9"
  %"sum_$C1.addr_a0$" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %"sum_$C1.priv", i64 0, i32 0
  %"sum_$C1.addr_a0$_fetch.10" = load ptr, ptr %"sum_$C1.addr_a0$", align 8
  %"sum_$C1.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %"sum_$C1.priv", i64 0, i32 6, i64 0, i32 1
  %"sum_$C1.dim_info$.spacing$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"sum_$C1.dim_info$.spacing$", i32 0)
  %"sum_$C1.dim_info$.spacing$[]_fetch.11" = load i64, ptr %"sum_$C1.dim_info$.spacing$[]", align 1
  %wide.trip.count = zext i32 %"sum_$N_fetch.1" to i64
  br label %omp.pdo.body6

DIR.OMP.END.SIMD.2:                               ; preds = %omp.pdo.body6
  %add.4.le = add nuw i32 %"sum_$N_fetch.1", 1
  store i32 %add.4.le, ptr %"sum_$J.linear.iv", align 4
  br label %DIR.OMP.END.SIMD.220

DIR.OMP.END.SIMD.220:                             ; preds = %DIR.OMP.END.SIMD.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.412

DIR.OMP.END.SIMD.412:                             ; preds = %DIR.OMP.SIMD.1, %DIR.OMP.END.SIMD.220
  ret i32 1
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #0

; Function Attrs: nounwind
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

declare i64 @_f90_dope_vector_init2(ptr, ptr) local_unnamed_addr

attributes #0 = { nounwind }
