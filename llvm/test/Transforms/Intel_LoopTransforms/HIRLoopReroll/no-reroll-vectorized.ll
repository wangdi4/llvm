; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Make sure loop reroll doesn't happen. The loop is already vectorized. Rerolling the loop is unlikely to help performance.

; CHECK: Function: mkl_unrollcopy
;
; CHECK:          BEGIN REGION { modified }
; CHECK:                + DO i1 = 0, ((-256 + %smax) /u 256), 1   <DO_LOOP>
; CHECK:                |   (<16 x float>*)(%dest)[256 * i1] = (<16 x float>*)(%src)[256 * i1];
; CHECK:                |   (<16 x float>*)(%dest)[256 * i1 + 64] = (<16 x float>*)(%src)[256 * i1 + 64];
; CHECK:                |   (<16 x float>*)(%dest)[256 * i1 + 128] = (<16 x float>*)(%src)[256 * i1 + 128];
; CHECK:                |   (<16 x float>*)(%dest)[256 * i1 + 192] = (<16 x float>*)(%src)[256 * i1 + 192];
; CHECK:                + END LOOP
; CHECK:                   %add.ptr11 = &((%src)[256 * ((-256 + %smax) /u 256) + 256]);
; CHECK:                   %add.ptr12 = &((%dest)[256 * ((-256 + %smax) /u 256) + 256]);
; CHECK:                   %sub = %smax + -256 * ((-256 + %smax) /u 256)  +  -256;
; CHECK:          END REGION

; CHECK: Function: mkl_unrollcopy

; CHECK:         BEGIN REGION { modified }
; CHECK:               + DO i1 = 0, ((-256 + %smax) /u 256), 1   <DO_LOOP>
; CHECK:               |   (<16 x float>*)(%dest)[256 * i1] = (<16 x float>*)(%src)[256 * i1];
; CHECK:               |   (<16 x float>*)(%dest)[256 * i1 + 64] = (<16 x float>*)(%src)[256 * i1 + 64];
; CHECK:               |   (<16 x float>*)(%dest)[256 * i1 + 128] = (<16 x float>*)(%src)[256 * i1 + 128];
; CHECK:               |   (<16 x float>*)(%dest)[256 * i1 + 192] = (<16 x float>*)(%src)[256 * i1 + 192];
; CHECK:               + END LOOP
; CHECK:                  %add.ptr11 = &((%src)[256 * ((-256 + %smax) /u 256) + 256]);
; CHECK:                  %add.ptr12 = &((%dest)[256 * ((-256 + %smax) /u 256) + 256]);
; CHECK:                  %sub = %smax + -256 * ((-256 + %smax) /u 256)  +  -256;
; CHECK:         END REGION

;Module Before HIR
; ModuleID = 'new-short.c'
source_filename = "new-short.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind uwtable
define void @mkl_unrollcopy(ptr noalias noundef %dest, i64 noundef %dmax, ptr noalias noundef %src, i64 noundef %smax) local_unnamed_addr #0 {
entry:
  %cmp41 = icmp ugt i64 %smax, 255
  br i1 %cmp41, label %while.body.preheader, label %while.end

while.body.preheader:                             ; preds = %entry
  br label %while.body

while.body:                                       ; preds = %while.body.preheader, %while.body
  %tail.044 = phi i64 [ %sub, %while.body ], [ %smax, %while.body.preheader ]
  %dp.043 = phi ptr [ %add.ptr12, %while.body ], [ %dest, %while.body.preheader ]
  %sp.042 = phi ptr [ %add.ptr11, %while.body ], [ %src, %while.body.preheader ]
  %sp.0.val = load <16 x float>, ptr %sp.042, align 1, !tbaa !4
  store <16 x float> %sp.0.val, ptr %dp.043, align 1, !tbaa !4, !alias.scope !7
  %add.ptr = getelementptr inbounds i8, ptr %dp.043, i64 64, !intel-tbaa !4
  %add.ptr1 = getelementptr inbounds i8, ptr %sp.042, i64 64, !intel-tbaa !4
  %add.ptr1.val = load <16 x float>, ptr %add.ptr1, align 1, !tbaa !4
  store <16 x float> %add.ptr1.val, ptr %add.ptr, align 1, !tbaa !4, !alias.scope !10
  %add.ptr3 = getelementptr inbounds i8, ptr %dp.043, i64 128, !intel-tbaa !4
  %add.ptr4 = getelementptr inbounds i8, ptr %sp.042, i64 128, !intel-tbaa !4
  %add.ptr4.val = load <16 x float>, ptr %add.ptr4, align 1, !tbaa !4
  store <16 x float> %add.ptr4.val, ptr %add.ptr3, align 1, !tbaa !4, !alias.scope !13
  %add.ptr7 = getelementptr inbounds i8, ptr %dp.043, i64 192
  %add.ptr9 = getelementptr inbounds i8, ptr %sp.042, i64 192
  %add.ptr9.val = load <16 x float>, ptr %add.ptr9, align 1, !tbaa !4
  store <16 x float> %add.ptr9.val, ptr %add.ptr7, align 1, !tbaa !4, !alias.scope !16
  %add.ptr11 = getelementptr inbounds i8, ptr %sp.042, i64 256, !intel-tbaa !4
  %add.ptr12 = getelementptr inbounds i8, ptr %dp.043, i64 256, !intel-tbaa !4
  %sub = add i64 %tail.044, -256
  %cmp = icmp ugt i64 %sub, 255
  br i1 %cmp, label %while.body, label %while.end.loopexit, !llvm.loop !19

