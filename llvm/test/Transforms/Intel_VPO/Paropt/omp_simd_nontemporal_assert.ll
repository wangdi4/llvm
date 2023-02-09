; RUN: opt -opaque-pointers=1 -enable-new-pm=0 -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=1 -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; subroutine ntscale(A, B, N)
;   implicit none
;
;   integer, intent(in) :: N
;   real, intent(in) :: B(N)
;   real, intent(out) :: A(N)
;
;   integer :: i
;
; !$OMP SIMD NONTEMPORAL(A)
;   do i=1,N
;     A(i) = 2.0*B(i)
;   end do
; end subroutine ntscale

; Check that paropt does not assert while lowering nontemporal clause with a value that
; has only one use which is the clause itself.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$float*$rank1$.0" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank1$.1" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

define void @ntscale_(ptr dereferenceable(4) "ptrnoalias" %"ntscale_$A", ptr readonly dereferenceable(4) "ptrnoalias" %"ntscale_$B", ptr readonly dereferenceable(4) %"ntscale_$N") local_unnamed_addr {
DIR.OMP.SIMD.2:
  %"var$3" = alloca %"QNCA_a0$float*$rank1$.0", align 8
  %"var$5" = alloca %"QNCA_a0$float*$rank1$.1", align 8
  %"ntscale_$I" = alloca i32, align 8
  %"ntscale_$N_fetch.5" = load i32, ptr %"ntscale_$N", align 1
  %"var$3.flags$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", ptr %"var$3", i64 0, i32 3
  store i64 1, ptr %"var$3.flags$", align 8
  %int_sext = sext i32 %"ntscale_$N_fetch.5" to i64
  %0 = icmp sgt i64 %int_sext, 0
  %slct.1 = select i1 %0, i64 %int_sext, i64 0
  %"var$3.addr_length$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", ptr %"var$3", i64 0, i32 1
  store i64 4, ptr %"var$3.addr_length$", align 8
  %"var$3.dim$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", ptr %"var$3", i64 0, i32 4
  store i64 1, ptr %"var$3.dim$", align 8
  %"var$3.codim$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", ptr %"var$3", i64 0, i32 2
  store i64 0, ptr %"var$3.codim$", align 8
  %"var$3.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", ptr %"var$3", i64 0, i32 6, i64 0, i32 1
  %"var$3.dim_info$.spacing$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$3.dim_info$.spacing$", i32 0)
  store i64 4, ptr %"var$3.dim_info$.spacing$[]", align 1
  %"var$3.dim_info$.lower_bound$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", ptr %"var$3", i64 0, i32 6, i64 0, i32 2
  %"var$3.dim_info$.lower_bound$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$3.dim_info$.lower_bound$", i32 0)
  store i64 1, ptr %"var$3.dim_info$.lower_bound$[]", align 1
  %"var$3.dim_info$.extent$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", ptr %"var$3", i64 0, i32 6, i64 0, i32 0
  %"var$3.dim_info$.extent$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$3.dim_info$.extent$", i32 0)
  store i64 %slct.1, ptr %"var$3.dim_info$.extent$[]", align 1
  %"var$3.addr_a0$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", ptr %"var$3", i64 0, i32 0
  store ptr %"ntscale_$A", ptr %"var$3.addr_a0$", align 8
  %"var$3.flags$_fetch.2" = load i64, ptr %"var$3.flags$", align 8
  %or.1 = or i64 %"var$3.flags$_fetch.2", 1
  store i64 %or.1, ptr %"var$3.flags$", align 8
  %"var$5.flags$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", ptr %"var$5", i64 0, i32 3
  store i64 1, ptr %"var$5.flags$", align 8
  %"var$5.addr_length$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", ptr %"var$5", i64 0, i32 1
  store i64 4, ptr %"var$5.addr_length$", align 8
  %"var$5.dim$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", ptr %"var$5", i64 0, i32 4
  store i64 1, ptr %"var$5.dim$", align 8
  %"var$5.codim$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", ptr %"var$5", i64 0, i32 2
  store i64 0, ptr %"var$5.codim$", align 8
  %"var$5.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", ptr %"var$5", i64 0, i32 6, i64 0, i32 1
  %"var$5.dim_info$.spacing$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$5.dim_info$.spacing$", i32 0)
  store i64 4, ptr %"var$5.dim_info$.spacing$[]", align 1
  %"var$5.dim_info$.lower_bound$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", ptr %"var$5", i64 0, i32 6, i64 0, i32 2
  %"var$5.dim_info$.lower_bound$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$5.dim_info$.lower_bound$", i32 0)
  store i64 1, ptr %"var$5.dim_info$.lower_bound$[]", align 1
  %"var$5.dim_info$.extent$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", ptr %"var$5", i64 0, i32 6, i64 0, i32 0
  %"var$5.dim_info$.extent$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$5.dim_info$.extent$", i32 0)
  store i64 %slct.1, ptr %"var$5.dim_info$.extent$[]", align 1
  %"var$5.addr_a0$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", ptr %"var$5", i64 0, i32 0
  store ptr %"ntscale_$B", ptr %"var$5.addr_a0$", align 8
  %"var$5.flags$_fetch.4" = load i64, ptr %"var$5.flags$", align 8
  %or.2 = or i64 %"var$5.flags$_fetch.4", 1
  store i64 %or.2, ptr %"var$5.flags$", align 8
  %"var$3.addr_a0$_fetch.6" = load ptr, ptr %"var$3.addr_a0$", align 8
  %"ntscale_$N_fetch.7" = load i32, ptr %"ntscale_$N", align 1
  store i32 1, ptr %"ntscale_$I", align 8
  %temp18 = alloca i32, align 4
  %temp19 = alloca i32, align 4
  store volatile i32 0, ptr %temp18, align 4
  %sub.3 = add nsw i32 %"ntscale_$N_fetch.7", -1
  store volatile i32 %sub.3, ptr %temp19, align 4
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.SIMD.129

