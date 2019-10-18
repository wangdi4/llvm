; Check that both inner loops are marked as vectorizable.

; RUN: opt -hir-ssa-deconstruction -disable-output -hir-vec-dir-insert -print-after=hir-vec-dir-insert < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; CHECK: BEGIN REGION
; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   [[ENTRY:%.*]] = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;        |
; CHECK: |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK: |   |   (%p)[i2] = i2;
; CHECK: |   + END LOOP
;        |
; CHECK: |   @llvm.directive.region.exit([[ENTRY]]); [ DIR.VPO.END.AUTO.VEC() ]
;        |
; CHECK: |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK: |   |   (%q)[i2] = i2;
; CHECK: |   + END LOOP
; CHECK: + END LOOP
; CHECK: END REGION

;Module Before HIR; ModuleID = 'vec-dir-insertion-sibling.c'
source_filename = "vec-dir-insertion-sibling.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %p, i32* nocapture %q, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp33 = icmp sgt i32 %n, 0
  br i1 %cmp33, label %for.body4.preheader.preheader, label %for.cond.cleanup

for.body4.preheader.preheader:                    ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  %wide.trip.count37 = zext i32 %n to i64
  br label %for.body4.preheader

for.body4.preheader:                              ; preds = %for.body4.preheader.preheader, %for.cond.cleanup7
  %i.034 = phi i32 [ %inc15, %for.cond.cleanup7 ], [ 0, %for.body4.preheader.preheader ]
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup7
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body4:                                        ; preds = %for.body4, %for.body4.preheader
  %indvars.iv = phi i64 [ 0, %for.body4.preheader ], [ %indvars.iv.next, %for.body4 ]
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.body8.preheader, label %for.body4

for.body8.preheader:                              ; preds = %for.body4
  br label %for.body8

for.cond.cleanup7:                                ; preds = %for.body8
  %inc15 = add nuw nsw i32 %i.034, 1
  %exitcond39 = icmp eq i32 %inc15, %n
  br i1 %exitcond39, label %for.cond.cleanup.loopexit, label %for.body4.preheader

for.body8:                                        ; preds = %for.body8.preheader, %for.body8
  %indvars.iv35 = phi i64 [ %indvars.iv.next36, %for.body8 ], [ 0, %for.body8.preheader ]
  %arrayidx10 = getelementptr inbounds i32, i32* %q, i64 %indvars.iv35
  %1 = trunc i64 %indvars.iv35 to i32
  store i32 %1, i32* %arrayidx10, align 4
  %indvars.iv.next36 = add nuw nsw i64 %indvars.iv35, 1
  %exitcond38 = icmp eq i64 %indvars.iv.next36, %wide.trip.count37
  br i1 %exitcond38, label %for.cond.cleanup7, label %for.body8
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }


