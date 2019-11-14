; RUN: opt -hir-ssa-deconstruction -hir-general-unroll -hir-cg -print-after=hir-general-unroll -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-general-unroll,print<hir>,hir-cg" -S < %s 2>&1 | FileCheck %s

; Check that the loop is not unrolled due to small max trip count estimate.

; CHECK: + DO i1 = 0, %SRIndex.185 + -1 * smin(0, (-1 + %SRIndex.185)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 8> <nounroll>
; CHECK: |   %0 = (%ShiftRegister)[0][-1 * i1 + %N + -1];
; CHECK: |   (%ShiftRegister)[0][-1 * i1 + %N] = %0;
; CHECK: + END LOOP


; Verify that underlying LLVM loop got disabling metadata.

; CHECK: !llvm.loop !0
; CHECK: !0 = distinct !{!0, !1}
; CHECK: !1 = !{!"llvm.loop.unroll.disable"}

target datalayout = "p:32:32"

define void @convolutionalEncode(i32 %N, i16 %SRIndex.185) {
entry:
  %ShiftRegister = alloca [8 x i8], align 1
  br label %for.body15

for.body15:                                       ; preds = %for.body15.preheader, %for.body15
  %indvars.iv = phi i32 [ %sub17, %for.body15 ], [ %N, %entry ]
  %SRIndex.188 = phi i16 [ %SRIndex.1, %for.body15 ], [ %SRIndex.185, %entry ]
  %sub17 = add nsw i32 %indvars.iv, -1
  %arrayidx18 = getelementptr inbounds [8 x i8], [8 x i8]* %ShiftRegister, i32 0, i32 %sub17
  %0 = load i8, i8* %arrayidx18, align 1
  %arrayidx20 = getelementptr inbounds [8 x i8], [8 x i8]* %ShiftRegister, i32 0, i32 %indvars.iv
  store i8 %0, i8* %arrayidx20, align 1
  %SRIndex.1 = add i16 %SRIndex.188, -1
  %cmp13 = icmp sgt i16 %SRIndex.1, 0
  br i1 %cmp13, label %for.body15, label %for.end22.loopexit

for.end22.loopexit:
  ret void;
}
