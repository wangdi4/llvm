; Check that def@1 blob prevents the transformation

; RUN: opt -hir-ssa-deconstruction -S -print-after=hir-opt-var-predicate -print-before=hir-opt-var-predicate -disable-output -hir-opt-var-predicate < %s 2>&1 | FileCheck %s

; Source code:
;
; void foo(int *p, int *q, long n, long d) {
;   int i,j;
;   for (i=0;i<d;++i) {
;     int x = p[i];
;     for (j=0;j<n;++j) {
;       if (x > i) {
;         p[j] = j;
;       }
;     }
;   }
; }
;
; *** IR Dump Before HIR Var OptPredicate ***
; Function: foo
; 
;           BEGIN REGION { }
; <27>            + DO i1 = 0, zext.i32.i64((-1 + %d)), 1   <DO_LOOP>
; <8>             |      %0 = (%p)[i1];
; <26>            |   + DO i2 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; <14>            |   |   if (%0 > i1)
; <14>            |   |   {
; <20>            |   |      (%p)[i2] = i2;
; <14>            |   |   }
; <26>            |   + END LOOP
; <27>            + END LOOP
;           END REGION
;
; CHECK: *** IR Dump After
; Function: foo
;
; CHECK:    BEGIN REGION { }
; CHECK:    END REGION

;Module Before HIR; ModuleID = 'iv-outer-non-linear-blob.c'
source_filename = "iv-outer-non-linear-blob.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %p, i32 %n, i32 %d) local_unnamed_addr #0 {
entry:
  %cmp22 = icmp sgt i32 %d, 0
  br i1 %cmp22, label %for.body.lr.ph, label %for.end9

for.body.lr.ph:                                   ; preds = %entry
  %cmp220 = icmp sgt i32 %n, 0
  br label %for.body

for.body:                                         ; preds = %for.end, %for.body.lr.ph
  %indvars.iv24 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next25, %for.end ]
  br i1 %cmp220, label %for.body3.lr.ph, label %for.end

for.body3.lr.ph:                                  ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %indvars.iv24
  %0 = load i32, i32* %arrayidx, align 4
  %1 = sext i32 %0 to i64
  %cmp4 = icmp sgt i64 %1, %indvars.iv24
  br label %for.body3

for.body3:                                        ; preds = %for.inc, %for.body3.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.next, %for.inc ]
  br i1 %cmp4, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body3
  %arrayidx6 = getelementptr inbounds i32, i32* %p, i64 %indvars.iv
  %2 = trunc i64 %indvars.iv to i32
  store i32 %2, i32* %arrayidx6, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body3

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %for.body
  %indvars.iv.next25 = add nuw nsw i64 %indvars.iv24, 1
  %lftr.wideiv26 = trunc i64 %indvars.iv.next25 to i32
  %exitcond27 = icmp eq i32 %lftr.wideiv26, %d
  br i1 %exitcond27, label %for.end9.loopexit, label %for.body

for.end9.loopexit:                                ; preds = %for.end
  br label %for.end9

for.end9:                                         ; preds = %for.end9.loopexit, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }


