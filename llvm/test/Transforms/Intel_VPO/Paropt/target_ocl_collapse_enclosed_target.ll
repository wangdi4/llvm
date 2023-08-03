; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -switch-to-offload -S %s | FileCheck %s

; Original code:
; module col_f_module
;   contains
;     subroutine col_fm_angle_avg_s(Ms)
;       implicit none
;       real (8), dimension(10,5,10) :: Ms
;       integer :: index_dz, index_J, index_jp, index_rz
;       real (8) :: r, a, dz
;       real (8) :: I2, I3, temp_cons
;       integer :: mesh_Nrm1, mesh_Nzm1
;
; !$omp target enter data map(to:mesh_Nrm1, mesh_Nzm1)
; !$omp target teams distribute collapse(2) default(none)        &
; !$omp& private(index_J, index_dz, index_jp, r, index_rz, a, dz)  &
; !$omp& private(I2, I3, temp_cons)             &
; !$omp& shared(Ms, mesh_Nrm1, mesh_Nzm1)
;      do index_J=1,mesh_Nrm1
;         do index_dz=0,mesh_Nzm1-1
; !$omp parallel do simd default(none)                                    &
; !$omp& private(dz, a, index_rz, I2, I3, temp_cons, r)   &
; !$omp& shared(Ms, mesh_Nrm1)
;            do index_jp=1, mesh_Nrm1
;               Ms(index_jp,5,index_rz) = temp_cons*dz*(I2*a-I3*r)
;            enddo  !index_jp
;         enddo ! index_dz
;      enddo !index_J
; !$omp end target teams distribute
;
; !$omp target exit data map(from:Ms) map(delete:mesh_Nrm1, mesh_Nzm1)
;     end subroutine col_fm_angle_avg_s
;
; end module col_f_module

; Check that QUAL.OMP.OFFLOAD.NDRANGE is set only once, and that only
; the distribute loop has QUAL.OMP.OFFLOAD.KNOWN.NDRANGE set:
; CHECK: QUAL.OMP.OFFLOAD.NDRANGE
; CHECK-NOT: QUAL.OMP.OFFLOAD.NDRANGE
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(){{.*}}QUAL.OMP.OFFLOAD.KNOWN.NDRANGE
; CHECK-NOT: QUAL.OMP.OFFLOAD.KNOWN.NDRANGE

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define void @col_f_module._() #0 {
alloca_0:
  ret void

}

