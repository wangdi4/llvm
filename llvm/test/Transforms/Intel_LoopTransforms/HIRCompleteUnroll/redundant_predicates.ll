; After complete unroll the redundant predicate elimination utility should remove
; useless HLIfs

; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll 2>&1 < %s | FileCheck %s

; Input source:
;
; for (i=0;i<n;++i) {
;   for (j=0;j<10;++j) {
;     if (j < 5) {
;       p[j] = j;
;     }
;   }
;  }
; }

; CHECK: BEGIN REGION { modified }
; CHECK:  + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK-NOT:  if
; CHECK:  |   (%p)[0] = 0;
; CHECK:  |   (%p)[1] = 1;
; CHECK:  |   (%p)[2] = 2;
; CHECK:  |   (%p)[3] = 3;
; CHECK:  |   (%p)[4] = 4;
; CHECK-NOT:  (%p)[5]
; CHECK:  + END LOOP
; CHECK: END REGION

;Module Before HIR; ModuleID = 'pankaj.c'
source_filename = "pankaj.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %p, i32 %n, i32 %d) local_unnamed_addr #0 {
entry:
  %cmp16 = icmp sgt i32 %n, 0
  br i1 %cmp16, label %for.cond1.preheader.preheader, label %for.end7

for.cond1.preheader.preheader:                    ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.inc5
  %i.017 = phi i32 [ %inc6, %for.inc5 ], [ 0, %for.cond1.preheader.preheader ]
  br label %for.body3

for.body3:                                        ; preds = %for.inc, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.inc ]
  %cmp4 = icmp slt i64 %indvars.iv, 5
  br i1 %cmp4, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body3
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.inc5, label %for.body3

for.inc5:                                         ; preds = %for.inc
  %inc6 = add nuw nsw i32 %i.017, 1
  %exitcond18 = icmp eq i32 %inc6, %n
  br i1 %exitcond18, label %for.end7.loopexit, label %for.cond1.preheader

for.end7.loopexit:                                ; preds = %for.inc5
  br label %for.end7

for.end7:                                         ; preds = %for.end7.loopexit, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

