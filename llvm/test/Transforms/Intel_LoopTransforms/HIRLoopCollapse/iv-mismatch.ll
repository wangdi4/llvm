; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Test checks that loop collaping works successfully despite types mismatch
; between i1 (i32) and i2 (i64).

; HIR before Loop Collapsing:
;            BEGIN REGION { }
;                  + Ztt: No
;                  + NumExits: 1
;                  + Innermost: No
;                  + HasSignedIV: Yes
;                  + LiveIn symbases: 4, 12
;                  + LiveOut symbases:
;                  + Loop metadata: !llvm.loop !11
;                  + DO i32 i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;                  | <RVAL-REG> LINEAR i32 %n + -1 {sb:2}
;                  |    <BLOB> LINEAR i32 %n {sb:4}
;                  |
;                  |   + Ztt: No
;                  |   + NumExits: 1
;                  |   + Innermost: Yes
;                  |   + HasSignedIV: Yes
;                  |   + LiveIn symbases: 4, 12
;                  |   + LiveOut symbases:
;                  |   + Loop metadata: !llvm.loop !9
;                  |   + DO i64 i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;                  |   | <RVAL-REG> LINEAR i64 zext.i32.i64(%n) + -1 {sb:2}
;                  |   |    <BLOB> LINEAR i32 %n {sb:4}
;                  |   |
;                  |   |   (%a)[%n * i1 + i2] = %n;
;                  |   |   <LVAL-REG> {al:2}(LINEAR ptr %a)[LINEAR zext.i32.i64(%n * i1 + i2)] inbounds  !tbaa !5 {sb:17}
;                  |   |      <BLOB> LINEAR i32 %n {sb:4}
;                  |   |      <BLOB> LINEAR ptr %a {sb:12}
;                  |   |   <RVAL-REG> LINEAR trunc.i32.i16(%n) {sb:2}
;                  |   |      <BLOB> LINEAR i32 %n {sb:4}
;                  |   |
;                  |   + END LOOP
;                  + END LOOP
;            END REGION


; HIR after Loop Collapsing:
; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, (zext.i32.i64(%n) * zext.i32.i64(%n)) + -1, 1   <DO_LOOP>
; CHECK:           |   (%a)[i1] = %n;
; CHECK:           + END LOOP
; CHECK:     END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local i32 @foo(ptr nocapture noundef %a, i32 noundef %n) local_unnamed_addr #0 {
entry:
  %cmp23 = icmp sgt i32 %n, 0
  br i1 %cmp23, label %for.cond1.preheader.lr.ph, label %for.end9

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %conv = zext i32 %n to i64
  %conv5 = trunc i32 %n to i16
  br label %for.body4.lr.ph

for.body4.lr.ph:                                  ; preds = %for.inc7, %for.cond1.preheader.lr.ph
  %indvars.iv = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next, %for.inc7 ]
  %mul0 = mul nsw i32 %indvars.iv, %n
  br label %for.body4

for.body4:                                        ; preds = %for.body4.lr.ph, %for.body4
  %j.022 = phi i64 [ 0, %for.body4.lr.ph ], [ %inc, %for.body4 ]
  %trunc1 = trunc i64 %j.022 to i32
  %add = add nuw nsw i32 %trunc1, %mul0
  %ext = zext i32 %add to i64
  %arrayidx = getelementptr inbounds i16, ptr %a, i64 %ext
  store i16 %conv5, ptr %arrayidx, align 2, !tbaa !3
  %inc = add nuw nsw i64 %j.022, 1
  %exitcond.not = icmp eq i64 %inc, %conv
  br i1 %exitcond.not, label %for.inc7, label %for.body4, !llvm.loop !7

for.inc7:                                         ; preds = %for.body4
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond26.not = icmp eq i32 %indvars.iv.next, %n
  br i1 %exitcond26.not, label %for.end9.loopexit, label %for.body4.lr.ph, !llvm.loop !9

for.end9.loopexit:                                ; preds = %for.inc7
  br label %for.end9

for.end9:                                         ; preds = %for.end9.loopexit, %entry
  %div = sdiv i32 %n, 2
  %idxprom = sext i32 %div to i64
  %arrayidx10 = getelementptr inbounds i16, ptr %a, i64 %idxprom
  %ld1 = load i16, ptr %arrayidx10, align 2, !tbaa !3
  %conv11 = sext i16 %ld1 to i32
  ret i32 %conv11
}

attributes #0 = { nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"short", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
!9 = distinct !{!9, !8}