DIR.OMP.SIMD.129:                                 ; preds = %DIR.OMP.SIMD.2
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LINEAR:TYPED.IV"(ptr %"ntscale_$I", i32 0, i32 1, i32 1),
    "QUAL.OMP.NONTEMPORAL"(ptr %"var$3.addr_a0$_fetch.6"),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %temp18, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %temp19, i32 0),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()
; CHECK-SAME: "QUAL.OMP.NONTEMPORAL"(ptr null)
  br label %DIR.OMP.SIMD.230

DIR.OMP.SIMD.230:                                 ; preds = %DIR.OMP.SIMD.129
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.SIMD.4, label %DIR.OMP.SIMD.1

omp.pdo.top7:                                     ; preds = %DIR.OMP.SIMD.1
  store volatile i32 0, ptr %temp18, align 4
  %temp_fetch.15 = load volatile i32, ptr %temp18, align 4
  %add.1 = add nsw i32 %temp_fetch.15, 1
  store i32 %add.1, ptr %"ntscale_$I", align 1
  br label %bb2

bb2:                                              ; preds = %bb2, %omp.pdo.top7
  %temp_fetch.18 = load volatile i32, ptr %temp18, align 4
  %add.2 = add nsw i32 %temp_fetch.18, 1
  store i32 %add.2, ptr %"ntscale_$I", align 1
  %"var$5.addr_a0$_fetch.21" = load ptr, ptr %"var$5.addr_a0$", align 8
  %int_sext11 = sext i32 %add.2 to i64
  %"var$5.addr_a0$_fetch.21[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %"var$5.addr_a0$_fetch.21", i64 %int_sext11)
  %"var$5.addr_a0$_fetch.21[]_fetch.23" = load float, ptr %"var$5.addr_a0$_fetch.21[]", align 1
  %mul.3 = fmul fast float %"var$5.addr_a0$_fetch.21[]_fetch.23", 2.000000e+00
  %"var$3.addr_a0$_fetch.24" = load ptr, ptr %"var$3.addr_a0$", align 8
  %"var$3.addr_a0$_fetch.24[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %"var$3.addr_a0$_fetch.24", i64 %int_sext11)
  store float %mul.3, ptr %"var$3.addr_a0$_fetch.24[]", align 1
  %temp_fetch.27 = load volatile i32, ptr %temp18, align 4
  %add.3 = add nsw i32 %temp_fetch.27, 1
  store volatile i32 %add.3, ptr %temp18, align 4
  %temp_fetch.28 = load volatile i32, ptr %temp18, align 4
  %temp_fetch.29 = load volatile i32, ptr %temp19, align 4
  %rel.4.not = icmp sgt i32 %temp_fetch.28, %temp_fetch.29
  br i1 %rel.4.not, label %DIR.OMP.END.SIMD.4, label %bb2

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.230
  store volatile i32 0, ptr %temp18, align 4
  %temp_fetch.13 = load volatile i32, ptr %temp19, align 4
  %temp_fetch.14 = load volatile i32, ptr %temp18, align 4
  %rel.3 = icmp slt i32 %temp_fetch.13, %temp_fetch.14
  br i1 %rel.3, label %DIR.OMP.END.SIMD.4, label %omp.pdo.top7

DIR.OMP.END.SIMD.4:                               ; preds = %DIR.OMP.SIMD.1, %bb2, %DIR.OMP.SIMD.230
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.4
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.431

DIR.OMP.END.SIMD.431:                             ; preds = %DIR.OMP.END.SIMD.3
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #1

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone speculatable }
