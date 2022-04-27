; INTEL_CUSTOMIZATION
; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,loop-simplify,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; ! Fortran test source:
; subroutine pArray(nDOFX)
;   integer::nDOFX, k
;   REAL(8)::uCR(1:nDOFX)
;   !$OMP TARGET PARALLEL DO PRIVATE(uCR)
;   do k = 1, 8
;      uCR(k) = k
;   end do
; end subroutine pArray
;
; This LIT test checks that the privaized array uCR is in addrspace(4) for device code

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define void @parray_(i32 addrspace(4)* noalias dereferenceable(4) %"parray_$NDOFX$argptr") #0 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8
  %"parray_$NDOFX$locptr" = alloca i32 addrspace(4)*, align 8
  %"parray_$K" = alloca i32, align 8
  %"parray_$UCR" = alloca double, align 16
  %"var$1" = alloca i32, align 4
  %"ascastB$val3.array.elements" = alloca i64, align 8
  store i32 addrspace(4)* %"parray_$NDOFX$argptr", i32 addrspace(4)** %"parray_$NDOFX$locptr", align 1
  %"parray_$NDOFX.1" = load i32 addrspace(4)*, i32 addrspace(4)** %"parray_$NDOFX$locptr", align 1
  %"ascast$val" = addrspacecast i64* %"ascastB$val3.array.elements" to i64 addrspace(4)*
  %"ascast$val5" = addrspacecast i32* %"parray_$K" to i32 addrspace(4)*
  %"ascast$val6" = addrspacecast i32* %"var$1" to i32 addrspace(4)*
  %"parray_$NDOFX.1_fetch.3" = load i32, i32 addrspace(4)* %"parray_$NDOFX.1", align 1
  store i32 %"parray_$NDOFX.1_fetch.3", i32* %"var$1", align 1
  %"$stacksave" = call i8* @llvm.stacksave()
  %"var$1_fetch.2" = load i32, i32* %"var$1", align 1
  %int_sext = sext i32 %"var$1_fetch.2" to i64
  %rel.1 = icmp sgt i64 %int_sext, 0
  %slct.1 = select i1 %rel.1, i64 %int_sext, i64 0
  %mul.1 = mul nsw i64 8, %slct.1
  %div.1 = sdiv i64 %mul.1, 8
  %"ascastB$val2" = alloca double, i64 %div.1, align 8
  %"ascastB$val3" = addrspacecast double* %"ascastB$val2" to double addrspace(4)*
  %"var$1_fetch.4" = load i32, i32* %"var$1", align 1
  %int_sext4 = sext i32 %"var$1_fetch.4" to i64
  %sub.1 = sub nsw i64 %int_sext4, 1
  %add.1 = add nsw i64 1, %sub.1
  %rel.2 = icmp sle i64 1, %int_sext4
  %rel.3 = icmp sle i64 1, %int_sext4
  %rel.4 = icmp ne i1 %rel.2, false
  %slct.2 = select i1 %rel.4, i64 %add.1, i64 0
  store i64 %slct.2, i64 addrspace(4)* %"ascast$val", align 1
  %"ascast$val_fetch.5" = load i64, i64 addrspace(4)* %"ascast$val", align 1
  %mul.2 = mul nsw i64 %"ascast$val_fetch.5", 8
  %temp = alloca i32, align 4
  %omp.pdo.start = addrspacecast i32* %temp to i32 addrspace(4)*
  store i32 1, i32 addrspace(4)* %omp.pdo.start, align 1
  %temp10 = alloca i32, align 4
  %omp.pdo.end = addrspacecast i32* %temp10 to i32 addrspace(4)*
  store i32 8, i32 addrspace(4)* %omp.pdo.end, align 1
  %temp11 = alloca i32, align 4
  %omp.pdo.step = addrspacecast i32* %temp11 to i32 addrspace(4)*
  store i32 1, i32 addrspace(4)* %omp.pdo.step, align 1
  %temp12 = alloca i64, align 8
  %omp.pdo.norm.iv = addrspacecast i64* %temp12 to i64 addrspace(4)*
  %temp13 = alloca i64, align 8
  %omp.pdo.norm.lb = addrspacecast i64* %temp13 to i64 addrspace(4)*
  store i64 0, i64 addrspace(4)* %omp.pdo.norm.lb, align 1
  %temp14 = alloca i64, align 8
  %omp.pdo.norm.ub = addrspacecast i64* %temp14 to i64 addrspace(4)*
  %omp.pdo.end_fetch.8 = load i32, i32 addrspace(4)* %omp.pdo.end, align 1
  %omp.pdo.start_fetch.9 = load i32, i32 addrspace(4)* %omp.pdo.start, align 1
  %sub.2 = sub nsw i32 %omp.pdo.end_fetch.8, %omp.pdo.start_fetch.9
  %omp.pdo.step_fetch.10 = load i32, i32 addrspace(4)* %omp.pdo.step, align 1
  %div.2 = sdiv i32 %sub.2, %omp.pdo.step_fetch.10
  %int_sext15 = sext i32 %div.2 to i64
  store i64 %int_sext15, i64 addrspace(4)* %omp.pdo.norm.ub, align 1
  %"ascast$val_fetch.6" = load i64, i64 addrspace(4)* %"ascast$val", align 1
  br label %bb_new7

