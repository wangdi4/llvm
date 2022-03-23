; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S %s | FileCheck %s
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

@"col_f_module_mp_col_fm_angle_avg_s_$MESH_NZM1" = internal addrspace(1) global i32 0, align 8
@"col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1" = internal addrspace(1) global i32 0, align 8
@"col_f_module_mp_col_fm_angle_avg_s_$TEMP_CONS" = internal addrspace(1) global double 0.000000e+00, align 8
@"col_f_module_mp_col_fm_angle_avg_s_$I3" = internal addrspace(1) global double 0.000000e+00, align 8
@"col_f_module_mp_col_fm_angle_avg_s_$I2" = internal addrspace(1) global double 0.000000e+00, align 8
@"col_f_module_mp_col_fm_angle_avg_s_$DZ" = internal addrspace(1) global double 0.000000e+00, align 8
@"col_f_module_mp_col_fm_angle_avg_s_$A" = internal addrspace(1) global double 0.000000e+00, align 8
@"col_f_module_mp_col_fm_angle_avg_s_$R" = internal addrspace(1) global double 0.000000e+00, align 8
@"col_f_module_mp_col_fm_angle_avg_s_$INDEX_RZ" = internal addrspace(1) global i32 0, align 8

; Function Attrs: nounwind uwtable
define void @col_f_module._() #0 {
alloca_0:
  ret void
}

; Function Attrs: nounwind uwtable
define void @col_f_module_mp_col_fm_angle_avg_s_(double addrspace(4)* %MS) #0 {
alloca_1:
  %"var$1" = alloca [8 x i64], align 8
  %"col_f_module_mp_col_fm_angle_avg_s_$INDEX_JP" = alloca i32, align 8
  %"col_f_module_mp_col_fm_angle_avg_s_$INDEX_J" = alloca i32, align 8
  %"col_f_module_mp_col_fm_angle_avg_s_$INDEX_DZ" = alloca i32, align 8
  %"var$2" = alloca i32, align 4
  %MS_entry = bitcast double addrspace(4)* %MS to [10 x [5 x [10 x double]]] addrspace(4)*
  %0 = addrspacecast i32* %"col_f_module_mp_col_fm_angle_avg_s_$INDEX_J" to i32 addrspace(4)*
  %1 = addrspacecast i32* %"col_f_module_mp_col_fm_angle_avg_s_$INDEX_DZ" to i32 addrspace(4)*
  %2 = addrspacecast i32* %"col_f_module_mp_col_fm_angle_avg_s_$INDEX_JP" to i32 addrspace(4)*
  %3 = addrspacecast i32* %"var$2" to i32 addrspace(4)*
  br label %bb2

bb2:                                              ; preds = %alloca_1
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.ENTER.DATA"(), "QUAL.OMP.MAP.TO"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1" to i32 addrspace(4)*)), "QUAL.OMP.MAP.TO"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$MESH_NZM1" to i32 addrspace(4)*)) ]
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TARGET.ENTER.DATA"() ]
  br label %bb5

bb8:                                              ; preds = %bb7
  br label %bb11

bb11:                                             ; preds = %bb8
  store i32 0, i32 addrspace(4)* %21, align 1
  %_fetch = load i32, i32 addrspace(4)* %17, align 1
  %_fetch2 = load i32, i32 addrspace(4)* %19, align 1
  %_fetch4 = load i32, i32 addrspace(4)* %21, align 1
  %mul = mul nsw i32 %_fetch4, %_fetch2
  %add = add nsw i32 %mul, %_fetch
  store i32 %add, i32 addrspace(4)* %0, align 1
  br label %bb12