; Function Attrs: noinline nounwind optnone uwtable
define void @col_f_module_mp_col_fm_angle_avg_s_(double addrspace(4)* noalias dereferenceable(8) %"MS$argptr") #0 {
alloca_1:
  %"$io_ctx" = alloca [8 x i64], align 8, !llfort.type_idx !3
  %"MS$locptr" = alloca double addrspace(4)*, align 8
  %"col_f_module_mp_col_fm_angle_avg_s_$MESH_NZM1" = alloca i32, align 4, !llfort.type_idx !4
  %"col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1" = alloca i32, align 4, !llfort.type_idx !5
  %"col_f_module_mp_col_fm_angle_avg_s_$TEMP_CONS" = alloca double, align 8, !llfort.type_idx !6
  %"col_f_module_mp_col_fm_angle_avg_s_$I3" = alloca double, align 8, !llfort.type_idx !7
  %"col_f_module_mp_col_fm_angle_avg_s_$I2" = alloca double, align 8, !llfort.type_idx !8
  %"col_f_module_mp_col_fm_angle_avg_s_$DZ" = alloca double, align 8, !llfort.type_idx !9
  %"col_f_module_mp_col_fm_angle_avg_s_$A" = alloca double, align 8, !llfort.type_idx !10
  %"col_f_module_mp_col_fm_angle_avg_s_$R" = alloca double, align 8, !llfort.type_idx !11
  %"col_f_module_mp_col_fm_angle_avg_s_$INDEX_RZ" = alloca i32, align 4, !llfort.type_idx !12
  %"col_f_module_mp_col_fm_angle_avg_s_$INDEX_JP" = alloca i32, align 4, !llfort.type_idx !13
  %"col_f_module_mp_col_fm_angle_avg_s_$INDEX_J" = alloca i32, align 4, !llfort.type_idx !14
  %"col_f_module_mp_col_fm_angle_avg_s_$INDEX_DZ" = alloca i32, align 4, !llfort.type_idx !15
  store double addrspace(4)* %"MS$argptr", double addrspace(4)** %"MS$locptr", align 8
  %MS.1 = load double addrspace(4)*, double addrspace(4)** %"MS$locptr", align 8
  %MS.1_entry = bitcast double addrspace(4)* %MS.1 to [10 x [5 x [10 x double]]] addrspace(4)*
  %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1" = addrspacecast i32* %"col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1" to i32 addrspace(4)*
  %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NZM1" = addrspacecast i32* %"col_f_module_mp_col_fm_angle_avg_s_$MESH_NZM1" to i32 addrspace(4)*
  %"ascast$col_f_module_mp_col_fm_angle_avg_s_$I2" = addrspacecast double* %"col_f_module_mp_col_fm_angle_avg_s_$I2" to double addrspace(4)*, !llfort.type_idx !8
  %"ascast$col_f_module_mp_col_fm_angle_avg_s_$I3" = addrspacecast double* %"col_f_module_mp_col_fm_angle_avg_s_$I3" to double addrspace(4)*, !llfort.type_idx !7
  %"ascast$col_f_module_mp_col_fm_angle_avg_s_$TEMP_CONS" = addrspacecast double* %"col_f_module_mp_col_fm_angle_avg_s_$TEMP_CONS" to double addrspace(4)*, !llfort.type_idx !6
  %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_J" = addrspacecast i32* %"col_f_module_mp_col_fm_angle_avg_s_$INDEX_J" to i32 addrspace(4)*, !llfort.type_idx !14
  %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_DZ" = addrspacecast i32* %"col_f_module_mp_col_fm_angle_avg_s_$INDEX_DZ" to i32 addrspace(4)*, !llfort.type_idx !15
  %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_JP" = addrspacecast i32* %"col_f_module_mp_col_fm_angle_avg_s_$INDEX_JP" to i32 addrspace(4)*, !llfort.type_idx !13
  %"ascast$col_f_module_mp_col_fm_angle_avg_s_$R" = addrspacecast double* %"col_f_module_mp_col_fm_angle_avg_s_$R" to double addrspace(4)*, !llfort.type_idx !11
  %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_RZ" = addrspacecast i32* %"col_f_module_mp_col_fm_angle_avg_s_$INDEX_RZ" to i32 addrspace(4)*, !llfort.type_idx !12
  %"ascast$col_f_module_mp_col_fm_angle_avg_s_$A" = addrspacecast double* %"col_f_module_mp_col_fm_angle_avg_s_$A" to double addrspace(4)*, !llfort.type_idx !10
  %"ascast$col_f_module_mp_col_fm_angle_avg_s_$DZ" = addrspacecast double* %"col_f_module_mp_col_fm_angle_avg_s_$DZ" to double addrspace(4)*, !llfort.type_idx !9
  br label %bb_new2

