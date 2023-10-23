; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-distribute-memrec,print<hir>" -aa-pipeline="basic-aa" -disable-output %s 2>&1 | FileCheck %s

; Loop is not distributed as it is vectorizable.
; TODO: Investigate recreating scalar-expansion behavior using distribute points.
; XFAIL: *

; Check that correct HIR is generated when we have potential recomputed temps during
; scalar expansion. There is a dependence for the following temp:

; %mul.866 = %"band_new_$pde_[]_fetch.3428"  *  %"band_new_$pde_[]_fetch.3428";

; However, %"band_new_$pde_[]_fetch.3428" is also a scalar expansion candidate. Due to
; LCA logic, we load the value for %"band_new_$pde_[]_fetch.3428" under a later if node


; TODO: investigate if we can allow recomputation by making LCA consider
; recomputation dependencies when calculating insertion points.

; HIR Before
;        BEGIN REGION { }
;              + DO i1 = 0, -2, 1   <DO_LOOP>
;              |   %"band_new_$piwc_[]_fetch.3410" = (%"band_new_$piwc_")[i1];
;              |   if (%"band_new_$piwc_[]_fetch.3410" < 0x3EE4F8B580000000)
;              |   {
;              |      (%"band_new_$ti_")[i1] = 0.000000e+00;
;              |      (%"band_new_$wi_")[i1] = 0.000000e+00;
;              |      (null)[i1] = 0.000000e+00;
;              |      (null)[i1] = 0.000000e+00;
;              |      (null)[i1] = 0.000000e+00;
;              |      (null)[i1] = 0.000000e+00;
;              |   }
;              |   else
;              |   {
;              |      %"band_new_$pde_[]_fetch.3428" = (%"band_new_$pde_")[i1];
;              |      %mul.866 = %"band_new_$pde_[]_fetch.3428"  *  %"band_new_$pde_[]_fetch.3428";
;              |      %mul.867 = %"band_new_$pde_[]_fetch.3428"  *  %mul.866;
;              |      if (%"band_new_$ib__fetch.3435" < 7)
;              |      {
;              |         %mul.868 = (null)[0]  *  1.000000e+03;
;              |         %mul.869 = %"band_new_$piwc_[]_fetch.3410"  *  %mul.868;
;              |         %div.181 = (null)[0]  /  %"band_new_$pde_[]_fetch.3428";
;              |         %add.514 = (null)[0]  +  %div.181;
;              |         %mul.872 = %mul.869  *  %add.514;
;              |         %add.516 = 0.000000e+00  +  0.000000e+00;
;              |         %add.517 = %add.516  +  0.000000e+00;
;              |         %sub.264 = 1.000000e+00  -  %add.517;
;              |         %mul.882 = %"band_new_$pde_[]_fetch.3428"  *  (null)[1];
;              |         %add.518 = (null)[0]  +  %mul.882;
;              |         %mul.884 = %mul.866  *  (null)[2];
;              |         %mul.886 = %mul.867  *  (null)[3];
;              |         %add.520 = 0.000000e+00  +  %mul.886;
;              |         %mul.889 = %"band_new_$pde_[]_fetch.3428"  *  (null)[1];
;              |         %add.521 = (null)[0]  +  %mul.889;
;              |         %mul.891 = %mul.866  *  (null)[0];
;              |         %mul.893 = %mul.867  *  0.000000e+00;
;              |         %mul.894 = %sub.264  *  0.000000e+00;
;              |         %mul.895 = %mul.872  *  0.000000e+00;
;              |         (%"band_new_$wi_")[i1] = 0.000000e+00;
;              |         %sub.268 = %add.520  -  0.000000e+00;
;              |      }
;              |      else
;              |      {
;              |         %mul.912 = %mul.866  *  0.000000e+00;
;              |         %mul.914 = %mul.867  *  0.000000e+00;
;              |         %mul.918 = %"band_new_$pde_[]_fetch.3428"  *  (null)[0];
;              |      }
;              |   }
;              + END LOOP
;        END REGION

; HIR After


