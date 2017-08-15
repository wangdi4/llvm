; Check that the assignment of RHS with a division is simplified after complete unroll.

; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll 2>&1 < %s | FileCheck %s

; Input source:
;
; unsigned int i, j;
; for (j=0;j<2;j++) {
;   for (i=0;i<2;i++) {
;     a[i] = (i + j)/2;
;   }
; }

; CHECK: BEGIN REGION { modified }
; CHECK: (%a)[0] = 0;
; CHECK: (%a)[1] = 0;
; CHECK: (%a)[0] = 0;
; CHECK: (%a)[1] = 1;
; CHECK: END REGION

;Module Before HIR; ModuleID = '1.c'
source_filename = "1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define i32 @foo(i8* nocapture %a, i32 %b) local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc4, %entry
  %indvars.iv17 = phi i64 [ 0, %entry ], [ %indvars.iv.next18, %for.inc4 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %0 = add nuw nsw i64 %indvars.iv, %indvars.iv17
  %1 = trunc i64 %0 to i32
  %div = lshr i32 %1, 1
  %conv = trunc i32 %div to i8
  %arrayidx = getelementptr inbounds i8, i8* %a, i64 %indvars.iv
  store i8 %conv, i8* %arrayidx, align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 2
  br i1 %exitcond, label %for.inc4, label %for.body3

for.inc4:                                         ; preds = %for.body3
  %indvars.iv.next18 = add nuw nsw i64 %indvars.iv17, 1
  %exitcond19 = icmp eq i64 %indvars.iv.next18, 2
  br i1 %exitcond19, label %for.end6, label %for.cond1.preheader

for.end6:                                         ; preds = %for.inc4
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

