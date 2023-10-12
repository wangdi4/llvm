;
; Test to check that we use a wide vector version of for_random_number when
; it's possible (svml and advanced opts are enabled and target has
; corresponding support).
;
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vector-library=SVML -enable-intel-advanced-opts %s 2>&1 | FileCheck %s
; RUN: opt -S -passes="vplan-vec" -vector-library=SVML -enable-intel-advanced-opts %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @for_simd_random_number_avx()

; Function Attrs: nofree nounwind uwtable
define void @tests_mp_zg0007_(ptr noalias nocapture writeonly dereferenceable(8) %TDA2L, ptr noalias nocapture readonly dereferenceable(4) %N) local_unnamed_addr #0 {
alloca_1:
  %N_fetch.1 = load i32, ptr %N, align 1
  %rel.3.not3 = icmp slt i32 %N_fetch.1, 1
  br i1 %rel.3.not3, label %loop_exit11, label %loop_body10.preheader0

loop_body10.preheader0:                            ; preds = %alloca_1
  %int_sext = zext i32 %N_fetch.1 to i64
  %0 = add nuw nsw i64 %int_sext, 1
  br label %loop_body10.preheader

loop_body10.preheader:                            ; preds = %alloca_1
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %loop_body10

loop_body10:                                      ; preds = %loop_body10.preheader, %loop_body10
  %"$loop_ctr.04" = phi i64 [ %add.1, %loop_body10 ], [ 1, %loop_body10.preheader ]
  %"TDA2L[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %TDA2L, i64 %"$loop_ctr.04")
  %func_result = tail call reassoc ninf nsz arcp contract afn double @for_random_number() #2
  store double %func_result, ptr %"TDA2L[]", align 1
  %add.1 = add nuw nsw i64 %"$loop_ctr.04", 1
  %exitcond = icmp eq i64 %add.1, %0
  br i1 %exitcond, label %loop_exit11.loopexit, label %loop_body10

loop_exit11.loopexit:                             ; preds = %loop_body10
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %loop_exit11

loop_exit11:                                      ; preds = %loop_exit11.loopexit, %alloca_1
  ret void
}

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)

; Function Attrs: nofree
declare !llfort.intrin_id !0 double @for_random_number() local_unnamed_addr #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { nofree nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none"  "advanced-optim"="true" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree nounwind uwtable "intel-lang"="fortran" }
attributes #2 = { nounwind }
!0 = !{i32 764}