; CHECK: BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, 288230376151711743, 1   <DO_LOOP>
;              |   %min = (-64 * i1 + -2 <= 63) ? -64 * i1 + -2 : 63;
;              |
; CHECK:       |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
;              |   |   %"band_new_$piwc_[]_fetch.3410" = (%"band_new_$piwc_")[64 * i1 + i2];
;              |   |   if (%"band_new_$piwc_[]_fetch.3410" < 0x3EE4F8B580000000)
;              |   |   {
;              |   |      (%"band_new_$ti_")[64 * i1 + i2] = 0.000000e+00;
;              |   |      (%"band_new_$wi_")[64 * i1 + i2] = 0.000000e+00;
;              |   |      (null)[64 * i1 + i2] = 0.000000e+00;
;              |   |      (null)[64 * i1 + i2] = 0.000000e+00;
;              |   |      (null)[64 * i1 + i2] = 0.000000e+00;
;              |   |      (null)[64 * i1 + i2] = 0.000000e+00;
;              |   |   }
;              |   |   else
;              |   |   {
; CHECK:       |   |      %"band_new_$pde_[]_fetch.3428" = (%"band_new_$pde_")[64 * i1 + i2];
; CHECK:       |   |      %mul.866 = %"band_new_$pde_[]_fetch.3428"  *  %"band_new_$pde_[]_fetch.3428";
; CHECK:       |   |      (%.TempArray)[0][i2] = %mul.866;
;              |   |      %mul.867 = %"band_new_$pde_[]_fetch.3428"  *  %mul.866;
;              |   |      (%.TempArray1)[0][i2] = %mul.867;
;              |   |      if (%"band_new_$ib__fetch.3435" < 7)
;              |   |      {
;              |   |         %mul.868 = (null)[0]  *  1.000000e+03;
;              |   |         %mul.869 = %"band_new_$piwc_[]_fetch.3410"  *  %mul.868;
;              |   |         %div.181 = (null)[0]  /  %"band_new_$pde_[]_fetch.3428";
;              |   |         %add.514 = (null)[0]  +  %div.181;
;              |   |         %mul.872 = %mul.869  *  %add.514;
;              |   |         (%.TempArray3)[0][i2] = %mul.872;
;              |   |         %add.516 = 0.000000e+00  +  0.000000e+00;
;              |   |         %add.517 = %add.516  +  0.000000e+00;
;              |   |         %sub.264 = 1.000000e+00  -  %add.517;
;              |   |         (%.TempArray5)[0][i2] = %sub.264;
;              |   |         %mul.882 = %"band_new_$pde_[]_fetch.3428"  *  (null)[1];
;              |   |         %add.518 = (null)[0]  +  %mul.882;
;              |   |         %mul.884 = %mul.866  *  (null)[2];
;              |   |         %mul.886 = %mul.867  *  (null)[3];
;              |   |         %add.520 = 0.000000e+00  +  %mul.886;
;              |   |         (%.TempArray7)[0][i2] = %add.520;
;              |   |         %mul.889 = %"band_new_$pde_[]_fetch.3428"  *  (null)[1];
;              |   |         (%.TempArray9)[0][i2] = %mul.889;
;              |   |      }
;              |   |   }
; CHECK:       |   + END LOOP
;              |
; CHECK:       |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
;              |   |   %"band_new_$piwc_[]_fetch.3410" = (%"band_new_$piwc_")[64 * i1 + i2];
;              |   |   if (%"band_new_$piwc_[]_fetch.3410" >=u 0x3EE4F8B580000000)
;              |   |   {
; CHECK:       |   |      %mul.866 = (%.TempArray)[0][i2];
; CHECK:       |   |      %mul.867 = (%.TempArray1)[0][i2];
;              |   |      if (%"band_new_$ib__fetch.3435" < 7)
;              |   |      {
;              |   |         %mul.872 = (%.TempArray3)[0][i2];
;              |   |         %sub.264 = (%.TempArray5)[0][i2];
;              |   |         %add.520 = (%.TempArray7)[0][i2];
;              |   |         %mul.889 = (%.TempArray9)[0][i2];
;              |   |         %add.521 = (null)[0]  +  %mul.889;
;              |   |         %mul.891 = %mul.866  *  (null)[0];
;              |   |         %mul.893 = %mul.867  *  0.000000e+00;
;              |   |         %mul.894 = %sub.264  *  0.000000e+00;
;              |   |         %mul.895 = %mul.872  *  0.000000e+00;
;              |   |         (%"band_new_$wi_")[64 * i1 + i2] = 0.000000e+00;
;              |   |         %sub.268 = %add.520  -  0.000000e+00;
;              |   |      }
;              |   |      else
;              |   |      {
; CHECK:       |   |         %"band_new_$pde_[]_fetch.3428" = (%"band_new_$pde_")[64 * i1 + i2];
;              |   |         %mul.912 = %mul.866  *  0.000000e+00;
;              |   |         %mul.914 = %mul.867  *  0.000000e+00;
;              |   |         %mul.918 = %"band_new_$pde_[]_fetch.3428"  *  (null)[0];
;              |   |      }
;              |   |   }
;              |   + END LOOP
;              + END LOOP
;        END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(read, argmem: readwrite, inaccessiblemem: none) uwtable
define void @module_ra_flg_mp_ice_98_(ptr noalias nocapture readonly dereferenceable(4) %"band_new_$nv_", ptr noalias nocapture readonly dereferenceable(4) %"band_new_$ib_", ptr noalias nocapture readonly dereferenceable(4) %"band_new_$pde_", ptr noalias nocapture readonly dereferenceable(4) %"band_new_$piwc_", ptr noalias nocapture writeonly dereferenceable(4) %"band_new_$ti_", ptr noalias nocapture writeonly dereferenceable(4) %"band_new_$wi_", ptr noalias nocapture writeonly dereferenceable(4) %"band_new_$wwi_") #0 {
alloca_37:
  %"band_new_$ib__fetch.3435" = load i32, ptr %"band_new_$ib_", align 1
  %rel.492 = icmp slt i32 %"band_new_$ib__fetch.3435", 7
  %"ice5_mp_cp_[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) null, i64 1)
  %"ice5_mp_cp_[][]29" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) null, i64 2)
  %"ice5_mp_cp_[][]32" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) null, i64 3)
  %"ice5_mp_cp_[][]35" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) null, i64 4)
  %"ice5_mp_dps_[][]39" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) null, i64 2)
  br label %bb673

