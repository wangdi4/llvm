; l1reversal-cq1.ll
; (1-level loop, real testcase1, original CQ 377964)
; CQLink: https://jf.clearquest.intel.com/cqweb/#/CQMS.DPD.JF/DPD2/RECORD/33932428&recordType=Defect&format=HTML&noframes=false&version=cqwj
; 
; Sanity Test(s) on HIR Loop Reversal: simple 1-level (l1) loop that can be reversed
; 
; [REASONS]
; - Applicalbe: YES, HAS valid (1) negative memory-access addresses, and 0 positive memory-access address; 
; - Profitable: YES
;   Accumulated negative weight is higher than accumulated positive weight;
; - Legal:      YES (according to the revised isLegal() model)
;
;Note:
; - Need to use clang-3.8 (or earlier) FE to generate the LLVM IR;
; - Clang-3.9 (and later) improved quite a bit on its optimizer, so the said loop is actually reversed
;   inside LLVM.
; *** Source Code ***
;
;[BEFORE LOOP REVERSAL]
;
;void foo(unsigned short *p, int wsize, int n) {
;  unsigned m;
;  int i;
;  for (i = 0; i < n; i++) {
;    m = *--p;
;    *p = (unsigned short)(m >= wsize ? m - wsize : 0);
;  }
;}
;
;[AFTER LOOP REVERSAL]{Expected!}
;
;void foo(unsigned short *p, int wsize, int n) {
;  unsigned m;
;  int i;
;  for (i = 0; i < n; i++) {
;    m = *++p;
;    *p = (unsigned short)(m >= wsize ? m - wsize : 0);
;  }
;}
; 
; ===-----------------------------------===
; *** Run0: BEFORE HIR Loop Reversal ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-before=hir-loop-reversal -S 2>&1	\
; RUN: < %s  |	FileCheck %s -check-prefix=BEFORE 
;
;
; ===-----------------------------------===
; *** Run1: AFTER HIR Loop Reversal ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-after=hir-loop-reversal -S 2>&1	\
; RUN: < %s  |	FileCheck %s -check-prefix=AFTER
;
;
; === -------------------------------------- ===
; *** Tests0: W/O HIR Loop Reversal Output ***
; === -------------------------------------- ===
; Expected output before Loop Reversal
; 
;          BEGIN REGION { }
;<17>            + DO i1 = 0, %n + -1, 1   <DO_LOOP>
;<3>             |   %0 = (%p)[-1 * i1 + -1];
;<8>             |   %conv3 = (%0 <u %wsize) ? 0 : -1 * %wsize + zext.i16.i32(%0);
;<9>             |   (%p)[-1 * i1 + -1] = %conv3;
;<17>            + END LOOP
;          END REGION
;
;
; BEFORE:  BEGIN REGION { }
; BEFORE:        + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; BEFORE:        |   %0 = (%p)[-1 * i1 + -1];
; BEFORE:        |   %conv3 = (%0 <u %wsize) ? 0 : -1 * %wsize + zext.i16.i32(%0);
; BEFORE:        |   (%p)[-1 * i1 + -1] = %conv3;
; BEFORE:        + END LOOP
; BEFORE:  END REGION

; === -------------------------------------- ===
; *** Tests1: With HIR Loop Reversal Output ***
; === -------------------------------------- ===
;
; Expected HIR output after Loop-Reversal is enabled:
;
;          BEGIN REGION { modified }
;<17>            + DO i1 = 0, %n + -1, 1   <DO_LOOP>
;<3>             |   %0 = (%p)[i1 + -1 * zext.i32.i64((-1 + %n)) + -1];
;<8>             |   %conv3 = (%0 <u %wsize) ? 0 : -1 * %wsize + zext.i16.i32(%0);
;<9>             |   (%p)[i1 + -1 * zext.i32.i64((-1 + %n)) + -1] = %conv3;
;<17>            + END LOOP
;          END REGION
;
;
; AFTER:   BEGIN REGION { modified }
; AFTER:         + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; AFTER:         |   %0 = (%p)[i1 + -1 * zext.i32.i64((-1 + %n)) + -1];
; AFTER:         |   %conv3 = (%0 <u %wsize) ? 0 : -1 * %wsize + zext.i16.i32(%0);
; AFTER:         |   (%p)[i1 + -1 * zext.i32.i64((-1 + %n)) + -1] = %conv3;
; AFTER:         + END LOOP
; AFTER:   END REGION
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
; ModuleID = '<stdin>'
source_filename = "<stdin>"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i16* nocapture %p, i32 %wsize, i32 %n) #0 {
entry:
  %cmp10 = icmp sgt i32 %n, 0
  br i1 %cmp10, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.012 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %p.addr.011 = phi i16* [ %incdec.ptr, %for.body ], [ %p, %for.body.preheader ]
  %incdec.ptr = getelementptr inbounds i16, i16* %p.addr.011, i64 -1
  %0 = load i16, i16* %incdec.ptr, align 2, !tbaa !1
  %conv = zext i16 %0 to i32
  %cmp1 = icmp ult i32 %conv, %wsize
  %sub = sub i32 %conv, %wsize
  %1 = trunc i32 %sub to i16
  %conv3 = select i1 %cmp1, i16 0, i16 %1
  store i16 %conv3, i16* %incdec.ptr, align 2, !tbaa !1
  %inc = add nuw nsw i32 %i.012, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 2039) (llvm/branches/loopopt 2101)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"short", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