bb_new7:                                          ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(double addrspace(4)* %"ascastB$val3", double addrspace(4)* %"ascastB$val3", i64 %mul.2, i64 547, i8 addrspace(4)* null, i8 addrspace(4)* null), "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascastB$val3", double 0.000000e+00, i64 %"ascast$val_fetch.6"), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %"ascast$val6"), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %"ascast$val5"), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %"ascast$val"), "QUAL.OMP.PRIVATE"(i64 addrspace(4)* %omp.pdo.norm.iv), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %omp.pdo.norm.lb), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %omp.pdo.norm.ub), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %omp.pdo.step), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %omp.pdo.start) ]
  %"ascast$val_fetch.7" = load i64, i64 addrspace(4)* %"ascast$val", align 1
  br label %bb_new13

bb_new13:                                         ; preds = %bb_new7
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SHARED"(i32 addrspace(4)* %"ascast$val6"), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %"ascast$val5"), "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascastB$val3", double 0.000000e+00, i64 %"ascast$val_fetch.7"), "QUAL.OMP.SHARED"(i32 addrspace(4)* %omp.pdo.step), "QUAL.OMP.SHARED"(i32 addrspace(4)* %omp.pdo.start), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %omp.pdo.norm.lb), "QUAL.OMP.NORMALIZED.IV"(i64 addrspace(4)* %omp.pdo.norm.iv), "QUAL.OMP.NORMALIZED.UB"(i64 addrspace(4)* %omp.pdo.norm.ub) ]
  %omp.pdo.norm.lb_fetch.11 = load i64, i64 addrspace(4)* %omp.pdo.norm.lb, align 1
  store i64 %omp.pdo.norm.lb_fetch.11, i64 addrspace(4)* %omp.pdo.norm.iv, align 1
  br label %omp.pdo.cond10

omp.pdo.cond10:                                   ; preds = %omp.pdo.body11, %bb_new13
  %omp.pdo.norm.iv_fetch.12 = load i64, i64 addrspace(4)* %omp.pdo.norm.iv, align 1
  %omp.pdo.norm.ub_fetch.13 = load i64, i64 addrspace(4)* %omp.pdo.norm.ub, align 1
  %rel.5 = icmp sle i64 %omp.pdo.norm.iv_fetch.12, %omp.pdo.norm.ub_fetch.13
  br i1 %rel.5, label %omp.pdo.body11, label %omp.pdo.epilog12

omp.pdo.body11:                                   ; preds = %omp.pdo.cond10
  %omp.pdo.norm.iv_fetch.14 = load i64, i64 addrspace(4)* %omp.pdo.norm.iv, align 1
  %int_sext8 = trunc i64 %omp.pdo.norm.iv_fetch.14 to i32
  %omp.pdo.step_fetch.15 = load i32, i32 addrspace(4)* %omp.pdo.step, align 1
  %mul.3 = mul nsw i32 %int_sext8, %omp.pdo.step_fetch.15
  %omp.pdo.start_fetch.16 = load i32, i32 addrspace(4)* %omp.pdo.start, align 1
  %add.2 = add nsw i32 %mul.3, %omp.pdo.start_fetch.16
  store i32 %add.2, i32 addrspace(4)* %"ascast$val5", align 1
  %"ascast$val5_fetch.17" = load i32, i32 addrspace(4)* %"ascast$val5", align 1
  %"(double)ascast$val5_fetch.17$" = sitofp i32 %"ascast$val5_fetch.17" to double
  %"ascast$val5_fetch.18" = load i32, i32 addrspace(4)* %"ascast$val5", align 1
  %int_sext9 = sext i32 %"ascast$val5_fetch.18" to i64

; Check that the privatized uCR array is in addrspace(4)
; CHECK:  [[PRIVARRAY:%[^ ]+]] = alloca double, i64 %"ascast$val_fetch.7", align 1
; CHECK:  [[CAST:%[^ ]+]] = addrspacecast double* [[PRIVARRAY]] to double addrspace(4)*
; CHECK:  %"ascastB$val3[]" = call double addrspace(4)* @llvm.intel.subscript.p4f64.i64.i64.p4f64.i64(i8 0, i64 1, i64 8, double addrspace(4)* elementtype(double) [[CAST]], i64 %int_sext9)

  %"ascastB$val3[]" = call double addrspace(4)* @llvm.intel.subscript.p4f64.i64.i64.p4f64.i64(i8 0, i64 1, i64 8, double addrspace(4)* elementtype(double) %"ascastB$val3", i64 %int_sext9)
  store double %"(double)ascast$val5_fetch.17$", double addrspace(4)* %"ascastB$val3[]", align 1
  %omp.pdo.norm.iv_fetch.19 = load i64, i64 addrspace(4)* %omp.pdo.norm.iv, align 1
  %add.3 = add nsw i64 %omp.pdo.norm.iv_fetch.19, 1
  store i64 %add.3, i64 addrspace(4)* %omp.pdo.norm.iv, align 1
  br label %omp.pdo.cond10

omp.pdo.epilog12:                                 ; preds = %omp.pdo.cond10
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.stackrestore(i8* %"$stacksave")
  ret void

}

; Function Attrs: nofree nosync nounwind willreturn
declare i8* @llvm.stacksave() #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind readnone speculatable
declare double addrspace(4)* @llvm.intel.subscript.p4f64.i64.i64.p4f64.i64(i8 %0, i64 %1, i64 %2, double addrspace(4)* %3, i64 %4) #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #2

; Function Attrs: nofree nosync nounwind willreturn
declare void @llvm.stackrestore(i8* %0) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "frame-pointer"="all" "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" }
attributes #1 = { nofree nosync nounwind willreturn }
attributes #2 = { nounwind }
attributes #3 = { nounwind readnone speculatable }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}

!0 = !{i32 0, i32 57, i32 -696906343, !"parray_", i32 6, i32 0, i32 0}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"openmp-device", i32 50}
; end INTEL_CUSTOMIZATION