bb673:                                            ; preds = %bb678_endif, %alloca_37
  %indvars.iv = phi i64 [ 1, %alloca_37 ], [ %indvars.iv.next, %bb678_endif ]
  %"band_new_$piwc_[]126" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"band_new_$piwc_", i64 %indvars.iv)
  %"band_new_$piwc_[]_fetch.3410" = load float, ptr %"band_new_$piwc_[]126", align 1
  %rel.491 = fcmp olt float %"band_new_$piwc_[]_fetch.3410", 0x3EE4F8B580000000
  br i1 %rel.491, label %bb_new1647_then, label %bb_new1655_else

bb_new1659_then:                                  ; preds = %bb_new1655_else
  %"band_new_$dz_[]_fetch.3437" = load float, ptr null, align 1
  %mul.868 = fmul float %"band_new_$dz_[]_fetch.3437", 1.000000e+03
  %mul.869 = fmul float %"band_new_$piwc_[]_fetch.3410", %mul.868
  %"ice5_mp_ap_[][]_fetch.3441" = load float, ptr null, align 4
  %"ice5_mp_ap_[][]_fetch.3443" = load float, ptr null, align 4
  %div.181 = fdiv float %"ice5_mp_ap_[][]_fetch.3443", %"band_new_$pde_[]_fetch.3428"
  %add.514 = fadd float %"ice5_mp_ap_[][]_fetch.3441", %div.181
  %mul.872 = fmul float %mul.869, %add.514
  %add.516 = fadd float 0.000000e+00, 0.000000e+00
  %add.517 = fadd float %add.516, 0.000000e+00
  %sub.264 = fsub float 1.000000e+00, %add.517
  %"ice5_mp_cp_[][]_fetch.3457" = load float, ptr %"ice5_mp_cp_[][]", align 4
  %"ice5_mp_cp_[][]_fetch.3459" = load float, ptr %"ice5_mp_cp_[][]29", align 4
  %mul.882 = fmul float %"band_new_$pde_[]_fetch.3428", %"ice5_mp_cp_[][]_fetch.3459"
  %add.518 = fadd float %"ice5_mp_cp_[][]_fetch.3457", %mul.882
  %"ice5_mp_cp_[][]_fetch.3462" = load float, ptr %"ice5_mp_cp_[][]32", align 4
  %mul.884 = fmul float %mul.866, %"ice5_mp_cp_[][]_fetch.3462"
  %"ice5_mp_cp_[][]_fetch.3465" = load float, ptr %"ice5_mp_cp_[][]35", align 4
  %mul.886 = fmul float %mul.867, %"ice5_mp_cp_[][]_fetch.3465"
  %add.520 = fadd float 0.000000e+00, %mul.886
  %"ice5_mp_dps_[][]_fetch.3468" = load float, ptr null, align 4
  %"ice5_mp_dps_[][]_fetch.3470" = load float, ptr %"ice5_mp_dps_[][]39", align 4
  %mul.889 = fmul float %"band_new_$pde_[]_fetch.3428", %"ice5_mp_dps_[][]_fetch.3470"
  %add.521 = fadd float %"ice5_mp_dps_[][]_fetch.3468", %mul.889
  %"ice5_mp_dps_[][]_fetch.3473" = load float, ptr null, align 4
  %mul.891 = fmul float %mul.866, %"ice5_mp_dps_[][]_fetch.3473"
  %mul.893 = fmul float %mul.867, 0.000000e+00
  %mul.894 = fmul float %sub.264, 0.000000e+00
  %mul.895 = fmul float %mul.872, 0.000000e+00
  %"band_new_$wi_[]48" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"band_new_$wi_", i64 %indvars.iv)
  store float 0.000000e+00, ptr %"band_new_$wi_[]48", align 1
  %sub.268 = fsub float %add.520, 0.000000e+00
  br label %bb678_endif