bb_new2:  ; preds = %alloca_1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.ENTER.DATA"(),
    "QUAL.OMP.MAP.TO"(i32 addrspace(4)* %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1", i32 addrspace(4)* %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1", i64 4, i64 1, i8 addrspace(4)* null, i8 addrspace(4)* null), ; MAP type: 1 = 0x1 = TO (0x1)
    "QUAL.OMP.MAP.TO"(i32 addrspace(4)* %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NZM1", i32 addrspace(4)* %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NZM1", i64 4, i64 1, i8 addrspace(4)* null, i8 addrspace(4)* null) ] ; MAP type: 1 = 0x1 = TO (0x1)
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.ENTER.DATA"() ]
  %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NZM1_fetch.8" = load i32, i32 addrspace(4)* %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NZM1", align 4, !llfort.type_idx !4
  %sub.3 = sub nsw i32 %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NZM1_fetch.8", 1
  %temp = alloca i32, align 4, !llfort.type_idx !16
  %do.end = addrspacecast i32* %temp to i32 addrspace(4)*
  store i32 %sub.3, i32 addrspace(4)* %do.end, align 4
  %temp1 = alloca i32, align 4, !llfort.type_idx !16
  %do.norm.lb = addrspacecast i32* %temp1 to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %do.norm.lb, align 4
  %temp2 = alloca i32, align 4, !llfort.type_idx !16
  %do.norm.ub = addrspacecast i32* %temp2 to i32 addrspace(4)*
  %do.end_fetch.9 = load i32, i32 addrspace(4)* %do.end, align 4, !llfort.type_idx !16
  %add.3 = add nsw i32 %do.end_fetch.9, 1
  %sub.4 = sub nsw i32 %add.3, 1
  store i32 %sub.4, i32 addrspace(4)* %do.norm.ub, align 4
  %temp3 = alloca i32, align 4, !llfort.type_idx !16
  %do.norm.iv = addrspacecast i32* %temp3 to i32 addrspace(4)*
  %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1_fetch.2" = load i32, i32 addrspace(4)* %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1", align 4, !llfort.type_idx !5
  %temp9 = alloca i32, align 4, !llfort.type_idx !16
  %omp.pdo.end10 = addrspacecast i32* %temp9 to i32 addrspace(4)*
  store i32 %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1_fetch.2", i32 addrspace(4)* %omp.pdo.end10, align 4
  %temp11 = alloca i32, align 4, !llfort.type_idx !16
  %omp.pdo.norm.iv12 = addrspacecast i32* %temp11 to i32 addrspace(4)*
  %temp13 = alloca i32, align 4, !llfort.type_idx !16
  %omp.pdo.norm.lb14 = addrspacecast i32* %temp13 to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %omp.pdo.norm.lb14, align 4
  %temp15 = alloca i32, align 4, !llfort.type_idx !16
  %omp.pdo.norm.ub16 = addrspacecast i32* %temp15 to i32 addrspace(4)*
  %omp.pdo.end_fetch.3 = load i32, i32 addrspace(4)* %omp.pdo.end10, align 4, !llfort.type_idx !16
  %sub.1 = sub nsw i32 %omp.pdo.end_fetch.3, 1
  %add.1 = add nsw i32 %sub.1, 1
  %sub.2 = sub nsw i32 %add.1, 1
  store i32 %sub.2, i32 addrspace(4)* %omp.pdo.norm.ub16, align 4
  br label %bb_new3

bb_new3:  ; preds = %bb_new2
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"([10 x [5 x [10 x double]]] addrspace(4)* %MS.1_entry, [10 x [5 x [10 x double]]] addrspace(4)* %MS.1_entry, i64 4000, i64 547, i8 addrspace(4)* null, i8 addrspace(4)* null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %do.norm.iv, i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %omp.pdo.norm.iv12, i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_DZ", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_J", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_JP", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_RZ", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$R", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$A", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$DZ", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$I2", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$I3", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$TEMP_CONS", double 0.000000e+00, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %do.norm.ub, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %do.norm.lb, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %omp.pdo.norm.ub16, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %omp.pdo.norm.lb14, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1", i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NZM1", i32 0, i64 1) ]
  br label %bb_new4

