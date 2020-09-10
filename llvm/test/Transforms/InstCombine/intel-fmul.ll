; RUN: opt %s -instcombine -S | FileCheck %s

; Make sure we don't sink this invariant fdiv into the loop.
define void @fmul_loop_invariant_fdiv(float* %a, float %x) {
; CHECK-LABEL: @fmul_loop_invariant_fdiv(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = fdiv fast float 1.000000e+00, [[X:%.*]]
; CHECK-NEXT:    br label [[FOR_BODY:%.*]]
; CHECK:       for.cond.cleanup:
; CHECK-NEXT:    ret void
; CHECK:       for.body:
; CHECK-NEXT:    [[I_08:%.*]] = phi i32 [ 0, [[ENTRY:%.*]] ], [ [[INC:%.*]], [[FOR_BODY]] ]
; CHECK-NEXT:    [[IDXPROM:%.*]] = zext i32 [[I_08]] to i64
; CHECK-NEXT:    [[ARRAYIDX:%.*]] = getelementptr inbounds float, float* [[A:%.*]], i64 [[IDXPROM]]
; CHECK-NEXT:    [[TMP1:%.*]] = load float, float* [[ARRAYIDX]], align 4
; CHECK-NEXT:    [[TMP2:%.*]] = fmul fast float [[TMP1]], [[TMP0]]
; CHECK-NEXT:    store float [[TMP2]], float* [[ARRAYIDX]], align 4
; CHECK-NEXT:    [[INC]] = add nuw nsw i32 [[I_08]], 1
; CHECK-NEXT:    [[CMP_NOT:%.*]] = icmp eq i32 [[INC]], 1024
; CHECK-NEXT:    br i1 [[CMP_NOT]], label [[FOR_COND_CLEANUP:%.*]], label [[FOR_BODY]]
;
entry:
  %0 = fdiv fast float 1.000000e+00, %x
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %i.08 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %idxprom = zext i32 %i.08 to i64
  %arrayidx = getelementptr inbounds float, float* %a, i64 %idxprom
  %1 = load float, float* %arrayidx, align 4
  %2 = fmul fast float %1, %0
  store float %2, float* %arrayidx, align 4
  %inc = add nuw nsw i32 %i.08, 1
  %cmp.not = icmp eq i32 %inc, 1024
  br i1 %cmp.not, label %for.cond.cleanup, label %for.body
}