bb_new1676_else:                                  ; preds = %bb_new1655_else
  %mul.912 = fmul float %mul.866, 0.000000e+00
  %mul.914 = fmul float %mul.867, 0.000000e+00
  %"ice5_mp_cp_[][]_fetch.3543" = load float, ptr null, align 4
  %mul.918 = fmul float %"band_new_$pde_[]_fetch.3428", %"ice5_mp_cp_[][]_fetch.3543"
  br label %bb678_endif

bb_new1647_then:                                  ; preds = %bb673
  %"band_new_$ti_[]122" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"band_new_$ti_", i64 %indvars.iv)
  store float 0.000000e+00, ptr %"band_new_$ti_[]122", align 1
  %"band_new_$wi_[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"band_new_$wi_", i64 %indvars.iv)
  store float 0.000000e+00, ptr %"band_new_$wi_[]", align 1
  %"band_new_$wwi_[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) null, i64 %indvars.iv)
  store float 0.000000e+00, ptr %"band_new_$wwi_[][]", align 1
  %"band_new_$wwi_[][]6" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) null, i64 %indvars.iv)
  store float 0.000000e+00, ptr %"band_new_$wwi_[][]6", align 1
  %"band_new_$wwi_[][]9" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) null, i64 %indvars.iv)
  store float 0.000000e+00, ptr %"band_new_$wwi_[][]9", align 1
  %"band_new_$wwi_[][]12" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) null, i64 %indvars.iv)
  store float 0.000000e+00, ptr %"band_new_$wwi_[][]12", align 1
  br label %bb678_endif

bb_new1655_else:                                  ; preds = %bb673
  %"band_new_$pde_[]124" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"band_new_$pde_", i64 %indvars.iv)
  %"band_new_$pde_[]_fetch.3428" = load float, ptr %"band_new_$pde_[]124", align 1
  %mul.866 = fmul float %"band_new_$pde_[]_fetch.3428", %"band_new_$pde_[]_fetch.3428"
  %mul.867 = fmul float %"band_new_$pde_[]_fetch.3428", %mul.866
  br i1 %rel.492, label %bb_new1659_then, label %bb_new1676_else

bb678_endif:                                      ; preds = %bb_new1647_then, %bb_new1676_else, %bb_new1659_then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 0
  br i1 %exitcond, label %bb674.loopexit, label %bb673

bb674.loopexit:                                   ; preds = %bb678_endif
  ret void

; uselistorder directives
  uselistorder float %"band_new_$pde_[]_fetch.3428", { 0, 1, 2, 6, 5, 4, 3 }
  uselistorder float %mul.866, { 0, 3, 2, 1 }
  uselistorder float %mul.867, { 2, 1, 0 }
}

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

; uselistorder directives
uselistorder ptr @llvm.intel.subscript.p0.i64.i64.p0.i64, { 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 }

attributes #0 = { nofree norecurse nosync nounwind memory(read, argmem: readwrite, inaccessiblemem: none) uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
