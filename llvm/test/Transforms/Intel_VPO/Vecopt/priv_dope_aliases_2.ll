; REQUIRES: asserts
; RUN: opt -passes="vplan-vec" -disable-output -debug-only=vploop-analysis < %s 2>&1 | FileCheck %s
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -debug-only=vploop-analysis -vplan-enable-hir-f90-dv < %s 2>&1 | FileCheck %s

; Dope vector is used outside the loop and alias shouldn't be generated.

; CHECK:      Aliases:
; CHECK-NOT: ptr %"C1.addr_tmp$_fetch"
; CHECK-NOT:   %"C1.addr_tmp$_fetch" = load ptr, ptr %"C1.addr_tmp$", align 8

; ModuleID = '<stdin>'
source_filename = "priv_dope_fixed.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$ptr$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

; Function Attrs: nounwind uwtable
define void @compare_dope_vectors_IP_sum_vec_(ptr nocapture readnone %SUM_VEC.uplevel_ptr2, ptr noalias dereferenceable(72) "assumed_shape" "ptrnoalias" %C1, ptr noalias nocapture dereferenceable(72) "assumed_shape" "ptrnoalias" %C2, ptr noalias nocapture dereferenceable(4) %N, ptr noalias nocapture dereferenceable(4) %M) local_unnamed_addr #0 {
DIR.OMP.SIMD.2:
  %"sum_vec$J$_1.linear.iv" = alloca i32, align 4
  %C1.priv = alloca %"QNCA_a0$ptr$rank1$", align 8
  %N_fetch.16 = load i32, ptr %N, align 1
  %.dv.init = call i64 @_f90_dope_vector_init2(ptr nonnull %C1.priv, ptr nonnull %C1) #0
  %is.allocated = icmp sgt i64 %.dv.init, 0
  br i1 %is.allocated, label %allocated.then, label %DIR.OMP.SIMD.1

allocated.then:                                   ; preds = %DIR.OMP.SIMD.2
  br label %DIR.OMP.SIMD.1

do.body48.preheader:                              ; preds = %loop_exit47
  %mul.7 = mul i32 %N_fetch.29, 0
  %mul.8 = mul i32 %mul.7, %M_fetch.31
  br label %do.body48

do.body48:                                        ; preds = %do.body48, %do.body48.preheader
  %indvars.iv = phi i64 [ 1, %do.body48.preheader ], [ %indvars.iv.next, %do.body48 ]
  %0 = add nsw i64 %indvars.iv, -1
  %1 = mul nsw i64 %0, %"C1.dim_info$.spacing$[]_fetch.23"
  %2 = getelementptr inbounds i8, ptr %"C1.addr_a0$_fetch.22", i64 %1
  %"C1.addr_a0$_fetch.34[]_fetch.38" = load i32, ptr %2, align 1
  %add.5 = add nsw i32 %mul.8, %"C1.addr_a0$_fetch.34[]_fetch.38"
  store i32 %add.5, ptr %2, align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond59.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond59.not, label %DIR.OMP.END.SIMD.2, label %do.body48

DIR.OMP.SIMD.1:                                   ; preds = %allocated.then, %DIR.OMP.SIMD.2
  %"C1.addr_tmp$_fetch" = load ptr, ptr %C1.priv, align 8
  %rel.3.not47 = icmp slt i32 %N_fetch.16, 1
  br i1 %rel.3.not47, label %DIR.OMP.END.SIMD.446, label %DIR.OMP.SIMD.166

DIR.OMP.SIMD.166:                                 ; preds = %DIR.OMP.SIMD.1
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:TYPED.IV"(ptr %"sum_vec$J$_1.linear.iv", i32 0, i64 1, i32 1), "QUAL.OMP.PRIVATE:F90_DV.TYPED"(ptr %C1.priv, %"QNCA_a0$ptr$rank1$" zeroinitializer, i32 0), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0), "QUAL.OMP.LIVEIN:F90_DV"(ptr %C2), "QUAL.OMP.LIVEIN"(ptr %M), "QUAL.OMP.LIVEIN"(ptr %N) ]
  br label %DIR.OMP.SIMD.167

DIR.OMP.SIMD.167:                                 ; preds = %DIR.OMP.SIMD.166
  %"C1.addr_a0$_fetch.22" = load ptr, ptr %C1.priv, align 8
  %"C1.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$ptr$rank1$", ptr %C1.priv, i64 0, i32 6, i64 0, i32 1
  %"C1.dim_info$.spacing$[]_fetch.23" = load i64, ptr %"C1.dim_info$.spacing$", align 8
  %"C1.dim_info$.extent$" = getelementptr inbounds %"QNCA_a0$ptr$rank1$", ptr %C1.priv, i64 0, i32 6
  %"C1.dim_info$.extent$[]_fetch.24" = load i64, ptr %"C1.dim_info$.extent$", align 8
  %rel.5.not55 = icmp slt i64 %"C1.dim_info$.extent$[]_fetch.24", 1
  %N_fetch.29 = load i32, ptr %N, align 1
  %rel.6 = icmp slt i32 %N_fetch.29, 1
  %M_fetch.31 = load i32, ptr %M, align 1
  %mul.9 = mul nsw i32 %M_fetch.31, %M_fetch.31
  %"C2.addr_a0$_fetch.55" = load ptr, ptr %C2, align 1
  %"C2.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$ptr$rank1$", ptr %C2, i64 0, i32 6, i64 0, i32 1
  %"C2.dim_info$.spacing$[]_fetch.56" = load i64, ptr %"C2.dim_info$.spacing$", align 1
  %4 = add i32 %N_fetch.29, 1
  %wide.trip.count = zext i32 %4 to i64
  br label %do.body48.preheader

DIR.OMP.END.SIMD.2:                               ; preds = %do.end_do54
  %add.9.le = add nuw i32 %N_fetch.16, 1
  store i32 %add.9.le, ptr %"sum_vec$J$_1.linear.iv", align 4
  br label %DIR.OMP.END.SIMD.268

DIR.OMP.END.SIMD.268:                             ; preds = %DIR.OMP.END.SIMD.2
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.446

DIR.OMP.END.SIMD.446:                             ; preds = %DIR.OMP.END.SIMD.268, %DIR.OMP.SIMD.1
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

declare i64 @_f90_dope_vector_init2(ptr, ptr) local_unnamed_addr

attributes #0 = { nounwind }