bb12:                                             ; preds = %bb14, %bb11
  %_fetch6 = load i32, i32 addrspace(4)* %17, align 1
  %_fetch8 = load i32, i32 addrspace(4)* %19, align 1
  %_fetch10 = load i32, i32 addrspace(4)* %21, align 1
  %mul12 = mul nsw i32 %_fetch10, %_fetch8
  %add14 = add nsw i32 %mul12, %_fetch6
  store i32 %add14, i32 addrspace(4)* %0, align 1
  %_fetch16 = load i32, i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$MESH_NZM1" to i32 addrspace(4)*), align 1
  %sub = sub nsw i32 %_fetch16, 1
  store i32 %sub, i32 addrspace(4)* %3, align 1
  store i32 0, i32 addrspace(4)* %1, align 1
  %_fetch18 = load i32, i32 addrspace(4)* %3, align 1
  %rel = icmp slt i32 %_fetch18, 0
  br i1 %rel, label %bb14, label %bb15

bb15:                                             ; preds = %bb12
  br label %bb13

bb13:                                             ; preds = %bb23, %bb15
  br label %bb18

bb21:                                             ; preds = %bb20
  br label %bb24

bb24:                                             ; preds = %bb21
  store i32 0, i32 addrspace(4)* %9, align 1
  %_fetch20 = load i32, i32 addrspace(4)* %5, align 1
  %_fetch22 = load i32, i32 addrspace(4)* %7, align 1
  %_fetch24 = load i32, i32 addrspace(4)* %9, align 1
  %mul26 = mul nsw i32 %_fetch24, %_fetch22
  %add28 = add nsw i32 %mul26, %_fetch20
  store i32 %add28, i32 addrspace(4)* %2, align 1
  br label %bb25

bb25:                                             ; preds = %bb32, %bb24
  %_fetch30 = load i32, i32 addrspace(4)* %5, align 1
  %_fetch32 = load i32, i32 addrspace(4)* %7, align 1
  %_fetch34 = load i32, i32 addrspace(4)* %9, align 1
  %mul36 = mul nsw i32 %_fetch34, %_fetch32
  %add38 = add nsw i32 %mul36, %_fetch30
  store i32 %add38, i32 addrspace(4)* %2, align 1
  br label %bb26

bb27:                                             ; preds = %bb26
  %ptr_cast = bitcast [10 x [5 x [10 x double]]] addrspace(4)* %MS_entry to double addrspace(4)*
  br label %bb28

bb28:                                             ; preds = %bb27
  %"MS_entry[]" = call double addrspace(4)* @llvm.intel.subscript.p4f64.i64.i64.p4f64.i64(i8 2, i64 1, i64 400, double addrspace(4)* elementtype(double) %ptr_cast, i64 %int_sext52)
  br label %bb29

bb29:                                             ; preds = %bb28
  br label %bb30

bb30:                                             ; preds = %bb29
  %"MS_entry[][]" = call double addrspace(4)* @llvm.intel.subscript.p4f64.i64.i64.p4f64.i64(i8 1, i64 1, i64 80, double addrspace(4)* elementtype(double) %"MS_entry[]", i64 5)
  br label %bb31

bb31:                                             ; preds = %bb30
  br label %bb32

bb32:                                             ; preds = %bb31
  %"MS_entry[][][]" = call double addrspace(4)* @llvm.intel.subscript.p4f64.i64.i64.p4f64.i64(i8 0, i64 1, i64 8, double addrspace(4)* elementtype(double) %"MS_entry[][]", i64 %int_sext)
  store double %mul49, double addrspace(4)* %"MS_entry[][][]", align 1
  %_fetch54 = load i32, i32 addrspace(4)* %7, align 1
  %_fetch56 = load i32, i32 addrspace(4)* %9, align 1
  %add58 = add nsw i32 %_fetch56, 1
  store i32 %add58, i32 addrspace(4)* %9, align 1
  %_fetch60 = load i32, i32 addrspace(4)* %10, align 1
  %_fetch62 = load i32, i32 addrspace(4)* %9, align 1
  %rel64 = icmp sle i32 %_fetch62, %_fetch60
  br i1 %rel64, label %bb25, label %bb22

