; RUN: opt -scoped-noalias -tbaa -hir-ssa-deconstruction -hir-temp-cleanup -hir-runtime-dd -hir-lmm -hir-vec-dir-insert -VPlanDriverHIR -hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that we multiversion the loop, vectorize and unroll the inner loop and then the outer loop is unrolled.

; HIR-
; + DO i1 = 0, 15, 1   <DO_LOOP>  <MVTag: 43>
; |   + DO i2 = 0, 15, 1   <DO_LOOP>  <MVTag: 44>
; |   |   %t52 = (%t2)[sext.i32.i64(%t3) * i1 + i2];
; |   |   %t54 = (%t4)[0].3;
; |   |   %t56 = (%t4)[0].2;
; |   |   %t59 = 1  <<  trunc.i32.i5(%t56) + -1;
; |   |   %t62 = (zext.i8.i32(%t52) * %t54) + %t59  >>  trunc.i32.i5(%t56);
; |   |   %t63 = (%t4)[0].4;
; |   |   (%t0)[sext.i32.i64(%t1) * i1 + i2] = -1 * trunc.i32.i8(smax(-256, (-1 + (-1 * smax(0, (%t62 + %t63)))))) + -1;
; |   + END LOOP
; + END LOOP


; CHECK: modified

; CHECK: if (

; no loop inside the if case
; CHECK-NOT: DO i1

; CHECK: else


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.x264_weight_t = type { [8 x i16], [8 x i16], i32, i32, i32, void (i8*, i32, i8*, i32, %struct.x264_weight_t*, i32)**, [8 x i8] }

; Function Attrs: inlinehint norecurse nounwind uwtable
define hidden fastcc void @"mc_weight|_._._._._.16.16"(i8* nocapture %t0, i32 %t1, i8* nocapture readonly %t2, i32 %t3, %struct.x264_weight_t* nocapture readonly %t4) unnamed_addr #0 {
  %t6 = getelementptr inbounds %struct.x264_weight_t, %struct.x264_weight_t* %t4, i64 0, i32 2
  br label %t40

t40:                                     ; preds = %t5
  %t41 = sext i32 %t1 to i64
  %t42 = sext i32 %t3 to i64
  %t43 = getelementptr inbounds %struct.x264_weight_t, %struct.x264_weight_t* %t4, i64 0, i32 3
  %t44 = getelementptr inbounds %struct.x264_weight_t, %struct.x264_weight_t* %t4, i64 0, i32 4
  br label %t45

t45:                                     ; preds = %t73, %t40
  %t46 = phi i32 [ %t74, %t73 ], [ 0, %t40 ]
  %t47 = phi i8* [ %t75, %t73 ], [ %t0, %t40 ]
  %t48 = phi i8* [ %t76, %t73 ], [ %t2, %t40 ]
  br label %t49

t49:                                     ; preds = %t49, %t45
  %t50 = phi i64 [ 0, %t45 ], [ %t71, %t49 ]
  %t51 = getelementptr inbounds i8, i8* %t48, i64 %t50
  %t52 = load i8, i8* %t51, align 1, !tbaa !11
  %t53 = zext i8 %t52 to i32
  %t54 = load i32, i32* %t43, align 4, !tbaa !12
  %t55 = mul nsw i32 %t54, %t53
  %t56 = load i32, i32* %t6, align 16, !tbaa !3
  %t57 = add i32 %t56, 31
  %t58 = and i32 %t57, 31
  %t59 = shl i32 1, %t58
  %t60 = add nsw i32 %t59, %t55
  %t61 = and i32 %t56, 31
  %t62 = ashr i32 %t60, %t61
  %t63 = load i32, i32* %t44, align 8, !tbaa !13
  %t64 = add nsw i32 %t62, %t63
  %t65 = icmp sgt i32 %t64, 0
  %t66 = select i1 %t65, i32 %t64, i32 0
  %t67 = icmp slt i32 %t66, 255
  %t68 = select i1 %t67, i32 %t66, i32 255
  %t69 = trunc i32 %t68 to i8
  %t70 = getelementptr inbounds i8, i8* %t47, i64 %t50
  store i8 %t69, i8* %t70, align 1, !tbaa !11
  %t71 = add nuw nsw i64 %t50, 1
  %t72 = icmp eq i64 %t71, 16
  br i1 %t72, label %t73, label %t49

t73:                                     ; preds = %t49
  %t74 = add nuw nsw i32 %t46, 1
  %t75 = getelementptr inbounds i8, i8* %t47, i64 %t41
  %t76 = getelementptr inbounds i8, i8* %t48, i64 %t42
  %t77 = icmp eq i32 %t74, 16
  br i1 %t77, label %t78, label %t45

t78:                                     ; preds = %t73
  br label %t80

t80:                                     ; preds = %t79, %t78
  ret void
}

attributes #0 = { inlinehint norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "prefer-vector-width"="256" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+rtm,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2}

!0 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang a53756907774b7d85a523756d285be3e3ac08d1c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 6fddea4e5028365a0b1e546a5fcda322f025b45f)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{!4, !9, i64 32}
!4 = !{!"struct@x264_weight_t", !5, i64 0, !5, i64 16, !9, i64 32, !9, i64 36, !9, i64 40, !10, i64 48}
!5 = !{!"array@_ZTSA8_s", !6, i64 0}
!6 = !{!"short", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!"int", !7, i64 0}
!10 = !{!"unspecified pointer", !7, i64 0}
!11 = !{!7, !7, i64 0}
!12 = !{!4, !9, i64 36}
!13 = !{!4, !9, i64 40}