bb_new4:  ; preds = %bb_new3
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(i32 addrspace(4)* %do.norm.ub, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(i32 addrspace(4)* %do.norm.lb, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(i32 addrspace(4)* %omp.pdo.norm.ub16, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(i32 addrspace(4)* %omp.pdo.norm.lb14, i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(i32 addrspace(4)* %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1", i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(i32 addrspace(4)* %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NZM1", i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"([10 x [5 x [10 x double]]] addrspace(4)* %MS.1_entry, double 0.000000e+00, i64 500),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %do.norm.iv, i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %omp.pdo.norm.iv12, i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_DZ", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_J", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_JP", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_RZ", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$R", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$A", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$DZ", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$I2", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$I3", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$TEMP_CONS", double 0.000000e+00, i64 1) ]
  br label %bb_new5

bb_new5:  ; preds = %bb_new4
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.COLLAPSE"(i32 2),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_DZ", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_J", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_JP", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_RZ", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$R", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$A", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$DZ", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$I2", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$I3", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$TEMP_CONS", double 0.000000e+00, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %do.norm.lb, i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %omp.pdo.norm.lb14, i32 0, i64 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(i32 addrspace(4)* %omp.pdo.norm.iv12, i32 0, i32 addrspace(4)* %do.norm.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(i32 addrspace(4)* %omp.pdo.norm.ub16, i32 0, i32 addrspace(4)* %do.norm.ub, i32 0) ]
  %omp.pdo.norm.lb_fetch.4 = load i32, i32 addrspace(4)* %omp.pdo.norm.lb14, align 4, !llfort.type_idx !16
  store i32 %omp.pdo.norm.lb_fetch.4, i32 addrspace(4)* %omp.pdo.norm.iv12, align 4
  br label %omp.pdo.cond7

omp.pdo.cond7:  ; preds = %do.epilog14, %bb_new5
  %omp.pdo.norm.iv_fetch.5 = load i32, i32 addrspace(4)* %omp.pdo.norm.iv12, align 4, !llfort.type_idx !16
  %omp.pdo.norm.ub_fetch.6 = load i32, i32 addrspace(4)* %omp.pdo.norm.ub16, align 4, !llfort.type_idx !16
  %rel.1 = icmp sle i32 %omp.pdo.norm.iv_fetch.5, %omp.pdo.norm.ub_fetch.6
  br i1 %rel.1, label %omp.pdo.body8, label %omp.pdo.epilog9

omp.pdo.body8:  ; preds = %omp.pdo.cond7
  %omp.pdo.norm.iv_fetch.7 = load i32, i32 addrspace(4)* %omp.pdo.norm.iv12, align 4, !llfort.type_idx !16
  %add.2 = add nsw i32 %omp.pdo.norm.iv_fetch.7, 1
  store i32 %add.2, i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_J", align 4
  %do.norm.lb_fetch.10 = load i32, i32 addrspace(4)* %do.norm.lb, align 4, !llfort.type_idx !16
  store i32 %do.norm.lb_fetch.10, i32 addrspace(4)* %do.norm.iv, align 4
  br label %do.cond12

do.cond12:  ; preds = %omp.pdo.body8, %omp.pdo.epilog19
  %do.norm.iv_fetch.11 = load i32, i32 addrspace(4)* %do.norm.iv, align 4, !llfort.type_idx !16
  %do.norm.ub_fetch.12 = load i32, i32 addrspace(4)* %do.norm.ub, align 4, !llfort.type_idx !16
  %rel.2 = icmp sle i32 %do.norm.iv_fetch.11, %do.norm.ub_fetch.12
  br i1 %rel.2, label %do.body13, label %do.epilog14

do.body13:  ; preds = %do.cond12
  %do.norm.iv_fetch.13 = load i32, i32 addrspace(4)* %do.norm.iv, align 4, !llfort.type_idx !16
  store i32 %do.norm.iv_fetch.13, i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_DZ", align 4
  %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1_fetch.14" = load i32, i32 addrspace(4)* %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1", align 4, !llfort.type_idx !5
  %temp5 = alloca i32, align 4, !llfort.type_idx !16
  %omp.pdo.end = addrspacecast i32* %temp5 to i32 addrspace(4)*
  store i32 %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1_fetch.14", i32 addrspace(4)* %omp.pdo.end, align 4
  %temp6 = alloca i32, align 4, !llfort.type_idx !16
  %omp.pdo.norm.iv = addrspacecast i32* %temp6 to i32 addrspace(4)*
  %temp7 = alloca i32, align 4, !llfort.type_idx !16
  %omp.pdo.norm.lb = addrspacecast i32* %temp7 to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %omp.pdo.norm.lb, align 4
  %temp8 = alloca i32, align 4, !llfort.type_idx !16
  %omp.pdo.norm.ub = addrspacecast i32* %temp8 to i32 addrspace(4)*
  %omp.pdo.end_fetch.15 = load i32, i32 addrspace(4)* %omp.pdo.end, align 4, !llfort.type_idx !16
  %sub.5 = sub nsw i32 %omp.pdo.end_fetch.15, 1
  %add.4 = add nsw i32 %sub.5, 1
  %sub.6 = sub nsw i32 %add.4, 1
  store i32 %sub.6, i32 addrspace(4)* %omp.pdo.norm.ub, align 4
  br label %bb_new15

bb_new15:  ; preds = %do.body13
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.DEFAULT.NONE"(),
    "QUAL.OMP.SHARED:TYPED"(i32 addrspace(4)* %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1", i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"([10 x [5 x [10 x double]]] addrspace(4)* %MS.1_entry, double 0.000000e+00, i64 500),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_RZ", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$R", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$A", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$DZ", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$I2", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$I3", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$TEMP_CONS", double 0.000000e+00, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %omp.pdo.norm.lb, i32 0, i64 1),
    "QUAL.OMP.LASTPRIVATE:TYPED"(i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_JP", i32 0, i64 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(i32 addrspace(4)* %omp.pdo.norm.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(i32 addrspace(4)* %omp.pdo.norm.ub, i32 0) ]
  br label %bb_new20

bb_new20:  ; preds = %bb_new15
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LINEAR:TYPED.IV"(i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_JP", i32 0, i64 1, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_RZ", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$R", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$A", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$DZ", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$I2", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$I3", double 0.000000e+00, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$TEMP_CONS", double 0.000000e+00, i64 1),
    "QUAL.OMP.LIVEIN"([10 x [5 x [10 x double]]] addrspace(4)* %MS.1_entry) ]
  %omp.pdo.norm.lb_fetch.16 = load i32, i32 addrspace(4)* %omp.pdo.norm.lb, align 4, !llfort.type_idx !16
  store i32 %omp.pdo.norm.lb_fetch.16, i32 addrspace(4)* %omp.pdo.norm.iv, align 4
  %omp.pdo.norm.iv_fetch.17 = load i32, i32 addrspace(4)* %omp.pdo.norm.iv, align 4, !llfort.type_idx !16
  %omp.pdo.norm.ub_fetch.18 = load i32, i32 addrspace(4)* %omp.pdo.norm.ub, align 4, !llfort.type_idx !16
  %rel.3 = icmp sle i32 %omp.pdo.norm.iv_fetch.17, %omp.pdo.norm.ub_fetch.18
  br i1 %rel.3, label %omp.pdo.body18, label %omp.pdo.epilog19

omp.pdo.body18:  ; preds = %omp.pdo.body18, %bb_new20
  %omp.pdo.norm.iv_fetch.19 = load i32, i32 addrspace(4)* %omp.pdo.norm.iv, align 4, !llfort.type_idx !16
  %add.5 = add nsw i32 %omp.pdo.norm.iv_fetch.19, 1
  store i32 %add.5, i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_JP", align 4
  %"ascast$col_f_module_mp_col_fm_angle_avg_s_$TEMP_CONS_fetch.20" = load double, double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$TEMP_CONS", align 8, !llfort.type_idx !6
  %"ascast$col_f_module_mp_col_fm_angle_avg_s_$DZ_fetch.21" = load double, double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$DZ", align 8, !llfort.type_idx !9
  %mul.1 = fmul reassoc ninf nsz arcp contract afn double %"ascast$col_f_module_mp_col_fm_angle_avg_s_$TEMP_CONS_fetch.20", %"ascast$col_f_module_mp_col_fm_angle_avg_s_$DZ_fetch.21"
  %"ascast$col_f_module_mp_col_fm_angle_avg_s_$I2_fetch.22" = load double, double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$I2", align 8, !llfort.type_idx !8
  %"ascast$col_f_module_mp_col_fm_angle_avg_s_$A_fetch.23" = load double, double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$A", align 8, !llfort.type_idx !10
  %mul.2 = fmul reassoc ninf nsz arcp contract afn double %"ascast$col_f_module_mp_col_fm_angle_avg_s_$I2_fetch.22", %"ascast$col_f_module_mp_col_fm_angle_avg_s_$A_fetch.23"
  %"ascast$col_f_module_mp_col_fm_angle_avg_s_$I3_fetch.24" = load double, double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$I3", align 8, !llfort.type_idx !7
  %"ascast$col_f_module_mp_col_fm_angle_avg_s_$R_fetch.25" = load double, double addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$R", align 8, !llfort.type_idx !11
  %mul.3 = fmul reassoc ninf nsz arcp contract afn double %"ascast$col_f_module_mp_col_fm_angle_avg_s_$I3_fetch.24", %"ascast$col_f_module_mp_col_fm_angle_avg_s_$R_fetch.25"
  %sub.7 = fsub reassoc ninf nsz arcp contract afn double %mul.2, %mul.3
  %mul.4 = fmul reassoc ninf nsz arcp contract afn double %mul.1, %sub.7
  %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_JP_fetch.26" = load i32, i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_JP", align 4, !llfort.type_idx !13
  %int_sext = sext i32 %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_JP_fetch.26" to i64, !llfort.type_idx !17
  %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_RZ_fetch.27" = load i32, i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_RZ", align 4, !llfort.type_idx !12
  %int_sext4 = sext i32 %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_RZ_fetch.27" to i64, !llfort.type_idx !17
  %"(ptr)MS.1_entry$" = bitcast [10 x [5 x [10 x double]]] addrspace(4)* %MS.1_entry to double addrspace(4)*, !llfort.type_idx !18
  %"MS.1_entry[]" = call double addrspace(4)* @llvm.intel.subscript.p4f64.i64.i64.p4f64.i64(i8 2, i64 1, i64 400, double addrspace(4)* elementtype(double) %"(ptr)MS.1_entry$", i64 %int_sext4), !llfort.type_idx !18
  %"MS.1_entry[][]" = call double addrspace(4)* @llvm.intel.subscript.p4f64.i64.i64.p4f64.i64(i8 1, i64 1, i64 80, double addrspace(4)* elementtype(double) %"MS.1_entry[]", i64 5), !llfort.type_idx !18
  %"MS.1_entry[][][]" = call double addrspace(4)* @llvm.intel.subscript.p4f64.i64.i64.p4f64.i64(i8 0, i64 1, i64 8, double addrspace(4)* elementtype(double) %"MS.1_entry[][]", i64 %int_sext), !llfort.type_idx !18
  store double %mul.4, double addrspace(4)* %"MS.1_entry[][][]", align 8
  %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_JP_fetch.28" = load i32, i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_JP", align 4, !llfort.type_idx !13
  %add.6 = add nsw i32 %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_JP_fetch.28", 1
  store i32 %add.6, i32 addrspace(4)* %"ascast$col_f_module_mp_col_fm_angle_avg_s_$INDEX_JP", align 4
  %omp.pdo.norm.iv_fetch.29 = load i32, i32 addrspace(4)* %omp.pdo.norm.iv, align 4, !llfort.type_idx !16
  %add.7 = add nsw i32 %omp.pdo.norm.iv_fetch.29, 1
  store i32 %add.7, i32 addrspace(4)* %omp.pdo.norm.iv, align 4
  %omp.pdo.norm.iv_fetch.30 = load i32, i32 addrspace(4)* %omp.pdo.norm.iv, align 4, !llfort.type_idx !16
  %omp.pdo.norm.ub_fetch.31 = load i32, i32 addrspace(4)* %omp.pdo.norm.ub, align 4, !llfort.type_idx !16
  %rel.4 = icmp sle i32 %omp.pdo.norm.iv_fetch.30, %omp.pdo.norm.ub_fetch.31
  br i1 %rel.4, label %omp.pdo.body18, label %omp.pdo.epilog19

omp.pdo.epilog19:  ; preds = %omp.pdo.body18, %bb_new20
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.SIMD"() ]
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %do.norm.iv_fetch.32 = load i32, i32 addrspace(4)* %do.norm.iv, align 4, !llfort.type_idx !16
  %add.8 = add nsw i32 %do.norm.iv_fetch.32, 1
  store i32 %add.8, i32 addrspace(4)* %do.norm.iv, align 4
  br label %do.cond12

do.epilog14:  ; preds = %do.cond12
  %omp.pdo.norm.iv_fetch.33 = load i32, i32 addrspace(4)* %omp.pdo.norm.iv12, align 4, !llfort.type_idx !16
  %add.9 = add nsw i32 %omp.pdo.norm.iv_fetch.33, 1
  store i32 %add.9, i32 addrspace(4)* %omp.pdo.norm.iv12, align 4
  br label %omp.pdo.cond7

omp.pdo.epilog9:  ; preds = %omp.pdo.cond7
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.DISTRIBUTE"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  br label %bb_new23

bb_new23:  ; preds = %omp.pdo.epilog9
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.EXIT.DATA"(),
    "QUAL.OMP.MAP.DELETE"(i32 addrspace(4)* %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1", i32 addrspace(4)* %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1", i64 4, i64 8, i8 addrspace(4)* null, i8 addrspace(4)* null), ; MAP type: 8 = 0x8 = DELETE (0x8)
    "QUAL.OMP.MAP.DELETE"(i32 addrspace(4)* %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NZM1", i32 addrspace(4)* %"ascastB$col_f_module_mp_col_fm_angle_avg_s_$MESH_NZM1", i64 4, i64 8, i8 addrspace(4)* null, i8 addrspace(4)* null), ; MAP type: 8 = 0x8 = DELETE (0x8)
    "QUAL.OMP.MAP.FROM"([10 x [5 x [10 x double]]] addrspace(4)* %MS.1_entry, [10 x [5 x [10 x double]]] addrspace(4)* %MS.1_entry, i64 4000, i64 2, i8 addrspace(4)* null, i8 addrspace(4)* null) ] ; MAP type: 2 = 0x2 = FROM (0x2)
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TARGET.EXIT.DATA"() ]
  ret void

}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare double addrspace(4)* @llvm.intel.subscript.p4f64.i64.i64.p4f64.i64(i8, i64, i64, double addrspace(4)*, i64) #2

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "frame-pointer"="all" "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" }
attributes #1 = { nounwind }
attributes #2 = { nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}

!0 = !{i32 0, i32 52, i32 -704196305, !"col_f_module_mp_col_fm_angle_avg_s_", i32 12, i32 0, i32 0}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"openmp-device", i32 50}
!3 = !{i64 30}
!4 = !{i64 40}
!5 = !{i64 41}
!6 = !{i64 42}
!7 = !{i64 43}
!8 = !{i64 44}
!9 = !{i64 45}
!10 = !{i64 46}
!11 = !{i64 47}
!12 = !{i64 48}
!13 = !{i64 49}
!14 = !{i64 50}
!15 = !{i64 51}
!16 = !{i64 2}
!17 = !{i64 3}
!18 = !{i64 6}
; end INTEL_CUSTOMIZATION