bb26:                                             ; preds = %bb25
  %_fetch39 = load double, double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$TEMP_CONS" to double addrspace(4)*), align 1
  %_fetch40 = load double, double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$DZ" to double addrspace(4)*), align 1
  %mul41 = fmul double %_fetch39, %_fetch40
  %_fetch42 = load double, double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$I2" to double addrspace(4)*), align 1
  %_fetch43 = load double, double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$A" to double addrspace(4)*), align 1
  %mul44 = fmul double %_fetch42, %_fetch43
  %_fetch45 = load double, double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$I3" to double addrspace(4)*), align 1
  %_fetch46 = load double, double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$R" to double addrspace(4)*), align 1
  %mul47 = fmul double %_fetch45, %_fetch46
  %sub48 = fsub double %mul44, %mul47
  %mul49 = fmul double %mul41, %sub48
  %_fetch50 = load i32, i32 addrspace(4)* %2, align 1
  %int_sext = sext i32 %_fetch50 to i64
  %_fetch51 = load i32, i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$INDEX_RZ" to i32 addrspace(4)*), align 1
  %int_sext52 = sext i32 %_fetch51 to i64
  br label %bb27

bb22:                                             ; preds = %bb32
  br label %bb23

bb17:                                             ; preds = %bb5
  %_fetch65 = load i32, i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1" to i32 addrspace(4)*), align 1
  %temp = alloca i32, align 1
  %5 = addrspacecast i32* %temp to i32 addrspace(4)*
  %temp66 = alloca i32, align 1
  %6 = addrspacecast i32* %temp66 to i32 addrspace(4)*
  %temp67 = alloca i32, align 1
  %7 = addrspacecast i32* %temp67 to i32 addrspace(4)*
  store i32 1, i32 addrspace(4)* %5, align 1
  store i32 %_fetch65, i32 addrspace(4)* %6, align 1
  store i32 1, i32 addrspace(4)* %7, align 1
  %_fetch68 = load i32, i32 addrspace(4)* %5, align 1
  store i32 %_fetch68, i32 addrspace(4)* %2, align 1
  %temp69 = alloca i32, align 1
  %8 = addrspacecast i32* %temp69 to i32 addrspace(4)*
  %temp70 = alloca i32, align 1
  %9 = addrspacecast i32* %temp70 to i32 addrspace(4)*
  %temp71 = alloca i32, align 1
  %10 = addrspacecast i32* %temp71 to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %8, align 1
  store i32 0, i32 addrspace(4)* %9, align 1
  %_fetch72 = load i32, i32 addrspace(4)* %7, align 1
  %_fetch73 = load i32, i32 addrspace(4)* %5, align 1
  %_fetch74 = load i32, i32 addrspace(4)* %6, align 1
  %sub75 = sub nsw i32 %_fetch74, %_fetch73
  %div = sdiv i32 %sub75, %_fetch72
  store i32 %div, i32 addrspace(4)* %10, align 1
  br label %bb3

bb3:                                              ; preds = %bb17
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"([10 x [5 x [10 x double]]] addrspace(4)* %MS_entry), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$DZ" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$A" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$INDEX_RZ" to i32 addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$R" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %2), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %1), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %0), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$TEMP_CONS" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$I3" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$I2" to double addrspace(4)*)), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %2), "QUAL.OMP.FIRSTPRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$R" to double addrspace(4)*)), "QUAL.OMP.FIRSTPRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$TEMP_CONS" to double addrspace(4)*)), "QUAL.OMP.FIRSTPRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$I3" to double addrspace(4)*)), "QUAL.OMP.FIRSTPRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$I2" to double addrspace(4)*)), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$INDEX_RZ" to i32 addrspace(4)*)), "QUAL.OMP.FIRSTPRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$A" to double addrspace(4)*)), "QUAL.OMP.FIRSTPRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$DZ" to double addrspace(4)*)), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$MESH_NZM1" to i32 addrspace(4)*)), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1" to i32 addrspace(4)*)), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %1), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %0), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %8), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %9), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %10), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %5), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %6), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %7), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %20), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %21), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %22), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %17), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %18), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %19) ]
  br label %bb4

