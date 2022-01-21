target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; This test checks that the sum of absolute differences idiom recognition is performed.
;
;#include <stdint.h>
;#include <stdlib.h>
;
; int foo(uint8_t *pix1, uint8_t *pix2) {
;   int i_sum = 0;
;   for(int x = 0; x < 16; x++)
;     i_sum += abs(pix1[x] - pix2[x]);
;   return i_sum;
; }
;
; int main() {
;   uint8_t p1[16], p2[16];
;   unsigned i;
;   int sum = 0;
;   for (i = 0; i < 16; i++) {
;     p1[i] = i;
;     p2[i] = i + 1;
;   }
;   sum = foo(p1, p2);
;   printf("sum: %d\n", sum);
; }

; RUN: opt %s -enable-new-pm=0 -O2 -enable-nested-blob-vec -enable-blob-coeff-vec -vplan-disable-verification -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -hir-cg -print-after=hir-vplan-vec -mem2reg -o - | llc -mcpu=skx -print-after-isel 2>&1 | FileCheck %s
; RUN: opt %s -passes='default<O2>,hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,hir-cg,mem2reg' -enable-nested-blob-vec -enable-blob-coeff-vec -vplan-disable-verification -print-after=hir-vplan-vec -o - | llc -mcpu=skx -print-after-isel 2>&1 | FileCheck %s

; CHECK: VPSADBW

; Function Attrs: norecurse nounwind readonly uwtable
define i32 @foo(i8* nocapture readonly %pix1, i8* nocapture readonly %pix2) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %add.lcssa = phi i32 [ %add, %for.body ]
  ret i32 %add.lcssa

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %i_sum.010 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds i8, i8* %pix1, i64 %indvars.iv
  %0 = load i8, i8* %arrayidx, align 1
  %conv = zext i8 %0 to i32
  %arrayidx2 = getelementptr inbounds i8, i8* %pix2, i64 %indvars.iv
  %1 = load i8, i8* %arrayidx2, align 1
  %conv3 = zext i8 %1 to i32
  %sub = sub nsw i32 %conv, %conv3
  %ispos = icmp sgt i32 %sub, -1
  %neg = sub nsw i32 0, %sub
  %2 = select i1 %ispos, i32 %sub, i32 %neg
  %add = add nsw i32 %2, %i_sum.010
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 16
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind readnone }