while.end.loopexit:                               ; preds = %while.body
  %add.ptr11.lcssa = phi ptr [ %add.ptr11, %while.body ]
  %add.ptr12.lcssa = phi ptr [ %add.ptr12, %while.body ]
  %sub.lcssa = phi i64 [ %sub, %while.body ]
  br label %while.end

while.end:                                        ; preds = %while.end.loopexit, %entry
  %sp.0.lcssa = phi ptr [ %src, %entry ], [ %add.ptr11.lcssa, %while.end.loopexit ]
  %dp.0.lcssa = phi ptr [ %dest, %entry ], [ %add.ptr12.lcssa, %while.end.loopexit ]
  %tail.0.lcssa = phi i64 [ %smax, %entry ], [ %sub.lcssa, %while.end.loopexit ]
  %div37 = lshr i64 %tail.0.lcssa, 2
  %notmask = shl nsw i64 -1, %div37
  %0 = trunc i64 %notmask to i16
  %conv = xor i16 %0, -1
  %1 = bitcast i16 %conv to <16 x i1>
  %2 = tail call fast <16 x float> @llvm.masked.load.v16f32.p0(ptr %sp.0.lcssa, i32 1, <16 x i1> %1, <16 x float> zeroinitializer) #3, !alias.scope !21
  tail call void @llvm.masked.store.v16f32.p0(<16 x float> %2, ptr %dp.0.lcssa, i32 1, <16 x i1> %1) #3, !alias.scope !24
  ret void
}

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn writeonly
declare void @llvm.masked.store.v16f32.p0(<16 x float>, ptr, i32 immarg, <16 x i1>) #1

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind readonly willreturn
declare <16 x float> @llvm.masked.load.v16f32.p0(ptr, i32 immarg, <16 x i1>, <16 x float>) #2

attributes #0 = { nofree nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="512" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly mustprogress nocallback nofree nosync nounwind willreturn writeonly }
attributes #2 = { argmemonly mustprogress nocallback nofree nosync nounwind readonly willreturn }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!4 = !{!5, !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8}
!8 = distinct !{!8, !9, !"_mm512_storeu_ps: %__P"}
!9 = distinct !{!9, !"_mm512_storeu_ps"}
!10 = !{!11}
!11 = distinct !{!11, !12, !"_mm512_storeu_ps: %__P"}
!12 = distinct !{!12, !"_mm512_storeu_ps"}
!13 = !{!14}
!14 = distinct !{!14, !15, !"_mm512_storeu_ps: %__P"}
!15 = distinct !{!15, !"_mm512_storeu_ps"}
!16 = !{!17}
!17 = distinct !{!17, !18, !"_mm512_storeu_ps: %__P"}
!18 = distinct !{!18, !"_mm512_storeu_ps"}
!19 = distinct !{!19, !20}
!20 = !{!"llvm.loop.mustprogress"}
!21 = !{!22}
!22 = distinct !{!22, !23, !"_mm512_mask_loadu_ps: %__P"}
!23 = distinct !{!23, !"_mm512_mask_loadu_ps"}
!24 = !{!25}
!25 = distinct !{!25, !26, !"_mm512_mask_storeu_ps: %__P"}
!26 = distinct !{!26, !"_mm512_mask_storeu_ps"}