bb4:                                              ; preds = %bb3
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %0), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %1), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %2), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$R" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$INDEX_RZ" to i32 addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$A" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$DZ" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$I2" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$I3" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$TEMP_CONS" to double addrspace(4)*)), "QUAL.OMP.SHARED"([10 x [5 x [10 x double]]] addrspace(4)* %MS_entry), "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1" to i32 addrspace(4)*)), "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$MESH_NZM1" to i32 addrspace(4)*)), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %1), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %0), "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$MESH_NZM1" to i32 addrspace(4)*)), "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1" to i32 addrspace(4)*)), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %8), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %9), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %10), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %5), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %6), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %7), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1" to i32 addrspace(4)*)), "QUAL.OMP.FIRSTPRIVATE"([10 x [5 x [10 x double]]] addrspace(4)* %MS_entry), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %20), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %21), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %22), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %17), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %18), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %19) ]
  br label %bb6

bb6:                                              ; preds = %bb4
  %13 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(), "QUAL.OMP.COLLAPSE"(i32 2), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %0), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %1), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %2), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$R" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$INDEX_RZ" to i32 addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$A" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$DZ" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$I2" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$I3" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$TEMP_CONS" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %0), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %21), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %22), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %20), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %21), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %22), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %1), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %3) ]
  br label %bb7

bb7:                                              ; preds = %bb6
  %_fetch76 = load i32, i32 addrspace(4)* %20, align 1
  store i32 %_fetch76, i32 addrspace(4)* %21, align 1
  %_fetch77 = load i32, i32 addrspace(4)* %21, align 1
  %_fetch78 = load i32, i32 addrspace(4)* %22, align 1
  %rel79 = icmp slt i32 %_fetch78, %_fetch77
  br i1 %rel79, label %bb10, label %bb8

bb18:                                             ; preds = %bb13
  %14 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.LINEAR"(i32 addrspace(4)* %2, i32 1), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %2), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$R" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$TEMP_CONS" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$I3" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$I2" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$INDEX_RZ" to i32 addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$A" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$DZ" to double addrspace(4)*)), "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1" to i32 addrspace(4)*)), "QUAL.OMP.SHARED"([10 x [5 x [10 x double]]] addrspace(4)* %MS_entry), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %9), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %10), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %8), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %9), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %10), "QUAL.OMP.DEFAULT.NONE"() ]
  br label %bb19

bb19:                                             ; preds = %bb18
  %15 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR"(i32 addrspace(4)* %2, i32 1), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$DZ" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$A" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$INDEX_RZ" to i32 addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$I2" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$I3" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$TEMP_CONS" to double addrspace(4)*)), "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspacecast (double addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$R" to double addrspace(4)*)) ]
  br label %bb20

bb20:                                             ; preds = %bb19
  %_fetch80 = load i32, i32 addrspace(4)* %8, align 1
  store i32 %_fetch80, i32 addrspace(4)* %9, align 1
  %_fetch81 = load i32, i32 addrspace(4)* %9, align 1
  %_fetch82 = load i32, i32 addrspace(4)* %10, align 1
  %rel83 = icmp slt i32 %_fetch82, %_fetch81
  br i1 %rel83, label %bb23, label %bb21

bb23:                                             ; preds = %bb22, %bb20
  call void @llvm.directive.region.exit(token %15) [ "DIR.OMP.END.SIMD"() ]
  call void @llvm.directive.region.exit(token %14) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %_fetch85 = load i32, i32 addrspace(4)* %1, align 1
  %add87 = add nsw i32 %_fetch85, 1
  store i32 %add87, i32 addrspace(4)* %1, align 1
  %_fetch89 = load i32, i32 addrspace(4)* %3, align 1
  %_fetch91 = load i32, i32 addrspace(4)* %1, align 1
  %rel93 = icmp sle i32 %_fetch91, %_fetch89
  br i1 %rel93, label %bb13, label %bb16

bb16:                                             ; preds = %bb23
  br label %bb14

bb14:                                             ; preds = %bb16, %bb12
  %_fetch95 = load i32, i32 addrspace(4)* %19, align 1
  %_fetch97 = load i32, i32 addrspace(4)* %21, align 1
  %add99 = add nsw i32 %_fetch97, 1
  store i32 %add99, i32 addrspace(4)* %21, align 1
  %_fetch101 = load i32, i32 addrspace(4)* %22, align 1
  %_fetch103 = load i32, i32 addrspace(4)* %21, align 1
  %rel105 = icmp sle i32 %_fetch103, %_fetch101
  br i1 %rel105, label %bb12, label %bb9

bb9:                                              ; preds = %bb14
  br label %bb10

bb10:                                             ; preds = %bb9, %bb7
  call void @llvm.directive.region.exit(token %13) [ "DIR.OMP.END.DISTRIBUTE"() ]
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.TARGET"() ]
  %16 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.EXIT.DATA"(), "QUAL.OMP.MAP.DELETE"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1" to i32 addrspace(4)*)), "QUAL.OMP.MAP.DELETE"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$MESH_NZM1" to i32 addrspace(4)*)), "QUAL.OMP.MAP.FROM"([10 x [5 x [10 x double]]] addrspace(4)* %MS_entry) ]
  call void @llvm.directive.region.exit(token %16) [ "DIR.OMP.END.TARGET.EXIT.DATA"() ]
  br label %bb1

bb5:                                              ; preds = %bb2
  %_fetch106 = load i32, i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @"col_f_module_mp_col_fm_angle_avg_s_$MESH_NRM1" to i32 addrspace(4)*), align 1
  %temp107 = alloca i32, align 1
  %17 = addrspacecast i32* %temp107 to i32 addrspace(4)*
  %temp108 = alloca i32, align 1
  %18 = addrspacecast i32* %temp108 to i32 addrspace(4)*
  %temp109 = alloca i32, align 1
  %19 = addrspacecast i32* %temp109 to i32 addrspace(4)*
  store i32 1, i32 addrspace(4)* %17, align 1
  store i32 %_fetch106, i32 addrspace(4)* %18, align 1
  store i32 1, i32 addrspace(4)* %19, align 1
  %_fetch110 = load i32, i32 addrspace(4)* %17, align 1
  store i32 %_fetch110, i32 addrspace(4)* %0, align 1
  %temp111 = alloca i32, align 1
  %20 = addrspacecast i32* %temp111 to i32 addrspace(4)*
  %temp112 = alloca i32, align 1
  %21 = addrspacecast i32* %temp112 to i32 addrspace(4)*
  %temp113 = alloca i32, align 1
  %22 = addrspacecast i32* %temp113 to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %20, align 1
  store i32 0, i32 addrspace(4)* %21, align 1
  %_fetch114 = load i32, i32 addrspace(4)* %19, align 1
  %_fetch115 = load i32, i32 addrspace(4)* %17, align 1
  %_fetch116 = load i32, i32 addrspace(4)* %18, align 1
  %sub117 = sub nsw i32 %_fetch116, %_fetch115
  %div118 = sdiv i32 %sub117, %_fetch114
  store i32 %div118, i32 addrspace(4)* %22, align 1
  br label %bb17

bb1:                                              ; preds = %bb10
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #1

; Function Attrs: nounwind readnone speculatable
declare double addrspace(4)* @llvm.intel.subscript.p4f64.i64.i64.p4f64.i64(i8 %0, i64 %1, i64 %2, double addrspace(4)* elementtype(double) %3, i64 %4) #2

attributes #0 = { nounwind uwtable "contains-openmp-target"="true" "intel-lang"="fortran" "min-legal-vector-width"="0" }
attributes #1 = { nounwind }
attributes #2 = { nounwind readnone speculatable }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2054, i32 1837986, !"col_f_module_mp_col_fm_angle_avg_s_", i32 12, i32 0, i32 0}
