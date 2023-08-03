; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-post-vec-complete-unroll,print<hir>" -S 2>&1 -disable-output < %s | FileCheck %s

; Check that the prefetch loop will not be removed by redundant node removal after complete unroll of the second loop.

; BEGIN REGION { }
;       + DO i1 = 0, 999, 1   <DO_LOOP>
;       |   + DO i2 = 0, 999, 1   <DO_LOOP>
;       |   |   @llvm.prefetch(&((i8*)(%a)[1000 * i1 + i2]),  1,  3,  1);
;       |   + END LOOP
;       |
;       |
;       |   + DO i2 = 0, 2, 1   <DO_LOOP>
;       |   |   (%a)[i2] = i2;
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK: modified
; CHECK: DO i1

; CHECK: DO i2
; CHECK: prefetch
; CHECK: END LOOP

; CHECK-NOT: DO i2
; CHECK: END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(ptr nocapture %a) {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup10, %entry
  %indvars.iv35 = phi i64 [ 0, %entry ], [ %indvars.iv.next36, %for.cond.cleanup10 ]
  %0 = mul nuw nsw i64 %indvars.iv35, 1000
  %add.ptr = getelementptr inbounds i32, ptr %a, i64 %0
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup10
  ret void

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %add.ptr6 = getelementptr inbounds i32, ptr %add.ptr, i64 %indvars.iv
  tail call void @llvm.prefetch(ptr %add.ptr6, i32 1, i32 3, i32 1)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.body11.preheader, label %for.body4

for.body11.preheader:                             ; preds = %for.body4
  br label %for.body11

for.cond.cleanup10:                               ; preds = %for.body11
  %indvars.iv.next36 = add nuw nsw i64 %indvars.iv35, 1
  %exitcond38 = icmp eq i64 %indvars.iv.next36, 1000
  br i1 %exitcond38, label %for.cond.cleanup, label %for.cond1.preheader

for.body11:                                       ; preds = %for.body11.preheader, %for.body11
  %indvars.iv32 = phi i64 [ %indvars.iv.next33, %for.body11 ], [ 0, %for.body11.preheader ]
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv32
  %1 = trunc i64 %indvars.iv32 to i32
  store i32 %1, ptr %arrayidx, align 4
  %indvars.iv.next33 = add nuw nsw i64 %indvars.iv32, 1
  %exitcond34 = icmp eq i64 %indvars.iv.next33, 3
  br i1 %exitcond34, label %for.cond.cleanup10, label %for.body11
}

declare void @llvm.prefetch(ptr nocapture readonly, i32 immarg, i32 immarg, i32) #0

attributes #0 = { inaccessiblemem_or_argmemonly nounwind }

